//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "Math/BsBounds.h"
#include "RenderAPI/BsSubMesh.h"

namespace bs
{
	namespace ct
	{
		class MeshBase;
	}

	/** @addtogroup Resources
	 *  @{
	 */

	/**
	 * Planned usage for the mesh. These options usually affect performance and you should specify static if you don't plan
	 * on modifying the mesh often, otherwise specify dynamic.
	 */
	enum B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) MeshUsage
	{
		/** Specify for a mesh that is not often updated from the CPU. */
		MU_STATIC B3D_SCRIPT_EXPORT(ExportName(Static)) = 1 << 0,

		/** Specify for a mesh that is often updated from the CPU. */
		MU_DYNAMIC B3D_SCRIPT_EXPORT(ExportName(Dynamic)) = 1 << 1,
		/**
		 * All mesh data will also be cached in CPU memory, making it available for fast read access from the CPU. Can be
		 * combined with other usage flags.
		 */
		MU_CPUCACHED B3D_SCRIPT_EXPORT(ExportName(CPUCached)) = 0x1000,
	};

	/** Properties of a Mesh. Shared between main and render thread counterparts of a Mesh. */
	class B3D_CORE_EXPORT MeshProperties
	{
	public:
		MeshProperties();
		MeshProperties(u32 vertexCount, u32 indexCount, DrawOperationType primitiveType);
		MeshProperties(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes);

		/** Contains data used for rendering a certain portion(s) of this mesh. */
		Vector<SubMesh> SubMeshes;

		/**	Maximum number of vertices the mesh may store. */
		u32 VertexCount;

		/**	Maximum number of indices the mesh may store. */
		u32 IndexCount;

		/**	Bounds of the geometry contained in the vertex buffers for all sub-meshes. */
		Bounds Bounds;
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/**
	 * Base class all mesh implementations derive from. Meshes hold geometry information, normally in the form of one or
	 * several index or vertex buffers. Different mesh implementations might choose to manage those buffers differently.
	 *
	 * @note	Main thread.
	 */
	class B3D_CORE_EXPORT MeshBase : public Resource
	{
	public:
		/**
		 * Constructs a new mesh with no sub-meshes.
		 *
		 * @param[in]	vertexCount		Number of vertices in the mesh.
		 * @param[in]	indexCount		Number of indices in the mesh.
		 * @param[in]	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is triangles, where three indices represent a single triangle.
		 */
		MeshBase(u32 vertexCount, u32 indexCount, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/**
		 * Constructs a new mesh with one or multiple sub-meshes. (When using just one sub-mesh it is equivalent to using
		 * the other overload).
		 *
		 * @param[in]	vertexCount		Number of vertices in the mesh.
		 * @param[in]	indexCount		Number of indices in the mesh.
		 * @param[in]	subMeshes		Defines how are indices separated into sub-meshes, and how are those sub-meshes
		 *								rendered.
		 */
		MeshBase(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes);

		virtual ~MeshBase();

		/**	Returns properties that contain information about the mesh. */
		const MeshProperties& GetProperties() const { return mProperties; }

		/**	Retrieves the render proxy. */
		SPtr<ct::MeshBase> GetCore() const;

	protected:
		friend class ct::MeshBase;
		struct SyncPacket;

		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

		MeshProperties mProperties;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	private:
		MeshBase() {} // Serialization only

	public:
		friend class MeshBaseRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	namespace ct
	{
		/**
		 * Render proxy  used as a basis for all implemenations of meshes.
		 *
		 * @see		bs::MeshBase
		 *
		 * @note	Render thread.
		 */
		class B3D_CORE_EXPORT MeshBase : public RenderProxy
		{
		public:
			MeshBase(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes);

			virtual ~MeshBase() {}

			/**	Get vertex data used for rendering. */
			virtual SPtr<VertexData> GetVertexData() const = 0;

			/**	Get index data used for rendering. */
			virtual SPtr<GpuBuffer> GetIndexBuffer() const = 0;

			/**
			 * Returns an offset into the vertex buffers that is returned by getVertexData() that signifies where this meshes
			 * vertices begin.
			 *
			 * @note	Used when multiple meshes share the same buffers.
			 */
			virtual u32 GetVertexOffset() const { return 0; }

			/**
			 * Returns an offset into the index buffer that is returned by getIndexData() that signifies where this meshes
			 * indices begin.
			 *
			 * @note	Used when multiple meshes share the same buffers.
			 */
			virtual u32 GetIndexOffset() const { return 0; }

			/** Returns a structure that describes how are the vertices stored in the mesh's vertex buffer. */
			virtual SPtr<VertexDescription> GetVertexDescription() const = 0;

			/**
			 * Called whenever this mesh starts being used on the GPU.
			 *
			 * @note	Needs to be called after all commands referencing this mesh have been sent to the GPU.
			 */
			virtual void NotifyUsedOnGPU() {}

			/**	Returns properties that contain information about the mesh. */
			const MeshProperties& GetProperties() const { return mProperties; }

		protected:
			friend class bs::MeshBase;

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			MeshProperties mProperties;
		};
	} // namespace ct

	/** @} */
} // namespace bs
