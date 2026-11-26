//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	class SkinCacheTestSuite;
} // namespace b3d

namespace b3d::render
{
	class FrameGraph;
	class RayTracingGeometry;
	class SkinCache;

	/** @addtogroup Renderer
	 *  @{
	 */

	/** Flags that control what a SkinCacheEntry produces. */
	enum class SkinCacheEntryFlag : u8
	{
		None = 0,

		/** The entry keeps the positions its vertices had before the latest deformation, as POSITION1, for motion vectors. */
		PreviousPositions = 1 << 0,

		/**
		 * The entry owns a bottom level acceleration structure of its deformed vertices and keeps it up to date. Has no effect if the mesh
		 * doesn't support ray tracing, or on devices without acceleration structures.
		 */
		RayTracingGeometry = 1 << 1
	};

	using SkinCacheEntryFlags = Flags<SkinCacheEntryFlag, u8>;
	B3D_FLAGS_OPERATORS_EXT(SkinCacheEntryFlag, u8)

	/** Describes a SkinCacheEntry to create. See SkinCache::CreateEntry(). */
	struct SkinCacheEntryCreateInformation
	{
		SkinCacheEntryCreateInformation() = default;

		/**
		 * @param	mesh	Mesh to deform. Its skeleton and morph shapes decide which deformations apply.
		 * @param	flags	Flags that control what the entry produces.
		 * @param	name	Optional name of the entry's buffers and structure, for debugging purposes.
		 */
		SkinCacheEntryCreateInformation(const TShared<render::Mesh>& mesh, SkinCacheEntryFlags flags = SkinCacheEntryFlag::None, const StringView& name = StringView())
			: Mesh(mesh), Flags(flags), Name(name)
		{ }

		TShared<render::Mesh> Mesh; /**< Mesh to deform. Its skeleton and morph shapes decide which deformations apply. */
		SkinCacheEntryFlags Flags; /**< Flags that control what the entry produces. */
		String Name; /**< Optional name of the entry's buffers and structure, for debugging purposes. */
	};

	/**
	 * Deformed vertices of one instance of an animated mesh: its positions, normals and tangents after morph shapes and skinning, in vertex
	 * buffers that draws, acceleration structure builds and ray tracing shaders read. Starts in the bind pose.
	 *
	 * @note	Render thread.
	 */
	class B3D_EXPORT SkinCacheEntry
	{
	public:
		~SkinCacheEntry();

		SkinCacheEntry(const SkinCacheEntry&) = delete;
		SkinCacheEntry& operator=(const SkinCacheEntry&) = delete;

		/**
		 * Sets the pose that the next SkinCache::Update() deforms the vertices to. A pose equal to the current one changes nothing.
		 *
		 * @param	boneTransforms		Transform of each bone of the mesh's skeleton, from the bind pose to the pose, in mesh space. Empty
		 *								for a mesh without a skeleton.
		 * @param	morphShapeWeights	Weight of each morph shape of the mesh, in channel order and then shape order within a channel.
		 *								Empty for a mesh without morph shapes.
		 */
		void SetPose(TArrayView<const Matrix4> boneTransforms, TArrayView<const float> morphShapeWeights = TArrayView<const float>());

		/** Returns true if the entry holds deformed vertices. False until the first SkinCache::Update() that deforms the entry. */
		bool HasDeformedVertices() const { return mHasDeformedVertices; }

		/**
		 * Returns the vertices to draw the mesh with: the deformed positions, normals and tangents, the mesh's other attributes, and as
		 * POSITION1 the positions before the latest deformation if the entry keeps them. Draw them with the mesh's index buffer and
		 * sub-meshes. Null while HasDeformedVertices() is false. An entry that keeps previous positions returns other vertex data after
		 * each update, so ask for it on every draw.
		 */
		const TShared<VertexData>& GetVertexData() const
		{
			return mEntriesMovedByLastUpdateIndex != kInvalidIndex ? mVertexDataWithMotion[mCurrentPositionBufferIndex] : mVertexDataWithoutMotion[mCurrentPositionBufferIndex];
		}

		/**
		 * Returns the ray tracing geometry of the deformed vertices, with the layouts ray tracing shaders read them through. Null if the
		 * entry has none, and while HasDeformedVertices() is false.
		 */
		const TShared<RayTracingGeometry>& GetRayTracingGeometry() const { return mRayTracingGeometry; }

		/** Returns the mesh the entry deforms. */
		const TShared<render::Mesh>& GetMesh() const { return mMesh; }

		/** Returns the flags the entry was created with. */
		SkinCacheEntryFlags GetFlags() const { return mFlags; }

		/** Returns the size of the entry's vertex buffers and acceleration structure, in bytes. */
		u32 GetMemorySize() const;

	private:
		friend class SkinCache;
		friend class b3d::SkinCacheTestSuite;

		/**
		 * Creates an entry in the bind pose and adds it to @p cache. The mesh of @p createInformation must have a skeleton or morph shapes,
		 * and vertices IsSupportedFormat() accepts.
		 */
		SkinCacheEntry(SkinCache& cache, const SkinCacheEntryCreateInformation& createInformation);

