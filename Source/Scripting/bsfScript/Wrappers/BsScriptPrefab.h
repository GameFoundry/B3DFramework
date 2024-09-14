//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "Scene/BsPrefab.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Prefab. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPrefab : public TScriptResourceWrapper<Prefab, ScriptPrefab>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Prefab")

		ScriptPrefab(const HPrefab& nativeObject);

		/** Retrieves the underlying native object cast to the correct type. */
		Prefab* GetNativeObject() const;

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* scriptObject, ScriptSceneObject* so, bool isScene);
		static MonoObject* InternalInstantiate(ScriptPrefab* self);
		static bool InternalIsScene(ScriptPrefab* self);
	};

	/** @} */
} // namespace bs
