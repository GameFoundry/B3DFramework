//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "Math/BsVector4.h"
#include "Math/BsVector2.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshData.h"

namespace bs
{
	class RendererMeshData;
}

namespace bs
{
	class MeshDataEx;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptRendererMeshData : public ScriptObject<ScriptRendererMeshData>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MeshData")

		ScriptRendererMeshData(MonoObject* managedInstance, const SPtr<RendererMeshData>& value);

		SPtr<RendererMeshData> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<RendererMeshData>& value);

	private:
		SPtr<RendererMeshData> mInternal;

		static void InternalCreate(MonoObject* managedInstance, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType);
		static MonoArray* InternalGetPositions(ScriptRendererMeshData* thisPtr);
		static void InternalSetPositions(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* InternalGetNormals(ScriptRendererMeshData* thisPtr);
		static void InternalSetNormals(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* InternalGetTangents(ScriptRendererMeshData* thisPtr);
		static void InternalSetTangents(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* InternalGetColors(ScriptRendererMeshData* thisPtr);
		static void InternalSetColors(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* InternalGetUV0(ScriptRendererMeshData* thisPtr);
		static void InternalSetUV0(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* InternalGetUV1(ScriptRendererMeshData* thisPtr);
		static void InternalSetUV1(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* InternalGetBoneWeights(ScriptRendererMeshData* thisPtr);
		static void InternalSetBoneWeights(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* InternalGetIndices(ScriptRendererMeshData* thisPtr);
		static void InternalSetIndices(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static int32_t InternalGetVertexCount(ScriptRendererMeshData* thisPtr);
		static int32_t InternalGetIndexCount(ScriptRendererMeshData* thisPtr);
	};
} // namespace bs
