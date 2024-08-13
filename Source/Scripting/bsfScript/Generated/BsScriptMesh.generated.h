//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Math/BsSphere.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshBase.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsSubMesh.h"
#include "Math/BsAABox.h"

namespace bs { class Mesh; }
namespace bs { class MeshEx; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMesh : public TScriptResource<ScriptMesh, Mesh>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Mesh")

		ScriptMesh(MonoObject* managedInstance, const TResourceHandle<Mesh>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptMesh* self);

		static MonoObject* InternalGetSkeleton(ScriptMesh* self);
		static MonoObject* InternalGetMorphShapes(ScriptMesh* self);
		static void InternalCreate(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index);
		static void InternalCreate0(MonoObject* managedInstance, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index);
		static void InternalCreate1(MonoObject* managedInstance, MonoObject* data, DrawOperationType topology, MeshUsage usage);
		static void InternalCreate2(MonoObject* managedInstance, MonoObject* data, MonoArray* subMeshes, MeshUsage usage);
		static MonoArray* InternalGetSubMeshes(ScriptMesh* self);
		static uint32_t InternalGetSubMeshCount(ScriptMesh* self);
		static void InternalGetBounds(ScriptMesh* self, AABox* box, Sphere* sphere);
		static MonoObject* InternalGetMeshData(ScriptMesh* self);
		static void InternalSetMeshData(ScriptMesh* self, MonoObject* value);
	};
}
