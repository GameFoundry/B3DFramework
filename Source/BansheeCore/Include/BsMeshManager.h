//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsMesh.h"

namespace bs
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

		/** Returns some dummy mesh data with one triangle you may use for initializing a mesh. */
		SPtr<MeshData> getDummyMeshData() const { return mDummyMeshData; }

		/**	Returns a dummy mesh containing one triangle. */
		HMesh getDummyMesh() const { return mDummyMesh; }

	protected:
		/** @copydoc Module::onStartUp */
		void onStartUp() override;

	private:
		SPtr<MeshData> mDummyMeshData;
		HMesh mDummyMesh;
	};

	/** @} */
}