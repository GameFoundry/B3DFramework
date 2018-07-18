//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Mesh/BsMesh.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Manager that handles creation of Mesh%es. */
	class BS_CORE_EXPORT MeshManager : public Module<MeshManager>
	{
	public:
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