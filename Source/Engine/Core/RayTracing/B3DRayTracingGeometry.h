//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"

namespace b3d::render
{
	class FrameGraph;
	class FrameGraphAccelerationStructurePass;
	class RayTracingScene;

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

	/** Flags that control how a RayTracingGeometry is built. */
	enum class RayTracingGeometryFlag : u8
	{
		None = 0,

		/**
		 * The vertices of the geometry move. The structure is built for fast builds rather than fast ray tracing, it can be refitted to
		 * moved vertices (Refit()), and SetVertexData() may replace its vertex buffers. The indices and the number of vertices must not
		 * change.
		 */
		Dynamic = 1 << 0,

		/**
		 * The structure allows compaction, so once it is built, RayTracingManager::Update() can replace it with a smaller copy. The
		 * compaction of a build must be requested (RayTracingManager::RequestCompaction()), as RayTracingScene::Build() does. Ignored with
		 * Dynamic.
		 */
		AllowCompaction = 1 << 1
	};

	using RayTracingGeometryFlags = Flags<RayTracingGeometryFlag, u8>;
	B3D_FLAGS_OPERATORS_EXT(RayTracingGeometryFlag, u8)

	/**
	 * Bottom level acceleration structure of the sub-meshes of a vertex and index buffer, with one geometry per triangle list sub-mesh. Built
	 * from those buffers in place, so the only new memory is the structure.
	 *
	 * @note	Render thread.
	 */
	class B3D_EXPORT RayTracingGeometry : public std::enable_shared_from_this<RayTracingGeometry>
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

		/** Returns true if the structure holds the current contents of its buffers. MarkOutOfDate() makes it false. */
		bool IsBuilt() const { return mIsBuilt; }

		/**
		 * Adds a pass to @p graph that builds the structure from its buffers, and exports it, so later passes and later work can trace
		 * rays against it without declaring it. RayTracingScene::Build() builds the structures of its instances that aren't built itself,
		 * in one pass. The geometry must stay alive until the graph executes.
		 */
		void Build(FrameGraph& graph);

		/**
		 * Adds a pass to @p graph that refits the structure to the current positions of its vertices, and exports it. A refit is several
		 * times faster than a build, but keeps the tree of the last build, so rays slow down as the vertices move away from where they were
		 * then. Only for a geometry created with RayTracingGeometryFlag::Dynamic that was built at least once; others are built instead,
		 * with an error logged. The geometry must stay alive until the graph executes.
		 */
		void Refit(FrameGraph& graph);

		/** Returns the flags the geometry was created with. */
		RayTracingGeometryFlags GetFlags() const { return mFlags; }

		/** Returns the bottom level acceleration structure. */
		const TShared<GpuAccelerationStructure>& GetAccelerationStructure() const { return mAccelerationStructure; }

		/**
		 * Returns the geometry index that rays report for the triangles of each sub-mesh the geometry was created from, or ~0u for
		 * sub-meshes that aren't triangle lists, or have no triangles.
		 */
		TArrayView<const u32> GetSubMeshGeometryIndices() const { return mSubMeshGeometryIndices; }

		/**
		 * Returns where the triangles and vertex attributes of each geometry live, in geometry index order. A copy taken at another
		 * GetVersion() may be out of date.
		 */
		TArrayView<const RayTracingGeometryLayout> GetGeometryLayouts() const { return mLayouts; }

		/**
		 * Replaces the vertex buffers the geometry is built from and that its layouts point at, and marks the structure as out of date
		 * (MarkOutOfDate()). The next build reads the new buffers, and GetGeometryLayouts() returns the new layouts right away. Only for a
		 * geometry created with RayTracingGeometryFlag::Dynamic.
		 *
		 * @param	vertexData	Vertices with the same description and the same number of vertices as the ones the geometry was created with.
		 */
		void SetVertexData(const VertexData& vertexData);

