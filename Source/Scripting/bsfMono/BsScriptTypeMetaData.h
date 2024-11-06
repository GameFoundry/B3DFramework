//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsMonoPrerequisites.h"

namespace bs
{
	class IScriptExportable;

	/** @addtogroup Mono
	 *  @{
	 */

	/** Type of create callback specified in ScriptTypeMetaData. */
	enum class ScriptWrapperCreateCallbackType
	{
		None,
		Reflectable,
		Resource,
		GameObject,
	};

	/**	Contains information about a class exported to script. */
	struct B3D_MONO_EXPORT ScriptTypeMetaData
	{
		ScriptTypeMetaData() = default;
		ScriptTypeMetaData(const String& assembly, const String& nameSpace, const String& name, std::function<void()> setupScriptBindingsCallback);

		// TODO - These should be const char
		String Namespace; /**< Namespace the script class is located in. */
		String Name; /**< Type name of the script class. */
		String Assembly; /**< Name of the assembly the script class is located in. */

		u32 TypeId = ~0u; /**< RTTI ID of the native object. */
		ScriptWrapperCreateCallbackType CreateCallbackType = ScriptWrapperCreateCallbackType::None; /**< Specifies which of the callbacks in the *CreateCallback union is valid. */

		/** Callbacks use to create the correct script object and script wrapper object from a native object. */
		union
		{
			MonoObject* (*ReflectableCreateCallback)(const SPtr<IReflectable>&) = nullptr;
			MonoObject* (*ResourceCreateCallback)(const HResource&);
			MonoObject* (*GameObjectCreateCallback)(const HGameObject&);
		};

		IScriptExportable* (*GetScriptExportable)(IReflectable*) = nullptr; /** Casts the IReflectable type to IScriptExportable. */

		/** Callback that will be triggered when assembly containing the class is loaded or refreshed. Used for initialization of script bindings for the type. */
		std::function<void()> SetupScriptBindingsCallback;

		/** Class object describing the script class. Only valid after assembly containing this type was loaded.  */
		MonoClass* ScriptClass = nullptr;

		/** Field object that contains a native pointer to the script object wrapper. Only valid after assembly containing this type was loaded. */
		MonoField* ScriptObjectWrapperPointerField = nullptr;
	};

	/** @} */
} // namespace bs
