//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsMonoPrerequisites.h"

namespace bs
{
	/** @addtogroup Mono
	 *  @{
	 */

	/**	Contains information required for initializing and handling a single script class. */
	struct BS_MONO_EXPORT ScriptMeta
	{
		ScriptMeta();
		ScriptMeta(const String& assembly, const String& ns, const String& name, std::function<void()> initCallback);

		String ns; /**< Namespace the script class is located in. */
		String name; /**< Type name of the script class. */
		String assembly; /**< Name of the assembly the script class is located in. */
		/**
		 * Callback that will be triggered when assembly containing the class is loaded or refreshed. Used for one time 
		 * initialization.
		 */
		std::function<void()> initCallback;
		/** Class object describing the script class. Only valid after assembly containing this type was loaded.  */
		MonoClass* scriptClass;	
		/**
		 * Field object that contains a pointer to the native instance of the script object. Only valid after assembly
		 * containing this type was loaded.
		 */
		MonoField* thisPtrField;
	};

	/** @} */
}