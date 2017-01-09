//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsMeshBase.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**
	 * Represents a single mesh entry in the MeshHeap. This can be used as a normal mesh but due to the nature of the 
	 * mesh heap it is not the type of mesh you should use for storing static data.
	 *
	 * Transient meshes don't keep internal index/vertex buffers but instead use the ones provided by their parent mesh heap.
	 *
	 * @note	Sim thread.
	 */
	class BS_CORE_EXPORT TransientMesh : public MeshBase
	{
	public:
		virtual ~TransientMesh();

		/** Retrieves a core implementation of a mesh usable only from the core thread. */
		SPtr<ct::TransientMeshCore> getCore() const;

	protected:
		friend class MeshHeap;

		/**
		 * Constructs a new transient mesh.
		 *
		 * @see		MeshHeap::alloc
		 */
		TransientMesh(const SPtr<MeshHeap>& parentHeap, UINT32 id, UINT32 numVertices,
			UINT32 numIndices, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/** Marks the mesh as destroyed so we know that we don't need to destroy it ourselves. */
		void markAsDestroyed() { mIsDestroyed = true; }

		/** @copydoc RenderTarget::createCore */
		SPtr<ct::CoreObject> createCore() const override;

	protected:
		bool mIsDestroyed;
		SPtr<MeshHeap> mParentHeap;
		UINT32 mId;
	};

	/** @} */

	namespace ct
	{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Core thread portion of a TransientMesh.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT TransientMeshCore : public MeshCoreBase
	{
	public:
		TransientMeshCore(const SPtr<MeshHeapCore>& parentHeap, UINT32 id, UINT32 numVertices,
			UINT32 numIndices, const Vector<SubMesh>& subMeshes);

		/** @copydoc MeshCoreBase::getVertexData */
		SPtr<VertexData> getVertexData() const override;

		 /** @copydoc MeshCoreBase::getIndexBuffer */
		SPtr<IndexBufferCore> getIndexBuffer() const override;

		/** @copydoc MeshCoreBase::getVertexDesc */
		SPtr<VertexDataDesc> getVertexDesc() const override;

		/**	Returns the ID that uniquely identifies this mesh in the parent heap. */
		UINT32 getMeshHeapId() const { return mId; }

		/** @copydoc MeshCoreBase::getVertexOffset */
		UINT32 getVertexOffset() const override;

		 /** @copydoc MeshCoreBase::getIndexOffset */
		UINT32 getIndexOffset() const override;

		 /** @copydoc MeshCoreBase::_notifyUsedOnGPU */
		void _notifyUsedOnGPU() override;

	protected:
		friend class TransientMesh;

		SPtr<MeshHeapCore> mParentHeap;
		UINT32 mId;
	};

	/** @} */
	}
}