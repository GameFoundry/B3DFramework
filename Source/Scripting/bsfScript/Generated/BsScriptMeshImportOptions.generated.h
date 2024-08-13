//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"

namespace bs { class MeshImportOptions; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMeshImportOptions : public TScriptReflectable<ScriptMeshImportOptions, MeshImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MeshImportOptions")

		ScriptMeshImportOptions(MonoObject* managedInstance, const SPtr<MeshImportOptions>& value);

		static MonoObject* Create(const SPtr<MeshImportOptions>& value);

	private:
		static bool InternalGetCpuCached(ScriptMeshImportOptions* self);
		static void InternalSetCpuCached(ScriptMeshImportOptions* self, bool value);
		static bool InternalGetImportNormals(ScriptMeshImportOptions* self);
		static void InternalSetImportNormals(ScriptMeshImportOptions* self, bool value);
		static bool InternalGetImportTangents(ScriptMeshImportOptions* self);
		static void InternalSetImportTangents(ScriptMeshImportOptions* self, bool value);
		static bool InternalGetImportBlendShapes(ScriptMeshImportOptions* self);
		static void InternalSetImportBlendShapes(ScriptMeshImportOptions* self, bool value);
		static bool InternalGetImportSkin(ScriptMeshImportOptions* self);
		static void InternalSetImportSkin(ScriptMeshImportOptions* self, bool value);
		static bool InternalGetImportAnimation(ScriptMeshImportOptions* self);
		static void InternalSetImportAnimation(ScriptMeshImportOptions* self, bool value);
		static bool InternalGetReduceKeyFrames(ScriptMeshImportOptions* self);
		static void InternalSetReduceKeyFrames(ScriptMeshImportOptions* self, bool value);
		static bool InternalGetImportRootMotion(ScriptMeshImportOptions* self);
		static void InternalSetImportRootMotion(ScriptMeshImportOptions* self, bool value);
		static float InternalGetImportScale(ScriptMeshImportOptions* self);
		static void InternalSetImportScale(ScriptMeshImportOptions* self, float value);
		static CollisionMeshType InternalGetCollisionMeshType(ScriptMeshImportOptions* self);
		static void InternalSetCollisionMeshType(ScriptMeshImportOptions* self, CollisionMeshType value);
		static MonoArray* InternalGetAnimationSplits(ScriptMeshImportOptions* self);
		static void InternalSetAnimationSplits(ScriptMeshImportOptions* self, MonoArray* value);
		static MonoArray* InternalGetAnimationEvents(ScriptMeshImportOptions* self);
		static void InternalSetAnimationEvents(ScriptMeshImportOptions* self, MonoArray* value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
}
