//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalGpuProgram.h"
#include "B3DMetalGpuDevice.h"
#include "Material/B3DShaderCompiler.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Profiling/B3DRenderStats.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		struct MetalGpuProgram::Impl
		{
			id<MTLLibrary> Library = nil;
			id<MTLFunction> Function = nil;
		};

		MetalGpuProgram::MetalGpuProgram(MetalGpuDevice& gpuDevice, const GpuProgramCreateInformation& createInformation)
			: GpuProgram(createInformation)
			, mGpuDevice(gpuDevice)
			, mImpl(B3DMakeUnique<Impl>())
		{ }

		MetalGpuProgram::~MetalGpuProgram()
		{
			if (mImpl)
			{
				mImpl->Function = nil;
				mImpl->Library = nil;
			}

			B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_GpuProgram);
		}

		id<MTLFunction> MetalGpuProgram::GetMetalFunction() const
		{
			return mImpl->Function;
		}

		id<MTLLibrary> MetalGpuProgram::GetMetalLibrary() const
		{
			return mImpl->Library;
		}

		void MetalGpuProgram::Initialize()
		{
			if (!IsSupported())
			{
				mIsCompiled = false;
				mCompileMessages = "Specified program is not supported by the current render system.";

				GpuProgram::Initialize();
				return;
			}

			// Recompile when a bytecode compiler is registered (importer-enabled builds) and the bytecode is missing or stale.
			const char* language = MetalGpuDevice::kGpuProgramLanguageName;
			const TShared<IGpuBytecodeCompiler> bytecodeCompiler = ShaderCompilers::Instance().GetBytecodeCompiler(language);
			if (bytecodeCompiler && (!mBytecode || !bytecodeCompiler->IsUpToDate(*mBytecode)))
			{
				GpuProgramCreateInformation createInformation;
				createInformation.Name = mName;
				createInformation.Type = mType;
				createInformation.EntryPoint = mEntryPoint;
				createInformation.Language = language;
				createInformation.Source = mSource;

				mBytecode = mGpuDevice.CompileGpuProgramBytecode(createInformation);
			}

			mCompileMessages = mBytecode ? mBytecode->Messages : String();
			mIsCompiled = mBytecode && mBytecode->Instructions.Data != nullptr;

			if (mIsCompiled)
			{
				// Parse the bytecode envelope. Layout contract lives in B3DMetalBytecodeLayout.h — the
				// reader verifies the magic, strips the optional compute workgroup triple, and trims
				// trailing zero padding. A payload that fails validation is a cache corruption / version
				// mismatch scenario; fall through to the same failure path as a magic mismatch.
				const MetalBytecodePayload payload = ReadMetalBytecode(mType, mBytecode->Instructions.Data, mBytecode->Instructions.Size);
				if (!payload.IsValid)
				{
					mIsCompiled = false;
					mCompileMessages = "Metal GPU program bytecode magic mismatch; expected MSL source payload.";
					B3D_LOG(Error, LogRenderBackend, "{0}", mCompileMessages);
					GpuProgram::Initialize();
					return;
				}

				if (mType == GPT_COMPUTE_PROGRAM)
				{
					mWorkgroupSize[0] = payload.WorkgroupSize[0];
					mWorkgroupSize[1] = payload.WorkgroupSize[1];
					mWorkgroupSize[2] = payload.WorkgroupSize[2];
				}

				const u8* code = payload.MslSource;
				u32 codeSize = payload.MslSize;

				id<MTLDevice> device = mGpuDevice.GetMetalDevice();
				if (device == nil)
				{
					mIsCompiled = false;
					mCompileMessages = "Metal device unavailable when creating GPU program.";
					B3D_LOG(Error, LogRenderBackend, "{0}", mCompileMessages);
					GpuProgram::Initialize();
					return;
				}

				NSString* source = [[NSString alloc] initWithBytes:code length:codeSize encoding:NSUTF8StringEncoding];
				if (source == nil)
				{
					mIsCompiled = false;
					mCompileMessages = "Failed to decode Metal GPU program source as UTF-8.";
					B3D_LOG(Error, LogRenderBackend, "{0}", mCompileMessages);
					GpuProgram::Initialize();
					return;
				}

				MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
				options.fastMathEnabled = YES;

				NSError* error = nil;
				mImpl->Library = [device newLibraryWithSource:source options:options error:&error];
#if !__has_feature(objc_arc)
				[options release];
				[source release];
#endif

				if (mImpl->Library == nil)
				{
					mIsCompiled = false;
					NSString* errorString = error ? [error localizedDescription] : @"unknown error";
					mCompileMessages = String([errorString UTF8String]);
					B3D_LOG(Error, LogRenderBackend, "Failed to compile Metal GPU program '{0}': {1}", mName, mCompileMessages);
					GpuProgram::Initialize();
					return;
				}

				if (!mName.empty())
				{
					NSString* nsName = [NSString stringWithUTF8String:mName.c_str()];
					[mImpl->Library setLabel:nsName];
				}

				NSString* entryPointName = [NSString stringWithUTF8String:mEntryPoint.c_str()];
				mImpl->Function = [mImpl->Library newFunctionWithName:entryPointName];
				if (mImpl->Function == nil)
				{
					// SPIRV-Cross renames the default entry point to "main0"; fall back to that if the
					// caller asked for the generic "main" name.
					if (mEntryPoint == "main")
						mImpl->Function = [mImpl->Library newFunctionWithName:@"main0"];
				}

				if (mImpl->Function == nil)
				{
					mIsCompiled = false;
					mCompileMessages = String("Metal library does not contain entry point '") + mEntryPoint + "'.";
					B3D_LOG(Error, LogRenderBackend, "{0}", mCompileMessages);
					GpuProgram::Initialize();
					return;
				}

				mParametersDescription = mBytecode->ParameterDescription;

				if (mType == GPT_VERTEX_PROGRAM)
					mVertexInputDescription = B3DMakeShared<VertexDescription>(mBytecode->VertexInput, false);
			}

			B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_GpuProgram);

			GpuProgram::Initialize();
		}
	} // namespace render
} // namespace b3d
