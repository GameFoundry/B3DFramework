//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalClearPipeline.h"
#include "B3DMetalGpuDevice.h"
#include "Threading/B3DThreading.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		namespace
		{
			/**
			 * Builds the MSL source for the clear shaders.
			 *
			 * The vertex stage emits an oversized triangle covering the whole of NDC from @c vertex_id
			 * alone, so the pipeline declares no vertex inputs and the caller needs to bind no vertex
			 * buffers. Rasterization is confined to the region being cleared by the viewport and scissor
			 * the caller sets before drawing.
			 *
			 * The fragment stage is generated once per (color attachment count, writes depth) pair,
			 * because Metal requires a fragment function's color outputs to match the render pass'
			 * attachment layout exactly, and rejects a @c [[depth]] output when no depth attachment is
			 * bound. Attachments the clear must preserve are masked off in the pipeline descriptor
			 * rather than the shader, so a single function serves every write-mask combination at a
			 * given attachment count.
			 */
			String BuildClearShaderSource()
			{
				StringStream source;
				source <<
					"#include <metal_stdlib>\n"
					"using namespace metal;\n"
					"\n"
					"struct B3DClearParameters\n"
					"{\n"
					"\tfloat4 Color[" << B3D_MAXIMUM_RENDER_TARGET_COUNT << "];\n"
					"\tfloat Depth;\n"
					"};\n"
					"\n"
					"struct B3DClearVertexOutput\n"
					"{\n"
					"\tfloat4 Position [[position]];\n"
					"};\n"
					"\n"
					"vertex B3DClearVertexOutput b3dClearVertex(uint vertexId [[vertex_id]])\n"
					"{\n"
					"\tfloat2 corners[3] = { float2(-1.0f, -1.0f), float2(3.0f, -1.0f), float2(-1.0f, 3.0f) };\n"
					"\tB3DClearVertexOutput output;\n"
					"\toutput.Position = float4(corners[vertexId], 0.0f, 1.0f);\n"
					"\treturn output;\n"
					"}\n";

				for (u32 colorCount = 0; colorCount <= B3D_MAXIMUM_RENDER_TARGET_COUNT; colorCount++)
				{
					for (u32 depthVariant = 0; depthVariant < 2; depthVariant++)
					{
						const bool writesDepth = depthVariant != 0;

						// Nothing to write: no color outputs and no depth output. Such a clear touches
						// stencil only, which comes from the depth-stencil state, so the pipeline runs
						// without a fragment function at all and no variant is needed here.
						if (colorCount == 0 && !writesDepth)
							continue;

						const String suffix = (writesDepth ? String("Depth") : String("")) + ToString(colorCount);

						source << "\nstruct B3DClearOutput" << suffix << "\n{\n";
						for (u32 attachmentIndex = 0; attachmentIndex < colorCount; attachmentIndex++)
							source << "\tfloat4 Color" << attachmentIndex << " [[color(" << attachmentIndex << ")]];\n";
						if (writesDepth)
							source << "\tfloat Depth [[depth(any)]];\n";
						source << "};\n";

						source << "\nfragment B3DClearOutput" << suffix << " b3dClearFragment" << suffix
							<< "(constant B3DClearParameters& parameters [[buffer(" << kMetalClearParametersBufferSlot << ")]])\n{\n"
							<< "\tB3DClearOutput" << suffix << " output;\n";
						for (u32 attachmentIndex = 0; attachmentIndex < colorCount; attachmentIndex++)
							source << "\toutput.Color" << attachmentIndex << " = parameters.Color[" << attachmentIndex << "];\n";
						if (writesDepth)
							source << "\toutput.Depth = parameters.Depth;\n";
						source << "\treturn output;\n}\n";
					}
				}

				return source.str();
			}

			/** Number of color attachments present in the render pass the key describes. */
			u32 GetColorAttachmentCount(const MetalClearPipeline::Key& key)
			{
				u32 count = 0;
				for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
				{
					if (key.ColorFormats[attachmentIndex] != 0)
						count = attachmentIndex + 1;
				}
				return count;
			}
		} // namespace

		struct MetalClearPipeline::Impl
		{
			id<MTLLibrary> Library = nil;
			id<MTLFunction> VertexFunction = nil;

			// Depth-stencil states indexed by (writeDepth << 1) | writeStencil. Entry 0 writes neither
			// and is still a valid object: a color-only clear needs a state that leaves depth alone.
			id<MTLDepthStencilState> DepthStencilStates[4] = { nil, nil, nil, nil };

			// Compiled pipelines keyed by attachment layout + write mask. A nil value is a cached
			// failure, so a pipeline that cannot be built is compiled (and reported) only once.
			UnorderedMap<Key, id<MTLRenderPipelineState>, KeyHash> Pipelines;

			Mutex CacheMutex;

			// Latched once the library compile has been attempted, so a failure does not re-run the
			// (expensive) source compile on every subsequent clear.
			bool LibraryInitialized = false;
		};

		MetalClearPipeline::MetalClearPipeline(MetalGpuDevice& gpuDevice)
			: mGpuDevice(gpuDevice), mImpl(B3DMakeUnique<Impl>())
		{ }

		MetalClearPipeline::~MetalClearPipeline()
		{
			if (!mImpl)
				return;

#if !__has_feature(objc_arc)
			for (auto& entry : mImpl->Pipelines)
				[entry.second release];
			for (id<MTLDepthStencilState>& state : mImpl->DepthStencilStates)
				[state release];
			[mImpl->VertexFunction release];
			[mImpl->Library release];
#endif
			mImpl->Pipelines.clear();
		}

		bool MetalClearPipeline::EnsureLibrary()
		{
			if (mImpl->LibraryInitialized)
				return mImpl->Library != nil && mImpl->VertexFunction != nil;

			mImpl->LibraryInitialized = true;

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
			{
				// Leave the latch set: without a device there is nothing to compile against, and the
				// clear path has already reported the wider failure.
				return false;
			}

			@autoreleasepool
			{
				const String source = BuildClearShaderSource();
				NSError* error = nil;
				MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
				mImpl->Library = [device newLibraryWithSource:[NSString stringWithUTF8String:source.c_str()]
					options:options
					error:&error];
#if !__has_feature(objc_arc)
				[options release];
#endif
				if (mImpl->Library == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "Failed to compile the internal clear shader library: {0}",
						error ? String([[error localizedDescription] UTF8String]) : String("no error details were provided"));
					return false;
				}

				mImpl->VertexFunction = [mImpl->Library newFunctionWithName:@"b3dClearVertex"];
				if (mImpl->VertexFunction == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "The internal clear shader library is missing its vertex function.");
					return false;
				}
			}

			return true;
		}

		id<MTLRenderPipelineState> MetalClearPipeline::GetOrCreatePipelineState(const Key& key)
		{
			Lock lock(mImpl->CacheMutex);

			auto found = mImpl->Pipelines.find(key);
			if (found != mImpl->Pipelines.end())
				return found->second;

			if (!EnsureLibrary())
				return nil;

			id<MTLRenderPipelineState> pipeline = nil;
			@autoreleasepool
			{
				const u32 colorCount = GetColorAttachmentCount(key);

				MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];
				descriptor.label = @"B3D Clear";
				descriptor.vertexFunction = mImpl->VertexFunction;
				descriptor.rasterSampleCount = key.SampleCount;

				// A clear that writes neither color nor depth only needs rasterization to reach the
				// stencil test, so it runs without a fragment function.
				if (colorCount != 0 || key.WritesDepth)
				{
					const String suffix = (key.WritesDepth ? String("Depth") : String("")) + ToString(colorCount);
					const String functionName = "b3dClearFragment" + suffix;
					id<MTLFunction> fragmentFunction = [mImpl->Library newFunctionWithName:[NSString stringWithUTF8String:functionName.c_str()]];
					if (fragmentFunction == nil)
					{
						B3D_LOG(Error, LogRenderBackend, "The internal clear shader library is missing fragment function '{0}'.", functionName);
#if !__has_feature(objc_arc)
						[descriptor release];
#endif
						mImpl->Pipelines[key] = nil;
						return nil;
					}
					descriptor.fragmentFunction = fragmentFunction;
#if !__has_feature(objc_arc)
					[fragmentFunction release];
#endif
				}

				for (u32 attachmentIndex = 0; attachmentIndex < colorCount; attachmentIndex++)
				{
					MTLRenderPipelineColorAttachmentDescriptor* attachment = descriptor.colorAttachments[attachmentIndex];
					attachment.pixelFormat = (MTLPixelFormat)key.ColorFormats[attachmentIndex];
					// Attachments outside the clear mask keep their contents; masking them off in the
					// pipeline lets one fragment function serve every mask at this attachment count.
					attachment.writeMask = (key.ColorWriteMask & (1u << attachmentIndex)) != 0
						? MTLColorWriteMaskAll
						: MTLColorWriteMaskNone;
				}

				descriptor.depthAttachmentPixelFormat = (MTLPixelFormat)key.DepthFormat;
				descriptor.stencilAttachmentPixelFormat = (MTLPixelFormat)key.StencilFormat;

				NSError* error = nil;
				pipeline = [mGpuDevice.GetMetalDevice() newRenderPipelineStateWithDescriptor:descriptor error:&error];
#if !__has_feature(objc_arc)
				[descriptor release];
#endif
				if (pipeline == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "Failed to create the internal clear pipeline state: {0}",
						error ? String([[error localizedDescription] UTF8String]) : String("no error details were provided"));
				}
			}

			mImpl->Pipelines[key] = pipeline;
			return pipeline;
		}

		id<MTLDepthStencilState> MetalClearPipeline::GetOrCreateDepthStencilState(bool writeDepth, bool writeStencil)
		{
			const u32 index = (writeDepth ? 2u : 0u) | (writeStencil ? 1u : 0u);

			Lock lock(mImpl->CacheMutex);
			if (mImpl->DepthStencilStates[index] != nil)
				return mImpl->DepthStencilStates[index];

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			@autoreleasepool
			{
				MTLDepthStencilDescriptor* descriptor = [[MTLDepthStencilDescriptor alloc] init];
				descriptor.label = @"B3D Clear";
				// Always-pass so the clear overwrites whatever depth the attachment already holds.
				descriptor.depthCompareFunction = MTLCompareFunctionAlways;
				descriptor.depthWriteEnabled = writeDepth ? YES : NO;

				if (writeStencil)
				{
					// Metal has no shader-writable stencil, so the clear value is delivered through the
					// encoder's stencil reference and written by the pass operation.
					MTLStencilDescriptor* stencil = [[MTLStencilDescriptor alloc] init];
					stencil.stencilCompareFunction = MTLCompareFunctionAlways;
					stencil.depthStencilPassOperation = MTLStencilOperationReplace;
					stencil.stencilFailureOperation = MTLStencilOperationReplace;
					stencil.depthFailureOperation = MTLStencilOperationReplace;
					stencil.readMask = 0xFF;
					stencil.writeMask = 0xFF;
					descriptor.frontFaceStencil = stencil;
					descriptor.backFaceStencil = stencil;
#if !__has_feature(objc_arc)
					[stencil release];
#endif
				}

				mImpl->DepthStencilStates[index] = [device newDepthStencilStateWithDescriptor:descriptor];
#if !__has_feature(objc_arc)
				[descriptor release];
#endif
			}

			if (mImpl->DepthStencilStates[index] == nil)
				B3D_LOG(Error, LogRenderBackend, "Failed to create the internal clear depth-stencil state.");

			return mImpl->DepthStencilStates[index];
		}
	} // namespace render
} // namespace b3d
