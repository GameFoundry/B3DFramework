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
		MESH_DESC() {}

		/** Number of vertices in the mesh. */
		u32 NumVertices = 0;

		/** Number of indices in the mesh. */
		u32 NumIndices = 0;

		/**
		 * Vertex description structure that describes how are vertices organized in the vertex buffer. When binding a mesh
		 * to the pipeline you must ensure vertex description at least partially matches the input description of the
		 * currently bound vertex GPU program.
		 */
		SPtr<VertexDataDesc> VertexDesc;

		/**
		 * Defines how are indices separated into sub-meshes, and how are those sub-meshes rendered. Sub-meshes may be
		 * rendered independently.
		 */
		Vector<SubMesh> SubMeshes;

		/** Optimizes performance depending on planned usage of the mesh. */
		i32 Usage = MU_STATIC;

		/**
		 * Size of indices, use smaller size for better performance, however be careful not to go over the number of
		 * vertices limited by the size.
		 */
		IndexType IndexType = IT_32BIT;

		/** Optional skeleton that can be used for skeletal animation of the mesh. */
		SPtr<Skeleton> Skeleton;

		/** Optional set of morph shapes that can be used for per-vertex animation of the mesh. */
		SPtr<MorphShapes> MorphShapes;

		static MESH_DESC DEFAULT;
	};

	/**
	 * Primary class for holding geometry. Stores data in the form of vertex buffers and optionally an index buffer, which
	 * may be bound to the pipeline for drawing. May contain multiple sub-meshes.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Mesh : public MeshBase
	{
	public:
		virtual ~Mesh() = default;

		void Initialize() override;

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
		AsyncOp WriteData(const SPtr<MeshData>& data, bool discardEntireBuffer);

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
		AsyncOp ReadData(const SPtr<MeshData>& data);

		/**
		 * Allocates a buffer that exactly matches the size of this mesh. This is a helper function, primarily meant for
		 * creating buffers when reading from, or writing to a mesh.
		 *
		 * @note	Thread safe.
		 */
		SPtr<MeshData> AllocBuffer() const;

		/**
		 * Returns mesh data cached in the system memory. If the mesh wasn't created with CPU cached usage flag this
		 * method will not return any data. Caller should not modify the returned data.
		 *
		 * @note
		 * The data read is the cached mesh data. Any data written to the mesh from the GPU or core thread will not be
		 * reflected in this data. Use readData() if you require those changes.
		 */
		SPtr<MeshData> GetCachedData() const { return mCPUData; }

		/** Gets the skeleton required for animation of this mesh, if any is available. */
		BS_SCRIPT_EXPORT(Property(Getter), ExportName(Skeleton))

		SPtr<Skeleton> GetSkeleton() const { return mSkeleton; }

		/** Returns an object containing all shapes used for morph animation, if any are available. */
		BS_SCRIPT_EXPORT(Property(Getter), ExportName(MorphShapes))

		SPtr<MorphShapes> GetMorphShapes() const { return mMorphShapes; }

		/** Retrieves a core implementation of a mesh usable only from the core thread. */
		SPtr<ct::Mesh> GetCore() const;

		/**	Returns a dummy mesh, containing just one triangle. Don't modify the returned mesh. */
		static HMesh Dummy();

	protected:
		friend class MeshManager;

		Mesh(const MESH_DESC& desc);
		Mesh(const SPtr<MeshData>& initialMeshData, const MESH_DESC& desc);

		/**	Updates bounds by calculating them from the vertices in the provided mesh data object. */
		void UpdateBounds(const MeshData& meshData);

		SPtr<ct::CoreObject> CreateCore() const override;

		/**
		 * Creates buffers used for caching of CPU mesh data.
		 *
		 * @note	Make sure to initialize all mesh properties before calling this.
		 */
		void CreateCpuBuffer();

		/**	Updates the cached CPU buffers with new data. */
		void UpdateCpuBuffer(u32 subresourceIdx, const MeshData& data);

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
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

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
		static HMesh Create(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST, IndexType indexType = IT_32BIT);

		/**
		 * Creates a new empty mesh.
		 *
		 * @param[in]	desc	Descriptor containing the properties of the mesh to create.
		 */
		static HMesh Create(const MESH_DESC& desc);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param[in]	initialData		Vertex and index data to initialize the mesh with.
		 * @param[in]	desc			Descriptor containing the properties of the mesh to create. Vertex and index count,
		 *								vertex descriptor and index type properties are ignored and are read from provided
		 *								mesh data instead.
		 */
		static HMesh Create(const SPtr<MeshData>& initialData, const MESH_DESC& desc);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param[in]	initialData		Vertex and index data to initialize the mesh with.
		 * @param[in]	usage			Optimizes performance depending on planned usage of the mesh.
		 * @param[in]	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle strip, where three indices represent a single triangle.
		 */
		static HMesh Create(const SPtr<MeshData>& initialData, int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/** @name Internal
		 *  @{
		 */

		/**
		 * @copydoc	Create(const MESH_DESC&)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreatePtrInternal(const MESH_DESC& desc);

		/**
		 * @copydoc	Create(const SPtr<MeshData>&, const MESH_DESC&)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreatePtrInternal(const SPtr<MeshData>& initialData, const MESH_DESC& desc);

		/**
		 * @copydoc	Create(const SPtr<MeshData>&, int, DrawOperationType)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreatePtrInternal(const SPtr<MeshData>& initialData, int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/**
		 * Creates a new empty and uninitialized mesh. You will need to manually initialize the mesh before using it.
		 *
		 * @note	This should only be used for special cases like serialization and is not meant for normal use.
		 */
		static SPtr<Mesh> CreateEmpty();

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

			void Initialize() override;

			/** @copydoc MeshBase::GetVertexData */
			SPtr<VertexData> GetVertexData() const;

			/** @copydoc MeshBase::GetIndexBuffer */
			SPtr<IndexBuffer> GetIndexBuffer() const;

			/** @copydoc MeshBase::GetVertexDesc */
			SPtr<VertexDataDesc> GetVertexDesc() const;

			/** Returns a skeleton that can be used for animating the mesh. */
			SPtr<Skeleton> GetSkeleton() const { return mSkeleton; }

			/** Returns an object containing all shapes used for morph animation, if any are available. */
			SPtr<MorphShapes> GetMorphShapes() const { return mMorphShapes; }

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
			virtual void WriteData(const MeshData& data, bool discardEntireBuffer, bool updateBounds = true, u32 queueIdx = 0);

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
			virtual void ReadData(MeshData& data, u32 deviceIdx = 0, u32 queueIdx = 0);

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
			static SPtr<Mesh> Create(u32 numVertices, u32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST, IndexType indexType = IT_32BIT, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * Creates a new empty mesh.
			 *
			 * @param[in]	desc			Descriptor containing the properties of the mesh to create.
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			static SPtr<Mesh> Create(const MESH_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

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
			static SPtr<Mesh> Create(const SPtr<MeshData>& initialData, const MESH_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

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
			static SPtr<Mesh> Create(const SPtr<MeshData>& initialData, int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		protected:
			friend class bs::Mesh;

			/** Updates bounds by calculating them from the vertices in the provided mesh data object. */
			void UpdateBounds(const MeshData& meshData);

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
	} // namespace ct
} // namespace bs
