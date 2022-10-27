//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptObject.h"
#include "Resources/BsPlainText.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for PlainText. */
	class BS_SCR_BE_EXPORT ScriptPlainText : public TScriptResource<ScriptPlainText, PlainText>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PlainText")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* CreateInstance();

	private:
		friend class ScriptResourceManager;
		friend class BuiltinResourceTypes;

		ScriptPlainText(MonoObject* instance, const HPlainText& plainText);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalCreateInstance(MonoObject* instance, MonoString* text);
		static MonoString* InternalGetText(ScriptPlainText* thisPtr);
		static void InternalSetText(ScriptPlainText* thisPtr, MonoString* text);
	};

	/** @} */
} // namespace bs
