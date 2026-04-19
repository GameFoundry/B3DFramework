//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoManager.h"
#include "B3DScriptTypeMetaData.h"
#include "B3DMonoAssembly.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "FileSystem/B3DFileSystem.h"

#if B3D_USE_DOTNETCORE
#include "B3DMonoLoader.h"
#else
#include "mono/jit/jit.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/mono-debug.h>
#include <mono/utils/mono-logger.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/appdomain.h>
#endif

using namespace b3d;

const String kMonoCompilerDir = "bin/Mono/compiler/";

#if B3D_USE_DOTNETCORE
static const char* kMonoAssembliesFolder = "bin/Assemblies/";
#else
const String kMonoLibDir = "bin/Mono/lib/";
const String kMonoEtcDir = "bin/Mono/etc/";
const MonoVersion kMonoVersion = MonoVersion::v4_5;

struct MonoVersionData
{
	String Path;
	String Version;
};

static const MonoVersionData kMonoVersionData[1] = {
	{ kMonoLibDir + "mono/4.5", "v4.0.30319" }
};
#endif

namespace b3d
{
	void MonoLogCallback(const char* logDomain, const char* logLevel, const char* message, mono_bool fatal, void* userData)
	{
		static const char* monoErrorLevels[] = {
			nullptr,
			"error",
			"critical",
			"warning",
			"message",
			"info",
			"debug"
		};

		u32 errorLevel = 0;
		if(logLevel != nullptr)
		{
			for(u32 i = 1; i < 7; i++)
			{
				if(strcmp(monoErrorLevels[i], logLevel) == 0)
				{
					errorLevel = i;
					break;
				}
			}
		}

		if(errorLevel == 0)
		{
			B3D_LOG(Error, LogScript, "Mono: {0} in domain {1}", message, logDomain);
		}
		else if(errorLevel <= 2)
		{
			B3D_LOG(Error, LogScript, "Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
		else if(errorLevel <= 3)
		{
			B3D_LOG(Warning, LogScript, "Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
		else
		{
			B3D_LOG(Info, LogParticles, "Mono: {0} in domain {1} [{2}]", message, logDomain, logLevel);
		}
	}

	void MonoPrintCallback(const char* string, mono_bool isStdout)
	{
		B3D_LOG(Warning, LogScript, "Mono error: {0}", string);
	}

	void MonoPrintErrorCallback(const char* string, mono_bool isStdout)
	{
		B3D_LOG(Error, LogScript, "Mono error: {0}", string);
	}
} // namespace b3d

MonoManager::MonoManager()
	: mScriptDomain(nullptr), mRootDomain(nullptr), mCorlibAssembly(nullptr)
{
#if B3D_USE_DOTNETCORE
	MonoLoader::StartUp();
#endif

	LoadMonoLibrary();
}

MonoManager::~MonoManager()
{
	UnloadMonoLibrary();

	// Make sure to explicitly clear this meta-data, as it contains structures allocated from other dynamic libraries,
	// which will likely get unloaded right after shutdown
	GetScriptWrapperTypeInformation().clear();

#if B3D_USE_DOTNETCORE
	MonoLoader::ShutDown();
#endif
}

void MonoManager::LoadMonoLibrary()
{
#if !B3D_USE_DOTNETCORE
	Path libDir = Paths::FindPath(kMonoLibDir);
	Path etcDir = GetMonoEtcFolder();
	Path assembliesDir = GetFrameworkAssembliesFolder();

	mono_set_dirs(libDir.ToString().c_str(), etcDir.ToString().c_str());
	mono_set_assemblies_path(assembliesDir.ToString().c_str());
#else
	MonoLoader::Instance().Load();

	const Path assembliesFolder = GetFrameworkAssembliesFolder();
	mono_set_assemblies_path(assembliesFolder.ToString().c_str());
#endif

#if B3D_DEBUG
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

	mono_trace_set_log_handler(MonoLogCallback, this);
	mono_trace_set_print_handler(MonoPrintCallback);
	mono_trace_set_printerr_handler(MonoPrintErrorCallback);

	mono_config_parse(nullptr);

#if B3D_USE_DOTNETCORE
	mScriptDomain = mRootDomain = mono_jit_init("bsfMono");
#else
	mRootDomain = mono_jit_init_version("bsfMono", kMonoVersionData[(int)kMonoVersion].Version.c_str());
#endif

	if(mRootDomain == nullptr)
		B3D_LOG(Fatal, LogGeneric, "Cannot initialize Mono runtime.");

	mono_thread_set_main(mono_thread_current());

	// Load corlib
	mCorlibAssembly = new(B3DAllocate<MonoAssembly>()) MonoAssembly("", "corlib");
	mCorlibAssembly->LoadFromImage(mono_get_corlib());

	mAssemblies["corlib"] = mCorlibAssembly;
}

void MonoManager::UnloadMonoLibrary()
{
	UnloadAll();

#if B3D_USE_DOTNETCORE
	MonoLoader::Instance().Unload();
#endif
}

b3d::MonoAssembly& MonoManager::LoadAssembly(const Path& path, const String& name)
{
	MonoAssembly* assembly = nullptr;

#if !B3D_USE_DOTNETCORE
	// Application domains not present with .NET Core
	if(mScriptDomain == nullptr)
	{
		String appDomainName = "ScriptDomain";

		mScriptDomain = mono_domain_create_appdomain(const_cast<char*>(appDomainName.c_str()), nullptr);
		if(mScriptDomain == nullptr)
			B3D_LOG(Fatal, LogGeneric, "Cannot create script app domain.");

		if(!mono_domain_set(mScriptDomain, true))
			B3D_LOG(Fatal, LogGeneric, "Cannot set script app domain.");
	}
#endif

	auto iterFind = mAssemblies.find(name);
	if(iterFind != mAssemblies.end())
	{
		assembly = iterFind->second;
	}
	else
	{
		assembly = new(B3DAllocate<MonoAssembly>()) MonoAssembly(path, name);
		mAssemblies[name] = assembly;
	}

	if(!assembly->mIsLoaded)
	{
		assembly->Load();
		RefreshScriptTypeMetaDataAndBindings(*assembly); // TODO - This should probably be moved to ScriptAssemblyManager
	}

	return *assembly;
}

void MonoManager::RefreshScriptTypeMetaDataAndBindings(MonoAssembly& assembly)
{
	// Fully initialize all types that use this assembly
	Vector<RegisteredScriptWrapperTypeInformation>& typeMetas = GetScriptWrapperTypeInformation()[assembly.mName];

	// Ensure we resolve types with smaller number of generic parameters first, as later types will depend on them
	std::sort(typeMetas.begin(), typeMetas.end(), [](RegisteredScriptWrapperTypeInformation& lhs, RegisteredScriptWrapperTypeInformation& rhs)
	{
		return lhs.MetaData->Identifier.GenericTypeParameters.Size() < rhs.MetaData->Identifier.GenericTypeParameters.Size();
	});

	UnorderedMap<MonoTypeIdentifier, MonoClass*> resolvedTypes;
	for(auto& entry : typeMetas)
	{
		ScriptTypeMetaData* meta = entry.MetaData;
		*meta = entry.LocalMetaData;

		meta->ScriptClass = FindClass(meta->Identifier);
		if(!B3D_ENSURE(meta->ScriptClass != nullptr))
			continue;

		resolvedTypes[meta->Identifier] = meta->ScriptClass;

		if(meta->ScriptClass->HasField("mCachedPtr"))
			meta->ScriptObjectWrapperPointerField = meta->ScriptClass->GetField("mCachedPtr");
		else
			meta->ScriptObjectWrapperPointerField = nullptr;

		if(meta->SetupScriptBindingsCallback != nullptr)
			meta->SetupScriptBindingsCallback();
	}
}

void MonoManager::UnloadAll()
{
	for(auto& entry : mAssemblies)
	{
		B3DDelete(entry.second);

		// Metas hold references to various assembly objects that were just deleted, so clear them
		Vector<RegisteredScriptWrapperTypeInformation>& typeMetas = GetScriptWrapperTypeInformation()[entry.first];
		for(auto& entry : typeMetas)
		{
			entry.MetaData->ScriptClass = nullptr;
			entry.MetaData->ScriptObjectWrapperPointerField = nullptr;
		}
	}

	mAssemblies.clear();
	mCorlibAssembly = nullptr;

	UnloadScriptDomain();

	if(mRootDomain != nullptr)
	{
		mono_jit_cleanup(mRootDomain);
		mRootDomain = nullptr;
	}
}

b3d::MonoAssembly* MonoManager::GetAssembly(const String& name) const
{
	auto iterFind = mAssemblies.find(name);

	if(iterFind != mAssemblies.end())
		return iterFind->second;

	return nullptr;
}

void MonoManager::RegisterScriptType(ScriptTypeMetaData* metaData, const ScriptTypeMetaData& localMetaData)
{
	Vector<RegisteredScriptWrapperTypeInformation>& mMetas = GetScriptWrapperTypeInformation()[localMetaData.Identifier.Assembly];
	mMetas.push_back({ metaData, localMetaData });
}

b3d::MonoClass* MonoManager::FindClass(const MonoTypeIdentifier& typeIdentifier)
{
	auto fnFindClass = [this](const String& assemblyName, const String& nameSpace, const String& typeName) -> MonoClass*
	{
		if(assemblyName.empty())
			return FindClass(nameSpace, typeName);

		const MonoAssembly* assembly = GetAssembly(assemblyName);
		if(assembly == nullptr)
			return nullptr;
		
		return assembly->GetClass(nameSpace, typeName);
	};

	if(typeIdentifier.GenericTypeParameters.Size() == 0)
		return fnFindClass(typeIdentifier.Assembly, typeIdentifier.Namespace, typeIdentifier.TypeName);

	const String& genericTypeName = typeIdentifier.TypeName + '`' + ToString(typeIdentifier.GenericTypeParameters.Size());
	MonoClass* const genericClass = fnFindClass(typeIdentifier.Assembly, typeIdentifier.Namespace, genericTypeName);
	if(genericClass == nullptr)
		return nullptr;
	
	TInlineArray<::MonoClass*, 4> genericArgumentClasses;
	genericArgumentClasses.Reserve(typeIdentifier.GenericTypeParameters.Size());

	for(const auto& genericTypeParameter : typeIdentifier.GenericTypeParameters)
	{
		::MonoClass* genericArgumentInternalClass = nullptr;
		if(MonoClass* const genericArgumentClass = FindClass(genericTypeParameter))
			genericArgumentInternalClass = genericArgumentClass->GetInternalClass();
		else if(genericTypeParameter.Namespace.empty() || typeIdentifier.Namespace == "System")
			genericArgumentInternalClass = MonoUtil::GetPrimitiveTypeClass(genericTypeParameter.TypeName);

		if(genericArgumentInternalClass == nullptr)
			return nullptr;

		genericArgumentClasses.Add(genericArgumentInternalClass);
	}

	::MonoClass* internalClass = MonoUtil::BindGenericParameters(genericClass->GetInternalClass(), genericArgumentClasses.Data(), (u32)genericArgumentClasses.Size());
	const MonoAssembly* const assembly = genericClass->GetAssembly();

	return assembly->GetClass(typeIdentifier.Namespace, typeIdentifier.GetTypeName(false), internalClass);
}

b3d::MonoClass* MonoManager::FindClass(const String& ns, const String& typeName)
{
	MonoClass* monoClass = nullptr;
	for(auto& assembly : mAssemblies)
	{
		monoClass = assembly.second->GetClass(ns, typeName);
		if(monoClass != nullptr)
			return monoClass;
	}

	return nullptr;
}

b3d::MonoClass* MonoManager::FindClass(::MonoClass* rawMonoClass)
{
	MonoClass* monoClass = nullptr;
	for(auto& assembly : mAssemblies)
	{
		monoClass = assembly.second->GetClass(rawMonoClass);
		if(monoClass != nullptr)
			return monoClass;
	}

	return nullptr;
}

void MonoManager::UnloadScriptDomain()
{
#if B3D_USE_DOTNETCORE
	if(mScriptDomain != nullptr)
	{
		mono_domain_finalize(mScriptDomain, ~0u);
		mScriptDomain = nullptr;
	}
#else
	if(mScriptDomain != nullptr)
	{
		mono_domain_set(mono_get_root_domain(), true);

		MonoObject* exception = nullptr;
		mono_domain_try_unload(mScriptDomain, &exception);

		if(exception != nullptr)
			MonoUtil::ThrowIfException(exception);

		mScriptDomain = nullptr;
	}
#endif

	for(auto& assemblyEntry : mAssemblies)
	{
		assemblyEntry.second->Unload();

		// "corlib" assembly persists domain unload since it's in the root domain. However we make sure to clear its
		// class list as it could contain generic instances that use types from other assemblies.
		if(assemblyEntry.first != "corlib")
			B3DDelete(assemblyEntry.second);

		// Metas hold references to various assembly objects that were just deleted, so clear them
		Vector<RegisteredScriptWrapperTypeInformation>& typeMetas = GetScriptWrapperTypeInformation()[assemblyEntry.first];
		for(auto& entry : typeMetas)
		{
			entry.MetaData->ScriptClass = nullptr;
			entry.MetaData->ScriptObjectWrapperPointerField = nullptr;
		}
	}

	mAssemblies.clear();

	if(mCorlibAssembly != nullptr)
		mAssemblies["corlib"] = mCorlibAssembly;
}

Path MonoManager::GetFrameworkAssembliesFolder() const
{
#if B3D_USE_DOTNETCORE
	return Paths::FindPath(kMonoAssembliesFolder);
#else
	return Paths::FindPath(kMonoVersionData[(int)kMonoVersion].Path);
#endif
}

Path MonoManager::GetMonoEtcFolder() const
{
#if B3D_USE_DOTNETCORE
	return Path::kBlank;
#else
	return Paths::FindPath(kMonoEtcDir);
#endif
}

Path MonoManager::GetCompilerPath() const
{
	Path compilerPath = Paths::FindPath(kMonoCompilerDir);
	compilerPath.Append("mcs.exe");
	return compilerPath;
}

Path MonoManager::GetMonoExecPath() const
{
	Path path = Paths::GetBinariesPath();

#if B3D_PLATFORM_WIN32
	path.Append("MonoExec.exe");
#else
	path.append("MonoExec");
#endif

	return path;
}
