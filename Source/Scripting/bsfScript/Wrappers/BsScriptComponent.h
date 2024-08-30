//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectWrapper.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Component. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptComponent : public TScriptGameObjectWrapper<Component, ScriptComponent>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "Component")

		ScriptComponent(const HComponent& nativeObject, MonoObject* scriptObject);

		/** Dummy method to create the script object. Not used as components are always just base classes, not created directly. */
		static MonoObject* CreateScriptObject(bool construct)
		{
			return nullptr;
		}

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* InternalAddComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoObject* InternalGetComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoArray* InternalGetComponents(MonoObject* parentSceneObject);
		static MonoArray* InternalGetComponentsPerType(MonoObject* parentSceneObject, MonoReflectionType* type);
		static void InternalRemoveComponent(MonoObject* parentSceneObject, MonoReflectionType* type);
		static MonoObject* InternalGetSceneObject(ScriptGameObjectWrapper* self);
		static TransformChangedFlags InternalGetNotifyFlags(ScriptGameObjectWrapper* self);
		static void InternalSetNotifyFlags(ScriptGameObjectWrapper* self, TransformChangedFlags flags);
		static void InternalDestroy(ScriptGameObjectWrapper* self, bool immediate);
	};

	/** @} */
} // namespace bs
