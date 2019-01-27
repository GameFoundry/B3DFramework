//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Physics/BsPhysicsMesh.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for PhysicsMesh, for adding additional functionality for the script version of the class. */
	class BS_SCR_BE_EXPORT BS_SCRIPT_EXPORT(e:PhysicsMesh) PhysicsMeshEx
	{
	public:
		/** @copydoc PhysicsMesh::create() */
		BS_SCRIPT_EXPORT(ec:PhysicsMesh)
		static HPhysicsMesh create(const SPtr<RendererMeshData>& meshData, PhysicsMeshType type = PhysicsMeshType::Convex);

		/** @copydoc PhysicsMesh::getMeshData() */
		BS_SCRIPT_EXPORT(e:PhysicsMesh,n:MeshData,pr:getter)
		static SPtr<RendererMeshData> getMeshData(const HPhysicsMesh& thisPtr);
	};

	/** @endcond */
	/** @} */
}