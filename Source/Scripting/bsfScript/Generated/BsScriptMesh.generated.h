//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsSubMesh.h"
#include "Math/BsSphere.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshBase.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "Math/BsAABox.h"

namespace bs { class Mesh; }
namespace bs { class MeshEx; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptMesh : public TScriptResource<ScriptMesh, Mesh>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Mesh")

		ScriptMesh(MonoObject* managedInstance, const ResourceHandle<Mesh>& value);

		static MonoObject* createInstance();

	private:
		static MonoObject* Internal_getRef(ScriptMesh* thisPtr);

		static MonoObject* Internal_getSkeleton(ScriptMesh* thisPtr);
		static MonoObject* Internal_getMorphShapes(ScriptMesh* thisPtr);
		static void Internal_create(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index);
		static void Internal_create0(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index);
		static void Internal_create1(MonoObject* managedInstance, MonoObject* data, DrawOperationType topology, MeshUsage usage);
		static void Internal_create2(MonoObject* managedInstance, MonoObject* data, MonoArray* subMeshes, MeshUsage usage);
		static MonoArray* Internal_getSubMeshes(ScriptMesh* thisPtr);
		static uint32_t Internal_getSubMeshCount(ScriptMesh* thisPtr);
		static void Internal_getBounds(ScriptMesh* thisPtr, AABox* box, Sphere* sphere);
		static MonoObject* Internal_getMeshData(ScriptMesh* thisPtr);
		static void Internal_setMeshData(ScriptMesh* thisPtr, MonoObject* value);
	};
}