		/**
		 * Returns a number that increases every time a build of the structure is added to a frame graph, and when
		 * SetAccelerationStructure() replaces the structure. A copy of the geometry's layouts, or a top level structure built over it, taken at another version
		 * may be out of date.
		 */
		u32 GetVersion() const { return mVersion; }

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
		static TShared<RayTracingGeometry> Create(GpuDevice& device, const VertexData& vertexData, const TShared<GpuBuffer>& indexBuffer, TArrayView<const SubMesh> subMeshes, RayTracingGeometryFlags flags = RayTracingGeometryFlag::None);

		/**
		 * Adds @p pass to @p graph, with a build of the structure of each of @p builds and a refit of each of @p refits (see Refit()), each
		 * at its own offset of one scratch buffer. The pass exports the structures, and the geometries count as built and get a new
		 * GetVersion(). A geometry whose structure is smaller than a build needs, such as a compacted one, gets a new structure of full
		 * size for its build. Does nothing if both lists are empty. The geometries must stay alive until the graph executes.
		 *
		 * @param	graph		Graph to add the pass to.
		 * @param	name		Name of the pass, for debugging purposes.
		 * @param	builds		Geometries to build.
		 * @param	refits		Geometries to refit. A geometry may appear once, in one of the two lists.
		 * @param	pass		Pass to add the builds to. May already declare other work of the pass, such as exports of the vertex buffers
		 *						the builds read.
		 */
		static void AddBuildPass(FrameGraph& graph, const StringView& name, TArrayView<RayTracingGeometry* const> builds, TArrayView<RayTracingGeometry* const> refits, FrameGraphAccelerationStructurePass&& pass);

		/**
		 * @name Internal
		 * @{
		 */

		/**
		 * Marks the structure as out of date, so the next build rebuilds it. Needed after the contents of its buffers change. Ray tracing
		 * scenes with instances of the geometry build it in their next RayTracingScene::Build().
		 */
		void MarkOutOfDate();

		/** Called by @p scene when it gets its first instance of the geometry. The scene then learns when the geometry needs a build and when it was built. */
		void NotifyAddedToScene(RayTracingScene& scene);

		/** Called by @p scene when it loses its last instance of the geometry, or is destroyed. */
		void NotifyRemovedFromScene(RayTracingScene& scene);

		/**
		 * Replaces the structure with @p accelerationStructure, a structure with the same contents that a pass of a frame graph writes,
		 * such as a compacted copy, and gives the geometry a new GetVersion(). Scenes with instances of the geometry rebuild their top level
		 * structure over it.
		 */
		void SetAccelerationStructure(const TShared<GpuAccelerationStructure>& accelerationStructure);

		/** @} */

	private:
		RayTracingGeometry() = default;

		/**
		 * Returns where the attributes of @p vertexData live, without the index buffer and first index. Attributes the vertices lack point at
		 * @p defaultAttributes if it isn't null, and have a stride of 0 otherwise.
		 */
		static RayTracingGeometryLayout GetVertexLayout(const VertexData& vertexData, const TShared<GpuBuffer>& defaultAttributes);

		RayTracingGeometryFlags mFlags;
		TShared<VertexDescription> mVertexDescription;
		u32 mVertexCount = 0;

		Vector<GpuRayTracingGeometry> mGeometries;
		GpuAccelerationStructureInputs mInputs;
		u32 mBuildScratchSize = 0;
		u32 mRefitScratchSize = 0; /**< Scratch memory a refit needs, in bytes. Zero for a geometry that isn't dynamic. */
		u32 mBuildSize = 0; /**< Size of the structure a build needs, in bytes. A compacted structure is smaller. */
		TShared<GpuAccelerationStructure> mAccelerationStructure;
		Vector<u32> mSubMeshGeometryIndices;
		Vector<RayTracingGeometryLayout> mLayouts;
		TShared<GpuBuffer> mDefaultAttributes;
		bool mIsBuilt = false;
		u32 mVersion = 0;

		/** Scenes with instances of the geometry, which learn when it needs a build and when it was built. Usually one. */
		TInlineArray<RayTracingScene*, 1> mScenes;
	};

	/** @} */
} // namespace b3d::render
