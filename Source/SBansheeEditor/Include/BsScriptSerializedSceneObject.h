//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsEditorUtility.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for SerializedSceneObject. */
	class BS_SCR_BED_EXPORT ScriptSerializedSceneObject : public ScriptObject <ScriptSerializedSceneObject>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "SerializedSceneObject")

	private:
		ScriptSerializedSceneObject(MonoObject* instance, const HSceneObject& so, bool recordHierarchy);
		~ScriptSerializedSceneObject();

		HSceneObject mSO;

		EditorUtility::SceneObjProxy mSceneObjectProxy;
		bool mRecordHierarchy;

		UINT8* mSerializedObject;
		UINT32 mSerializedObjectSize;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, ScriptSceneObject* so, bool hierarchy);
		static void internal_Restore(ScriptSerializedSceneObject* thisPtr);
	};

	/** @} */
}