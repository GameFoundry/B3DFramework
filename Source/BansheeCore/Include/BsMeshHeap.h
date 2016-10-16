//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCoreObject.h"
#include "BsDrawOps.h"
#include "BsIndexBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Core thread version of MeshHeap.
	 *
	 * @note	Core thread only.
	 */
	class BS_CORE_EXPORT MeshHeapCore : public CoreObjectCore
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
			UINT32 start, size;
		};

		/**	Represents an allocated piece of data representing a mesh. */
		struct AllocatedData
		{
			UINT32 vertChunkIdx;
			UINT32 idxChunkIdx;

			UseFlags useFlags;
			UINT32 eventQueryIdx;
			SPtr<TransientMeshCore> mesh;
		};

		/**	Data about a GPU query. */
		struct QueryData
		{
			SPtr<EventQuery> query;
			UINT32 queryId;
		};

	public:
		~MeshHeapCore();

	private:
		friend class MeshHeap;
		friend class TransientMesh;
		friend class TransientMeshCore;

		MeshHeapCore(UINT32 numVertices, UINT32 numIndices, 
			const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType, GpuDeviceFlags deviceMask);

		/** @copydoc CoreObjectCore::initialize() */
		void initialize() override;

		/**
		 * Allocates a new mesh in the heap, expanding the heap if needed. 
		 *
		 * @param[in]	mesh		Mesh for which we are allocating the data.
		 * @param[in]	meshData	Data to initialize the new mesh with.
		 */
		void alloc(SPtr<TransientMeshCore> mesh, const SPtr<MeshData>& meshData);

		/** Deallocates the provided mesh. Freed memory will be re-used as soon as the GPU is done with the mesh. */
		void dealloc(SPtr<TransientMeshCore> mesh);

		/** Resizes the vertex buffers so they max contain the provided number of vertices. */
		void growVertexBuffer(UINT32 numVertices);

		/** Resizes the index buffer so they max contain the provided number of indices. */
		void growIndexBuffer(UINT32 numIndices);

		/**
		 * Creates a new event query or returns an existing one from the pool if available. Returned value is an index 
		 * into event query array.
		 */
		UINT32 createEventQuery();

		/** Frees the event query with the specified index and returns it to the pool so it may be reused later. */
		void freeEventQuery(UINT32 idx);

		/**	Gets internal vertex data for all the meshes. */
		SPtr<VertexData> getVertexData() const;

		/**	Gets internal index data for all the meshes. */
		SPtr<IndexBufferCore> getIndexBuffer() const;

		/** Returns a structure that describes how are the vertices stored in the mesh's vertex buffer. */
		SPtr<VertexDataDesc> getVertexDesc() const;

		/**
		 * Returns the offset in vertices from the start of the buffer to the first vertex of the mesh with the provided ID.
		 */
		UINT32 getVertexOffset(UINT32 meshId) const;

		/**
		 * Returns the offset in indices from the start of the buffer to the first index of the mesh with the provided ID.
		 */
		UINT32 getIndexOffset(UINT32 meshId) const;

		/** Called by the render system when a mesh gets queued to the GPU. */
		void notifyUsedOnGPU(UINT32 meshId);

		/**
		 * Called by an GPU event query when GPU processes the query. Normally signals the heap that the GPU is done with 
		 * the mesh.
		 */
		static void queryTriggered(SPtr<MeshHeapCore> thisPtr, UINT32 meshId, UINT32 queryId);

		/**
		 * Attempts to reorganize the vertex and index buffer chunks in order to in order to make free memory contigous.
		 *
		 * @note	
		 * This will not actually copy any data from index/vertex buffers, and will only modify the chunk descriptors.
		 */
		void mergeWithNearbyChunks(UINT32 chunkVertIdx, UINT32 chunkIdxIdx);

	private:
		UINT32 mNumVertices;
		UINT32 mNumIndices;

		Vector<UINT8*> mCPUVertexData;
		UINT8* mCPUIndexData;

		SPtr<VertexData> mVertexData;
		SPtr<IndexBufferCore> mIndexBuffer;

		Map<UINT32, AllocatedData> mMeshAllocData;

		SPtr<VertexDataDesc> mVertexDesc;
		IndexType mIndexType;
		GpuDeviceFlags mDeviceMask;

		Vector<ChunkData> mVertChunks;
		Vector<ChunkData> mIdxChunks;

		Stack<UINT32> mEmptyVertChunks;
		Stack<UINT32> mEmptyIdxChunks;

		List<UINT32> mFreeVertChunks;
		List<UINT32> mFreeIdxChunks;

		Vector<QueryData> mEventQueries; 
		Stack<UINT32> mFreeEventQueries;

		UINT32 mNextQueryId;

		static const float GrowPercent;
	};

	/** @} */
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
		SPtr<TransientMesh> alloc(const SPtr<MeshData>& meshData, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/**
		 * Deallocates the provided mesh and makes that room on the heap re-usable as soon as the GPU is also done with the 
		 * mesh.
		 */
		void dealloc(const SPtr<TransientMesh>& mesh);

		/** Retrieves a core implementation of a mesh heap usable only from the core thread. */
		SPtr<MeshHeapCore> getCore() const;

		/**
		 * Creates a new mesh heap.
		 *
		 * @param[in]	numVertices	Initial number of vertices the heap may store. This will grow automatically if needed.
		 * @param[in]	numIndices	Initial number of indices the heap may store. This will grow automatically if needed.
		 * @param[in]	vertexDesc	Description of the stored vertices.
		 * @param[in]	indexType	Type of the stored indices.
		 */
		static SPtr<MeshHeap> create(UINT32 numVertices, UINT32 numIndices, 
			const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType = IT_32BIT);

	private:
		/** @copydoc create */
		MeshHeap(UINT32 numVertices, UINT32 numIndices, 
			const SPtr<VertexDataDesc>& vertexDesc, IndexType indexType = IT_32BIT);

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

	private:
		UINT32 mNumVertices;
		UINT32 mNumIndices;

		SPtr<VertexDataDesc> mVertexDesc;
		IndexType mIndexType;

		Map<UINT32, SPtr<TransientMesh>> mMeshes;
		UINT32 mNextFreeId;
	};

	/** @} */
}