//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptObject.h"
#include "Resources/BsScriptCode.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ScriptCode. */
	class BS_SCR_BE_EXPORT ScriptScriptCode : public TScriptResource<ScriptScriptCode, ScriptCode>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ScriptCode")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* CreateInstance();

	private:
		friend class ScriptResourceManager;
		friend class BuiltinResourceTypes;
		typedef std::pair<WString, WString> FullTypeName;

		ScriptScriptCode(MonoObject* instance, const HScriptCode& scriptCode);

		/** Parses the provided C# code and finds a list of all classes and their namespaces. Nested classes are ignored. */
		static Vector<FullTypeName> ParseTypes(const WString& code);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* text);
		static MonoString* InternalGetText(ScriptScriptCode* thisPtr);
		static void InternalSetText(ScriptScriptCode* thisPtr, MonoString* text);
		static bool InternalIsEditorScript(ScriptScriptCode* thisPtr);
		static void InternalSetEditorScript(ScriptScriptCode* thisPtr, bool value);
		static MonoArray* InternalGetTypes(ScriptScriptCode* thisPtr);
	};

	/** @} */
} // namespace bs
