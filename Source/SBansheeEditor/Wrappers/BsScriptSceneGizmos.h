//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class ScriptCCamera;

	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/** Interop class between C++ & CLR for SceneGizmos. */
	class BS_SCR_BED_EXPORT ScriptSceneGizmos : public ScriptObject <ScriptSceneGizmos>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "SceneGizmos")

	private:
		ScriptSceneGizmos(MonoObject* object, const SPtr<Camera>& camera);
		~ScriptSceneGizmos();

		SPtr<Camera> mCamera;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Create(MonoObject* managedInstance, ScriptCCamera* camera);
		static void internal_Draw(ScriptSceneGizmos* thisPtr);
	};

	/** @} */
}