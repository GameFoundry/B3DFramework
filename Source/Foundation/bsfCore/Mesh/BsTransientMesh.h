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
		SPtr<ct::TransientMesh> GetCore() const;

	protected:
		friend class MeshHeap;

		/**
		 * Constructs a new transient mesh.
		 *
		 * @see		MeshHeap::alloc
		 */
		TransientMesh(const SPtr<MeshHeap>& parentHeap, u32 id, u32 numVertices, u32 numIndices, DrawOperationType drawOp = DOT_TRIANGLE_LIST);

		/** Marks the mesh as destroyed so we know that we don't need to destroy it ourselves. */
		void MarkAsDestroyed() { mIsDestroyed = true; }

		SPtr<ct::CoreObject> CreateCore() const override;

	protected:
		bool mIsDestroyed;
		SPtr<MeshHeap> mParentHeap;
		u32 mId;
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
			TransientMesh(const SPtr<MeshHeap>& parentHeap, u32 id, u32 numVertices, u32 numIndices, const Vector<SubMesh>& subMeshes);

			/**	Returns the ID that uniquely identifies this mesh in the parent heap. */
			u32 GetMeshHeapId() const { return mId; }

			SPtr<VertexData> GetVertexData() const override;
			SPtr<IndexBuffer> GetIndexBuffer() const override;
			SPtr<VertexDataDesc> GetVertexDesc() const override;

			u32 GetVertexOffset() const override;
			u32 GetIndexOffset() const override;
			void NotifyUsedOnGPUInternal() override;

		protected:
			friend class bs::TransientMesh;

			SPtr<MeshHeap> mParentHeap;
			u32 mId;
		};

		/** @} */
	} // namespace ct
} // namespace bs
