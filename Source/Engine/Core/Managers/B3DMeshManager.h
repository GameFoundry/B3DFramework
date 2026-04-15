//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Mesh/B3DMesh.h"

namespace b3d
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/** Manager that handles creation of Mesh%es. */
	class B3D_EXPORT MeshManager : public Module<MeshManager>
	{
	public:
		/** Returns some dummy mesh data with one triangle you may use for initializing a mesh. */
		SPtr<MeshData> GetDummyMeshData() const { return mDummyMeshData; }

		/**	Returns a dummy mesh containing one triangle. */
		HMesh GetDummyMesh() const { return mDummyMesh; }

	protected:
		/** @copydoc Module::onStartUp */
		void OnStartUp() override;

	private:
		SPtr<MeshData> mDummyMeshData;
		HMesh mDummyMesh;
	};

	/** @} */
} // namespace b3d
