//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d::render
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * The vertex deltas of the morph shapes of a mesh (MorphShapeDeltas), in buffers the skin cache's deformation shader reads. Shapes are
	 * numbered in the order of the mesh's channels, then of the shapes within a channel.
	 */
	class B3D_EXPORT MorphShapeBuffers
	{
	public:
		MorphShapeBuffers(const MorphShapeBuffers&) = delete;
		MorphShapeBuffers& operator=(const MorphShapeBuffers&) = delete;

		/** Returns the index of each vertex's first delta in GetDeltas(), with one more value at the end: the number of deltas. One u32 each. */
		const TShared<GpuBuffer>& GetVertexRanges() const { return mVertexRanges; }

		/**
		 * Returns the deltas of all shapes, sorted by vertex: a position delta and a normal delta of three floats each, then the shape index
		 * as a u32 (MorphShapeDeltas::kDeltaSize bytes). Holds one unused delta if no shape moves a vertex.
		 */
		const TShared<GpuBuffer>& GetDeltas() const { return mDeltas; }

		/** Returns the number of shapes, which is the number of weights SkinCacheEntry::SetPose() expects. */
		u32 GetShapeCount() const { return mShapeCount; }

		/** Reads the deltas back from the GPU. Blocks until the GPU has executed the reads. */
		TShared<MorphShapeDeltas> ReadDeltas() const;

		/**
		 * Creates the buffers of @p deltas on @p device and queues their upload. Returns null if the deltas have no shapes, or if the buffers
		 * can't be created.
		 */
		static TShared<MorphShapeBuffers> Create(GpuDevice& device, const MorphShapeDeltas& deltas);

	private:
		MorphShapeBuffers(const TShared<GpuBuffer>& vertexRanges, const TShared<GpuBuffer>& deltas, u32 vertexCount, u32 deltaCount, u32 shapeCount);

		TShared<GpuBuffer> mVertexRanges;
		TShared<GpuBuffer> mDeltas;
		u32 mVertexCount = 0;
		u32 mDeltaCount = 0;
		u32 mShapeCount = 0;
	};

	/** @} */
} // namespace b3d::render
