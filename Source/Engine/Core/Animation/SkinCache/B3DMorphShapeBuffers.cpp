//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/SkinCache/B3DMorphShapeBuffers.h"

#include "Animation/B3DMorphShapes.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuWorkContext.h"

namespace b3d::render
{
	MorphShapeBuffers::MorphShapeBuffers(const TShared<GpuBuffer>& vertexRanges, const TShared<GpuBuffer>& deltas, u32 shapeCount)
		: mVertexRanges(vertexRanges), mDeltas(deltas), mShapeCount(shapeCount)
	{ }

	TShared<MorphShapeBuffers> MorphShapeBuffers::Create(GpuWorkContext& context, const MorphShapes& morphShapes)
	{
		const u32 shapeCount = morphShapes.GetShapeCount();
		if(shapeCount == 0)
			return nullptr;

		// Count the deltas of each vertex, then turn the counts into the index of each vertex's first delta
		const u32 vertexCount = morphShapes.GetVertexCount();
		Vector<u32> vertexRanges(vertexCount + 1, 0);
		for(const TShared<MorphChannel>& channel : morphShapes.GetChannels())
		{
			for(const TShared<MorphShape>& shape : channel->GetShapes())
			{
				for(const MorphVertex& vertex : shape->GetVertices())
				{
					if(B3D_ENSURE_LOG(vertex.SourceIdx < vertexCount, "Morph shape '{0}' moves vertex {1} of a mesh with {2} vertices.", shape->GetName(),
						vertex.SourceIdx, vertexCount))
					{
						vertexRanges[vertex.SourceIdx + 1]++;
					}
				}
			}
		}

		for(u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++)
			vertexRanges[vertexIndex + 1] += vertexRanges[vertexIndex];

		// Each delta goes to the next free slot of its vertex. Shapes are visited in order, so a vertex's deltas are sorted by shape.
		const u32 deltaCount = vertexRanges[vertexCount];
		Vector<u8> deltas(std::max(1u, deltaCount) * kDeltaSize, 0);
		Vector<u32> nextDeltaIndices(vertexRanges.begin(), vertexRanges.end() - 1);
		u32 shapeIndex = 0;
		for(const TShared<MorphChannel>& channel : morphShapes.GetChannels())
		{
			for(const TShared<MorphShape>& shape : channel->GetShapes())
			{
				for(const MorphVertex& vertex : shape->GetVertices())
				{
					if(vertex.SourceIdx >= vertexCount)
						continue;

					u8* const delta = &deltas[nextDeltaIndices[vertex.SourceIdx]++ * kDeltaSize];
					std::memcpy(delta, &vertex.DeltaPosition, sizeof(Vector3));
					std::memcpy(delta + 12, &vertex.DeltaNormal, sizeof(Vector3));
					std::memcpy(delta + 24, &shapeIndex, sizeof(u32));
				}

				shapeIndex++;
			}
		}

		// The deformation shader reads both buffers as raw bytes
		GpuDevice& device = context.GetDevice();
		const TShared<GpuBuffer> vertexRangeBuffer = device.CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), vertexCount + 1));
		const TShared<GpuBuffer> deltaBuffer = device.CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), (u32)deltas.size() / sizeof(u32)));
		if(!B3D_ENSURE_LOG(vertexRangeBuffer != nullptr && deltaBuffer != nullptr, "Couldn't create the buffers of morph shapes of {0} vertices and {1} deltas.",
			vertexCount, deltaCount))
		{
			return nullptr;
		}

		vertexRangeBuffer->SetName("Morph shape vertex ranges");
		deltaBuffer->SetName("Morph shape deltas");
		GpuBufferUtility::Write(vertexRangeBuffer, 0, (u32)(vertexRanges.size() * sizeof(u32)), vertexRanges.data());
		GpuBufferUtility::Write(deltaBuffer, 0, (u32)deltas.size(), deltas.data());

		return B3DMakeSharedFromExisting(new(B3DAllocate<MorphShapeBuffers>()) MorphShapeBuffers(vertexRangeBuffer, deltaBuffer, shapeCount));
	}
} // namespace b3d::render
