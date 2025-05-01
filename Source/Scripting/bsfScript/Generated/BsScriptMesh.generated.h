//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshBase.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsSubMesh.h"
#include "../../../Foundation/bsfUtility/Math/BsAABox.h"
#include "../../../Foundation/bsfUtility/Math/BsSphere.h"

namespace bs { class Mesh; }
namespace bs { class MeshEx; }
namespace bs { struct __TAABox_float_Interop; }
namespace bs { struct __TSphere_float_Interop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMesh : public TScriptResourceWrapper<Mesh, ScriptMesh>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Mesh")

		ScriptMesh(const TResourceHandle<Mesh>& nativeObject);
		~ScriptMesh();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptMesh* self);

		static MonoObject* InternalGetSkeleton(ScriptMesh* self);
		static MonoObject* InternalGetMorphShapes(ScriptMesh* self);
		static void InternalCreate(MonoObject* scriptObject, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshUsage usage, VertexLayout vertex, IndexType index);
		static void InternalCreate0(MonoObject* scriptObject, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, IndexType index);
		static void InternalCreate1(MonoObject* scriptObject, MonoObject* data, DrawOperationType topology, MeshUsage usage);
		static void InternalCreate2(MonoObject* scriptObject, MonoObject* data, MonoArray* subMeshes, MeshUsage usage);
		static MonoArray* InternalGetSubMeshes(ScriptMesh* self);
		static uint32_t InternalGetSubMeshCount(ScriptMesh* self);
		static void InternalGetBounds(ScriptMesh* self, __TAABox_float_Interop* box, __TSphere_float_Interop* sphere);
		static MonoObject* InternalGetMeshData(ScriptMesh* self);
		static void InternalSetMeshData(ScriptMesh* self, MonoObject* value);
	};
}