		/**
		 * Returns true if an entry can deform the vertices of @p mesh: positions of three floats, and normals, tangents and, if the mesh has
		 * a skeleton, blend indices and blend weights in the formats of RendererMeshData::VertexLayoutVertexDesc() in one stream, with
		 * buffers bound to both streams.
		 */
		static bool IsSupportedFormat(const render::Mesh& mesh);

		static constexpr u32 kInvalidIndex = ~0u; /**< Index of an entry that isn't in a list of its cache. */

		/** Stream of mVertexDescription with the first of the mesh's streams, after the deformed positions, normals and tangents. */
		static constexpr u32 kFirstSourceMeshStreamIndex = 2;

		SkinCache& mCache;
		u32 mEntriesIndex = kInvalidIndex; /**< Index of the entry in SkinCache::mEntries. */
		u32 mEntriesQueuedForUpdateIndex = kInvalidIndex; /**< Index of the entry in SkinCache::mEntriesQueuedForUpdate, if its pose changed since the last deformation. */
		/** Index of the entry in SkinCache::mEntriesMovedByLastUpdate. Valid while the latest update moved the entry's vertices. */
		u32 mEntriesMovedByLastUpdateIndex = kInvalidIndex;

		TShared<render::Mesh> mMesh;
		SkinCacheEntryFlags mFlags;
		String mName;
		u32 mVertexCount = 0;

		/**
		 * True if the entry builds ray tracing geometry: it has SkinCacheEntryFlag::RayTracingGeometry, the device has acceleration
		 * structures and the mesh supports ray tracing. Becomes false if the geometry can't be created. Known before mRayTracingGeometry
		 * exists, since the geometry is created over the vertex buffers, and their flags depend on it.
		 */
		bool mSupportsRayTracingGeometry = false;
		u32 mBoneCount = 0; /**< Number of bones of the mesh's skeleton. Zero for a mesh without one, which isn't skinned. */
		u32 mMorphShapeCount = 0; /**< Number of morph shapes of the mesh, in all of its channels. */

		/** @name Where the deformation reads the mesh's vertices
		 *  @{
		 */

		u32 mPositionStreamIndex = 0; /**< Stream of the mesh's vertices with the positions. */
		u32 mPositionOffset = 0; /**< Offset of the position in a vertex of the position stream, in bytes. */
		u32 mTangentFrameStreamIndex = 0; /**< Stream of the mesh's vertices with the normals, tangents, blend indices and blend weights. */
		u32 mNormalOffset = 0; /**< Offset of the normal in a vertex of the tangent frame stream, in bytes. */
		u32 mTangentOffset = 0; /**< Offset of the tangent in a vertex of the tangent frame stream, in bytes. */
		u32 mBlendIndicesOffset = 0; /**< Offset of the blend indices in a vertex of the tangent frame stream, in bytes. Zero without a skeleton. */
		u32 mBlendWeightsOffset = 0; /**< Offset of the blend weights in a vertex of the tangent frame stream, in bytes. Zero without a skeleton. */

		/** @} */

		/** Pose of the next deformation, as written to the pose buffer: three rows of each bone's transform, then the morph shape weights. */
		Vector<float> mPose;
		u64 mPoseHash = 0; /**< Hash of the bone transforms and morph shape weights of mPose. */
		bool mHasDeformedVertices = false;

		/**
		 * Deformed positions. The second buffer exists only for entries with SkinCacheEntryFlag::PreviousPositions, from their second
		 * deformation on. The two alternate between the current and the previous positions.
		 */
		TShared<GpuBuffer> mPositionBuffers[2];
		u32 mCurrentPositionBufferIndex = 0; /**< Index into mPositionBuffers of the buffer with the positions of the latest deformation. */
		TShared<GpuBuffer> mNormalTangentBuffer; /**< Deformed normals and tangents. */

		/**
		 * The deformed positions as stream 0, the normals and tangents as stream 1, the mesh's streams in mDrawnSourceMeshStreamIndices
		 * from kFirstSourceMeshStreamIndex with the deformed attributes as padding and, if the entry keeps previous positions, those as
		 * the last stream.
		 */
		TShared<VertexDescription> mVertexDescription;

		/**
		 * Streams of the mesh that the entry's draws bind, in the order of the streams of mVertexDescription from
		 * kFirstSourceMeshStreamIndex. Mesh streams that hold only deformed attributes and blend data are left out.
		 */
		TInlineArray<u32, 2> mDrawnSourceMeshStreamIndices;

		/**
		 * Vertex data that draws the positions of one position buffer, with the other position buffer as POSITION1, for the update after
		 * which the vertices moved. Indexed by the index of the buffer in mPositionBuffers. Only for entries that keep previous
		 * positions. Null until first used.
		 */
		TShared<VertexData> mVertexDataWithMotion[2];

		/**
		 * Vertex data that draws the positions of one position buffer, with the same buffer as POSITION1 so motion vectors are zero, for
		 * the updates after which the vertices didn't move. Indexed by the index of the buffer in mPositionBuffers. Entries that don't
		 * keep previous positions bind no POSITION1, and use only the first. Null until first used.
		 */
		TShared<VertexData> mVertexDataWithoutMotion[2];

