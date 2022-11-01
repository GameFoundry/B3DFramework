//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11GpuParamBlockBuffer.h"
#include "BsD3D11HardwareBuffer.h"
#include "BsD3D11RenderAPI.h"
#include "BsD3D11Device.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

D3D11GpuParamBlockBuffer::D3D11GpuParamBlockBuffer(u32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
	: GpuParamBlockBuffer(size, usage, deviceMask)
{
	assert((deviceMask == GDF_DEFAULT || deviceMask == GDF_PRIMARY) && "Multiple GPUs not supported natively on DirectX 11.");
}

D3D11GpuParamBlockBuffer::~D3D11GpuParamBlockBuffer()
{
	if(mBuffer != nullptr)
		B3DPoolDelete(static_cast<D3D11HardwareBuffer*>(mBuffer));
}

void D3D11GpuParamBlockBuffer::Initialize()
{
	D3D11RenderAPI* d3d11rs = static_cast<D3D11RenderAPI*>(RenderAPI::InstancePtr());
	D3D11Device& device = d3d11rs->GetPrimaryDevice();

	mBuffer = B3DPoolNew<D3D11HardwareBuffer>(D3D11HardwareBuffer::BT_CONSTANT, mUsage, 1, mSize, device);
	GpuParamBlockBuffer::Initialize();
}

ID3D11Buffer* D3D11GpuParamBlockBuffer::GetD3D11Buffer() const
{
	return static_cast<D3D11HardwareBuffer*>(mBuffer)->GetD3DBuffer();
}
