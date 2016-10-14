//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11HardwareBufferManager.h"
#include "BsD3D11VertexBuffer.h"
#include "BsD3D11IndexBuffer.h"
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11GpuParamBlockBuffer.h"
#include "BsGpuParamDesc.h"

namespace BansheeEngine
{
	D3D11HardwareBufferCoreManager::D3D11HardwareBufferCoreManager(D3D11Device& device)
		: mDevice(device)
	{ }

	SPtr<VertexBufferCore> D3D11HardwareBufferCoreManager::createVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<D3D11VertexBufferCore> ret = bs_shared_ptr_new<D3D11VertexBufferCore>(mDevice, desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<IndexBufferCore> D3D11HardwareBufferCoreManager::createIndexBufferInternal(const INDEX_BUFFER_DESC& desc, 
		GpuDeviceFlags deviceMask)
	{
		SPtr<D3D11IndexBufferCore> ret = bs_shared_ptr_new<D3D11IndexBufferCore>(mDevice, desc, deviceMask);
		ret->_setThisPtr(ret);

		return ret;
	}

	SPtr<GpuParamBlockBufferCore> D3D11HardwareBufferCoreManager::createGpuParamBlockBufferInternal(UINT32 size, GpuParamBlockUsage usage)
	{
		D3D11GpuParamBlockBufferCore* paramBlockBuffer = new (bs_alloc<D3D11GpuParamBlockBufferCore>()) D3D11GpuParamBlockBufferCore(size, usage);

		SPtr<GpuParamBlockBufferCore> paramBlockBufferPtr = bs_shared_ptr<D3D11GpuParamBlockBufferCore>(paramBlockBuffer);
		paramBlockBufferPtr->_setThisPtr(paramBlockBufferPtr);

		return paramBlockBufferPtr;
	}

	SPtr<GpuBufferCore> D3D11HardwareBufferCoreManager::createGpuBufferInternal(const GPU_BUFFER_DESC& desc,
		GpuDeviceFlags deviceMask)
	{
		D3D11GpuBufferCore* buffer = new (bs_alloc<D3D11GpuBufferCore>()) 
			D3D11GpuBufferCore(desc, deviceMask);

		SPtr<D3D11GpuBufferCore> bufferPtr = bs_shared_ptr<D3D11GpuBufferCore>(buffer);
		bufferPtr->_setThisPtr(bufferPtr);

		return bufferPtr;
	}
}