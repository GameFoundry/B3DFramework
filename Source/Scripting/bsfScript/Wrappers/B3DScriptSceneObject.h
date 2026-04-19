//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGameObjectWrapper.h"
#include "Scene/B3DSceneObject.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for SceneObject. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneObject : public TScriptGameObjectWrapper<SceneObject, ScriptSceneObject>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SceneObject")

		ScriptSceneObject(const HSceneObject& nativeObject);

		/** Retrieves the underlying native object cast to the correct type. */
		SceneObject* GetNativeObject() const;

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* scriptObject, MonoString* name, u32 flags);

		static void InternalSetName(ScriptSceneObject* self, MonoString* name);
		static MonoString* InternalGetName(ScriptSceneObject* self);
		static void InternalSetActive(ScriptSceneObject* self, bool value);
		static bool InternalGetActive(ScriptSceneObject* self);
		static bool InternalHasFlag(ScriptSceneObject* self, u32 flag);

		static void InternalSetMobility(ScriptSceneObject* self, int value);
		static int InternalGetMobility(ScriptSceneObject* self);

		static void InternalSetParent(ScriptSceneObject* self, MonoObject* parent);
		static MonoObject* InternalGetParent(ScriptSceneObject* self);
		static MonoObject* InternalGetScene(ScriptSceneObject* self);

		static void InternalBreakPrefabLink(ScriptSceneObject* self);
		static bool InternalIsPrefabInstance(ScriptSceneObject* self);
		static MonoObject* InternalGetPrefabInstanceRoot(ScriptSceneObject* self);
		static void InternalGetPrefabResourceId(ScriptSceneObject* self, UUID* uuid);

		static void InternalGetNumChildren(ScriptSceneObject* self, u32* value);
		static MonoObject* InternalGetChild(ScriptSceneObject* self, u32 childIndex);
		static MonoObject* InternalFindChild(ScriptSceneObject* self, MonoString* name, bool recursive);
		static MonoArray* InternalFindChildren(ScriptSceneObject* self, MonoString* name, bool recursive);

		static void InternalGetPosition(ScriptSceneObject* self, Vector3* value);
		static void InternalGetLocalPosition(ScriptSceneObject* self, Vector3* value);
		static void InternalGetRotation(ScriptSceneObject* self, Quaternion* value);
		static void InternalGetLocalRotation(ScriptSceneObject* self, Quaternion* value);
		static void InternalGetScale(ScriptSceneObject* self, Vector3* value);
		static void InternalGetLocalScale(ScriptSceneObject* self, Vector3* value);

		static void InternalSetPosition(ScriptSceneObject* self, Vector3* value);
		static void InternalSetLocalPosition(ScriptSceneObject* self, Vector3* value);
		static void InternalSetRotation(ScriptSceneObject* self, Quaternion* value);
		static void InternalSetLocalRotation(ScriptSceneObject* self, Quaternion* value);
		static void InternalSetLocalScale(ScriptSceneObject* self, Vector3* value);

		static void InternalGetLocalTransform(ScriptSceneObject* self, Matrix4* value);
		static void InternalGetWorldTransform(ScriptSceneObject* self, Matrix4* value);
		static void InternalLookAt(ScriptSceneObject* self, Vector3* direction, Vector3* up);
		static void InternalMove(ScriptSceneObject* self, Vector3* value);
		static void InternalMoveLocal(ScriptSceneObject* self, Vector3* value);
		static void InternalRotate(ScriptSceneObject* self, Quaternion* value);
		static void InternalRoll(ScriptSceneObject* self, Radian* value);
		static void InternalYaw(ScriptSceneObject* self, Radian* value);
		static void InternalPitch(ScriptSceneObject* self, Radian* value);
		static void InternalSetForward(ScriptSceneObject* self, Vector3* value);
		static void InternalGetForward(ScriptSceneObject* self, Vector3* value);
		static void InternalGetUp(ScriptSceneObject* self, Vector3* value);
		static void InternalGetRight(ScriptSceneObject* self, Vector3* value);

		static void InternalDestroy(ScriptSceneObject* self, bool immediate);
	};

	/** @} */
} // namespace b3d
