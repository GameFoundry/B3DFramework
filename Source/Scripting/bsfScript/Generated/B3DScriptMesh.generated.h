//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "../../../Foundation/Core/Mesh/B3DMeshBase.h"
#include "../../../Foundation/Core/Utility/B3DCommonTypes.h"
#include "../../../Foundation/Core/Renderer/B3DRendererMeshData.h"
#include "../../../Foundation/Core/Utility/B3DCommonTypes.h"
#include "../../../Foundation/Core/RenderAPI/B3DSubMesh.h"
#include "../../../Foundation/Utility/Math/B3DAABox.h"
#include "../../../Foundation/Utility/Math/B3DSphere.h"

namespace b3d { class Mesh; }
namespace b3d { class MeshEx; }
namespace b3d { struct __TAABox_float_Interop; }
namespace b3d { struct __TSphere_float_Interop; }
namespace b3d
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
		static void InternalCreate(MonoObject* scriptObject, int32_t numVertices, int32_t numIndices, DrawOperationType topology, MeshFlag flags, VertexLayout vertex, IndexType index);
		static void InternalCreate0(MonoObject* scriptObject, int32_t numVertices, int32_t numIndices, MonoArray* subMeshes, MeshFlag flags, VertexLayout vertex, IndexType index);
		static void InternalCreate1(MonoObject* scriptObject, MonoObject* data, DrawOperationType topology, MeshFlag flags);
		static void InternalCreate2(MonoObject* scriptObject, MonoObject* data, MonoArray* subMeshes, MeshFlag flags);
		static MonoArray* InternalGetSubMeshes(ScriptMesh* self);
		static uint32_t InternalGetSubMeshCount(ScriptMesh* self);
		static void InternalGetBounds(ScriptMesh* self, __TAABox_float_Interop* box, __TSphere_float_Interop* sphere);
		static MonoObject* InternalGetMeshData(ScriptMesh* self);
		static void InternalSetMeshData(ScriptMesh* self, MonoObject* value);
	};
}
