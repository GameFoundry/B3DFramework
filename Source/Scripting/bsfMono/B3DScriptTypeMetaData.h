//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMonoPrerequisites.h"

namespace b3d
{
	class IScriptExportable;

	/** @addtogroup Mono
	 *  @{
	 */

	/** Identifies a managed type. */
	struct B3D_MONO_EXPORT MonoTypeIdentifier
	{
		MonoTypeIdentifier() = default;
		MonoTypeIdentifier(const String& assembly, const String& nameSpace, const String& typeName)
			:Assembly(assembly), Namespace(nameSpace), TypeName(typeName)
		{ }

		/** Constructs the full type name from the identifier. */
		String GetTypeName(bool includeNamespace) const;

		bool operator==(const MonoTypeIdentifier& other) const
		{
			return Assembly == other.Assembly && Namespace == other.Namespace && TypeName == other.TypeName && GenericTypeParameters == other.GenericTypeParameters;
		}

		bool operator!=(const MonoTypeIdentifier& other) const
		{
			return !(*this == other);
		}

		/**
		 * Parses a type containing generic arguments. Recursively parses generic arguments of any parent generic arguments.
		 * Output type identifier will only have the type name and generic arguments fields populated, and optionally namespace if explicitly provided.
		 */
		static MonoTypeIdentifier Parse(const String& typeName);

		String Assembly; /**< Name of the assembly the type is the located in, without the .dll extension. */
		String Namespace; /**< Namespace of the type. */
		String TypeName; /**< Name of the type. If type is a generic use `N syntax, where N is the number of generic parameters. e.g. List`1. */

		TArray<MonoTypeIdentifier> GenericTypeParameters; /**< If type is a generic type, these should be the generic parameters to used to specialize the type. */
	};

	/** Type of create callback specified in ScriptTypeMetaData. */
	enum class ScriptWrapperCreateCallbackType
	{
		None,
		Reflectable,
		Resource,
		GameObject,
		GUIElement,
	};


	/**	Contains information about a class exported to script. */
	struct B3D_MONO_EXPORT ScriptTypeMetaData
	{
		ScriptTypeMetaData() = default;
		ScriptTypeMetaData(const char* assembly, const char* nameSpace, const char* typeName, std::function<void()> setupScriptBindingsCallback);

		MonoTypeIdentifier Identifier;

		u32 TypeId = ~0u; /**< RTTI ID of the native object. */
		ScriptWrapperCreateCallbackType CreateCallbackType = ScriptWrapperCreateCallbackType::None; /**< Specifies which of the callbacks in the *CreateCallback union is valid. */

		/** Callbacks use to create the correct script object and script wrapper object from a native object. */
		union
		{
			MonoObject* (*ReflectableCreateCallback)(const SPtr<IReflectable>&) = nullptr;
			MonoObject* (*ResourceCreateCallback)(const HResource&);
			MonoObject* (*GameObjectCreateCallback)(const HGameObject&);
			MonoObject* (*GUIElementCreateCallback)(GUIElement*);
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
} // namespace b3d

/** @cond STDLIB */

namespace std
{
	/** Hash value generator for ScriptTypeIdentifier. */
	template <>
	struct hash<b3d::MonoTypeIdentifier>
	{
		size_t operator()(const b3d::MonoTypeIdentifier& key) const
		{
			size_t hash = 0;
			b3d::B3DCombineHash(hash, key.Assembly);
			b3d::B3DCombineHash(hash, key.Namespace);
			b3d::B3DCombineHash(hash, key.TypeName);

			for(const auto& entry : key.GenericTypeParameters)
				b3d::B3DCombineHash(hash, b3d::B3DHash(entry));

			return hash;
		}
	};
} // namespace std

/** @endcond */
