//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11HardwareBufferManager.h"
#include "BsD3D11VertexBuffer.h"
#include "BsD3D11IndexBuffer.h"
#include "BsD3D11GpuBuffer.h"
#include "BsD3D11GpuParamBlockBuffer.h"
#include "RenderAPI/BsGpuParamDesc.h"

using namespace bs;
using namespace bs::ct;

D3D11HardwareBufferManager::D3D11HardwareBufferManager(D3D11Device& device)
	: mDevice(device)
{}

SPtr<ct::VertexBuffer> D3D11HardwareBufferManager::CreateVertexBufferInternal(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<D3D11VertexBuffer> ret = B3DMakeShared<D3D11VertexBuffer>(mDevice, desc, deviceMask);
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<ct::IndexBuffer> D3D11HardwareBufferManager::CreateIndexBufferInternal(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	SPtr<D3D11IndexBuffer> ret = B3DMakeShared<D3D11IndexBuffer>(mDevice, desc, deviceMask);
	ret->SetThisPtrInternal(ret);

	return ret;
}

SPtr<ct::GpuParamBlockBuffer> D3D11HardwareBufferManager::CreateGpuParamBlockBufferInternal(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
{
	D3D11GpuParamBlockBuffer* paramBlockBuffer =
		new(B3DAllocate<D3D11GpuParamBlockBuffer>()) D3D11GpuParamBlockBuffer(size, usage, deviceMask);

	SPtr<GpuParamBlockBuffer> paramBlockBufferPtr = B3DMakeSharedFromExisting<D3D11GpuParamBlockBuffer>(paramBlockBuffer);
	paramBlockBufferPtr->SetThisPtrInternal(paramBlockBufferPtr);

	return paramBlockBufferPtr;
}

SPtr<ct::GpuBuffer> D3D11HardwareBufferManager::CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
{
	D3D11GpuBuffer* buffer = new(B3DAllocate<D3D11GpuBuffer>()) D3D11GpuBuffer(desc, deviceMask);

	SPtr<D3D11GpuBuffer> bufferPtr = B3DMakeSharedFromExisting<D3D11GpuBuffer>(buffer);
	bufferPtr->SetThisPtrInternal(bufferPtr);

	return bufferPtr;
}

SPtr<ct::GpuBuffer> D3D11HardwareBufferManager::CreateGpuBufferInternal(const GPU_BUFFER_DESC& desc, SPtr<HardwareBuffer> underlyingBuffer)
{
	D3D11GpuBuffer* buffer = new(B3DAllocate<D3D11GpuBuffer>()) D3D11GpuBuffer(desc, std::move(underlyingBuffer));

	SPtr<D3D11GpuBuffer> bufferPtr = B3DMakeSharedFromExisting<D3D11GpuBuffer>(buffer);
	bufferPtr->SetThisPtrInternal(bufferPtr);

	return bufferPtr;
}
