//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsRendererMeshData.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Valid index types for mesh indices. */
	// Note: Do not modify, it must match the layout of C# enum IndexType
	enum class ScriptIndexType
	{
		Index16,
		Index32
	};

	/**	Interop class between C++ & CLR for RendererMeshData. */
	class BS_SCR_BE_EXPORT ScriptMeshData : public ScriptObject <ScriptMeshData>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "MeshData")

		/**	Returns the internal wrapped mesh data. */
		SPtr<RendererMeshData> getInternalValue() const { return mMeshData; }

		/**	Creates a new managed MeshData object from the provided native render mesh data. */
		static MonoObject* create(const SPtr<RendererMeshData>& meshData);

		/**	Creates a new managed MeshData object from the provided native mesh data. */
		static MonoObject* create(const SPtr<MeshData>& meshData);
	private:
		ScriptMeshData(MonoObject* managedInstance);
		~ScriptMeshData();

		/**	Initializes the object. Must be called after construction and before use. */
		void initialize(const SPtr<RendererMeshData>& meshData);

		/**	Checks is the underlying mesh data of the provided object locked. When locked mesh data cannot be accessed. */
		static bool checkIsLocked(ScriptMeshData* thisPtr);

		SPtr<RendererMeshData> mMeshData;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, int numVertices,
			int numIndices, VertexLayout vertex, ScriptIndexType index);
		static MonoArray* internal_GetPositions(ScriptMeshData* thisPtr);
		static void internal_SetPositions(ScriptMeshData* thisPtr, MonoArray* value);
		static MonoArray* internal_GetNormals(ScriptMeshData* thisPtr);
		static void internal_SetNormals(ScriptMeshData* thisPtr, MonoArray* value);
		static MonoArray* internal_GetTangents(ScriptMeshData* thisPtr);
		static void internal_SetTangents(ScriptMeshData* thisPtr, MonoArray* value);
		static MonoArray* internal_GetColors(ScriptMeshData* thisPtr);
		static void internal_SetColors(ScriptMeshData* thisPtr, MonoArray* value);
		static MonoArray* internal_GetUV0(ScriptMeshData* thisPtr);
		static void internal_SetUV0(ScriptMeshData* thisPtr, MonoArray* value);
		static MonoArray* internal_GetUV1(ScriptMeshData* thisPtr);
		static void internal_SetUV1(ScriptMeshData* thisPtr, MonoArray* value);
		static MonoArray* internal_GetBoneWeights(ScriptMeshData* thisPtr);
		static void internal_SetBoneWeights(ScriptMeshData* thisPtr, MonoArray* value);
		static MonoArray* internal_GetIndices(ScriptMeshData* thisPtr);
		static void internal_SetIndices(ScriptMeshData* thisPtr, MonoArray* value);
		static int internal_GetVertexCount(ScriptMeshData* thisPtr);
		static int internal_GetIndexCount(ScriptMeshData* thisPtr);
	};

	/** @} */
}