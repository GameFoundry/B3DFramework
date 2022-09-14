//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Mesh/BsMeshBase.h"
#include "Mesh/BsMeshData.h"
#include "RenderAPI/BsVertexData.h"
#include "RenderAPI/BsSubMesh.h"
#include "Math/BsBounds.h"

namespace bs
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
	 * Primary class for holding geometry. Stores data in the form of vertex buffers and optionally an index buffer, which
	 * may be bound to the pipeline for drawing. May contain multiple sub-meshes.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering) Mesh : public MeshBase
	{
	public:
		virtual ~Mesh() = default;

		/** @copydoc MeshBase::initialize */
		void initialize() override;

		/**
		 * Updates the mesh with new data. Provided data buffer will be locked until the operation completes.
		 *
		 * @param[in]	data				Data of valid size and format to write to the subresource.
		 * @param[in]	discardEntireBuffer When true the existing contents of the resource you are updating will be
		 *									discarded. This can make the operation faster. Resources with certain buffer
		 *									types might require this flag to be in a specific state otherwise the operation
		 *									will fail.
		 * @return							Async operation object you can use to track operation completion.
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		AsyncOp writeData(const SPtr<MeshData>& data, bool discardEntireBuffer);

		/**
		 * Reads internal mesh data to the provided previously allocated buffer. Provided data buffer will be locked until
		 * the operation completes.
		 *
		 * @param[out]	data			Pre-allocated buffer of proper vertex/index format and size where data will be read
		 *								to. You can use allocBuffer() to allocate a buffer of a correct format and size.
		 * @return						Async operation object you can use to track operation completion.
		 *
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		AsyncOp readData(const SPtr<MeshData>& data);

		/**
		 * Allocates a buffer that exactly matches the size of this mesh. This is a helper function, primarily meant for
		 * creating buffers when reading from, or writing to a mesh.
		 * 			
		 * @note	Thread safe.
		 */
		SPtr<MeshData> allocBuffer() const;

		/**
		 * Returns mesh data cached in the system memory. If the mesh wasn't created with CPU cached usage flag this
		 * method will not return any data. Caller should not modify the returned data.
		 *
		 * @note	
		 * The data read is the cached mesh data. Any data written to the mesh from the GPU or core thread will not be
		 * reflected in this data. Use readData() if you require those changes.
		 */
		SPtr<MeshData> getCachedData() const { return mCPUData; }

		/** Gets the skeleton required for animation of this mesh, if any is available. */
		BS_SCRIPT_EXPORT(pr:getter,n:Skeleton)
		SPtr<Skeleton> getSkeleton() const { return mSkeleton; }

		/** Returns an object containing all shapes used for morph animation, if any are available. */
		BS_SCRIPT_EXPORT(pr:getter,n:MorphShapes)
		SPtr<MorphShapes> getMorphShapes() const { return mMorphShapes; }

		/** Retrieves a core implementation of a mesh usable only from the core thread. */
		SPtr<ct::Mesh> getCore() const;

		/**	Returns a dummy mesh, containing just one triangle. Don't modify the returned mesh. */
		static HMesh dummy();

	protected:
		friend class MeshManager;

		Mesh(const MESH_DESC& desc);
		Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc);

		/**	Updates bounds by calculating them from the vertices in the provided mesh data object. */
		void updateBounds(const MeshData& meshData);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

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
		int mUsage = MU_STATIC;
		IndexType mIndexType = IT_32BIT;
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
		static HMesh create(const SPtr<MeshData>& initialData, const MESH_DESC& desc);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param[in]	initialData		Vertex and index data to initialize the mesh with.
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 * @param[in]	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle strip, where three indices represent a single triangle.
		 */
		static HMesh create(const SPtr<MeshData>& initialData, int usage = MU_STATIC,
			DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/** @name Internal
		 *  @{
		 */

		/**
		 * @copydoc	create(const MESH_DESC&)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreatePtrInternal(const MESH_DESC& desc);

		/**
		 * @copydoc	create(const SPtr<MeshData>&, const MESH_DESC&)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreatePtrInternal(const SPtr<MeshData>& initialData, const MESH_DESC& desc);

		/**
		 * @copydoc	create(const SPtr<MeshData>&, int, DrawOperationType)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreatePtrInternal(const SPtr<MeshData>& initialData, int usage = MU_STATIC,
			DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/**
		 * Creates a new empty and uninitialized mesh. You will need to manually initialize the mesh before using it.
		 *	
		 * @note	This should only be used for special cases like serialization and is not meant for normal use.
		 */
		static SPtr<Mesh> createEmpty();

		/** @} */
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Core thread portion of a bs::Mesh.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT Mesh : public MeshBase
	{
	public:
		Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc, GpuDeviceFlags deviceMask);

		~Mesh();

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc MeshBase::getVertexData */
		SPtr<VertexData> getVertexData() const override;

		/** @copydoc MeshBase::getIndexBuffer */
		SPtr<IndexBuffer> getIndexBuffer() const override;

		/** @copydoc MeshBase::getVertexDesc */
		SPtr<VertexDataDesc> getVertexDesc() const override;

		/** Returns a skeleton that can be used for animating the mesh. */
		SPtr<Skeleton> getSkeleton() const { return mSkeleton; }

		/** Returns an object containing all shapes used for morph animation, if any are available. */
		SPtr<MorphShapes> getMorphShapes() const { return mMorphShapes; }

		/**
		 * Updates the current mesh with the provided data.
		 *
		 * @param[in]	data				Data to update the mesh with.
		 * @param[in]	discardEntireBuffer When true the existing contents of the resource you are updating will be
		 *									discarded. This can make the operation faster. Resources with certain buffer
		 *									types might require this flag to be in a specific state otherwise the operation
		 *									will fail.
		 * @param[in]	updateBounds		If true the internal bounds of the mesh will be recalculated based on the
		 *									provided data.
		 * @param[in]	queueIdx			Device queue to perform the write operation on. See @ref queuesDoc.
		 */
		virtual void writeData(const MeshData& data, bool discardEntireBuffer, bool updateBounds = true,
			UINT32 queueIdx = 0);

		/**
		 * Reads the current mesh data into the provided @p data parameter. Data buffer needs to be pre-allocated.
		 *
		 * @param[out]	data				Pre-allocated buffer of proper vertex/index format and size where data will be
		 *									read to. You can use Mesh::allocBuffer() to allocate a buffer of a correct
		 *									format and size.
		 * @param[in]	deviceIdx			Index of the device whose memory to read. If the buffer doesn't exist on this
		 *									device, no data will be read.
		 * @param[in]	queueIdx			Device queue to perform the read operation on. See @ref queuesDoc.
		 */
		virtual void readData(MeshData& data, UINT32 deviceIdx = 0, UINT32 queueIdx = 0);

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
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		static SPtr<Mesh> create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc,
			int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST, IndexType indexType = IT_32BIT,
			GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/**
		 * Creates a new empty mesh.
		 *
		 * @param[in]	desc			Descriptor containing the properties of the mesh to create.
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		static SPtr<Mesh> create(const MESH_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly.
		 *
		 * @param[in]	initialData		Vertex and index data to initialize the mesh with.
		 * @param[in]	desc			Descriptor containing the properties of the mesh to create. Vertex and index count,
		 *								vertex descriptor and index type properties are ignored and are read from provided
		 *								mesh data instead.
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		static SPtr<Mesh> create(const SPtr<MeshData>& initialData, const MESH_DESC& desc,
			GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param[in]	initialData		Vertex and index data to initialize the mesh with.
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 * @param[in]	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle strip, where three indices represent a single triangle.
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		static SPtr<Mesh> create(const SPtr<MeshData>& initialData, int usage = MU_STATIC,
			DrawOperationType drawOp = DOT_TRIANGLE_LIST, GpuDeviceFlags deviceMask = GDF_DEFAULT);

	protected:
		friend class bs::Mesh;

		/** Updates bounds by calculating them from the vertices in the provided mesh data object. */
		void updateBounds(const MeshData& meshData);

		SPtr<VertexData> mVertexData;
		SPtr<IndexBuffer> mIndexBuffer;

		SPtr<VertexDataDesc> mVertexDesc;
		int mUsage;
		IndexType mIndexType;
		GpuDeviceFlags mDeviceMask;
		SPtr<MeshData> mTempInitialMeshData;
		SPtr<Skeleton> mSkeleton; // Immutable
		SPtr<MorphShapes> mMorphShapes; // Immutable
	};

	/** @} */
	}
}
