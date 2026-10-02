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
			/** Returns the name suffix of the fragment function writing @p colorCount attachments, and optionally depth. */
			String GetFragmentFunctionSuffix(u32 colorCount, bool writesDepth)
			{
				return (writesDepth ? String("Depth") : String("")) + ToString(colorCount);
			}

			/**
			 * Clear shader source shared by every fragment function. The vertex function generates a full-screen triangle from
			 * the vertex index alone, so no vertex buffers are needed.
			 */
			constexpr const char* kClearShaderCommonSource = R"(
#include <metal_stdlib>
using namespace metal;

struct B3DClearParameters
{
	float4 Color[B3D_MAXIMUM_RENDER_TARGET_COUNT];
	float Depth;
};

struct B3DClearVertexOutput
{
	float4 Position [[position]];
};

vertex B3DClearVertexOutput b3dClearVertex(uint vertexId [[vertex_id]])
{
	float2 corners[3] = { float2(-1.0f, -1.0f), float2(3.0f, -1.0f), float2(-1.0f, 3.0f) };
	B3DClearVertexOutput output;
	output.Position = float4(corners[vertexId], 0.0f, 1.0f);
	return output;
}
)";

			/**
			 * Builds the MSL source of the clear shaders. Expects B3D_MAXIMUM_RENDER_TARGET_COUNT and
			 * B3D_CLEAR_PARAMETERS_BUFFER_SLOT to be provided as preprocessor macros.
			 *
			 * A fragment function is generated per color attachment count, with and without depth output, since Metal
			 * requires the fragment outputs to match the attachments. Attachments that aren't cleared are masked off by the
			 * pipeline state instead, so the same function serves every write mask.
			 */
			String BuildClearShaderSource()
			{
				StringStream source;
				source << kClearShaderCommonSource;

				for (u32 colorCount = 0; colorCount <= B3D_MAXIMUM_RENDER_TARGET_COUNT; colorCount++)
				{
					for (u32 depthVariant = 0; depthVariant < 2; depthVariant++)
					{
						const bool writesDepth = depthVariant != 0;

						// Stencil-only clears run without a fragment function
						if (colorCount == 0 && !writesDepth)
							continue;

						const String suffix = GetFragmentFunctionSuffix(colorCount, writesDepth);

						source << "\nstruct B3DClearOutput" << suffix << "\n{\n";
						for (u32 attachmentIndex = 0; attachmentIndex < colorCount; attachmentIndex++)
							source << "\tfloat4 Color" << attachmentIndex << " [[color(" << attachmentIndex << ")]];\n";

						if (writesDepth)
							source << "\tfloat Depth [[depth(any)]];\n";

						source << "};\n";

						source << "\nfragment B3DClearOutput" << suffix << " b3dClearFragment" << suffix
							<< "(constant B3DClearParameters& parameters [[buffer(B3D_CLEAR_PARAMETERS_BUFFER_SLOT)]])\n{\n"
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

			/** Returns the number of color attachments, up to and including the last one present in @p key. */
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

		MetalClearPipeline::MetalClearPipeline(MetalGpuDevice& gpuDevice)
			: mGpuDevice(gpuDevice)
		{ }

		bool MetalClearPipeline::EnsureLibrary()
		{
			if (mLibraryInitialized)
				return mLibrary != nil && mVertexFunction != nil;

			mLibraryInitialized = true;

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
				return false;

			const String source = BuildClearShaderSource();
			NSError* error = nil;
			MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
			options.preprocessorMacros = @{
				@"B3D_MAXIMUM_RENDER_TARGET_COUNT": @(B3D_MAXIMUM_RENDER_TARGET_COUNT),
				@"B3D_CLEAR_PARAMETERS_BUFFER_SLOT": @(kMetalClearParametersBufferSlot)
			};

			mLibrary = [device newLibraryWithSource:[NSString stringWithUTF8String:source.c_str()] options:options error:&error];
			if (mLibrary == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to compile the internal clear shader library: {0}", error ? String([[error localizedDescription] UTF8String]) : String("no error details were provided"));
				return false;
			}

			mVertexFunction = [mLibrary newFunctionWithName:@"b3dClearVertex"];
			if (mVertexFunction == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "The internal clear shader library is missing its vertex function.");
				return false;
			}

			return true;
		}

		id<MTLRenderPipelineState> MetalClearPipeline::GetOrCreatePipelineState(const Key& key)
		{
			Lock lock(mCacheMutex);

			auto found = mPipelines.find(key);
			if (found != mPipelines.end())
				return found->second;

			if (!EnsureLibrary())
				return nil;

			id<MTLRenderPipelineState> pipeline = nil;
			const u32 colorCount = GetColorAttachmentCount(key);

			MTLRenderPipelineDescriptor* descriptor = [[MTLRenderPipelineDescriptor alloc] init];
			descriptor.label = @"B3D Clear";
			descriptor.vertexFunction = mVertexFunction;
			descriptor.rasterSampleCount = key.SampleCount;

			// Stencil-only clears run without a fragment function
			if (colorCount != 0 || key.WritesDepth)
			{
				const String functionName = "b3dClearFragment" + GetFragmentFunctionSuffix(colorCount, key.WritesDepth);
				id<MTLFunction> fragmentFunction = [mLibrary newFunctionWithName:[NSString stringWithUTF8String:functionName.c_str()]];
				if (fragmentFunction == nil)
				{
					B3D_LOG(Error, LogRenderBackend, "The internal clear shader library is missing fragment function '{0}'.", functionName);
					mPipelines[key] = nil;
					return nil;
				}
				descriptor.fragmentFunction = fragmentFunction;
			}

			for (u32 attachmentIndex = 0; attachmentIndex < colorCount; attachmentIndex++)
			{
				MTLRenderPipelineColorAttachmentDescriptor* attachment = descriptor.colorAttachments[attachmentIndex];
				attachment.pixelFormat = (MTLPixelFormat)key.ColorFormats[attachmentIndex];
				attachment.writeMask = (key.ColorWriteMask & (1u << attachmentIndex)) != 0
					? MTLColorWriteMaskAll
					: MTLColorWriteMaskNone;
			}

			descriptor.depthAttachmentPixelFormat = (MTLPixelFormat)key.DepthFormat;
			descriptor.stencilAttachmentPixelFormat = (MTLPixelFormat)key.StencilFormat;

			NSError* error = nil;
			pipeline = [mGpuDevice.GetMetalDevice() newRenderPipelineStateWithDescriptor:descriptor error:&error];
			if (pipeline == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to create the internal clear pipeline state: {0}",
					error ? String([[error localizedDescription] UTF8String]) : String("no error details were provided"));
			}

			mPipelines[key] = pipeline;
			return pipeline;
		}

		id<MTLDepthStencilState> MetalClearPipeline::GetOrCreateDepthStencilState(bool writeDepth, bool writeStencil)
		{
			const u32 stateIndex = (writeDepth ? 2u : 0u) | (writeStencil ? 1u : 0u);

			Lock lock(mCacheMutex);
			if (mDepthStencilStates[stateIndex] != nil)
				return mDepthStencilStates[stateIndex];

			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
				return nil;

			MTLDepthStencilDescriptor* descriptor = [[MTLDepthStencilDescriptor alloc] init];
			descriptor.label = @"B3D Clear";
			descriptor.depthCompareFunction = MTLCompareFunctionAlways;
			descriptor.depthWriteEnabled = writeDepth ? YES : NO;

			if (writeStencil)
			{
				// Fragment shaders cannot write stencil, so the value comes from the encoder's stencil reference
				MTLStencilDescriptor* stencil = [[MTLStencilDescriptor alloc] init];
				stencil.stencilCompareFunction = MTLCompareFunctionAlways;
				stencil.depthStencilPassOperation = MTLStencilOperationReplace;
				stencil.stencilFailureOperation = MTLStencilOperationReplace;
				stencil.depthFailureOperation = MTLStencilOperationReplace;
				stencil.readMask = 0xFF;
				stencil.writeMask = 0xFF;
				descriptor.frontFaceStencil = stencil;
				descriptor.backFaceStencil = stencil;
			}

			mDepthStencilStates[stateIndex] = [device newDepthStencilStateWithDescriptor:descriptor];

			if (mDepthStencilStates[stateIndex] == nil)
				B3D_LOG(Error, LogRenderBackend, "Failed to create the internal clear depth-stencil state.");

			return mDepthStencilStates[stateIndex];
		}
	} // namespace render
} // namespace b3d
