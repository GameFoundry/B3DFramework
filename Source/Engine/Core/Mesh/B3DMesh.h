//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Mesh/B3DMeshBase.h"
#include "Mesh/B3DMeshData.h"
#include "RenderAPI/B3DVertexData.h"
#include "RenderAPI/B3DSubMesh.h"
#include "Math/B3DBounds.h"

namespace b3d
{
	/** @addtogroup Mesh
	 *  @{
	 */

	/** Information used for creation of a new Mesh object. */
	struct B3D_EXPORT MeshCreateInformation
	{
		MeshCreateInformation() {}

		/** Number of vertices in the mesh. */
		u32 VertexCount = 0;

		/** Number of indices in the mesh. */
		u32 IndexCount = 0;

		/**
		 * Vertex description structure that describes how are vertices organized in the vertex buffer. When binding a mesh
		 * to the pipeline you must ensure vertex description at least partially matches the input description of the
		 * currently bound vertex GPU program.
		 */
		SPtr<VertexDescription> VertexDescription;

		/**
		 * Defines how are indices separated into sub-meshes, and how are those sub-meshes rendered. Sub-meshes may be
		 * rendered independently.
		 */
		Vector<SubMesh> SubMeshes;

		/** Optimizes performance depending on planned usage of the mesh. */
		MeshFlags Flags = MeshFlag::Static;

		/**
		 * Size of indices, use smaller size for better performance, however be careful not to go over the number of
		 * vertices limited by the size.
		 */
		IndexType IndexType = IT_32BIT;

		/** Optional skeleton that can be used for skeletal animation of the mesh. */
		SPtr<Skeleton> Skeleton;

		/** Optional set of morph shapes that can be used for per-vertex animation of the mesh. */
		SPtr<MorphShapes> MorphShapes;

		static const MeshCreateInformation kDefault;
	};

	/**
	 * Primary class for holding geometry. Stores data in the form of vertex buffers and optionally an index buffer, which
	 * may be bound to the pipeline for drawing. May contain multiple sub-meshes.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Mesh : public MeshBase
	{
	public:
		virtual ~Mesh() = default;

		void Initialize() override;

		/**
		 * Updates the mesh with new data. Provided data buffer will be locked until the operation completes.
		 *
		 * @param	data				Data of valid size and format to write to the subresource.
		 * @param	discardEntireBuffer When true the existing contents of the resource you are updating will be
		 *									discarded. This can make the operation faster. Resources with certain buffer
		 *									types might require this flag to be in a specific state otherwise the operation
		 *									will fail.
		 * @return							Async operation object you can use to track operation completion.
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		TAsyncOp<void> WriteData(const SPtr<MeshData>& data, bool discardEntireBuffer);

		/**
		 * Reads internal mesh data to the provided previously allocated buffer. Provided data buffer will be locked until
		 * the operation completes.
		 *
		 * @param	data			Pre-allocated buffer of proper vertex/index format and size where data will be read
		 *								to. You can use allocBuffer() to allocate a buffer of a correct format and size.
		 * @return						Async operation object you can use to track operation completion.
		 *
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		TAsyncOp<void> ReadData(const SPtr<MeshData>& data);

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
		 * The data read is the cached mesh data. Any data written to the mesh from the GPU or render thread will not be
		 * reflected in this data. Use readData() if you require those changes.
		 */
		SPtr<MeshData> GetCachedData() const { return mCPUData; }

		/** Gets the skeleton required for animation of this mesh, if any is available. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Skeleton))
		SPtr<Skeleton> GetSkeleton() const { return mSkeleton; }

		/** Returns an object containing all shapes used for morph animation, if any are available. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(MorphShapes))
		SPtr<MorphShapes> GetMorphShapes() const { return mMorphShapes; }

		/**	Returns a dummy mesh, containing just one triangle. Don't modify the returned mesh. */
		static HMesh Dummy();

	protected:
		friend class MeshManager;

		Mesh(const MeshCreateInformation& meshCreateInformation);
		Mesh(const SPtr<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation);

		/**	Updates bounds by calculating them from the vertices in the provided mesh data object. */
		void UpdateBounds(const MeshData& meshData);

		SPtr<render::RenderProxy> CreateRenderProxy() const override;

		/**
		 * Creates buffers used for caching of CPU mesh data.
		 *
		 * @note	Make sure to initialize all mesh properties before calling this.
		 */
		void CreateCpuBuffer();

