//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DID3D12RenderWindowSurface.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Texture.h"
#include "Image/B3DPixelUtility.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Returns a completed null result for reads that could not be issued. */
	TAsyncOp<TShared<PixelData>> CompletedNullReadOp()
	{
		TAsyncOp<TShared<PixelData>> op;
		op.CompleteOperation(nullptr);
		return op;
	}
}

TAsyncOp<TShared<PixelData>> ID3D12RenderWindowSurface::ReadAsync(GpuCommandBuffer& commandBuffer)
{
	D3D12Image* colorImage = GetCurrentColorImage();
	if (colorImage == nullptr)
		return CompletedNullReadOp();

	const u32 width = GetWidth();
	const u32 height = GetHeight();
	const PixelFormat pixelFormat = GetColorPixelFormat();
	const TShared<PixelData> pixelData = B3DMakeShared<PixelData>(width, height, 1, pixelFormat);

	const u32 tightRowPitch = width * PixelUtility::GetBlockSize(pixelFormat);
	const u32 paddedRowPitch = Math::CeilToMultiple(tightRowPitch, (u32)D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
	const u32 bufferSize = paddedRowPitch * height;

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::StagingRead;
	bufferCreateInformation.Staging.Size = bufferSize;

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(*GetD3D12GpuBackend().GetPrimaryDevice());
	TShared<GpuBuffer> stagingBuffer = device.CreateGpuBuffer(bufferCreateInformation, GpuObjectCreateFlag::None);
	if (stagingBuffer == nullptr)
		return CompletedNullReadOp();

	D3D12Buffer* d3d12StagingBuffer = static_cast<D3D12GpuBuffer*>(stagingBuffer.get())->GetD3D12Buffer();
	D3D12GpuCommandBuffer& d3d12CommandBuffer = static_cast<D3D12GpuCommandBuffer&>(commandBuffer);
	d3d12CommandBuffer.CopyImageToBuffer(colorImage, d3d12StagingBuffer, width, height, paddedRowPitch);

	TAsyncOp<TShared<PixelData>> op;
	auto fnOnCommandBufferCompleted = [stagingBuffer, op, pixelData, tightRowPitch, paddedRowPitch, height]() mutable
	{
		GpuBufferMappedScope mapping = stagingBuffer->Map(GpuMapOption::Read);

		pixelData->AllocateInternalBuffer();

		const u8* source = (const u8*)mapping.GetMappedMemory();
		u8* destination = pixelData->GetData();

		if (paddedRowPitch == tightRowPitch)
			memcpy(destination, source, pixelData->GetSize());
		else
		{
			for (u32 row = 0; row < height; row++)
				memcpy(destination + row * (u64)tightRowPitch, source + row * (u64)paddedRowPitch, tightRowPitch);
		}

		op.CompleteOperation(pixelData);
	};

	auto fnOnCommandBufferDestroyed = [op](bool isSubmitted) mutable
	{
		if (isSubmitted)
			return;

		op.CompleteOperation(nullptr);
	};

	commandBuffer.OnDidComplete.Connect(fnOnCommandBufferCompleted);
	commandBuffer.OnDestroyed.Connect(fnOnCommandBufferDestroyed);

	return op;
}
