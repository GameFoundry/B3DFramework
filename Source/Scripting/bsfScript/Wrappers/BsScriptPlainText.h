//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptResourceWrapper.h"
#include "Resources/BsPlainText.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for PlainText. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlainText : public TScriptResourceWrapper<PlainText, ScriptPlainText>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "PlainText")

		/** Retrieves the underlying native object cast to the correct type. */
		PlainText* GetNativeObject() const;

		static MonoObject* CreateScriptObject(bool construct);

	private:
		ScriptPlainText(const HPlainText& nativeObject, MonoObject* scriptObject);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* scriptObject, MonoString* text);
		static MonoString* InternalGetText(ScriptPlainText* self);
		static void InternalSetText(ScriptPlainText* self, MonoString* text);
	};

	/** @} */
} // namespace bs