		/**	Updates the cached CPU buffers with new data. */
		void UpdateCpuBuffer(u32 subresourceIndex, const MeshData& data);

		mutable SPtr<MeshData> mCPUData;

		SPtr<VertexDescription> mVertexDescription;
		MeshFlags mFlags = MeshFlag::Static;
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
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

		/************************************************************************/
		/* 								STATICS		                     		*/
		/************************************************************************/

	public:
		/**
		 * Creates a new empty mesh. Created mesh will have no sub-meshes.
		 *
		 * @param	vertexCount		Number of vertices in the mesh.
		 * @param	indexCount		Number of indices in the mesh.
		 * @param	vertexDescription		Vertex description structure that describes how are vertices organized in the
		 *								vertex buffer. When binding a mesh to the pipeline you must ensure vertex
		 *								description at least partially matches the input description of the currently bound
		 *								vertex GPU program.
		 * @param	flags			Flags to control various mesh options.
		 * @param	primitiveType	Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle list, where three indices represent a single triangle.
		 * @param	indexType		Size of indices, use smaller size for better performance, however be careful not to
		 *								go over the number of vertices limited by the size.
		 */
		static HMesh Create(u32 vertexCount, u32 indexCount, const SPtr<VertexDescription>& vertexDescription, MeshFlags flags = MeshFlag::Static, DrawOperationType primitiveType = DOT_TRIANGLE_LIST, IndexType indexType = IT_32BIT);

		/**
		 * Creates a new empty mesh.
		 *
		 * @param	meshCreateInformation	Descriptor containing the properties of the mesh to create.
		 */
		static HMesh Create(const MeshCreateInformation& meshCreateInformation);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param	initialData						Vertex and index data to initialize the mesh with.
		 * @param	meshCreateInformation			Descriptor containing the properties of the mesh to create. Vertex and index count,
		 *												vertex descriptor and index type properties are ignored and are read from provided
		 *												mesh data instead.
		 */
		static HMesh Create(const SPtr<MeshData>& initialData, const MeshCreateInformation& meshCreateInformation);

		/**
		 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
		 * by the mesh data exactly. Mesh will have no sub-meshes.
		 *
		 * @param	initialData		Vertex and index data to initialize the mesh with.
		 * @param	flags			Flags to control various mesh options.
		 * @param	primitiveType	Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is a triangle strip, where three indices represent a single triangle.
		 */
		static HMesh Create(const SPtr<MeshData>& initialData, MeshFlags flags = MeshFlag::Static, DrawOperationType primitiveType = DOT_TRIANGLE_LIST);

		/** @name Internal
		 *  @{
		 */

		/**
		 * @copydoc	Create(const MeshCreateInformation&)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreateShared(const MeshCreateInformation& meshCreateInformation);

		/**
		 * @copydoc	Create(const SPtr<MeshData>&, const MeshCreateInformation&)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreateShared(const SPtr<MeshData>& initialData, const MeshCreateInformation& meshCreateInformation);

		/**
		 * @copydoc	Create(const SPtr<MeshData>&, int, DrawOperationType)
		 *
		 * @note	Internal method. Use create() for normal use.
		 */
		static SPtr<Mesh> CreateShared(const SPtr<MeshData>& initialData, MeshFlags flags = MeshFlag::Static, DrawOperationType primitiveType = DOT_TRIANGLE_LIST);

		/**
		 * Creates a new empty and uninitialized mesh. You will need to manually initialize the mesh before using it.
		 *
		 * @note	This should only be used for special cases like serialization and is not meant for normal use.
		 */
		static SPtr<Mesh> CreateEmptyShared();

		/** @} */
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/**
		 * Render thread portion of a b3d::Mesh.
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT Mesh : public MeshBase
		{
		public:
			Mesh(const SPtr<MeshData>& initialMeshData, const MeshCreateInformation& meshCreateInformation);

			~Mesh();

			void Initialize() override;

			SPtr<VertexData> GetVertexData() const override;
			SPtr<GpuBuffer> GetIndexBuffer() const override;
			SPtr<VertexDescription> GetVertexDescription() const override;

			/** Returns a skeleton that can be used for animating the mesh. */
			SPtr<Skeleton> GetSkeleton() const { return mSkeleton; }

			/** Returns an object containing all shapes used for morph animation, if any are available. */
			SPtr<MorphShapes> GetMorphShapes() const { return mMorphShapes; }

