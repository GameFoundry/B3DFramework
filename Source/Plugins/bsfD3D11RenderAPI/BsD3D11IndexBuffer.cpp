//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11IndexBuffer.h"
#include "BsD3D11Device.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

static void DeleteHardwareBuffer(GpuBuffer* buffer)
{
	B3DPoolDelete(static_cast<D3D11HardwareBuffer*>(buffer));
}

D3D11IndexBuffer::D3D11IndexBuffer(D3D11Device& device, const IndexBufferCreateInformation& desc, GpuDeviceFlags deviceMask)
	: IndexBuffer(desc, deviceMask), mDevice(device)
{
	B3D_ASSERT((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX.");
}

void D3D11IndexBuffer::Initialize()
{
	mBuffer = B3DPoolNew<D3D11HardwareBuffer>(D3D11HardwareBuffer::BT_INDEX, mUsage, 1, mSize, mDevice);
	mBufferDeleter = &DeleteHardwareBuffer;

	IndexBuffer::Initialize();
}
