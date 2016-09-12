//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsMeshBase.h"
#include "BsMeshData.h"
#include "BsVertexData.h"
#include "BsDrawOps.h"
#include "BsSubMesh.h"
#include "BsBounds.h"

namespace BansheeEngine
{
	/** @addtogroup Resources
	 *  @{
	 */

	/** Descriptor object used for creation of a new Mesh object. */
	struct BS_CORE_EXPORT MESH_DESC
	{
		MESH_DESC() { }

		/** Number of vertices in the mesh. */
		UINT32 numVertices = 0; 

		/** Number of indices in the mesh. */
		UINT32 numIndices = 0; 

		/** 
		 * Vertex description structure that describes how are vertices organized in the vertex buffer. When binding a mesh
		 * to the pipeline you must ensure vertex description at least partially matches the input description of the
		 * currently bound vertex GPU program.
		 */
		SPtr<VertexDataDesc> vertexDesc;

		/** 
		 * Defines how are indices separated into sub-meshes, and how are those sub-meshes rendered. Sub-meshes may be
		 * rendered independently.
		 */
		Vector<SubMesh> subMeshes;

		/** Optimizes performance depending on planned usage of the mesh. */
		INT32 usage = MU_STATIC; 

		/** 
		 * Size of indices, use smaller size for better performance, however be careful not to go over the number of 
		 * vertices limited by the size.
		 */
		IndexType indexType = IT_32BIT;

		/** Optional skeleton that can be used for skeletal animation of the mesh. */
		SPtr<Skeleton> skeleton;

		/** Optional set of morph shapes that can be used for per-vertex animation of the mesh. */
		SPtr<MorphShapes> morphShapes;

		static MESH_DESC DEFAULT;
	};

	/**
	 * Primary class for holding geometry. Stores data in the form of a vertex buffers and optionally index buffer, which 
	 * may be bound to the pipeline for drawing. May contain multiple sub-meshes.
	 *
	 * @note	Sim thread.
	 */
	class BS_CORE_EXPORT Mesh : public MeshBase
	{
	public:
		virtual ~Mesh();

		/** @copydoc MeshBase::initialize */
		void initialize() override;

		/**
		 * Updates the mesh with new data. The actual write will be queued for later execution on the core thread. Provided 
		 * data buffer will be locked until the operation completes.
		 *
		 * @param[in]	accessor			Accessor to queue the operation on.
		 * @param[in]	subresourceIdx		Index of the subresource to write to. Ignored for now.
		 * @param[in]	data				Data of valid size and format to write to the subresource.
		 * @param[in]	discardEntireBuffer When true the existing contents of the resource you are updating will be
		 *									discarded. This can make the operation faster. Resources with certain buffer
		 *									types might require this flag to be in a specific state otherwise the operation
		 *									will fail.
		 * @return							Async operation object you can use to track operation completion.
		 *
		 * @see		MeshCore::writeSubresource
		 */
		AsyncOp writeSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const SPtr<MeshData>& data, 
			bool discardEntireBuffer);