			/**
			 * Updates the current mesh with the provided data.
			 *
			 * @param	data					Data to update the mesh with.
			 * @param	discardEntireBuffer		When true the existing contents of the resource you are updating will be
			 *									discarded. This can make the operation faster. Resources with certain buffer
			 *									types might require this flag to be in a specific state otherwise the operation
			 *									will fail.
			 * @param	updateBounds			If true the internal bounds of the mesh will be recalculated based on the provided data.
			 * @param	commandBuffer			Command buffer on which to issue a copy operation, in case the internal buffers aren't directly writeable.
			 */
			virtual void WriteData(const MeshData& data, bool discardEntireBuffer, bool updateBounds = true, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr);

			/**
			 * Reads the current mesh data into the provided @p data parameter. Data buffer needs to be pre-allocated.
			 *
			 * @param	data					Pre-allocated buffer of proper vertex/index format and size where data will be
			 *									read to. You can use Mesh::allocBuffer() to allocate a buffer of a correct
			 *									format and size.
			 * @param	commandBuffer			Command buffer on which to issue a copy operation, in case the internal buffers aren't directly readable.
			 */
			virtual void ReadData(MeshData& data, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr);

			/**
			 * Creates a new empty mesh. Created mesh will have no sub-meshes.
			 *
			 * @param	vertexCount			Number of vertices in the mesh.
			 * @param	indexCount			Number of indices in the mesh.
			 * @param	vertexDescription	Vertex description structure that describes how are vertices organized in the
			 *									vertex buffer. When binding a mesh to the pipeline you must ensure vertex
			 *									description at least partially matches the input description of the currently
			 *									bound vertex GPU program.
			 * @param	flags				Flags to control various mesh options.
			 * @param	primitiveType		Determines how should the provided indices be interpreted by the pipeline. Default
			 *									option is a triangle list, where three indices represent a single triangle.
			 * @param	indexType			Size of indices, use smaller size for better performance, however be careful not to
			 *									go over the number of vertices limited by the size.
			 */
			static SPtr<Mesh> Create(u32 vertexCount, u32 indexCount, const SPtr<VertexDescription>& vertexDescription, MeshFlags flags = MeshFlag::Static, DrawOperationType primitiveType = DOT_TRIANGLE_LIST, IndexType indexType = IT_32BIT);

			/**
			 * Creates a new empty mesh.
			 *
			 * @param	meshCreateInformation	Descriptor containing the properties of the mesh to create.
			 * @param	deviceMask				Mask that determines on which GPU devices should the object be created on.
			 */
			static SPtr<Mesh> Create(const MeshCreateInformation& meshCreateInformation);

			/**
			 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
			 * by the mesh data exactly.
			 *
			 * @param	initialData				Vertex and index data to initialize the mesh with.
			 * @param	meshCreateInformation	Descriptor containing the properties of the mesh to create. Vertex and index count,
			 *										vertex descriptor and index type properties are ignored and are read from provided
			 *										mesh data instead.
			 */
			static SPtr<Mesh> Create(const SPtr<MeshData>& initialData, const MeshCreateInformation& meshCreateInformation);

			/**
			 * Creates a new mesh from an existing mesh data. Created mesh will match the vertex and index buffers described
			 * by the mesh data exactly. Mesh will have no sub-meshes.
			 *
			 * @param	initialData		Vertex and index data to initialize the mesh with.
			 * @param	flags			Flags to control various mesh options.
			 * @param	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default
			 *								option is a triangle strip, where three indices represent a single triangle.
			 */
			static SPtr<Mesh> Create(const SPtr<MeshData>& initialData, MeshFlags flags = MeshFlag::Static, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		protected:
			friend class b3d::Mesh;

			/** Updates bounds by calculating them from the vertices in the provided mesh data object. */
			void UpdateBounds(const MeshData& meshData);

			SPtr<VertexData> mVertexData;
			SPtr<GpuBuffer> mIndexBuffer;

			SPtr<VertexDescription> mVertexDescription;
			MeshFlags mFlags;
			IndexType mIndexType;
			SPtr<MeshData> mTempInitialMeshData;
			SPtr<Skeleton> mSkeleton; // Immutable
			SPtr<MorphShapes> mMorphShapes; // Immutable
		};

		/** @} */
	} // namespace render
} // namespace b3d
