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
		{
			const Array<u32, 3>& threadGroupSize = createInformation.Bytecode != nullptr
				? createInformation.Bytecode->ThreadGroupSize
				: createInformation.ThreadGroupSize;
			mWorkgroupSize[0] = threadGroupSize[0];
			mWorkgroupSize[1] = threadGroupSize[1];
			mWorkgroupSize[2] = threadGroupSize[2];
		}

		MetalGpuProgram::~MetalGpuProgram()
		{
			if (mImpl)
			{
#if !__has_feature(objc_arc)
				[mImpl->Function release];
				[mImpl->Library release];
#endif
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
				Array<u32, 3> threadGroupSize = { mWorkgroupSize[0], mWorkgroupSize[1], mWorkgroupSize[2] };
				if(mBytecode != nullptr)
					threadGroupSize = mBytecode->ThreadGroupSize;
				createInformation.ThreadGroupSize = threadGroupSize;

				mBytecode = mGpuDevice.CompileGpuProgramBytecode(createInformation);
			}

			mCompileMessages = mBytecode ? mBytecode->Messages : String();
			mIsCompiled = mBytecode && mBytecode->Instructions.Data != nullptr
				&& mBytecode->Instructions.Size > 0 && mBytecode->ParameterDescription != nullptr
				&& mBytecode->ResourceTableLayout != nullptr;
			if(!mIsCompiled && mCompileMessages.empty())
				mCompileMessages = "Metal GPU program bytecode is empty or missing native reflection metadata.";

			if (mIsCompiled)
			{
				if (mType == GPT_COMPUTE_PROGRAM)
				{
					mWorkgroupSize[0] = mBytecode->ThreadGroupSize[0];
					mWorkgroupSize[1] = mBytecode->ThreadGroupSize[1];
					mWorkgroupSize[2] = mBytecode->ThreadGroupSize[2];
				}

				id<MTLDevice> device = mGpuDevice.GetMetalDevice();
				if (device == nil)
				{
					mIsCompiled = false;
					mCompileMessages = "Metal device unavailable when creating GPU program.";
					B3D_LOG(Error, LogRenderBackend, "{0}", mCompileMessages);
					GpuProgram::Initialize();
					return;
				}

				NSError* error = nil;
				dispatch_data_t libraryData = dispatch_data_create(mBytecode->Instructions.Data,
					mBytecode->Instructions.Size, dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
					DISPATCH_DATA_DESTRUCTOR_DEFAULT);
				if (libraryData != nullptr)
					mImpl->Library = [device newLibraryWithData:libraryData error:&error];

#if OS_OBJECT_USE_OBJC
#if !__has_feature(objc_arc)
				[(id)libraryData release];
#endif
#else
				if (libraryData != nullptr)
					dispatch_release(libraryData);
#endif

				if (mImpl->Library == nil)
				{
					mIsCompiled = false;
					NSString* errorString = error ? [error localizedDescription] : @"unknown error";
					mCompileMessages = String([errorString UTF8String]);
					B3D_LOG(Error, LogRenderBackend, "Failed to load Metal GPU program '{0}': {1}", mName, mCompileMessages);
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