		/**
		 * Reads internal mesh data to the provided previously allocated buffer. The read is queued for execution on the 
		 * core thread and not executed immediately. Provided data buffer will be locked until the operation completes.
		 *
		 * @param[in]	accessor		Accessor to queue the operation on.
		 * @param[in]	subresourceIdx	Index of the subresource to read from. Ignored for now.
		 * @param[out]	data			Previously allocated buffer of valid size and format to read the data into. Can be
		 *								allocated using allocateSubresourceBuffer().
		 * @return						Async operation object you can use to track operation completion.
		 *
		 * @see		MeshCore::readSubresource
		 */
		AsyncOp readSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const SPtr<MeshData>& data);

		/**
		 * Allocates a buffer you may use for storage when reading a subresource. You need to allocate such a buffer if you
		 * are calling readSubresource().
		 * 			
		 * @param[in]	subresourceIdx	Only 0 is supported. You can only update entire mesh at once.
		 *
		 * @note	Thread safe.
		 */
		SPtr<MeshData> allocateSubresourceBuffer(UINT32 subresourceIdx) const;

		/**
		 * Reads data from the cached system memory mesh buffer into the provided buffer. 
		 * 		  
		 * @param[in]	dest		Previously allocated buffer to read data into.
		 *
		 * @note	
		 * The data read is the cached mesh data. Any data written to the mesh from the GPU or core thread will not be 
		 * reflected in this data. Use readSubresource() if you require those changes. 
		 * @note
		 * The mesh must have been created with MU_CPUCACHED usage otherwise this method will not return any data.
		 */
		void readData(MeshData& dest);

		/** Gets the skeleton required for animation of this mesh, if any is available. */
		SPtr<Skeleton> getSkeleton() const { return mSkeleton; }

		/** Retrieves a core implementation of a mesh usable only from the core thread. */
		SPtr<MeshCore> getCore() const;

		/**	Returns a dummy mesh, containing just one triangle. Don't modify the returned mesh. */
		static HMesh dummy();

	protected:
		friend class MeshManager;

		Mesh(const MESH_DESC& desc);
		Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc);

		/**	Updates bounds by calculating them from the vertices in the provided mesh data object. */
		void updateBounds(const MeshData& meshData);

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		/**
		 * Creates buffers used for caching of CPU mesh data.
		 *
		 * @note	Make sure to initialize all mesh properties before calling this.
		 */
		void createCPUBuffer();

		/**	Updates the cached CPU buffers with new data. */
		void updateCPUBuffer(UINT32 subresourceIdx, const MeshData& data);

		mutable SPtr<MeshData> mCPUData;

		SPtr<VertexDataDesc> mVertexDesc;
		int mUsage;
		IndexType mIndexType;
		SPtr<Skeleton> mSkeleton; // Immutable
		SPtr<MorphShapes> mMorphShapes; // Immutable

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	private:
		Mesh(); // Serialization only

	public:
		friend class MeshRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

		/************************************************************************/
		/* 								STATICS		                     		*/
		/************************************************************************/
		
	public:
		/**
		 * Creates a new empty mesh. Created mesh will have no sub-meshes.
		 *
		 * @param[in]	numVertices		Number of vertices in the mesh.
		 * @param[in]	numIndices		Number of indices in the mesh. 
		 * @param[in]	vertexDesc		Vertex description structure that describes how are vertices organized in the
		 *								vertex buffer. When binding a mesh to the pipeline you must ensure vertex 
		 *								description at least partially matches the input description of the currently bound
		 *								vertex GPU program.
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 * @param[in]	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default 
		 *								option is a triangle list, where three indices represent a single triangle.
		 * @param[in]	indexType		Size of indices, use smaller size for better performance, however be careful not to
		 *								go over the number of vertices limited by the size.
		 */
		static HMesh create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, 
			int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST, IndexType indexType = IT_32BIT);

		/**
		 * Creates a new empty mesh. 
		 *
		 * @param[in]	desc	Descriptor containing the properties of the mesh to create.
		 */
		static HMesh create(const MESH_DESC& desc);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param[in]	initialData		Vertex and index data to initialize the mesh with.
		 * @param[in]	desc			Descriptor containing the properties of the mesh to create. Vertex and index count,
		 *								vertex descriptor and index type properties are ignored and are read from provided
		 *								mesh data instead.
		 */
		static HMesh create(const SPtr<MeshData>& initialData, const MESH_DESC& desc = MESH_DESC::DEFAULT);

		/** @name Internal
		 *  @{
		 */

		/**
		 * @copydoc	create(const MESH_DESC&)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> _createPtr(const MESH_DESC& desc);

		/**
		 * @copydoc	create(const SPtr<MeshData>&, int, DrawOperationType, const SPtr<Skeleton>&)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> _createPtr(const SPtr<MeshData>& initialData, const MESH_DESC& desc = MESH_DESC::DEFAULT);

		/**
		 * Creates a new empty and uninitialized mesh. You will need to manually initialize the mesh before using it.
		 *	
		 * @note	This should only be used for special cases like serialization and is not meant for normal use.
		 */
		static SPtr<Mesh> createEmpty();

		/** @} */
	};

	/** @} */

	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Core thread portion of a Mesh.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT MeshCore : public MeshCoreBase
	{
	public:
		MeshCore(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc);

		~MeshCore();

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/** @copydoc MeshCoreBase::getVertexData */
		SPtr<VertexData> getVertexData() const override;

		/** @copydoc MeshCoreBase::getIndexBuffer */
		SPtr<IndexBufferCore> getIndexBuffer() const override;

		/** @copydoc MeshCoreBase::getVertexDesc */
		SPtr<VertexDataDesc> getVertexDesc() const override;

		/** Returns a skeleton that can be used for animating the mesh. */
		SPtr<Skeleton> getSkeleton() const { return mSkeleton; }

		/**
		 * Updates a part of the current mesh with the provided data.
		 *
		 * @param[in]	subresourceIdx		Index of the subresource to update, if the mesh has more than one.
		 * @param[in]	data				Data to update the mesh with.
		 * @param[in]	discardEntireBuffer When true the existing contents of the resource you are updating will be 
		 *									discarded. This can make the operation faster. Resources with certain buffer 
		 *									types might require this flag to be in a specific state otherwise the operation 
		 *									will fail.
		 * @param[in]	updateBounds		If true the internal bounds of the mesh will be recalculated based on the 
		 *									provided data.
		 */
		virtual void writeSubresource(UINT32 subresourceIdx, const MeshData& data, bool discardEntireBuffer, bool updateBounds = true);

		/**
		 * Reads a part of the current resource into the provided @p data parameter.	Data buffer needs to be pre-allocated.
		 *
		 * @param[in]	subresourceIdx		Index of the subresource to update, if the mesh has more than one.
		 * @param[out]	data				Buffer that will receive the data. Should be allocated with 
		 *									allocateSubresourceBuffer() to ensure it is of valid type and size.
		 */
		virtual void readSubresource(UINT32 subresourceIdx, MeshData& data);

		/**
		 * Creates a new empty mesh. Created mesh will have no sub-meshes.
		 *
		 * @param[in]	numVertices		Number of vertices in the mesh.
		 * @param[in]	numIndices		Number of indices in the mesh. 
		 * @param[in]	vertexDesc		Vertex description structure that describes how are vertices organized in the
		 *								vertex buffer. When binding a mesh to the pipeline you must ensure vertex 
		 *								description at least partially matches the input description of the currently 
		 *								bound vertex GPU program.
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 * @param[in]	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle list, where three indices represent a single triangle.
		 * @param[in]	indexType		Size of indices, use smaller size for better performance, however be careful not to
		 *								go over the number of vertices limited by the size.
		 */
		static SPtr<MeshCore> create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, 
			int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST, IndexType indexType = IT_32BIT);

		/**
		 * Creates a new empty mesh. 
		 *
		 * @param[in]	desc	Descriptor containing the properties of the mesh to create.
		 */
		static SPtr<MeshCore> create(const MESH_DESC& desc);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param[in]	initialData		Vertex and index data to initialize the mesh with.
		 * @param[in]	desc			Descriptor containing the properties of the mesh to create. Vertex and index count,
		 *								vertex descriptor and index type properties are ignored and are read from provided
		 *								mesh data instead.
		 */
		static SPtr<MeshCore> create(const SPtr<MeshData>& initialData, const MESH_DESC& desc = MESH_DESC::DEFAULT);

	protected:
		friend class Mesh;

		/** Updates bounds by calculating them from the vertices in the provided mesh data object. */
		void updateBounds(const MeshData& meshData);

		SPtr<VertexData> mVertexData;
		SPtr<IndexBufferCore> mIndexBuffer;

		SPtr<VertexDataDesc> mVertexDesc;
		int mUsage;
		IndexType mIndexType;
		SPtr<MeshData> mTempInitialMeshData;
		SPtr<Skeleton> mSkeleton; // Immutable
		SPtr<MorphShapes> mMorphShapes; // Immutable
	};

	/** @} */
}