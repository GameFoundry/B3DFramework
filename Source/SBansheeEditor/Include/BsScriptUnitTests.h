//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for various managed unit tests. */
	class BS_SCR_BED_EXPORT ScriptUnitTests : public ScriptObject <ScriptUnitTests>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "UnitTests")

		/**	Starts execution of the managed tests. */
		static void runTests();

	private:
		static MonoMethod* RunTestsMethod;

		static SPtr<ManagedSerializableDiff> tempDiff;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_UT1_GameObjectClone(MonoObject* instance);
		static void internal_UT3_GenerateDiff(MonoObject* oldObj, MonoObject* newObj);
		static void internal_UT3_ApplyDiff(MonoObject* obj);
	};

	/** @} */
}