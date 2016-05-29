//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMonoManager.h"
#include "BsException.h"
#include "BsScriptMeta.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsFileSystem.h"
#include "BsException.h"
#include "BsApplication.h"

#include "mono/jit/jit.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-debug.h>
#include <mono/utils/mono-logger.h>

namespace BansheeEngine
{
	const String MONO_LIB_DIR = "bin\\Mono\\lib\\";
	const String MONO_ETC_DIR = "bin\\Mono\\etc\\";
	const String MONO_COMPILER_DIR = "bin\\Mono\\compiler\\";
	const MonoVersion MONO_VERSION = MonoVersion::v4_5;
	
	struct MonoVersionData
	{
		String path;
		String version;
	};

	static const MonoVersionData MONO_VERSION_DATA[1] =
	{
		{ MONO_LIB_DIR + "mono\\4.5", "v4.0.30319" }
	};

	void monoLogCallback(const char* logDomain, const char* logLevel, const char* message, mono_bool fatal, void* userData)
	{
		static const char* monoErrorLevels[] =
		{
			nullptr,
			"error",
			"critical",
			"warning",
			"message",
			"info",
			"debug"
		};

		UINT32 errorLevel = 0;
		if (logLevel != nullptr)
		{
			for (UINT32 i = 1; i < 7; i++)
			{
				if (strcmp(monoErrorLevels[i], logLevel) == 0)
				{
					errorLevel = i;
					break;
				}
			}
		}

		if (errorLevel == 0)
		{
			LOGERR(StringUtil::format("Mono: {0} in domain {1}", message, logDomain));
		}
		else if (errorLevel <= 2)
		{
			LOGERR(StringUtil::format("Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel));
		}
		else if (errorLevel <= 3)
		{
			LOGWRN(StringUtil::format("Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel));
		}
		else
		{
			LOGDBG(StringUtil::format("Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel));
		}
	}

	void monoPrintCallback(const char* string, mono_bool isStdout)
	{
		LOGWRN(StringUtil::format("Mono error: {0}", string));
	}

	void monoPrintErrorCallback(const char* string, mono_bool isStdout)
	{
		LOGERR(StringUtil::format("Mono error: {0}", string));
	}

	MonoManager::MonoManager()
		:mScriptDomain(nullptr), mRootDomain(nullptr), mIsCoreLoaded(false)
	{
		Path libDir = Paths::findPath(MONO_LIB_DIR);
		Path etcDir = getMonoEtcFolder();
		Path assembliesDir = getFrameworkAssembliesFolder();

		mono_set_dirs(libDir.toString().c_str(), etcDir.toString().c_str());
		mono_set_assemblies_path(assembliesDir.toString().c_str());

#if BS_DEBUG_MODE
		mono_debug_init(MONO_DEBUG_FORMAT_MONO);

		char* options[] = {
			"--soft-breakpoints",
			"--debugger-agent=transport=dt_socket,address=127.0.0.1:17615,embedding=1,server=y,suspend=n"
		};
		mono_jit_parse_options(2, options);
		mono_trace_set_level_string("warning"); // Note: Switch to "debug" for detailed output, disabled for now due to spam
#else
		mono_trace_set_level_string("warning");
#endif

		mono_trace_set_log_handler(monoLogCallback, this);
		mono_trace_set_print_handler(monoPrintCallback);
		mono_trace_set_printerr_handler(monoPrintErrorCallback);

		mono_config_parse(nullptr);

		mRootDomain = mono_jit_init_version("BansheeMono", MONO_VERSION_DATA[(int)MONO_VERSION].version.c_str());
		if (mRootDomain == nullptr)
			BS_EXCEPT(InternalErrorException, "Cannot initialize Mono runtime.");
	}

	MonoManager::~MonoManager()
	{
		for (auto& entry : mAssemblies)
		{
			bs_delete(entry.second);
		}

		mAssemblies.clear();

		unloadScriptDomain();

		if (mRootDomain != nullptr)
		{
			mono_jit_cleanup(mRootDomain);
			mRootDomain = nullptr;
		}
	}

	MonoAssembly& MonoManager::loadAssembly(const WString& path, const String& name)
	{
		MonoAssembly* assembly = nullptr;

		if (mScriptDomain == nullptr)
		{
			String appDomainName = toString(path);

			mScriptDomain = mono_domain_create_appdomain(const_cast<char *>(appDomainName.c_str()), nullptr);
			mono_domain_set(mScriptDomain, true);

			if (mScriptDomain == nullptr)
			{
				BS_EXCEPT(InternalErrorException, "Cannot create script app domain.");
			}

#if BS_DEBUG_MODE
			mono_debug_domain_create(mScriptDomain);
#endif
		}

		auto iterFind = mAssemblies.find(name);
		if(iterFind != mAssemblies.end())
		{
			assembly = iterFind->second;
		}
		else
		{
			assembly = new (bs_alloc<MonoAssembly>()) MonoAssembly(path, name);
			mAssemblies[name] = assembly;
		}
		
		initializeAssembly(*assembly);

		return *assembly;
	}

	void MonoManager::initializeAssembly(MonoAssembly& assembly)
	{
		if (!assembly.mIsLoaded)
		{
			assembly.load(mScriptDomain);

			// Fully initialize all types that use this assembly
			Vector<ScriptMeta*>& mTypeMetas = getScriptMetaData()[assembly.mName];
			for (auto& meta : mTypeMetas)
			{
				meta->scriptClass = assembly.getClass(meta->ns, meta->name);
				if (meta->scriptClass == nullptr)
					BS_EXCEPT(InvalidParametersException, "Unable to find class of type: \"" + meta->ns + "::" + meta->name + "\"");

				if (meta->scriptClass->hasField("mCachedPtr"))
					meta->thisPtrField = meta->scriptClass->getField("mCachedPtr");
				else
					meta->thisPtrField = nullptr;

				meta->initCallback();
			}
		}

		if (!mIsCoreLoaded)
		{
			mIsCoreLoaded = true;

			MonoAssembly* corlib = nullptr;

			auto iterFind = mAssemblies.find("corlib");
			if (iterFind == mAssemblies.end())
			{
				corlib = new (bs_alloc<MonoAssembly>()) MonoAssembly(L"corlib", "corlib");
				mAssemblies["corlib"] = corlib;
			}
			else
				corlib = iterFind->second;

			corlib->loadFromImage(mono_get_corlib());
		}
	}

	MonoAssembly* MonoManager::getAssembly(const String& name) const
	{
		auto iterFind = mAssemblies.find(name);

		if(iterFind != mAssemblies.end())
			return iterFind->second;

		return nullptr;
	}

	void MonoManager::registerScriptType(ScriptMeta* metaData)
	{
		Vector<ScriptMeta*>& mMetas = getScriptMetaData()[metaData->assembly];
		mMetas.push_back(metaData);
	}

	MonoClass* MonoManager::findClass(const String& ns, const String& typeName)
	{
		MonoClass* monoClass = nullptr;
		for(auto& assembly : mAssemblies)
		{
			monoClass = assembly.second->getClass(ns, typeName);
			if(monoClass != nullptr)
				return monoClass;
		}

		return nullptr;
	}

	MonoClass* MonoManager::findClass(::MonoClass* rawMonoClass)
	{
		MonoClass* monoClass = nullptr;
		for(auto& assembly : mAssemblies)
		{
			monoClass = assembly.second->getClass(rawMonoClass);
			if(monoClass != nullptr)
				return monoClass;
		}

		return nullptr;
	}

	void MonoManager::unloadScriptDomain()
	{
		if (mScriptDomain != nullptr)
		{
			onDomainUnload();

			mono_domain_set(mono_get_root_domain(), true);
			mono_domain_finalize(mScriptDomain, 2000);

			MonoObject* exception = nullptr;
			mono_domain_try_unload(mScriptDomain, &exception);

			if (exception != nullptr)
				MonoUtil::throwIfException(exception);

			mono_gc_collect(mono_gc_max_generation());

			mScriptDomain = nullptr;
		}

		for (auto& assemblyEntry : mAssemblies)
		{
			assemblyEntry.second->unload();

			// Metas hold references to various assembly objects that were just deleted, so clear them
			Vector<ScriptMeta*>& typeMetas = getScriptMetaData()[assemblyEntry.first];
			for (auto& entry : typeMetas)
			{
				entry->scriptClass = nullptr;
				entry->thisPtrField = nullptr;
			}
		}

		mAssemblies.clear();
		mIsCoreLoaded = false;
	}

	void MonoManager::loadScriptDomain()
	{
		if (mScriptDomain != nullptr)
			unloadScriptDomain();

		if (mScriptDomain == nullptr)
		{
			char domainName[] = "ScriptDomain";

			mScriptDomain = mono_domain_create_appdomain(domainName, nullptr);
			mono_domain_set(mScriptDomain, false);

			if (mScriptDomain == nullptr)
			{
				BS_EXCEPT(InternalErrorException, "Cannot create script app domain.");
			}
		}

		if (mScriptDomain != nullptr)
		{
			for (auto& assemblyEntry : mAssemblies)
				initializeAssembly(*assemblyEntry.second);
		}
	}

	Path MonoManager::getFrameworkAssembliesFolder() const
	{
		return Paths::findPath(MONO_VERSION_DATA[(int)MONO_VERSION].path);
	}

	Path MonoManager::getMonoEtcFolder() const
	{
		return Paths::findPath(MONO_ETC_DIR);
	}

	Path MonoManager::getCompilerPath() const
	{
		Path compilerPath = FileSystem::getWorkingDirectoryPath();
		compilerPath.append(Paths::findPath(MONO_COMPILER_DIR));

#if BS_PLATFORM == BS_PLATFORM_WIN32
		compilerPath.append("mcs.exe");
#else
		static_assert("Not implemented");
#endif

		return compilerPath;
	}
}