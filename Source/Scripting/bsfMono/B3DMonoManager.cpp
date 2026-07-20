//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMonoManager.h"
#include "B3DScriptTypeMetaData.h"
#include "B3DMonoAssembly.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "FileSystem/B3DFileSystem.h"

#include "B3DMonoLoader.h"
#include "B3DMonoGCRegion.h"

using namespace b3d;

#ifndef B3D_MONO_CHECKED_RUNTIME
#define B3D_MONO_CHECKED_RUNTIME 0
#endif

const String kMonoCompilerDir = "bin/Mono/compiler/";

#if !B3D_MONO_AOT
// Flat (config-independent) assemblies folder used in JIT mode. Under B3D_MONO_AOT the assemblies are in different folders depending on the config.
static const char* kMonoAssembliesFolder = "bin/Assemblies/";
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
	MonoLoader::StartUp();

	LoadMonoLibrary();
}

MonoManager::~MonoManager()
{
	UnloadMonoLibrary();

	// Make sure to explicitly clear this meta-data, as it contains structures allocated from other dynamic libraries,
	// which will likely get unloaded right after shutdown
	GetScriptWrapperTypeInformation().clear();

	MonoLoader::ShutDown();
}

void MonoManager::LoadMonoLibrary()
{
	// We don't include 'System.Globalization.Native' library by default. This prevents runtime trying to load it.
	B3D_SETENV("DOTNET_SYSTEM_GLOBALIZATION_INVARIANT", "1");

	// Disable the .NET diagnostics server as it produces warning when performing hot-reload.
	B3D_SETENV("DOTNET_EnableDiagnostics", "0");

	MonoLoader::Instance().Load();

	const Path assembliesFolder = GetFrameworkAssembliesFolder();
	mono_set_assemblies_path(assembliesFolder.ToString().c_str());

#if B3D_BUILD_TYPE_DEVELOPMENT
#if B3D_DEBUG
	// Note: For proper debugging experience make sure to open a console window to display stdout and stderr, as Mono
	// uses them for much of its logging.
	mono_debug_init(MONO_DEBUG_FORMAT_MONO);
#endif

	// The managed soft-debugger. 
	Vector<const char*> options = {
		"--soft-breakpoints",
		"--debugger-agent=transport=dt_socket,address=127.0.0.1:17615,embedding=1,server=y,suspend=n",
	};

#if B3D_MONO_CHECKED_RUNTIME && B3D_DEBUG
	// check-remset-consistency: Makes sure that write barriers are properly issued in native code, and therefore
	//    all old->new generation references are properly present in the remset. This is easy to mess up in native
	//    code by performing a simple memory copy without a barrier, so it's important to keep the option on.
	// verify-before-collections: Unusure what exactly it does, but it sounds like it could help track down
	//    things like accessing released/moved objects, or attempting to release handles for an unloaded domain.
	options.push_back("--gc-debug=check-remset-consistency,verify-before-collections");
#endif

	mono_jit_parse_options((int)options.size(), const_cast<char**>(options.data()));
	mono_trace_set_level_string("warning"); // Note: Switch to "debug" for detailed output, disabled for now due to spam
#else
	mono_trace_set_level_string("warning");
#endif

	mono_trace_set_log_handler(MonoLogCallback, this);
	mono_trace_set_print_handler(MonoPrintCallback);
	mono_trace_set_printerr_handler(MonoPrintErrorCallback);

	mono_config_parse(nullptr);

#if B3D_MONO_AOT
	mono_jit_set_aot_mode(MONO_AOT_MODE_INTERP);
#endif

	mScriptDomain = mRootDomain = mono_jit_init("bsfMono");

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

	MonoLoader::Instance().Unload();
}

b3d::MonoAssembly& MonoManager::LoadAssembly(const Path& path, const String& name)
{
	MonoAssembly* assembly = nullptr;

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
	if(mScriptDomain != nullptr)
	{
		// mono_domain_finalize does a cooperative blocking wait on the finalizer thread, which requires the calling
		// thread to be in GC-Unsafe (managed-running) state, otherwise Release coreclr errors.
		ScopedGCUnsafeRegion gcUnsafe;
		mono_domain_finalize(mScriptDomain, ~0u);

		mScriptDomain = nullptr;
	}

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
#if B3D_MONO_AOT
	// AOT libraries in are in bin/Assemblies/<Config>/ folder
	const Path releaseFolder = Paths::FindPath(Paths::kReleaseAssemblyPath);
	const Path debugFolder = Paths::FindPath(Paths::kDebugAssemblyPath);

#if B3D_DEBUG
	if(FileSystem::Exists(debugFolder))
		return debugFolder;

	return releaseFolder;
#else
	if(FileSystem::Exists(releaseFolder))
		return releaseFolder;

	return debugFolder;
#endif
#else
	return Paths::FindPath(kMonoAssembliesFolder);
#endif
}

Path MonoManager::GetMonoEtcFolder() const
{
	return Path::kBlank;
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
	path.Append("MonoExec");
#endif

	return path;
}
