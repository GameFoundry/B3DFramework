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
	class BS_SCR_BE_EXPORT BS_SCRIPT_EXPORT(ExtensionMethodForType(PhysicsMesh)) PhysicsMeshEx
	{
	public:
		/** @copydoc PhysicsMesh::Create() */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(PhysicsMesh))
		static HPhysicsMesh Create(const SPtr<RendererMeshData>& meshData, PhysicsMeshType type = PhysicsMeshType::Convex);

		/** @copydoc PhysicsMesh::getMeshData() */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(PhysicsMesh), ExportName(MeshData), Property(Getter))
		static SPtr<RendererMeshData> GetMeshData(const HPhysicsMesh& thisPtr);
	};

	/** @endcond */
	/** @} */
} // namespace bs
