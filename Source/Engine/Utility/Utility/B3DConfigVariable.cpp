//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DConfigVariable.h"
#include "Utility/B3DCommandLine.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "Debug/B3DDebug.h"

namespace b3d
{
	B3D_LOG_CATEGORY_STATIC(LogConfigVariable, Log)

	// Thread-safe registration helper for static initialization order safety
	namespace ConfigVariableRegistry
	{
		static std::atomic<bool> sManagerStarted{false};
		static std::mutex sPendingMutex;

		static Vector<ConfigVariable*>& GetPendingVariables()
		{
			static Vector<ConfigVariable*> sPendingVariables;
			return sPendingVariables;
		}

		void Register(ConfigVariable* variable)
		{
			// Fast path: manager already started
			if (sManagerStarted.load(std::memory_order_acquire))
			{
				ConfigVariableManager::Instance().RegisterVariable(variable);
				return;
			}

			// Slow path: add to pending list with lock
			Lock lock(sPendingMutex);
			if (sManagerStarted.load(std::memory_order_relaxed))
			{
				ConfigVariableManager::Instance().RegisterVariable(variable);
				return;
			}

			GetPendingVariables().push_back(variable);
		}

		void Unregister(ConfigVariable* variable)
		{
			if (sManagerStarted.load(std::memory_order_acquire))
			{
				if (ConfigVariableManager::IsStarted())
					ConfigVariableManager::Instance().UnregisterVariable(variable);

				return;
			}

			// If manager hasn't started, remove from pending list
			Lock lock(sPendingMutex);
			if (sManagerStarted.load(std::memory_order_relaxed))
			{
				if (ConfigVariableManager::IsStarted())
					ConfigVariableManager::Instance().UnregisterVariable(variable);

				return;
			}

			Vector<ConfigVariable*>& pending = GetPendingVariables();

			auto found = std::find(pending.begin(), pending.end(), variable);
			if (found != pending.end())
				pending.erase(found);
		}

