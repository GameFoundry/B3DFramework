//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Scene/BsPrefab.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Prefab. */
	class BS_SCR_BE_EXPORT ScriptPrefab : public TScriptResource<ScriptPrefab, Prefab>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Prefab")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* CreateInstance();

	private:
		friend class ScriptResourceManager;
		friend class BuiltinResourceTypes;

		ScriptPrefab(MonoObject* instance, const HPrefab& prefab);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, ScriptSceneObject* so, bool isScene);
		static MonoObject* InternalInstantiate(ScriptPrefab* instance);
		static bool InternalIsScene(ScriptPrefab* instance);
	};

	/** @} */
} // namespace bs
