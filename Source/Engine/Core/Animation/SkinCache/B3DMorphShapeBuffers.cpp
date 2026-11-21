//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/SkinCache/B3DMorphShapeBuffers.h"

#include "Animation/B3DMorphShapes.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"

namespace b3d::render
{
	MorphShapeBuffers::MorphShapeBuffers(const TShared<GpuBuffer>& vertexRanges, const TShared<GpuBuffer>& deltas, u32 vertexCount, u32 deltaCount,
		u32 shapeCount)
		: mVertexRanges(vertexRanges), mDeltas(deltas), mVertexCount(vertexCount), mDeltaCount(deltaCount), mShapeCount(shapeCount)
	{ }

	TShared<MorphShapeDeltas> MorphShapeBuffers::ReadDeltas() const
	{
		const TShared<MorphShapeDeltas> deltas = MorphShapeDeltas::Create(mVertexCount, mDeltaCount, mShapeCount);
		TArrayView<u32> vertexRanges = deltas->GetVertexRanges();
		GpuBufferUtility::Read(mVertexRanges, 0, (u32)(vertexRanges.Size() * sizeof(u32)), vertexRanges.Data());
		if(mDeltaCount > 0)
		{
			TArrayView<u8> deltaBytes = deltas->GetDeltas();
			GpuBufferUtility::Read(mDeltas, 0, (u32)deltaBytes.Size(), deltaBytes.Data());
		}

		return deltas;
	}

	TShared<MorphShapeBuffers> MorphShapeBuffers::Create(GpuDevice& device, const MorphShapeDeltas& deltas)
	{
		if(deltas.GetShapeCount() == 0)
			return nullptr;

		// The deformation shader reads both buffers as raw bytes. A GPU buffer can't be empty, so with no deltas the delta buffer has one, never read.
		const TShared<GpuBuffer> vertexRangeBuffer = device.CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), deltas.GetVertexCount() + 1));
		const TShared<GpuBuffer> deltaBuffer = device.CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), std::max(1u, deltas.GetDeltaCount()) * MorphShapeDeltas::kDeltaSize / sizeof(u32)));
		if(!B3D_ENSURE_LOG(vertexRangeBuffer != nullptr && deltaBuffer != nullptr, "Couldn't create the buffers of morph shapes of {0} vertices and {1} deltas.", deltas.GetVertexCount(), deltas.GetDeltaCount()))
		{
			return nullptr;
		}

		vertexRangeBuffer->SetName("Morph shape vertex ranges");
		deltaBuffer->SetName("Morph shape deltas");
		const TArrayView<const u32> vertexRanges = deltas.GetVertexRanges();
		GpuBufferUtility::Write(vertexRangeBuffer, 0, (u32)(vertexRanges.Size() * sizeof(u32)), vertexRanges.Data());
		if(deltas.GetDeltaCount() > 0)
		{
			const TArrayView<const u8> deltaBytes = deltas.GetDeltas();
			GpuBufferUtility::Write(deltaBuffer, 0, (u32)deltaBytes.Size(), deltaBytes.Data());
		}

		return B3DMakeSharedFromExisting(new(B3DAllocate<MorphShapeBuffers>()) MorphShapeBuffers(vertexRangeBuffer, deltaBuffer, deltas.GetVertexCount(), deltas.GetDeltaCount(), deltas.GetShapeCount()));
	}
} // namespace b3d::render
