//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"

namespace b3d::render
{
	class FrameGraph;

	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * Where one vertex attribute of a mesh lives, so shaders can read it from a bindless vertex buffer. Mirrors RayTracingVertexStream in
	 * RayTracingGeometryAccess.bslinc.
	 */
	struct RayTracingVertexStream
	{
		/**
		 * Index of the vertex buffer in the bindless resource heap, which shaders read as a ByteAddressBuffer, or kGpuBindlessInvalidIndex
		 * if the device has no bindless heaps.
		 */
		u32 BufferIndex = kGpuBindlessInvalidIndex;

		u32 Offset = 0; /**< Offset of the attribute of the first vertex in the vertex buffer, in bytes. */

		/** Distance between the attributes of consecutive vertices, in bytes. 0 for a default value that every vertex shares. */
		u32 Stride = 0;
	};

	/**
	 * Where the triangles and vertex attributes of one geometry of a RayTracingGeometry live, so hit shaders and inline tracing can read
	 * them from bindless buffers. Mirrors RayTracingGeometryLayout in RayTracingGeometryAccess.bslinc.
	 */
	struct RayTracingGeometryLayout
	{
		/** Index of the index buffer in the bindless resource heap, or kGpuBindlessInvalidIndex if the device has no bindless heaps. */
		u32 IndexBufferIndex = kGpuBindlessInvalidIndex;

		u32 FirstIndex = 0; /**< Position of the geometry's first index in the index buffer. Triangle N starts at FirstIndex + 3 * N. */

		RayTracingVertexStream Position;
		RayTracingVertexStream Normal;
		RayTracingVertexStream Tangent;
		RayTracingVertexStream Uv; /**< First set of texture coordinates. */
	};

	static_assert(sizeof(RayTracingGeometryLayout) == 56, "RayTracingGeometryLayout must match the layout of its shader structure.");

	/**
	 * Bottom level acceleration structure of the sub-meshes of a vertex and index buffer, with one geometry per triangle list sub-mesh. Built
	 * from those buffers in place, so the only new memory is the structure.
	 *
	 * @note	Render thread.
	 */
	class B3D_EXPORT RayTracingGeometry
	{
	public:
		/**
		 * @name Formats
		 * Vertex and index formats of ray tracing geometry, the only formats ray tracing shaders decode. Positions and indices are required.
		 * Normals, tangents and texture coordinates are optional: shaders read normal (0, 0, 1), tangent (1, 0, 0) with handedness 1, and
		 * texture coordinates (0, 0) for vertices without them.
		 * @{
		 */

		static constexpr VertexElementType kPositionType = VET_FLOAT3; /**< Type of vertex positions. */
		static constexpr VertexElementType kNormalType = VET_UBYTE4_NORM; /**< Type of vertex normals, each component stored as N * 0.5 + 0.5. */

		/** Type of vertex tangents, with the handedness (1 or -1) in W, each component stored as T * 0.5 + 0.5. */
		static constexpr VertexElementType kTangentType = VET_UBYTE4_NORM;

		static constexpr VertexElementType kUvType = VET_FLOAT2; /**< Type of the first set of texture coordinates. */
		static constexpr IndexType kIndexType = IT_32BIT; /**< Type of indices. */

		/** @} */

		RayTracingGeometry(const RayTracingGeometry&) = delete;
		RayTracingGeometry& operator=(const RayTracingGeometry&) = delete;

		/** Returns true if the structure holds the current contents of its buffers. MarkNotBuilt() makes it false. */
		bool IsBuilt() const { return mIsBuilt; }

		/**
		 * Adds a pass to @p graph that builds the structure from its buffers, and exports it, so later passes and later work can trace
		 * rays against it without declaring it. RayTracingScene::Build() builds the structures of its instances itself, in one pass. The
		 * geometry must stay alive until the graph executes.
		 */
		void Build(FrameGraph& graph);

		/** Returns the bottom level acceleration structure. */
		const TShared<GpuAccelerationStructure>& GetAccelerationStructure() const { return mAccelerationStructure; }

		/**
		 * Returns the geometry index that rays report for the triangles of each sub-mesh the geometry was created from, or ~0u for
		 * sub-meshes that aren't triangle lists, or have no triangles.
		 */
		TArrayView<const u32> GetSubMeshGeometryIndices() const { return mSubMeshGeometryIndices; }

		/** Returns where the triangles and vertex attributes of each geometry live, in geometry index order. */
		TArrayView<const RayTracingGeometryLayout> GetGeometryLayouts() const { return mLayouts; }

		/**
		 * Returns true if vertices with @p vertexDescription and indices of @p indexType are in the formats above: positions of kPositionType,
		 * normals, tangents and the first texture coordinates of their types if the vertices have them, and indices of kIndexType.
		 */
		static bool IsSupportedFormat(const VertexDescription& vertexDescription, IndexType indexType);

		/**
		 * Creates the geometry of @p subMeshes, with one geometry per triangle list sub-mesh. Their vertices are in @p vertexData and their
		 * indices in @p indexBuffer. Returns null if no sub-mesh is a triangle list with triangles, or if the vertices or indices aren't in the
		 * formats above (IsSupportedFormat()), which also logs a warning.
		 */
		static TShared<RayTracingGeometry> Create(GpuDevice& device, const VertexData& vertexData, const TShared<GpuBuffer>& indexBuffer, TArrayView<const SubMesh> subMeshes);

		/**
		 * @name Internal
		 * @{
		 */

		/** Marks the structure as out of date, so the next build rebuilds it. Needed after the contents of its buffers change. */
		void MarkNotBuilt() { mIsBuilt = false; }

		/** Marks the structure as up to date. Called when a build of it is added to a frame graph. */
		void MarkBuilt() { mIsBuilt = true; }

		/** Returns the contents the structure is built from. The geometries they reference live as long as this object. */
		const GpuAccelerationStructureInputs& GetBuildInputs() const { return mInputs; }

		/** Returns the scratch memory a build of the structure needs, in bytes. */
		u32 GetBuildScratchSize() const { return mBuildScratchSize; }

		/** @} */

	private:
		RayTracingGeometry() = default;

		Vector<GpuRayTracingGeometry> mGeometries;
		GpuAccelerationStructureInputs mInputs;
		u32 mBuildScratchSize = 0;
		TShared<GpuAccelerationStructure> mAccelerationStructure;
		Vector<u32> mSubMeshGeometryIndices;
		Vector<RayTracingGeometryLayout> mLayouts;
		TShared<GpuBuffer> mDefaultAttributes;
		bool mIsBuilt = false;
	};

	/** @} */
} // namespace b3d::render
