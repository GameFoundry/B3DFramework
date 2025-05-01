//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector4.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfUtility/Image/BsColor.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshData.h"

namespace bs { class MeshDataEx; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRendererMeshData : public TScriptNonReflectableWrapper<RendererMeshData, ScriptRendererMeshData>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RendererMeshData")

		ScriptRendererMeshData(const SPtr<RendererMeshData>& nativeObject);
		~ScriptRendererMeshData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetData(ScriptRendererMeshData* self);
		static void InternalCreate(MonoObject* scriptObject, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType);
		static MonoArray* InternalGetPositions(ScriptRendererMeshData* self);
		static void InternalSetPositions(ScriptRendererMeshData* self, MonoArray* value);
		static MonoArray* InternalGetNormals(ScriptRendererMeshData* self);
		static void InternalSetNormals(ScriptRendererMeshData* self, MonoArray* value);
		static MonoArray* InternalGetTangents(ScriptRendererMeshData* self);
		static void InternalSetTangents(ScriptRendererMeshData* self, MonoArray* value);
		static MonoArray* InternalGetColors(ScriptRendererMeshData* self);
		static void InternalSetColors(ScriptRendererMeshData* self, MonoArray* value);
		static MonoArray* InternalGetUV0(ScriptRendererMeshData* self);
		static void InternalSetUV0(ScriptRendererMeshData* self, MonoArray* value);
		static MonoArray* InternalGetUV1(ScriptRendererMeshData* self);
		static void InternalSetUV1(ScriptRendererMeshData* self, MonoArray* value);
		static MonoArray* InternalGetBoneWeights(ScriptRendererMeshData* self);
		static void InternalSetBoneWeights(ScriptRendererMeshData* self, MonoArray* value);
		static MonoArray* InternalGetIndices(ScriptRendererMeshData* self);
		static void InternalSetIndices(ScriptRendererMeshData* self, MonoArray* value);
		static int32_t InternalGetVertexCount(ScriptRendererMeshData* self);
		static int32_t InternalGetIndexCount(ScriptRendererMeshData* self);
	};
}
