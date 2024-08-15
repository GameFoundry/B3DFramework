//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsMonoPrerequisites.h"

namespace bs
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**	Contains information required for initializing and handling a single script class. */
	struct B3D_MONO_EXPORT ScriptMeta
	{
		ScriptMeta();
		ScriptMeta(const String& assembly, const String& nameSpace, const String& name, std::function<void()> setupScriptBindingsCallback);

		// TODO - These should be const char
		String Namespace; /**< Namespace the script class is located in. */
		String Name; /**< Type name of the script class. */
		String Assembly; /**< Name of the assembly the script class is located in. */

		/** Callback that will be triggered when assembly containing the class is loaded or refreshed. Used for initialization of script bindings for the type. */
		std::function<void()> SetupScriptBindingsCallback;

		/** Class object describing the script class. Only valid after assembly containing this type was loaded.  */
		MonoClass* ScriptClass = nullptr;

		/** Field object that contains a native pointer to the script object wrapper. Only valid after assembly containing this type was loaded. */
		MonoField* ScriptObjectWrapperPointerField = nullptr;

		/** Field that signifies whether ScriptObjectWrapperPointerField holds new ScriptObjectWrapperType or old ScriptObjectBase type. */
		MonoField* IsUsingNewScriptObjectManagerField = nullptr; // TODO - Temporary while we transition
	};

	/** @} */
} // namespace bs
