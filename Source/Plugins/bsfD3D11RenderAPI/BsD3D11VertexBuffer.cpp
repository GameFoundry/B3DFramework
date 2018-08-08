//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11VertexBuffer.h"
#include "BsD3D11Device.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	static void deleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<D3D11HardwareBuffer*>(buffer));
	}

	D3D11VertexBuffer::D3D11VertexBuffer(D3D11Device& device, const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		: VertexBuffer(desc, deviceMask), mDevice(device), mStreamOut(desc.streamOut)
	{
		assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
	}

	void D3D11VertexBuffer::initialize()
	{
		mBuffer = bs_pool_new<D3D11HardwareBuffer>(D3D11HardwareBuffer::BT_VERTEX, mUsage, 1, mSize, mDevice, false, mStreamOut);
		mBufferDeleter = &deleteBuffer;

		VertexBuffer::initialize();
	}
}}