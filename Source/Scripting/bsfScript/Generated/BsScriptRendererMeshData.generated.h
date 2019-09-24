//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Renderer/BsRendererMeshData.h"
#include "../../../Foundation/bsfCore/Mesh/BsMeshData.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "Math/BsVector4.h"
#include "Math/BsVector2.h"
#include "Image/BsColor.h"

namespace bs { class RendererMeshData; }
namespace bs { class MeshDataEx; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptRendererMeshData : public ScriptObject<ScriptRendererMeshData>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MeshData")

		ScriptRendererMeshData(MonoObject* managedInstance, const SPtr<RendererMeshData>& value);

		SPtr<RendererMeshData> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<RendererMeshData>& value);

	private:
		SPtr<RendererMeshData> mInternal;

		static void Internal_create(MonoObject* managedInstance, uint32_t numVertices, uint32_t numIndices, VertexLayout layout, IndexType indexType);
		static MonoArray* Internal_getPositions(ScriptRendererMeshData* thisPtr);
		static void Internal_setPositions(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* Internal_getNormals(ScriptRendererMeshData* thisPtr);
		static void Internal_setNormals(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* Internal_getTangents(ScriptRendererMeshData* thisPtr);
		static void Internal_setTangents(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* Internal_getColors(ScriptRendererMeshData* thisPtr);
		static void Internal_setColors(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* Internal_getUV0(ScriptRendererMeshData* thisPtr);
		static void Internal_setUV0(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* Internal_getUV1(ScriptRendererMeshData* thisPtr);
		static void Internal_setUV1(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* Internal_getBoneWeights(ScriptRendererMeshData* thisPtr);
		static void Internal_setBoneWeights(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static MonoArray* Internal_getIndices(ScriptRendererMeshData* thisPtr);
		static void Internal_setIndices(ScriptRendererMeshData* thisPtr, MonoArray* value);
		static int32_t Internal_getVertexCount(ScriptRendererMeshData* thisPtr);
		static int32_t Internal_getIndexCount(ScriptRendererMeshData* thisPtr);
	};
}
