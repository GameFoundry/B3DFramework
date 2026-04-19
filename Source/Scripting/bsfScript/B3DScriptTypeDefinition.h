//*********************************** B3D Framework - Copyright 2026 Marko Pintera ***************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptTypeMetaData.h"
#include "B3DMonoManager.h"

namespace b3d
{
	/** @addtogroup Script-Internal
	 *  @{
	 */

	/** Checks if the type has a `static void InitializeAdditionalMetaData(ScriptTypeMetaData&&)` method. */
	template <typename T, typename = void>
	struct B3DHasInitializeAdditionalMetaDataMethod : std::false_type
	{};

	template <typename T>
	struct B3DHasInitializeAdditionalMetaDataMethod<T, std::enable_if_t<std::is_same_v<decltype(T::InitializeAdditionalMetaData(std::declval<ScriptTypeMetaData>())), void>>>
		: std::true_type
	{};

	/** Checks if the type has a `static void SetupScriptBindings()` method. */
	template <typename T, typename = void>
	struct B3DHasSetupScriptBindingsMethod : std::false_type
	{};

	template <typename T>
	struct B3DHasSetupScriptBindingsMethod<T, std::void_t<decltype(T::SetupScriptBindings())>>
		: std::true_type
	{};

	template <typename SelfType>
	class TScriptTypeDefinition;

	/** Ensures that TScriptTypeDefinition is initialized on application load. */
	template <typename SelfType>
	struct InitializeScriptTypeDefinitionOnLoadTime
	{
		InitializeScriptTypeDefinitionOnLoadTime()
		{
			TScriptTypeDefinition<SelfType>::InitializeMetaDataAtLoadTime();
		}

		void MakeSureIAmInstantiated() {}
	};

	/** @} */

	/** @addtogroup Script
	 *  @{
	 */

	/**
	 * Templated base class that provides information about a script type that maps to a native type. For each script type
	 * you should inherit this class and specify the B3D_SCRIPT_TYPE_DEFINITON macro that lets you specify the information
	 * about the script type.
	 *
	 * Optionally you may also specify a `static void SetupScriptBindings()` method, that you may use to bind to script methods,
	 * so they trigger in native code when script code is called. This method will trigger when script assemblies are loaded or
	 * reloaded.
	 *
	 * You may also optionally specify a `static void InitializeAdditionalMetaData(ScriptTypeMetaData&)` method that allows your class
	 * append additional information to the meta-data structure, if needed. Generally this includes the RTTI ID, or specialized
	 * creation callbacks required for creating native objects of the type. This method will trigger once when the application
	 * starts up, or the library containing the class is loaded.
	 *
	 * @tparam SelfType		Type that derives from TScriptTypeDefinition<SelfType> and specified the B3D_SCRIPT_TYPE_DEFINITON() macro, and
	 *						optionally the two static methods mentioned above.
	 */
	template <typename SelfType>
	class TScriptTypeDefinition
	{
	public:
		TScriptTypeDefinition()
		{
			sInitializeOnLoadTime.MakeSureIAmInstantiated();
		}

		/** Returns the meta-data storing information about the script type. */
		static const ScriptTypeMetaData* GetMetaData() { return &sInteropMetaData; }

		/**
		 * Takes care of initializing the meta-data when the application first load. The meta-data will be registered with a global manager that will ensure
		 * it is kept up-to-date after operations such as assembly (re)load.
		 */
		static void InitializeMetaDataAtLoadTime()
		{
			// Need to delay init of sInteropMetaData since it's also a static, and we can't guarantee the order
			// (if it gets initialized after this, it will just overwrite the data)
			ScriptTypeMetaData localMetaData = ScriptTypeMetaData(SelfType::GetAssemblyName(), SelfType::GetNamespace(), SelfType::GetTypeName(), nullptr);

			if constexpr(B3DHasSetupScriptBindingsMethod<SelfType>::value)
				localMetaData.SetupScriptBindingsCallback = &SelfType::SetupScriptBindings;

			if constexpr(B3DHasInitializeAdditionalMetaDataMethod<SelfType>::value)
				SelfType::InitializeAdditionalMetaData(localMetaData);

			MonoManager::RegisterScriptType(&sInteropMetaData, localMetaData);
		}

	protected:
		static ScriptTypeMetaData sInteropMetaData;
		static InitializeScriptTypeDefinitionOnLoadTime<SelfType> sInitializeOnLoadTime;
	};

	template <typename SelfType>
	InitializeScriptTypeDefinitionOnLoadTime<SelfType> TScriptTypeDefinition<SelfType>::sInitializeOnLoadTime;

	template <typename SelfType>
	ScriptTypeMetaData TScriptTypeDefinition<SelfType>::sInteropMetaData;

	/** Implements default methods required by script object wrapper implementations. */
#define B3D_SCRIPT_TYPE_DEFINITION(Assembly, Namespace, Name) \
	static const char* GetAssemblyName()      \
	{                                         \
		return Assembly;                      \
	}                                         \
	static const char* GetNamespace()         \
	{                                         \
		return Namespace;                     \
	}                                         \
	static const char* GetTypeName()          \
	{                                         \
		return Name;                          \
	}                                         \

	/** @} */
} // namespace b3d
