//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "Resources/B3DPlainText.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for PlainText. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlainText : public TScriptResourceWrapper<PlainText, ScriptPlainText>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PlainText")

		ScriptPlainText(const HPlainText& nativeObject);

		static void SetupScriptBindings();

		/** Retrieves the underlying native object cast to the correct type. */
		PlainText* GetNativeObject() const;

		static MonoObject* CreateScriptObject(bool construct);

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* scriptObject, MonoString* text);
		static MonoString* InternalGetText(ScriptPlainText* self);
		static void InternalSetText(ScriptPlainText* self, MonoString* text);
	};

	/** @} */
} // namespace b3d