		void OnManagerStarted()
		{
			Lock lock(sPendingMutex);

			Vector<ConfigVariable*>& pending = GetPendingVariables();
			for (ConfigVariable* variable : pending)
				ConfigVariableManager::Instance().RegisterVariable(variable);

			pending.clear();
			pending.shrink_to_fit();
			sManagerStarted.store(true, std::memory_order_release);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ConfigVariable
	//////////////////////////////////////////////////////////////////////////

	ConfigVariable::ConfigVariable(const char* name, const char* description, ConfigVariableFlags flags)
		: mName(name), mDescription(description), mFlags(flags)
	{
		// Note: Registration is done in TConfigVariable constructor, not here.
		// This avoids calling virtual methods (SetFromString) before the derived vtable is set up.
	}

	ConfigVariable::~ConfigVariable()
	{
		// Note: Unregistration is done in TConfigVariable destructor, not here.
		// This ensures the derived vtable is still valid if any virtual calls are needed.
	}

	//////////////////////////////////////////////////////////////////////////
	// TConfigVariable specializations
	//////////////////////////////////////////////////////////////////////////

	template<>
	const char* TConfigVariable<bool>::GetTypeName() const { return "bool"; }

	template<>
	const char* TConfigVariable<i32>::GetTypeName() const { return "i32"; }

	template<>
	const char* TConfigVariable<u32>::GetTypeName() const { return "u32"; }

	template<>
	const char* TConfigVariable<float>::GetTypeName() const { return "float"; }

	template<>
	bool TConfigVariable<bool>::SetFromString(const String& value, ConfigVariableSource source)
	{
		SetValueWithoutChecks(ParseBool(value), source);
		return true;
	}

	template<>
	bool TConfigVariable<i32>::SetFromString(const String& value, ConfigVariableSource source)
	{
		SetValueWithoutChecks(ParseI32(value), source);
		return true;
	}

	template<>
	bool TConfigVariable<u32>::SetFromString(const String& value, ConfigVariableSource source)
	{
		SetValueWithoutChecks(ParseU32(value), source);
		return true;
	}

	template<>
	bool TConfigVariable<float>::SetFromString(const String& value, ConfigVariableSource source)
	{
		SetValueWithoutChecks(ParseFloat(value), source);
		return true;
	}

	template<typename T>
	TConfigVariable<T>::TConfigVariable(const char* name, const char* description, T defaultValue, ConfigVariableFlags flags)
		: ConfigVariable(name, description, flags), mValue(defaultValue), mDefaultValue(defaultValue), mPendingValue(defaultValue), mHasPendingUpdate(false)
	{
		// Register here (not in base class) so vtable is set up before any virtual calls
		ConfigVariableRegistry::Register(this);
	}

	template<typename T>
	TConfigVariable<T>::~TConfigVariable()
	{
		// Unregister here (not in base class) so vtable is still valid
		ConfigVariableRegistry::Unregister(this);
	}

	template<typename T>
	bool TConfigVariable<T>::Set(T value)
	{
		if (mInitialized && IsReadOnly())
			return false;

		if (IsRenderThreadSafe())
		{
			mPendingValue.store(value, std::memory_order_relaxed);
			mHasPendingUpdate.store(true, std::memory_order_release);
		}
		else
			mValue.store(value, std::memory_order_relaxed);

		if (mInitialized)
			mSource.store(ConfigVariableSource::Runtime, std::memory_order_relaxed);

		return true;
	}

	template<typename T>
	String TConfigVariable<T>::GetValueAsString() const
	{
		return ToString(mValue.load(std::memory_order_relaxed));
	}

	template<typename T>
	String TConfigVariable<T>::GetDefaultValueAsString() const
	{
		return ToString(mDefaultValue);
	}

	template<typename T>
	void TConfigVariable<T>::ApplyPendingUpdate()
	{
		if (mHasPendingUpdate.exchange(false, std::memory_order_acq_rel))
			mValue.store(mPendingValue.load(std::memory_order_relaxed), std::memory_order_relaxed);
	}

	template<typename T>
	void TConfigVariable<T>::SetValueWithoutChecks(T value, ConfigVariableSource source)
	{
		// Only set if source priority is higher or equal
		if (static_cast<u8>(source) >= static_cast<u8>(mSource.load(std::memory_order_relaxed)))
		{
			mValue.store(value, std::memory_order_relaxed);
			mPendingValue.store(value, std::memory_order_relaxed);
			mSource.store(source, std::memory_order_relaxed);
		}
	}

	// Explicit template instantiations
	template class B3D_EXPORT TConfigVariable<bool>;
	template class B3D_EXPORT TConfigVariable<i32>;
	template class B3D_EXPORT TConfigVariable<u32>;
	template class B3D_EXPORT TConfigVariable<float>;

	//////////////////////////////////////////////////////////////////////////
	// ConfigVariableManager
	//////////////////////////////////////////////////////////////////////////

	String ConfigVariableManager::NormalizeName(StringView name)
	{
		String normalized(name);
		StringUtility::ToLowerCase(normalized);
		return normalized;
	}

	void ConfigVariableManager::StorePendingValue(StringView name, StringView value, ConfigVariableSource source)
	{
		String normalizedName = NormalizeName(name);

		auto iter = mPendingValues.find(normalizedName);
		if (iter != mPendingValues.end())
		{
			// Only overwrite if new source has higher or equal priority
			if (static_cast<u8>(source) >= static_cast<u8>(iter->second.source))
			{
				iter->second.value = String(value);
				iter->second.source = source;
			}
		}
		else
			mPendingValues[normalizedName] = PendingConfigValue{String(value), source};
	}

	void ConfigVariableManager::ApplyPendingValueIfExists(ConfigVariable* variable, const String& normalizedName)
	{
		auto iter = mPendingValues.find(normalizedName);
		if (iter != mPendingValues.end())
		{
			variable->SetFromString(iter->second.value, iter->second.source);
			mPendingValues.erase(iter);
		}
	}

	void ConfigVariableManager::OnStartUp()
	{
		ConfigVariableRegistry::OnManagerStarted();

		// Load config file (if exists)
		Path configPath = FileSystem::GetExecutableFolderPath();
		configPath.SetFilename("engine.ini");
		LoadFromFile(configPath);

		// Apply command-line overrides (highest priority)
		ApplyCommandLineOverrides();

		// Enable ReadOnly enforcement
		FinalizeInitialization();
	}

	void ConfigVariableManager::RegisterVariable(ConfigVariable* variable)
	{
		Lock lock(mMutex);

		const String normalizedName = NormalizeName(variable->GetName());
		auto iter = mVariables.find(normalizedName);
		if (iter != mVariables.end())
		{
			B3D_LOG(Warning, LogConfigVariable, "Duplicate config variable registration: {0}", variable->GetName());
			return;
		}

		mVariables[normalizedName] = variable;

		// Apply any pending value from INI/command line (for late-registered variables)
		ApplyPendingValueIfExists(variable, normalizedName);

		if (variable->IsRenderThreadSafe())
			mRenderThreadSafeVariables.push_back(variable);
	}

	void ConfigVariableManager::UnregisterVariable(ConfigVariable* variable)
	{
		Lock lock(mMutex);

		const String normalizedName = NormalizeName(variable->GetName());
		mVariables.erase(normalizedName);

		if (variable->IsRenderThreadSafe())
		{
			auto iter = std::find(mRenderThreadSafeVariables.begin(), mRenderThreadSafeVariables.end(), variable);
			if (iter != mRenderThreadSafeVariables.end())
				mRenderThreadSafeVariables.erase(iter);
		}
	}

	bool ConfigVariableManager::LoadFromFile(const Path& path)
	{
		if (!FileSystem::Exists(path))
		{
			B3D_LOG(Info, LogConfigVariable, "Config file not found: {0}", path);
			return false;
		}

		TShared<DataStream> stream = FileSystem::OpenFile(path, true);
		if (stream == nullptr)
		{
			B3D_LOG(Warning, LogConfigVariable, "Failed to open config file: {0}", path);
			return false;
		}

		String content = stream->GetAsString();
		stream->Close();

		StringView remaining(content);
		u32 lineNumber = 0;

		while (!remaining.empty())
		{
			lineNumber++;

			// Find end of line
			size_t lineEnd = remaining.find('\n');
			StringView line;
			if (lineEnd != StringView::npos)
			{
				line = remaining.substr(0, lineEnd);
				remaining = remaining.substr(lineEnd + 1);
			}
			else
			{
				line = remaining;
				remaining = StringView();
			}

			// Strip carriage return if present (Windows line endings)
			if (!line.empty() && line.back() == '\r')
				line = line.substr(0, line.size() - 1);

			ParseIniLine(line, lineNumber, path);
		}

		B3D_LOG(Info, LogConfigVariable, "Loaded config file: {0}", path.ToString());
		return true;
	}

	void ConfigVariableManager::ParseIniLine(StringView line, u32 lineNumber, const Path& filePath)
	{
		StringView trimmedLine = StringUtility::Trim(line);

		// Skip empty lines and comments
		if (trimmedLine.empty() || trimmedLine[0] == '#')
			return;

		// Remove inline comments
		size_t commentPos = trimmedLine.find('#');
		if (commentPos != StringView::npos)
			trimmedLine = StringUtility::Trim(trimmedLine.substr(0, commentPos));

		// Find the = separator
		size_t equalsPos = trimmedLine.find('=');
		if (equalsPos == StringView::npos)
		{
			B3D_LOG(Warning, LogConfigVariable, "Invalid config line {0} in {1}: missing '='", lineNumber, filePath);
			return;
		}

		StringView key = StringUtility::Trim(trimmedLine.substr(0, equalsPos));
		StringView value = StringUtility::Trim(trimmedLine.substr(equalsPos + 1));

		if (key.empty())
		{
			B3D_LOG(Warning, LogConfigVariable, "Invalid config line {0} in {1}: empty key", lineNumber, filePath);
			return;
		}

		Lock lock(mMutex);

		String normalizedKey = NormalizeName(key);
		auto iter = mVariables.find(normalizedKey);
		if (iter != mVariables.end())
			iter->second->SetFromString(String(value), ConfigVariableSource::ConfigFile);
		else
		{
			// Store for late-registered variables (e.g., from dynamically loaded libraries)
			StorePendingValue(key, value, ConfigVariableSource::ConfigFile);
		}
	}

	void ConfigVariableManager::ApplyCommandLineOverrides()
	{
		Lock lock(mMutex);

		// Iterate over all command-line parameters and apply or store them
		const auto& parameters = CommandLine::GetAllParameters();
		for (const auto& pair : parameters)
		{
			const String& name = pair.first;  // Already normalized (lowercase) by CommandLine
			const String& value = pair.second;

			auto iter = mVariables.find(name);
			if (iter != mVariables.end())
				iter->second->SetFromString(value, ConfigVariableSource::CommandLine);
			else
			{
				// Store for late-registered variables
				StorePendingValue(name, value, ConfigVariableSource::CommandLine);
			}
		}
	}

	void ConfigVariableManager::FinalizeInitialization()
	{
		Lock lock(mMutex);

		for (auto& pair : mVariables)
			pair.second->MarkInitialized();
	}

	void ConfigVariableManager::ApplyPendingUpdates()
	{
		// Copy the vector under lock to handle late registration from dynamic libraries
		Vector<ConfigVariable*> variablesToUpdate;
		{
			Lock lock(mMutex);
			variablesToUpdate = mRenderThreadSafeVariables;
		}

		for (ConfigVariable* variable : variablesToUpdate)
			variable->ApplyPendingUpdate();
	}

	void ConfigVariableManager::PrintHelp() const
	{
		Lock lock(mMutex);

		// Collect variables and sort by normalized name
		Vector<ConfigVariable*> variables;
		variables.reserve(mVariables.size());
		for (const auto& pair : mVariables)
			variables.push_back(pair.second);

		std::sort(variables.begin(), variables.end(), [](ConfigVariable* a, ConfigVariable* b)
		{
			String nameA(a->GetName());
			String nameB(b->GetName());
			return StringUtility::LexicographicalCompare(nameA, nameB, false) < 0;
		});

		std::cout << "\nConfiguration Variables:\n";
		std::cout << "========================\n\n";

		String currentPrefix;
		for (ConfigVariable* variable : variables)
		{
			const char* originalName = variable->GetName();

			// Group by prefix (everything before first '.')
			StringView nameView(originalName);
			size_t dotPos = nameView.find('.');
			String prefix = (dotPos != StringView::npos) ? String(nameView.substr(0, dotPos)) : "";

			// Normalize prefix for grouping comparison
			String normalizedPrefix = prefix;
			StringUtility::ToLowerCase(normalizedPrefix);
			String normalizedCurrentPrefix = currentPrefix;
			StringUtility::ToLowerCase(normalizedCurrentPrefix);

			if (normalizedPrefix != normalizedCurrentPrefix)
			{
				if (!currentPrefix.empty())
					std::cout << "\n";
				currentPrefix = prefix;
			}

			// Print variable info with original case
			std::cout << "  " << originalName << " (" << variable->GetTypeName() << ")\n";
			std::cout << "      " << variable->GetDescription() << "\n";
			std::cout << "      Default: " << variable->GetDefaultValueAsString().c_str();

			if (variable->IsReadOnly())
				std::cout << " [ReadOnly]";
			if (variable->IsRenderThreadSafe())
				std::cout << " [RenderThreadSafe]";

			std::cout << "\n";
		}

		std::cout << "\nUsage:\n";
		std::cout << "  Command line: --variable.name=value\n";
		std::cout << "  Config file (engine.ini): variable.name = value\n";
		std::cout << std::endl;
	}

	ConfigVariable* ConfigVariableManager::FindVariable(const String& name) const
	{
		Lock lock(mMutex);

		String normalizedName = NormalizeName(name);
		auto found = mVariables.find(normalizedName);
		if (found != mVariables.end())
			return found->second;

		return nullptr;
	}

	ConfigVariableManager& GetConfigVariableManager()
	{
		return ConfigVariableManager::Instance();
	}

} // namespace b3d
