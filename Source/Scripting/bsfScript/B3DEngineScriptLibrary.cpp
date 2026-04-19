//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DEngineScriptLibrary.h"
#include "B3DMonoManager.h"
#include "B3DMonoAssembly.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "B3DScriptResourceManager.h"
#include "Script/B3DScriptManager.h"
#include "B3DScriptObjectManager.h"
#include "B3DApplication.h"
#include "B3DMonoMethod.h"
#include "FileSystem/B3DFileSystem.h"
#include "B3DPlayInEditor.h"
#include "B3DScriptDebug.generated.h"
#include "B3DScriptInput.generated.h"
#include "GUI/B3DGUIManager.h"
#include "B3DScriptVirtualInput.generated.h"

using namespace b3d;
void EngineScriptLibrary::Initialize()
{
	Path engineAssemblyPath = GetEngineAssemblyPath();
	const String ASSEMBLY_ENTRY_POINT = "Program::Start";

#if B3D_IS_ENGINE
	MonoManager::StartUp();
	MonoAssembly& engineAssembly = MonoManager::Instance().LoadAssembly(engineAssemblyPath.ToString(), kEngineAssembly);
#endif

	ScriptDebug::StartUp();
	ScriptObjectManager::StartUp();
	ScriptAssemblyManager::StartUp();
	ScriptResourceManager::StartUp();
	ScriptInput::StartUp();
	ScriptVirtualInput::StartUp();

	ScriptAssemblyManager::Instance().LoadAssemblyInfo(kEngineAssembly);
	LoadMonoTypes();

	mOnAssemblyRefreshAssembliesLoadedConnection = ScriptObjectManager::Instance().OnRefreshAssembliesLoaded.Connect([this]() { LoadMonoTypes(); });
	mOnAssemblyRefreshDoneConnection = ScriptObjectManager::Instance().OnRefreshComplete.Connect([this]() { OnAssemblyRefreshComplete(); });

	TriggerOnInitialize();

#if B3D_IS_ENGINE
	engineAssembly.Invoke(ASSEMBLY_ENTRY_POINT);
#endif
}

void EngineScriptLibrary::Update()
{
	mUpdateMethod->Invoke(nullptr, nullptr);

	ScriptObjectManager::Instance().Update();
}

void EngineScriptLibrary::Reload()
{
#if B3D_IS_ENGINE
	Path engineAssemblyPath = GetEngineAssemblyPath();

	// Do a full refresh if we have already loaded script assemblies
	if(mScriptAssembliesLoaded)
	{
		Vector<AssemblyRefreshInfo> assemblies;
		assemblies.push_back(AssemblyRefreshInfo(kEngineAssembly, &engineAssemblyPath));

		Path gameAssemblyPath = GetGameAssemblyPath();
		if(FileSystem::Exists(gameAssemblyPath))
			assemblies.push_back(AssemblyRefreshInfo(kScriptGameAssembly, &gameAssemblyPath));

		ScriptObjectManager::Instance().RefreshAssemblies(assemblies);
	}
	else // Otherwise just additively load them
	{
		Path gameAssemblyPath = GetGameAssemblyPath();
		if(FileSystem::Exists(gameAssemblyPath))
		{
			MonoManager::Instance().LoadAssembly(gameAssemblyPath.ToString(), kScriptGameAssembly);
			ScriptAssemblyManager::Instance().LoadAssemblyInfo(kScriptGameAssembly);
		}

		mScriptAssembliesLoaded = true;
	}
#endif
}

void EngineScriptLibrary::Destroy()
{
	TriggerOnDestroy();

	UnloadAssemblies();
	ShutdownModules();
}

void EngineScriptLibrary::UnloadAssemblies()
{
	ScriptObjectManager::Instance().OnWillUnloadAssemblies();
	MonoManager::Instance().UnloadScriptDomain();
	ScriptObjectManager::Instance().ProcessFinalizedObjects();
}

void EngineScriptLibrary::ShutdownModules()
{
	ScriptVirtualInput::ShutDown();
	ScriptInput::ShutDown();

#if B3D_IS_ENGINE
	MonoManager::ShutDown();
#else
	MonoManager::Instance().UnloadAll();
#endif

	ScriptResourceManager::ShutDown();
	ScriptAssemblyManager::ShutDown();
	ScriptObjectManager::ShutDown();
	ScriptDebug::ShutDown();

	// Make sure all GUI elements are actually destroyed
	GUIManager::Instance().ProcessDestroyQueue();
}

void EngineScriptLibrary::LoadMonoTypes()
{
	const String engineAssemblyPath = GetEngineAssemblyPath().ToString();
	mEngineAssembly = &MonoManager::Instance().LoadAssembly(engineAssemblyPath, kEngineAssembly);

	MonoClass* const applicationClass = mEngineAssembly->GetClass(kEngineNs, "Application");
	mUpdateMethod = applicationClass->GetMethod("OnUpdate");
}

Path EngineScriptLibrary::GetEngineAssemblyPath() const
{
	Path assemblyPath = GetBuiltinAssemblyFolder();
	assemblyPath.Append(String(kEngineAssembly) + ".dll");

	return assemblyPath;
}

#if B3D_IS_ENGINE
Path EngineScriptLibrary::GetGameAssemblyPath() const
{
	Path assemblyPath = GetScriptAssemblyFolder();
	assemblyPath.Append(String(kScriptGameAssembly) + ".dll");

	return assemblyPath;
}
#endif

Path EngineScriptLibrary::GetBuiltinAssemblyFolder() const
{
	Path releaseAssemblyFolder = GetReleaseAssemblyPath();
	Path debugAssemblyFolder = GetDebugAssemblyPath();

#if B3D_DEBUG == 0
	if(FileSystem::Exists(releaseAssemblyFolder))
		return releaseAssemblyFolder;

	return debugAssemblyFolder;
#else
	if(FileSystem::Exists(debugAssemblyFolder))
		return debugAssemblyFolder;

	return releaseAssemblyFolder;
#endif
}

Path EngineScriptLibrary::GetScriptAssemblyFolder() const
{
	return GetBuiltinAssemblyFolder();
}

const Path& EngineScriptLibrary::GetReleaseAssemblyPath()
{
	static Path path = Paths::FindPath(Paths::kReleaseAssemblyPath);
	return path;
}

const Path& EngineScriptLibrary::GetDebugAssemblyPath()
{
	static Path path = Paths::FindPath(Paths::kDebugAssemblyPath);
	return path;
}

Path EngineScriptLibrary::GetBuiltinAssembliesPath() const
{
	return MonoManager::Instance().GetFrameworkAssembliesFolder();
}

Path EngineScriptLibrary::GetScriptingRuntimePath() const
{
	return MonoManager::Instance().GetMonoEtcFolder();
}

void EngineScriptLibrary::TriggerOnInitialize()
{
	static const String kApplicationOnInitialize = "Application::OnInitialize";
	mEngineAssembly->Invoke(kApplicationOnInitialize);
}

void EngineScriptLibrary::TriggerOnDestroy()
{
	static const String kApplicationOnDestroy = "Application::OnDestroy";
	mEngineAssembly->Invoke(kApplicationOnDestroy);
}

void EngineScriptLibrary::OnAssemblyRefreshComplete()
{
	TriggerOnInitialize();
}
