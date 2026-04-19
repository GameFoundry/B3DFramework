//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "Resources/B3DScriptCode.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for ScriptCode. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScriptCode : public TScriptResourceWrapper<ScriptCode, ScriptScriptCode>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ScriptCode")

		ScriptScriptCode(const HScriptCode& nativeObject);

		/** Retrieves the underlying native object cast to the correct type. */
		ScriptCode* GetNativeObject() const;

		static void SetupScriptBindings();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		typedef std::pair<WString, WString> FullTypeName;

		/** Parses the provided C# code and finds a list of all classes and their namespaces. Nested classes are ignored. */
		static Vector<FullTypeName> ParseTypes(const WString& code);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* scriptObject, MonoString* text);
		static MonoString* InternalGetText(ScriptScriptCode* self);
		static void InternalSetText(ScriptScriptCode* self, MonoString* text);
		static bool InternalIsEditorScript(ScriptScriptCode* self);
		static void InternalSetEditorScript(ScriptScriptCode* self, bool value);
		static MonoArray* InternalGetTypes(ScriptScriptCode* self);
	};

	/** @} */
} // namespace b3d
