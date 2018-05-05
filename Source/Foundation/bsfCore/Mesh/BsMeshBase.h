//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "Math/BsBounds.h"
#include "RenderAPI/BsSubMesh.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**
	 * Planned usage for the mesh. These options usually affect performance and you should specify static if you don't plan
	 * on modifying the mesh often, otherwise specify dynamic.
	 */
	enum BS_SCRIPT_EXPORT(m:Rendering) MeshUsage
	{ 
		/** Specify for a mesh that is not often updated from the CPU. */
		MU_STATIC		BS_SCRIPT_EXPORT(n:Static) = 1 << 0,

		/** Specify for a mesh that is often updated from the CPU. */
		MU_DYNAMIC		BS_SCRIPT_EXPORT(n:Dynamic) = 1 << 1,
		/** 
		 * All mesh data will also be cached in CPU memory, making it available for fast read access from the CPU. Can be 
		 * combined with other usage flags. 
		 */
		MU_CPUCACHED	BS_SCRIPT_EXPORT(n:CPUCached) = 0x1000, 
	};

	/** Properties of a Mesh. Shared between sim and core thread versions of a Mesh. */
	class BS_CORE_EXPORT MeshProperties
	{
	public:
		MeshProperties();
		MeshProperties(UINT32 numVertices, UINT32 numIndices, DrawOperationType drawOp);
		MeshProperties(UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes);

		/**
		 * Retrieves a sub-mesh containing data used for rendering a certain portion of this mesh. If no sub-meshes are
		 * specified manually a special sub-mesh containing all indices is returned.
		 */
		const SubMesh& getSubMesh(UINT32 subMeshIdx = 0) const;

		/** Retrieves a total number of sub-meshes in this mesh. */
		UINT32 getNumSubMeshes() const;

		/**	Returns maximum number of vertices the mesh may store. */
		UINT32 getNumVertices() const { return mNumVertices; }

		/**	Returns maximum number of indices the mesh may store. */
		UINT32 getNumIndices() const { return mNumIndices; }

		/**	Returns bounds of the geometry contained in the vertex buffers for all sub-meshes. */
		const Bounds& getBounds() const { return mBounds; }

	protected:
		friend class MeshBase;
		friend class ct::MeshBase;
		friend class Mesh;
		friend class ct::Mesh;
		friend class TransientMesh;
		friend class ct::TransientMesh;
		friend class MeshBaseRTTI;

		Vector<SubMesh> mSubMeshes;
		UINT32 mNumVertices;
		UINT32 mNumIndices;
		Bounds mBounds;
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/**
	 * Base class all mesh implementations derive from. Meshes hold geometry information, normally in the form of one or 
	 * several index or vertex buffers. Different mesh implementations might choose to manage those buffers differently.
	 *
	 * @note	Sim thread.
	 */
	class BS_CORE_EXPORT MeshBase : public Resource
	{
	public:
		/**
		 * Constructs a new mesh with no sub-meshes.
		 *
		 * @param[in]	numVertices		Number of vertices in the mesh.
		 * @param[in]	numIndices		Number of indices in the mesh. 
		 * @param[in]	drawOp			Determines how should the provided indices be interpreted by the pipeline. Default 
		 *								option is triangles, where three indices represent a single triangle.
		 */
		MeshBase(UINT32 numVertices, UINT32 numIndices, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/**
		 * Constructs a new mesh with one or multiple sub-meshes. (When using just one sub-mesh it is equivalent to using
		 * the other overload).
		 *
		 * @param[in]	numVertices		Number of vertices in the mesh.
		 * @param[in]	numIndices		Number of indices in the mesh.
		 * @param[in]	subMeshes		Defines how are indices separated into sub-meshes, and how are those sub-meshes 
		 *								rendered.
		 */
		MeshBase(UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes);

		virtual ~MeshBase();

		/**	Returns properties that contain information about the mesh. */
		const MeshProperties& getProperties() const { return mProperties; }

		/**	Retrieves a core implementation of a mesh usable only from the core thread. */
		SPtr<ct::MeshBase> getCore() const;

	protected:
		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		MeshProperties mProperties;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	private:
		MeshBase() { } // Serialization only

	public:
		friend class MeshBaseRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	namespace ct
	{
	/**
	 * Core version of a class used as a basis for all implemenations of meshes.
	 *
	 * @see		bs::MeshBase
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT MeshBase : public CoreObject
	{
	public:
		MeshBase(UINT32 numVertices, UINT32 numIndices, const Vector<SubMesh>& subMeshes);
		virtual ~MeshBase() { }

		/**	Get vertex data used for rendering. */
		virtual SPtr<VertexData> getVertexData() const = 0;

		/**	Get index data used for rendering. */
		virtual SPtr<IndexBuffer> getIndexBuffer() const = 0;

		/**
		 * Returns an offset into the vertex buffers that is returned by getVertexData() that signifies where this meshes
		 * vertices begin.
		 * 			
		 * @note	Used when multiple meshes share the same buffers.
		 */
		virtual UINT32 getVertexOffset() const { return 0; }

		/**
		 * Returns an offset into the index buffer that is returned by getIndexData() that signifies where this meshes
		 * indices begin.
		 * 			
		 * @note	Used when multiple meshes share the same buffers.
		 */
		virtual UINT32 getIndexOffset() const { return 0; }

		/** Returns a structure that describes how are the vertices stored in the mesh's vertex buffer. */
		virtual SPtr<VertexDataDesc> getVertexDesc() const = 0;

		/**
		 * Called whenever this mesh starts being used on the GPU.
		 * 			
		 * @note	Needs to be called after all commands referencing this mesh have been sent to the GPU.
		 */
		virtual void _notifyUsedOnGPU() { }

		/**	Returns properties that contain information about the mesh. */
		const MeshProperties& getProperties() const { return mProperties; }

	protected:
		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;

		MeshProperties mProperties;
	};
	}

	/** @} */
}