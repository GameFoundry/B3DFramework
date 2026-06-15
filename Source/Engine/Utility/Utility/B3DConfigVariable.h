//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup System
	 *  @{
	 */

	/** Indicates the source from which a config variable's value was set. */
	enum class ConfigVariableSource : u8
	{
		Default = 0,      /**< Compile-time default value. */
		ConfigFile = 1,   /**< Loaded from a configuration file. */
		CommandLine = 2,  /**< Set via command-line parameter. */
		Runtime = 3       /**< Modified at runtime. */
	};

	/** Flags that control the behavior of a config variable. */
	enum class ConfigVariableFlag : u8
	{
		None = 0x00,

		/**
		 * Value updates are deferred to frame boundaries to ensure value of the variable
		 * doesn't change in the middle of frame execution.
		 */
		RenderThreadSafe = 0x01,

		/** Value cannot be changed at runtime after initialization. */
		ReadOnly = 0x02
	};

	using ConfigVariableFlags = Flags<ConfigVariableFlag, u8>;
	B3D_FLAGS_OPERATORS_EXT(ConfigVariableFlag, u8)

	class ConfigVariableManager;

	/**
	 * Stores a pending configuration value for late-registered variables.
	 * Used when a value is loaded from INI/command line before the variable is registered.
	 */
	struct PendingConfigValue
	{
		String value;
		ConfigVariableSource source;
	};

	/**
	 * Base class for all configuration variables. Provides common interface for registration,
	 * serialization, and metadata access.
	 */
	class B3D_EXPORT ConfigVariable
	{
	public:
		ConfigVariable(const char* name, const char* description, ConfigVariableFlags flags);
		virtual ~ConfigVariable();

		/** Returns the name of this configuration variable. */
		const char* GetName() const { return mName; }

		/** Returns the description of this configuration variable. */
		const char* GetDescription() const { return mDescription; }

		/** Returns the source from which the current value was set. */
		ConfigVariableSource GetSource() const { return mSource.load(std::memory_order_relaxed); }

		/** Returns the flags controlling this variable's behavior. */
		ConfigVariableFlags GetFlags() const { return mFlags; }

		/** Returns true if this variable cannot be modified at runtime. */
		bool IsReadOnly() const { return mFlags.IsSet(ConfigVariableFlag::ReadOnly); }

		/** Returns true if this variable defers updates to frame boundaries. */
		bool IsRenderThreadSafe() const { return mFlags.IsSet(ConfigVariableFlag::RenderThreadSafe); }

		/** Returns the current value formatted as a string. */
		virtual String GetValueAsString() const = 0;

		/** Returns the default value formatted as a string. */
		virtual String GetDefaultValueAsString() const = 0;

		/** Returns the type name of this variable (e.g., "bool", "i32"). */
		virtual const char* GetTypeName() const = 0;

	protected:
		friend class ConfigVariableManager;

		/** Called by ConfigVariableManager to apply any pending deferred updates. */
		virtual void ApplyPendingUpdate() = 0;

		/**
		 * Sets the value from a string representation.
		 *
		 * @param	value	The string value to parse.
		 * @param	source	The source of this value (for priority handling).
		 * @return			True if the value was successfully set, false otherwise.
		 */
		virtual bool SetFromString(const String& value, ConfigVariableSource source) = 0;

		/** Marks this variable as initialized (enables ReadOnly enforcement). */
		void MarkInitialized() { mInitialized = true; }

		const char* mName;
		const char* mDescription;
		ConfigVariableFlags mFlags;
		std::atomic<ConfigVariableSource> mSource{ConfigVariableSource::Default};
		bool mInitialized = false;
	};

	/**
	 * Represents a globally accessible variable that can be configured from command line,
	 * config files, or at runtime.
	 *
	 * @tparam	T	The value type (bool, i32, u32, or float).
	 */
	template<typename T>
	class TConfigVariable final : public ConfigVariable
	{
		static_assert(
			std::is_same_v<T, bool> || std::is_same_v<T, i32> ||
			std::is_same_v<T, u32> || std::is_same_v<T, float>,
			"TConfigVariable only supports bool, i32, u32, and float types."
		);

	public:
		/**
		 * Creates a new configuration variable and registers it with the manager.
		 *
		 * @param	name			The unique name for this variable (e.g., "render.vsync").
		 * @param	description		A human-readable description of what this variable controls.
		 * @param	defaultValue	The default value if not set from config or command line.
		 * @param	flags			Optional flags controlling behavior (RenderThreadSafe, ReadOnly).
		 */
		TConfigVariable(const char* name, const char* description, T defaultValue, ConfigVariableFlags flags = ConfigVariableFlag::None);
		~TConfigVariable();

		/** Returns the current value. This is always a simple atomic load with no branching, fast to access. */
		B3D_FORCEINLINE T Get() const { return mValue.load(std::memory_order_relaxed); }

		/** Implicit conversion to the value type for convenient usage. */
		B3D_FORCEINLINE operator T() const { return Get(); }

		/** Returns the default value that was specified at construction. */
		B3D_FORCEINLINE T GetDefault() const { return mDefaultValue; }

		/**
		 * Sets a new value at runtime.
		 *
		 * @param	value	The new value to set.
		 * @return			True if the value was set, false if the variable is ReadOnly.
		 *
		 * @note	For RenderThreadSafe variables, the update is deferred until the next frame sync.
		 *			For non-RenderThreadSafe variables, the update is immediate.
		 */
		bool Set(T value);

		String GetValueAsString() const override;
		String GetDefaultValueAsString() const override;
		const char* GetTypeName() const override;

	private:
		friend class ConfigVariableManager;

		void ApplyPendingUpdate() override;
		bool SetFromString(const String& value, ConfigVariableSource source) override;

		/**
		 * Internal method to set the value, bypassing ReadOnly checks.
		 * Used during initialization from config file or command line.
		 */
		void SetValueWithoutChecks(T value, ConfigVariableSource source);

		std::atomic<T> mValue;
		T mDefaultValue;

		// Deferred update storage for RenderThreadSafe variables
		std::atomic<T> mPendingValue{};
		std::atomic<bool> mHasPendingUpdate{false};
	};

	// Per-type member specializations, declared before the instantiation declarations below as the
	// standard requires explicit specializations to precede any point of instantiation
	template<> B3D_EXPORT const char* TConfigVariable<bool>::GetTypeName() const;
	template<> B3D_EXPORT const char* TConfigVariable<i32>::GetTypeName() const;
	template<> B3D_EXPORT const char* TConfigVariable<u32>::GetTypeName() const;
	template<> B3D_EXPORT const char* TConfigVariable<float>::GetTypeName() const;
	template<> B3D_EXPORT bool TConfigVariable<bool>::SetFromString(const String& value, ConfigVariableSource source);
	template<> B3D_EXPORT bool TConfigVariable<i32>::SetFromString(const String& value, ConfigVariableSource source);
	template<> B3D_EXPORT bool TConfigVariable<u32>::SetFromString(const String& value, ConfigVariableSource source);
	template<> B3D_EXPORT bool TConfigVariable<float>::SetFromString(const String& value, ConfigVariableSource source);

	// Explicit instantiation declarations
	extern template class TConfigVariable<bool>;
	extern template class TConfigVariable<i32>;
	extern template class TConfigVariable<u32>;
	extern template class TConfigVariable<float>;

	/**
	 * Manages all configuration variables in the engine. Handles loading from config files,
	 * applying command-line overrides, and coordinating deferred updates for render-thread-safe variables.
	 */
	class B3D_EXPORT ConfigVariableManager : public Module<ConfigVariableManager>
	{
	public:
		ConfigVariableManager() = default;
		~ConfigVariableManager() = default;

		/**
		 * Finds a configuration variable by name.
		 *
		 * @param	name	The name of the variable to find.
		 * @return			Pointer to the variable, or nullptr if not found.
		 */
		ConfigVariable* FindVariable(const String& name) const;

		/** Returns a map of all registered configuration variables. */
		const UnorderedMap<String, ConfigVariable*>& GetAllVariables() const { return mVariables; }

		/**
		 * @name Internal
		 * @{
		 */

		/** Registers a configuration variable with the manager. */
		void RegisterVariable(ConfigVariable* variable);

		/** Unregisters a configuration variable from the manager. */
		void UnregisterVariable(ConfigVariable* variable);

		/**
		 * Loads configuration values from an INI file.
		 *
		 * @param	path	Path to the INI file.
		 * @return			True if the file was loaded successfully, false otherwise.
		 *
		 * @note	Format: key=value, with # for comments.
		 */
		bool LoadFromFile(const Path& path);

		/**
		 * Applies command-line parameter overrides to matching configuration variables.
		 * Command-line values have higher priority than config file values.
		 */
		void ApplyCommandLineOverrides();

		/**
		 * Marks all variables as initialized, enabling ReadOnly enforcement.
		 * Should be called after config file and command-line processing is complete.
		 */
		void FinalizeInitialization();

		/**
		 * Applies any pending deferred updates for RenderThreadSafe variables.
		 * Should be called at frame boundaries when the render thread is idle.
		 */
		void ApplyPendingUpdates();

		/**
		 * Prints information about all registered configuration variables to stdout.
		 */
		void PrintHelp() const;

		/** @} */

	protected:
		void OnStartUp() override;

	private:
		/**
		 * Parses a single line from an INI file.
		 *
		 * @param	line		The line to parse (as StringView into the file content).
		 * @param	lineNumber	Line number for error reporting.
		 * @param	filePath	File path for error reporting.
		 */
		void ParseIniLine(StringView line, u32 lineNumber, const Path& filePath);

		/**
		 * Normalizes a variable name for case-insensitive lookup.
		 * Converts the name to lowercase.
		 */
		static String NormalizeName(StringView name);

		/**
		 * Stores a pending value for a variable that hasn't been registered yet.
		 * If a value already exists, it's only overwritten if the new source has higher priority.
		 */
		void StorePendingValue(StringView name, StringView value, ConfigVariableSource source);

		/**
		 * Applies any pending value for the given variable, if one exists.
		 * Removes the pending value after applying.
		 */
		void ApplyPendingValueIfExists(ConfigVariable* variable, const String& normalizedName);

		UnorderedMap<String, ConfigVariable*> mVariables;  // Keys are normalized (lowercase)
		UnorderedMap<String, PendingConfigValue> mPendingValues;  // For late-registered variables
		Vector<ConfigVariable*> mRenderThreadSafeVariables;  // Cached for fast sync
		mutable std::mutex mMutex;
	};

	/** Returns a reference to the ConfigVariableManager instance. */
	B3D_EXPORT ConfigVariableManager& GetConfigVariableManager();

	/** @} */
} // namespace b3d
