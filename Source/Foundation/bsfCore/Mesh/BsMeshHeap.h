//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "CoreThread/BsCoreObject.h"
#include "RenderAPI/BsIndexBuffer.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**
	 * Mesh heap allows you to quickly allocate and deallocate a large amounts of temporary meshes without the large
	 * overhead of normal Mesh creation. Only requirement is that meshes share the same vertex description and index type.
	 *
	 * @note
	 * This class should be considered as a replacement for a normal Mesh if you are constantly updating the mesh (for
	 * example every frame) and you are not able to discard entire mesh contents on each update. Not using discard flag on
	 * normal meshes may introduce GPU-CPU sync points which may severely limit performance. Primary purpose of this class
	 * is to avoid those sync points by not forcing you to discard contents.
	 * Downside is that this class may allocate 2-3x (or more) memory than it is actually needed for your data.
	 * @note
	 * Sim thread only
	 */
	class BS_CORE_EXPORT MeshHeap : public CoreObject
	{
	public:
		/**
		 * Allocates a new mesh in the heap, expanding the heap if needed. Mesh will be initialized with the provided
		 * @p meshData. You may use the returned transient mesh for drawing.
		 *
		 * @note
		 * Offsets provided by MeshData are ignored. MeshHeap will determine where the data will be written internally.
		 */
		SPtr<TransientMesh> Alloc(const SPtr<MeshData>& meshData, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/**
		 * Deallocates the provided mesh and makes that room on the heap re-usable as soon as the GPU is also done with the
		 * mesh.
		 */
		void Dealloc(const SPtr<TransientMesh>& mesh);

		/** Retrieves a core implementation of a mesh heap usable only from the core thread. */
		SPtr<ct::MeshHeap> GetCore() const;

		/**
		 * Creates a new mesh heap.
		 *
		 * @param[in]	numVertices	Initial number of vertices the heap may store. This will grow automatically if needed.
		 * @param[in]	numIndices	Initial number of indices the heap may store. This will grow automatically if needed.
		 * @param[in]	vertexDesc	Description of the stored vertices.
		 * @param[in]	indexType	Type of the stored indices.
		 */
		static SPtr<MeshHeap> Create(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType = IT_32BIT);

	private:
		/** @copydoc Create */
		MeshHeap(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType = IT_32BIT);

		SPtr<ct::CoreObject> CreateCore() const override;

	private:
		u32 mNumVertices;
		u32 mNumIndices;

		SPtr<VertexDataDesc> mVertexDesc;
		IndexType mIndexType;

		Map<u32, SPtr<TransientMesh>> mMeshes;
		u32 mNextFreeId;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup Resources-Internal
		 *  @{
		 */

		/**
		 * Core thread version of bs::MeshHeap.
		 *
		 * @note	Core thread only.
		 */
		class BS_CORE_EXPORT MeshHeap : public CoreObject
		{
			/**	Signifies how is a data chunk used. */
			enum class UseFlags
			{
				Used, /**< Data chunk is used by both CPU and GPU. */
				CPUFree, /**< Data chunk was released by CPU but not GPU. */
				GPUFree, /**< Data chunk was released by GPU but not CPU. */
				Free /**< Data chunk was released by both CPU and GPU. */
			};

			/**	Represents a continuous chunk of memory. */
			struct ChunkData
			{
				u32 Start, Size;
			};

			/**	Represents an allocated piece of data representing a mesh. */
			struct AllocatedData
			{
				u32 VertChunkIdx;
				u32 IdxChunkIdx;

				UseFlags UseFlags;
				u32 EventQueryIdx;
				SPtr<TransientMesh> Mesh;
			};

			/**	Data about a GPU query. */
			struct QueryData
			{
				SPtr<EventQuery> Query;
				u32 QueryId;
			};

		public:
			~MeshHeap();

		private:
			friend class bs::MeshHeap;
			friend class bs::TransientMesh;
			friend class TransientMesh;

			MeshHeap(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType, GpuDeviceFlags deviceMask);

			void Initialize() override;

			/**
			 * Allocates a new mesh in the heap, expanding the heap if needed.
			 *
			 * @param[in]	mesh		Mesh for which we are allocating the data.
			 * @param[in]	meshData	Data to initialize the new mesh with.
			 */
			void Alloc(SPtr<TransientMesh> mesh, const SPtr<MeshData>& meshData);

			/** Deallocates the provided mesh. Freed memory will be re-used as soon as the GPU is done with the mesh. */
			void Dealloc(SPtr<TransientMesh> mesh);

			/** Resizes the vertex buffers so they max contain the provided number of vertices. */
			void GrowVertexBuffer(u32 numVertices);

			/** Resizes the index buffer so they max contain the provided number of indices. */
			void GrowIndexBuffer(u32 numIndices);

			/**
			 * Creates a new event query or returns an existing one from the pool if available. Returned value is an index
			 * into event query array.
			 */
			u32 CreateEventQuery();

			/** Frees the event query with the specified index and returns it to the pool so it may be reused later. */
			void FreeEventQuery(u32 idx);

			/**	Gets internal vertex data for all the meshes. */
			SPtr<VertexData> GetVertexData() const;

			/**	Gets internal index data for all the meshes. */
			SPtr<IndexBuffer> GetIndexBuffer() const;

			/** Returns a structure that describes how are the vertices stored in the mesh's vertex buffer. */
			SPtr<VertexDataDesc> GetVertexDesc() const;

			/**
			 * Returns the offset in vertices from the start of the buffer to the first vertex of the mesh with the provided ID.
			 */
			u32 GetVertexOffset(u32 meshId) const;

			/**
			 * Returns the offset in indices from the start of the buffer to the first index of the mesh with the provided ID.
			 */
			u32 GetIndexOffset(u32 meshId) const;

			/** Called by the render system when a mesh gets queued to the GPU. */
			void NotifyUsedOnGpu(u32 meshId);

			/**
			 * Called by an GPU event query when GPU processes the query. Normally signals the heap that the GPU is done with
			 * the mesh.
			 */
			static void QueryTriggered(SPtr<MeshHeap> thisPtr, u32 meshId, u32 queryId);

			/**
			 * Attempts to reorganize the vertex and index buffer chunks in order to in order to make free memory contigous.
			 *
			 * @note
			 * This will not actually copy any data from index/vertex buffers, and will only modify the chunk descriptors.
			 */
			void MergeWithNearbyChunks(u32 chunkVertIdx, u32 chunkIdxIdx);

		private:
			u32 mNumVertices;
			u32 mNumIndices;

			Vector<u8*> mCPUVertexData;
			u8* mCPUIndexData;

			SPtr<VertexData> mVertexData;
			SPtr<IndexBuffer> mIndexBuffer;

			Map<u32, AllocatedData> mMeshAllocData;

			SPtr<VertexDataDesc> mVertexDesc;
			IndexType mIndexType;
			GpuDeviceFlags mDeviceMask;

			Vector<ChunkData> mVertChunks;
			Vector<ChunkData> mIdxChunks;

			Stack<u32> mEmptyVertChunks;
			Stack<u32> mEmptyIdxChunks;

			List<u32> mFreeVertChunks;
			List<u32> mFreeIdxChunks;

			Vector<QueryData> mEventQueries;
			Stack<u32> mFreeEventQueries;

			u32 mNextQueryId;

			static const float kGrowPercent;
		};

		/** @} */
	} // namespace ct
} // namespace bs
