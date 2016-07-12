//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsMesh.h"

namespace BansheeEngine
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Manager that handles creation of Mesh%es. */
	class BS_CORE_EXPORT MeshManager : public Module<MeshManager>
	{
	public:
		MeshManager();
		~MeshManager();

		/** @copydoc Mesh::create(UINT32, UINT32, const SPtr<VertexDataDesc>&, int, DrawOperationType, IndexType, const SPtr<Skeleton>&) */
		SPtr<Mesh> create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, int usage = MU_STATIC, 
			DrawOperationType drawOp = DOT_TRIANGLE_LIST, IndexType indexType = IT_32BIT, const SPtr<Skeleton>& skeleton = nullptr);

		/** @copydoc Mesh::create(UINT32, UINT32, const SPtr<VertexDataDesc>&, const Vector<SubMesh>&, int, IndexType, const SPtr<Skeleton>&) */
		SPtr<Mesh> create(UINT32 numVertices, UINT32 numIndices, const SPtr<VertexDataDesc>& vertexDesc, const Vector<SubMesh>& subMeshes, 
			int usage = MU_STATIC, IndexType indexType = IT_32BIT, const SPtr<Skeleton>& skeleton = nullptr);

		/** @copydoc Mesh::create(const SPtr<MeshData>&, int, DrawOperationType, const SPtr<Skeleton>&) */
		SPtr<Mesh> create(const SPtr<MeshData>& initialData, int usage = MU_STATIC, DrawOperationType drawOp = DOT_TRIANGLE_LIST, 
			const SPtr<Skeleton>& skeleton = nullptr);

		/** @copydoc Mesh::create(const SPtr<MeshData>&, const Vector<SubMesh>&, int, const SPtr<Skeleton>&) */
		SPtr<Mesh> create(const SPtr<MeshData>& initialData, const Vector<SubMesh>& subMeshes, int usage = MU_STATIC, 
			const SPtr<Skeleton>& skeleton = nullptr);

		/**
		 * Creates a new empty and uninitialized mesh. You will need to manually initialize the mesh before using it.
		 *	
		 * @note	This should only be used for special cases and is not meant for normal use.
		 */
		SPtr<Mesh> createEmpty();

		/** Returns some dummy mesh data with one triangle you may use for initializing a mesh. */
		SPtr<MeshData> getDummyMeshData() const { return mDummyMeshData; }

		/**	Returns a dummy mesh containing one triangle. */
		HMesh getDummyMesh() const { return mDummyMesh; }

	protected:
		/** @copydoc Module::onStartUp */
		virtual void onStartUp() override;

	private:
		SPtr<MeshData> mDummyMeshData;
		HMesh mDummyMesh;
	};

	/** @} */
}