//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "Math/BsVector4.h"
#include "Image/BsColor.h"
#include "Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshData.h"

namespace bs { class RendererMeshData; }
namespace bs { class MeshDataEx; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMeshData : public ScriptObject<ScriptMeshData>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MeshData")

		ScriptMeshData(MonoObject* managedInstance, const SPtr<RendererMeshData>& value);

		SPtr<RendererMeshData> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<RendererMeshData>& value);

	private:
		SPtr<RendererMeshData> mInternal;

		static void InternalCreate(MonoObject* managedInstance, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType);
		static MonoArray* InternalGetPositions(ScriptMeshData* self);
		static void InternalSetPositions(ScriptMeshData* self, MonoArray* value);
		static MonoArray* InternalGetNormals(ScriptMeshData* self);
		static void InternalSetNormals(ScriptMeshData* self, MonoArray* value);
		static MonoArray* InternalGetTangents(ScriptMeshData* self);
		static void InternalSetTangents(ScriptMeshData* self, MonoArray* value);
		static MonoArray* InternalGetColors(ScriptMeshData* self);
		static void InternalSetColors(ScriptMeshData* self, MonoArray* value);
		static MonoArray* InternalGetUV0(ScriptMeshData* self);
		static void InternalSetUV0(ScriptMeshData* self, MonoArray* value);
		static MonoArray* InternalGetUV1(ScriptMeshData* self);
		static void InternalSetUV1(ScriptMeshData* self, MonoArray* value);
		static MonoArray* InternalGetBoneWeights(ScriptMeshData* self);
		static void InternalSetBoneWeights(ScriptMeshData* self, MonoArray* value);
		static MonoArray* InternalGetIndices(ScriptMeshData* self);
		static void InternalSetIndices(ScriptMeshData* self, MonoArray* value);
		static int32_t InternalGetVertexCount(ScriptMeshData* self);
		static int32_t InternalGetIndexCount(ScriptMeshData* self);
	};
}
