//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Physics/B3DPhysicsMesh.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for PhysicsMesh, for adding additional functionality for the script version of the class. */
	class B3D_SCRIPT_INTEROP_EXPORT B3D_SCRIPT_EXPORT(ExtensionClassForType(PhysicsMesh)) PhysicsMeshEx
	{
	public:
		/** @copydoc PhysicsMesh::Create() */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(PhysicsMesh))
		static HPhysicsMesh Create(const SPtr<RendererMeshData>& meshData, PhysicsMeshType type = PhysicsMeshType::Convex);

		/** @copydoc PhysicsMesh::GetMeshData() */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(PhysicsMesh), ExportName(MeshData), Property(Getter))
		static SPtr<RendererMeshData> GetMeshData(const HPhysicsMesh& thisPtr);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
