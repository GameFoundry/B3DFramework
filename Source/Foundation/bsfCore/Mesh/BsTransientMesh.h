//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Mesh/BsMeshBase.h"

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
		SPtr<ct::TransientMesh> getCore() const;

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

		/** @copydoc MeshBase::createCore */
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
	 * Core thread portion of a bs::TransientMesh.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT TransientMesh : public MeshBase
	{
	public:
		TransientMesh(const SPtr<MeshHeap>& parentHeap, UINT32 id, UINT32 numVertices,
			UINT32 numIndices, const Vector<SubMesh>& subMeshes);

		/** @copydoc MeshBase::getVertexData */
		SPtr<VertexData> getVertexData() const override;

		 /** @copydoc MeshBase::getIndexBuffer */
		SPtr<IndexBuffer> getIndexBuffer() const override;

		/** @copydoc MeshBase::getVertexDesc */
		SPtr<VertexDataDesc> getVertexDesc() const override;

		/**	Returns the ID that uniquely identifies this mesh in the parent heap. */
		UINT32 getMeshHeapId() const { return mId; }

		/** @copydoc MeshBase::getVertexOffset */
		UINT32 getVertexOffset() const override;

		 /** @copydoc MeshBase::getIndexOffset */
		UINT32 getIndexOffset() const override;

		 /** @copydoc MeshBase::_notifyUsedOnGPU */
		void NotifyUsedOnGPUInternal() override;

	protected:
		friend class bs::TransientMesh;

		SPtr<MeshHeap> mParentHeap;
		UINT32 mId;
	};

	/** @} */
	}
}
