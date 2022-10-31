//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullRenderAPI.h"
#include "CoreThread/BsCoreThread.h"
#include "Math/BsMatrix4.h"
#include "RenderAPI/BsGpuParamDesc.h"
#include "RenderAPI/BsGpuParams.h"
#include "Managers/BsGpuProgramManager.h"
#include "BsNullCommandBuffer.h"
#include "BsNullTexture.h"
#include "BsNullBuffers.h"
#include "BsNullRenderTargets.h"
#include "BsNullRenderStates.h"
#include "BsNullQueries.h"

using namespace bs;
using namespace bs::ct;

const StringID& NullRenderAPI::GetName() const
{
	static StringID strName("NullRenderAPI");
	return strName;
}

void NullRenderAPI::Initialize()
{
	THROW_IF_NOT_CORE_THREAD;

	mVideoModeInfo = B3DMakeShared<VideoModeInfo>();

	GPUInfo gpuInfo;
	gpuInfo.NumGpUs = 0;

	PlatformUtility::SetGPUInfoInternal(gpuInfo);

	CommandBufferManager::StartUp<NullCommandBufferManager>();

	bs::TextureManager::StartUp<bs::NullTextureManager>();
	TextureManager::StartUp<NullTextureManager>();

	// Create hardware buffer manager
	bs::HardwareBufferManager::StartUp();
	HardwareBufferManager::StartUp<NullHardwareBufferManager>();

	// Create render window manager
	bs::RenderWindowManager::StartUp<bs::NullRenderWindowManager>();
	RenderWindowManager::StartUp();

	// Create render state manager
	RenderStateManager::StartUp<NullRenderStateManager>();

	// Pretend as if we can parse HLSL
	mNullProgramFactory = B3DNew<NullProgramFactory>();
	GpuProgramManager::Instance().AddFactory("hlsl", mNullProgramFactory);

	mNumDevices = 1;
	mCurrentCapabilities = B3DNewMultiple<RenderAPICapabilities>(mNumDevices);
	mCurrentCapabilities->DeviceName = "Null";
	mCurrentCapabilities->RenderApiName = GetName();
	mCurrentCapabilities->DeviceVendor = GPU_UNKNOWN;

	RenderAPI::Initialize();
}

void NullRenderAPI::InitializeWithWindow(const SPtr<RenderWindow>& primaryWindow)
{
	QueryManager::StartUp<NullQueryManager>();

	RenderAPI::InitializeWithWindow(primaryWindow);
}

void NullRenderAPI::DestroyCore()
{
	THROW_IF_NOT_CORE_THREAD;

	mActiveRenderTarget = nullptr;

	if(mNullProgramFactory != nullptr)
	{
		B3DDelete(mNullProgramFactory);
		mNullProgramFactory = nullptr;
	}

	QueryManager::ShutDown();
	RenderStateManager::ShutDown();
	RenderWindowManager::ShutDown();
	bs::RenderWindowManager::ShutDown();
	HardwareBufferManager::ShutDown();
	bs::HardwareBufferManager::ShutDown();
	TextureManager::ShutDown();
	bs::TextureManager::ShutDown();
	CommandBufferManager::ShutDown();

	RenderAPI::DestroyCore();
}

void NullRenderAPI::ConvertProjectionMatrix(const Matrix4& matrix, Matrix4& dest)
{
	dest = matrix;
}

GpuParamBlockDesc NullRenderAPI::GenerateParamBlockDesc(const String& name, Vector<GpuParamDataDesc>& params)
{
	GpuParamBlockDesc block;
	block.BlockSize = 0;
	block.IsShareable = true;
	block.Name = name;
	block.Slot = 0;
	block.Set = 0;

	for(auto& param : params)
	{
		const GpuParamDataTypeInfo& typeInfo = bs::GpuParams::kParamSizes.Lookup[param.Type];

		if(param.ArraySize > 1)
		{
			// Arrays perform no packing and their elements are always padded and aligned to four component vectors
			u32 size;
			if(param.Type == GPDT_STRUCT)
				size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
			else
				size = Math::DivideAndRoundUp(typeInfo.Size, 16U) * 4;

			block.BlockSize = Math::DivideAndRoundUp(block.BlockSize, 4U) * 4;

			param.ElementSize = size;
			param.ArrayElementStride = size;
			param.CpuMemOffset = block.BlockSize;
			param.GpuMemOffset = 0;

			// Last array element isn't rounded up to four component vectors unless it's a struct
			if(param.Type != GPDT_STRUCT)
			{
				block.BlockSize += size * (param.ArraySize - 1);
				block.BlockSize += typeInfo.Size / 4;
			}
			else
				block.BlockSize += param.ArraySize * size;
		}
		else
		{
			u32 size;
			if(param.Type == GPDT_STRUCT)
			{
				// Structs are always aligned and arounded up to 4 component vectors
				size = Math::DivideAndRoundUp(param.ElementSize, 16U) * 4;
				block.BlockSize = Math::DivideAndRoundUp(block.BlockSize, 4U) * 4;
			}
			else
			{
				size = typeInfo.BaseTypeSize * (typeInfo.NumRows * typeInfo.NumColumns) / 4;

				// Pack everything as tightly as possible as long as the data doesn't cross 16 byte boundary
				u32 alignOffset = block.BlockSize % 4;
				if(alignOffset != 0 && size > (4 - alignOffset))
				{
					u32 padding = (4 - alignOffset);
					block.BlockSize += padding;
				}
			}

			param.ElementSize = size;
			param.ArrayElementStride = size;
			param.CpuMemOffset = block.BlockSize;
			param.GpuMemOffset = 0;

			block.BlockSize += size;
		}

		param.ParamBlockSlot = 0;
		param.ParamBlockSet = 0;
	}

	// Constant buffer size must always be a multiple of 16
	if(block.BlockSize % 4 != 0)
		block.BlockSize += (4 - (block.BlockSize % 4));

	return block;
}
