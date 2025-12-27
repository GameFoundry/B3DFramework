//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"
#include "Math/B3DBounds.h"
#include "RenderAPI/B3DSubMesh.h"

namespace b3d
{
	namespace render
	{
		class MeshBase;
	}

	/** @addtogroup Resources
	 *  @{
	 */

	/** Flags that control Mesh behaviour. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) MeshFlag
	{
		/** Mesh is not going to change ever, or is going to change rarely. Mesh vertex & index buffers will be allocated in GPU memory. */
		Static = 1 << 0,

		/** Mesh is going to change often (e.g. every frame). Mesh vertex & index buffers will be allocated in CPU memory which is GPU accessible. */
		Dynamic = 1 << 1,

		/**
		 * Mesh vertex and input buffers can be bound for unordered access (i.e. structured storage buffers) in the shaders. Provide this your shader is
		 * manually pulling vertex/index data in the shader.
		 */
		UnorderedAccess = 1 << 2,

		/**
		 * Normally when a mesh is uploaded to the GPU, the CPU memory is no longer needed so it will be released. If this flag is provided the CPU mesh data
		 * will be kept. This allows you to access mesh data on the CPU.
		 */
		KeepCPUCopy = 1 << 3,
	};

	using MeshFlags = Flags<MeshFlag>;
	B3D_FLAGS_OPERATORS(MeshFlag);

	/** Properties of a Mesh. Shared between main and render thread counterparts of a Mesh. */
	class B3D_EXPORT MeshProperties
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
		Bounds Bounds = Bounds::kEmpty;
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
	class B3D_EXPORT MeshBase : public Resource
	{
	public:
		/**
		 * Constructs a new mesh with no sub-meshes.
		 *
		 * @param	vertexCount		Number of vertices in the mesh.
		 * @param	indexCount		Number of indices in the mesh.
		 * @param	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default
		 *								option is triangles, where three indices represent a single triangle.
		 */
		MeshBase(u32 vertexCount, u32 indexCount, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/**
		 * Constructs a new mesh with one or multiple sub-meshes. (When using just one sub-mesh it is equivalent to using
		 * the other overload).
		 *
		 * @param	vertexCount		Number of vertices in the mesh.
		 * @param	indexCount		Number of indices in the mesh.
		 * @param	subMeshes		Defines how are indices separated into sub-meshes, and how are those sub-meshes
		 *								rendered.
		 */
		MeshBase(u32 vertexCount, u32 indexCount, const Vector<SubMesh>& subMeshes);

		virtual ~MeshBase();

		/**	Returns properties that contain information about the mesh. */
		const MeshProperties& GetProperties() const { return mProperties; }

	protected:
		friend class render::MeshBase;
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
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	namespace render
	{
		/**
		 * Render proxy  used as a basis for all implemenations of meshes.
		 *
		 * @see		b3d::MeshBase
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT MeshBase : public RenderProxy
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
			friend class b3d::MeshBase;

			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			MeshProperties mProperties;
		};
	} // namespace render

	/** @} */
} // namespace b3d
