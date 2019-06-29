//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsMonoManager.h"
#include "BsScriptMeta.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "FileSystem/BsFileSystem.h"

#include "mono/jit/jit.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-debug.h>
#include <mono/utils/mono-logger.h>
#include <mono/metadata/threads.h>

namespace bs
{
	const String MONO_LIB_DIR = "bin/Mono/lib/";
	const String MONO_ETC_DIR = "bin/Mono/etc/";
	const String MONO_COMPILER_DIR = "bin/Mono/compiler/";
	const MonoVersion MONO_VERSION = MonoVersion::v4_5;
	
	struct MonoVersionData
	{
		String path;
		String version;
	};

	static const MonoVersionData MONO_VERSION_DATA[1] =
	{
		{ MONO_LIB_DIR + "mono/4.5", "v4.0.30319" }
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
			BS_LOG(Error, Script, "Mono: {0} in domain {1}", message, logDomain);
		}
		else if (errorLevel <= 2)
		{
			BS_LOG(Error, Script, "Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
		else if (errorLevel <= 3)
		{
			BS_LOG(Warning, Script, "Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
		else
		{
			BS_LOG(Info, Particles, "Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
	}

	void monoPrintCallback(const char* string, mono_bool isStdout)
	{
		BS_LOG(Warning, Script, "Mono error: {0}", string);
	}

	void monoPrintErrorCallback(const char* string, mono_bool isStdout)
	{
		BS_LOG(Error, Script, "Mono error: {0}", string);
	}	
	
	MonoManager::MonoManager()
		:mScriptDomain(nullptr), mRootDomain(nullptr), mCorlibAssembly(nullptr)
	{
		Path libDir = Paths::findPath(MONO_LIB_DIR);
		Path etcDir = getMonoEtcFolder();
		Path assembliesDir = getFrameworkAssembliesFolder();

		mono_set_dirs(libDir.toString().c_str(), etcDir.toString().c_str());
		mono_set_assemblies_path(assembliesDir.toString().c_str());

#if BS_DEBUG_MODE
		// Note: For proper debugging experience make sure to open a console window to display stdout and stderr, as Mono
		// uses them for much of its logging.
		mono_debug_init(MONO_DEBUG_FORMAT_MONO);

		const char* options[] = {
			"--soft-breakpoints",
			"--debugger-agent=transport=dt_socket,address=127.0.0.1:17615,embedding=1,server=y,suspend=n",
			"--debug-domain-unload",

			// GC options:
			// check-remset-consistency: Makes sure that write barriers are properly issued in native code, and therefore
			//    all old->new generation references are properly present in the remset. This is easy to mess up in native
			//    code by performing a simple memory copy without a barrier, so it's important to keep the option on.
			// verify-before-collections: Unusure what exactly it does, but it sounds like it could help track down
			//    things like accessing released/moved objects, or attempting to release handles for an unloaded domain.
			// xdomain-checks: Makes sure that no references are left when a domain is unloaded.
			"--gc-debug=check-remset-consistency,verify-before-collections,xdomain-checks"
		};
		mono_jit_parse_options(4, (char**)options);
		mono_trace_set_level_string("warning"); // Note: Switch to "debug" for detailed output, disabled for now due to spam
#else
		mono_trace_set_level_string("warning");
#endif

		mono_trace_set_log_handler(monoLogCallback, this);
		mono_trace_set_print_handler(monoPrintCallback);
		mono_trace_set_printerr_handler(monoPrintErrorCallback);

		mono_config_parse(nullptr);

		mRootDomain = mono_jit_init_version("bsfMono", MONO_VERSION_DATA[(int)MONO_VERSION].version.c_str());
		if (mRootDomain == nullptr)
			BS_EXCEPT(InternalErrorException, "Cannot initialize Mono runtime.");

		mono_thread_set_main(mono_thread_current());

		// Load corlib
		mCorlibAssembly = new (bs_alloc<MonoAssembly>()) MonoAssembly("", "corlib");
		mCorlibAssembly->loadFromImage(mono_get_corlib());

		mAssemblies["corlib"] = mCorlibAssembly;
	}

	MonoManager::~MonoManager()
	{
		unloadAll();
	}

	MonoAssembly& MonoManager::loadAssembly(const Path& path, const String& name)
	{
		MonoAssembly* assembly = nullptr;

		if (mScriptDomain == nullptr)
		{
			String appDomainName = "ScriptDomain";

			mScriptDomain = mono_domain_create_appdomain(const_cast<char *>(appDomainName.c_str()), nullptr);
			if (mScriptDomain == nullptr)
				BS_EXCEPT(InternalErrorException, "Cannot create script app domain.");

			if(!mono_domain_set(mScriptDomain, true))
				BS_EXCEPT(InternalErrorException, "Cannot set script app domain.");
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
		
		if (!assembly->mIsLoaded)
		{
			assembly->load();
			initializeScriptTypes(*assembly);
		}

		return *assembly;
	}

	void MonoManager::initializeScriptTypes(MonoAssembly& assembly)
	{
		// Fully initialize all types that use this assembly
		Vector<ScriptMetaInfo>& typeMetas = getScriptMetaData()[assembly.mName];
		for (auto& entry : typeMetas)
		{
			ScriptMeta* meta = entry.metaData;
			*meta = entry.localMetaData;

			meta->scriptClass = assembly.getClass(meta->ns, meta->name);
			if (meta->scriptClass == nullptr)
			{
				BS_EXCEPT(InvalidParametersException,
					"Unable to find class of type: \"" + meta->ns + "::" + meta->name + "\"");
			}

			if (meta->scriptClass->hasField("mCachedPtr"))
				meta->thisPtrField = meta->scriptClass->getField("mCachedPtr");
			else
				meta->thisPtrField = nullptr;

			meta->initCallback();
		}
	}

	void MonoManager::unloadAll()
	{
		for (auto& entry : mAssemblies)
			bs_delete(entry.second);

		mAssemblies.clear();

		unloadScriptDomain();

		if (mRootDomain != nullptr)
		{
			mono_jit_cleanup(mRootDomain);
			mRootDomain = nullptr;
		}

		// Make sure to explicitly clear this meta-data, as it contains structures allocated from other dynamic libraries,
		// which will likely get unloaded right after shutdown
		getScriptMetaData().clear();
	}

	MonoAssembly* MonoManager::getAssembly(const String& name) const
	{
		auto iterFind = mAssemblies.find(name);

		if(iterFind != mAssemblies.end())
			return iterFind->second;

		return nullptr;
	}

	void MonoManager::registerScriptType(ScriptMeta* metaData, const ScriptMeta& localMetaData)
	{
		Vector<ScriptMetaInfo>& mMetas = getScriptMetaData()[localMetaData.assembly];
		mMetas.push_back({ metaData, localMetaData });
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

			MonoObject* exception = nullptr;
			mono_domain_try_unload(mScriptDomain, &exception);

			if (exception != nullptr)
				MonoUtil::throwIfException(exception);

			mScriptDomain = nullptr;
		}

		for (auto& assemblyEntry : mAssemblies)
		{
			assemblyEntry.second->unload();

			// "corlib" assembly persists domain unload since it's in the root domain. However we make sure to clear its
			// class list as it could contain generic instances that use types from other assemblies.
			if(assemblyEntry.first != "corlib")
				bs_delete(assemblyEntry.second);

			// Metas hold references to various assembly objects that were just deleted, so clear them
			Vector<ScriptMetaInfo>& typeMetas = getScriptMetaData()[assemblyEntry.first];
			for (auto& entry : typeMetas)
			{
				entry.metaData->scriptClass = nullptr;
				entry.metaData->thisPtrField = nullptr;
			}
		}

		mAssemblies.clear();
		mAssemblies["corlib"] = mCorlibAssembly;
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
		Path compilerPath = Paths::findPath(MONO_COMPILER_DIR);
		compilerPath.append("mcs.exe");
		return compilerPath;
	}

	Path MonoManager::getMonoExecPath() const
	{
		Path path = Paths::getBinariesPath();

#if BS_PLATFORM == BS_PLATFORM_WIN32
		path.append("MonoExec.exe");
#else
		path.append("MonoExec");
#endif

		return path;
	}
}
