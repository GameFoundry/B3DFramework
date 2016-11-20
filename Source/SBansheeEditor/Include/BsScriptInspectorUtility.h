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

	/**
	 * Interop class between C++ & CLR that deals with custom inspectors. Custom inspectors	allow the developer to control
	 * exactly how certain types are displayed in the inspector window in the editor.
	 */
	class BS_SCR_BED_EXPORT ScriptInspectorUtility : public ScriptObject<ScriptInspectorUtility>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "InspectorUtility");

		/**	Hooks up domain reload callback. Must be called on library load. */
		static void startUp();

		/**	Destroys domain reload callback. Must be called before library is unloaded. */
		static void shutDown();

	private:
		ScriptInspectorUtility(MonoObject* instance);

		/**
		 * Reloads all assembly types and attempts to find uses of CustomInspector attribute. Old data cleared and replaced
		 * with new.
		 */
		static void reloadAssemblyData();

		static MonoClass* mCustomInspectorAtribute;
		static MonoField* mTypeField;

		static UnorderedMap<MonoClass*, MonoClass*> mInspectorTypes;
		static UnorderedMap<MonoClass*, MonoClass*> mInspectableFieldTypes;

		static HEvent mDomainLoadedConn;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoObject* internal_GetCustomInspector(MonoReflectionType* reflType);
		static MonoReflectionType* internal_GetCustomInspectable(MonoReflectionType* reflType);
	};

	/** @} */
}