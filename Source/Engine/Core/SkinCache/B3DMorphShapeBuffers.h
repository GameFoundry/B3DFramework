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
	 * The vertex deltas of the morph shapes of a mesh, in buffers the skin cache's deformation shader reads. Shapes are numbered in the
	 * order of the mesh's channels, then of the shapes within a channel.
	 */
	class B3D_EXPORT MorphShapeBuffers
	{
	public:
		/** Size of a delta in GetDeltas(), in bytes. */
		static constexpr u32 kDeltaSize = 28;

		MorphShapeBuffers(const MorphShapeBuffers&) = delete;
		MorphShapeBuffers& operator=(const MorphShapeBuffers&) = delete;

		/** Returns the index of each vertex's first delta in GetDeltas(), with one more value at the end: the number of deltas. One u32 each. */
		const TShared<GpuBuffer>& GetVertexRanges() const { return mVertexRanges; }

		/**
		 * Returns the deltas of all shapes, sorted by vertex: a position delta and a normal delta of three floats each, then the shape index
		 * as a u32. Holds one unused delta if no shape moves a vertex.
		 */
		const TShared<GpuBuffer>& GetDeltas() const { return mDeltas; }

		/** Returns the number of shapes, which is the number of weights SkinCacheEntry::SetPose() expects. */
		u32 GetShapeCount() const { return mShapeCount; }

		/**
		 * Creates the buffers of @p morphShapes, and writes them through @p context. Returns null if it has no shapes, or if the buffers
		 * can't be created.
		 */
		static TShared<MorphShapeBuffers> Create(GpuWorkContext& context, const MorphShapes& morphShapes);

	private:
		MorphShapeBuffers(const TShared<GpuBuffer>& vertexRanges, const TShared<GpuBuffer>& deltas, u32 shapeCount);

		TShared<GpuBuffer> mVertexRanges;
		TShared<GpuBuffer> mDeltas;
		u32 mShapeCount = 0;
	};

	/** @} */
} // namespace b3d::render
