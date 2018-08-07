//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11GpuParamBlockBuffer.h"
#include "BsD3D11HardwareBuffer.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	D3D11GpuParamBlockBuffer::D3D11GpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
		:GpuParamBlockBuffer(size, usage, deviceMask)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
	}

	D3D11GpuParamBlockBuffer::~D3D11GpuParamBlockBuffer()
	{
		if (mBuffer != nullptr)
			bs_pool_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuParamBuffer);
	}

	void D3D11GpuParamBlockBuffer::initialize()
	{
		D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(RenderAPI::instancePtr());
		D3D11Device& device = d3d11rs->getPrimaryDevice();

		mBuffer = bs_pool_new<D3D11HardwareBuffer>(D3D11HardwareBuffer::BT_CONSTANT, mUsage, 1, mSize, device);

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuParamBuffer);

		GpuParamBlockBuffer::initialize();
	}


	ID3D11Buffer* D3D11GpuParamBlockBuffer::getD3D11Buffer() const
	{
		return mBuffer->getD3DBuffer();
	}

	void D3D11GpuParamBlockBuffer::writeToGPU(const UINT8* data, UINT32 queueIdx)
	{
		mBuffer->writeData(0, mSize, data, BWT_DISCARD);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuParamBuffer);
	}
}}