		/** Structure of the deformed vertices, created on the first deformation if mSupportsRayTracingGeometry is true. */
		TShared<RayTracingGeometry> mRayTracingGeometry;

		u32 mTriangleCount = 0; /**< Triangles of mRayTracingGeometry, which a rebuild of it counts against the cache's rebuild budget. */
		u32 mRefitsSinceRebuild = 0; /**< Number of refits of mRayTracingGeometry since it was last built. */

		/**
		 * Order in which the entry asked for a periodic rebuild of mRayTracingGeometry, from SkinCache::mRebuildRequestCount, or zero if it
		 * hasn't asked. Older requests are granted first.
		 */
		u64 mRebuildRequest = 0;

		/**
		 * Creates vertex data that binds the position buffer @p positionBufferIndex as POSITION, the mesh's streams in
		 * mDrawnSourceMeshStreamIndices and, if the entry keeps previous positions, the position buffer @p previousPositionBufferIndex as
		 * POSITION1. All of the entry's vertex data share mVertexDescription, so draws switch between them without new vertex inputs or pipelines.
		 */
		TShared<VertexData> CreateVertexData(u32 positionBufferIndex, u32 previousPositionBufferIndex) const;
	};

	/**
	 * Deforms the vertices of animated meshes on the GPU, once per pose, into buffers that every later use of the mesh reads. Each entry
	 * holds the deformed vertices of one mesh instance.
	 *
	 * @note	Render thread.
	 */
	class B3D_EXPORT SkinCache
	{
	public:
		/**
		 * @param	device	Device to create the buffers and structures on.
		 * @param	name	Name of the cache, for debugging purposes.
		 */
		SkinCache(GpuDevice& device, const StringView& name);
		~SkinCache();

		SkinCache(const SkinCache&) = delete;
		SkinCache& operator=(const SkinCache&) = delete;

		/**
		 * Creates an entry in the bind pose, which the next Update() deforms it to. Returns null if the mesh has neither a skeleton nor
		 * morph shapes, or if its vertices don't have positions of three floats, and normals, tangents and, if it has a skeleton, blend
		 * indices and blend weights in the formats of RendererMeshData::VertexLayoutVertexDesc() in one stream, which also logs a warning.
		 * Also returns null, logging an error, if the mesh has morph shapes but its morph shape buffers couldn't be created. Entries must be
		 * destroyed before the cache.
		 */
		TShared<SkinCacheEntry> CreateEntry(const SkinCacheEntryCreateInformation& createInformation);

		/**
		 * Adds passes to @p graph for the entries whose pose changed since the last update: a compute pass that deforms their vertices, then
		 * an acceleration structure pass that refits or rebuilds their ray tracing geometries. Everything the passes write is exported, so
		 * later passes and later work draw the vertices and trace rays against the geometries without declaring them. Call once per frame.
		 * The entries must stay alive until the graph executes.
		 *
		 * A geometry is built on its first deformation, and refitted on the ones after. After render.SkinCacheGeometryRebuildInterval
		 * refits its entry asks for a rebuild, which restores the ray tracing speed refits lose. Each update grants the oldest requests,
		 * up to render.SkinCacheGeometryRebuildTrianglesPerFrame triangles; the other entries refit and ask again in the next update.
		 *
		 * @return	True if any pass was added.
		 */
		bool Update(FrameGraph& graph);

		/** Returns the number of entries. */
		u32 GetEntryCount() const { return (u32)mEntries.size(); }

		/** Returns the size of the vertex buffers and acceleration structures of all entries, in bytes. */
		u64 GetMemorySize() const;

	private:
		friend class SkinCacheEntry;

		/** Appends @p entry to @p list, and stores its index in the list in the member @p listIndex of the entry. */
		static void AddToList(Vector<SkinCacheEntry*>& list, SkinCacheEntry& entry, u32 SkinCacheEntry::* listIndex);

		/**
		 * Removes @p entry from @p list by moving the list's last entry to its place, if the member @p listIndex of the entry says it's in the
		 * list. The order of the list changes.
		 */
		static void RemoveFromList(Vector<SkinCacheEntry*>& list, SkinCacheEntry& entry, u32 SkinCacheEntry::* listIndex);

		GpuDevice& mDevice;
		String mName;

		/** @name Lists of entries
		 *  Each entry stores its index in every list, so it's added and removed in constant time. The order of a list carries no meaning.
		 *  @{
		 */

		Vector<SkinCacheEntry*> mEntries;
		Vector<SkinCacheEntry*> mEntriesQueuedForUpdate; /**< Entries whose pose changed since the last update. */

		/** Entries whose vertices the last update moved, so their vertex data binds the positions before it as POSITION1. */
		Vector<SkinCacheEntry*> mEntriesMovedByLastUpdate;

		/** @} */

		u64 mRebuildRequestCount = 0; /**< Number of periodic rebuilds entries asked for, which orders their requests. */
	};

	/** @} */
} // namespace b3d::render
