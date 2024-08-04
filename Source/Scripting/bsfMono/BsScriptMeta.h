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
		ScriptMeta(const String& assembly, const String& ns, const String& name, std::function<void()> initializeBindingsCallback);

		// TODO - These should be const char
		String Ns; /**< Namespace the script class is located in. */
		String Name; /**< Type name of the script class. */
		String Assembly; /**< Name of the assembly the script class is located in. */

		/**
		 * Callback that will be triggered when assembly containing the class is loaded or refreshed. Used for one time
		 * initialization of type bindings.
		 */
		std::function<void()> InitializeBindingsCallback;

		/** Class object describing the script class. Only valid after assembly containing this type was loaded.  */
		MonoClass* ScriptClass = nullptr;

		/**
		 * Field object that contains a pointer to the native instance of the script object. Only valid after assembly
		 * containing this type was loaded.
		 */
		MonoField* ScriptObjectWrapperPointerField = nullptr;

		/** Field that signifies whether ScriptObjectWrapperPointerField holds new ScriptObjectWrapperType or old ScriptObjectBase type. */
		MonoField* IsUsingNewScriptObjectManagerField = nullptr; // TODO - Temporary while we transition
	};

	/** @} */
} // namespace bs
