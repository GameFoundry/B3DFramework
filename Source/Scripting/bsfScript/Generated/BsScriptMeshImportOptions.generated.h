//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"

namespace bs
{
	class MeshImportOptions;
}

namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptMeshImportOptions : public TScriptReflectable<ScriptMeshImportOptions, MeshImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MeshImportOptions")

		ScriptMeshImportOptions(MonoObject* managedInstance, const SPtr<MeshImportOptions>& value);

		static MonoObject* Create(const SPtr<MeshImportOptions>& value);

	private:
		static bool InternalGetCpuCached(ScriptMeshImportOptions* thisPtr);
		static void InternalSetCpuCached(ScriptMeshImportOptions* thisPtr, bool value);
		static bool InternalGetImportNormals(ScriptMeshImportOptions* thisPtr);
		static void InternalSetImportNormals(ScriptMeshImportOptions* thisPtr, bool value);
		static bool InternalGetImportTangents(ScriptMeshImportOptions* thisPtr);
		static void InternalSetImportTangents(ScriptMeshImportOptions* thisPtr, bool value);
		static bool InternalGetImportBlendShapes(ScriptMeshImportOptions* thisPtr);
		static void InternalSetImportBlendShapes(ScriptMeshImportOptions* thisPtr, bool value);
		static bool InternalGetImportSkin(ScriptMeshImportOptions* thisPtr);
		static void InternalSetImportSkin(ScriptMeshImportOptions* thisPtr, bool value);
		static bool InternalGetImportAnimation(ScriptMeshImportOptions* thisPtr);
		static void InternalSetImportAnimation(ScriptMeshImportOptions* thisPtr, bool value);
		static bool InternalGetReduceKeyFrames(ScriptMeshImportOptions* thisPtr);
		static void InternalSetReduceKeyFrames(ScriptMeshImportOptions* thisPtr, bool value);
		static bool InternalGetImportRootMotion(ScriptMeshImportOptions* thisPtr);
		static void InternalSetImportRootMotion(ScriptMeshImportOptions* thisPtr, bool value);
		static float InternalGetImportScale(ScriptMeshImportOptions* thisPtr);
		static void InternalSetImportScale(ScriptMeshImportOptions* thisPtr, float value);
		static CollisionMeshType InternalGetCollisionMeshType(ScriptMeshImportOptions* thisPtr);
		static void InternalSetCollisionMeshType(ScriptMeshImportOptions* thisPtr, CollisionMeshType value);
		static MonoArray* InternalGetAnimationSplits(ScriptMeshImportOptions* thisPtr);
		static void InternalSetAnimationSplits(ScriptMeshImportOptions* thisPtr, MonoArray* value);
		static MonoArray* InternalGetAnimationEvents(ScriptMeshImportOptions* thisPtr);
		static void InternalSetAnimationEvents(ScriptMeshImportOptions* thisPtr, MonoArray* value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
} // namespace bs
