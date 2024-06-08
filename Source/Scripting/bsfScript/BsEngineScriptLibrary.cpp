//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsEngineScriptLibrary.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptGameObjectManager.h"
#include "BsManagedResourceManager.h"
#include "Script/BsScriptManager.h"
#include "Wrappers/BsScriptInput.h"
#include "Wrappers/BsScriptVirtualInput.h"
#include "BsScriptObjectManager.h"
#include "BsApplication.h"
#include "FileSystem/BsFileSystem.h"
#include "Wrappers/BsScriptDebug.h"
#include "Wrappers/GUI/BsScriptGUI.h"
#include "BsPlayInEditor.h"
#include "Wrappers/BsScriptScene.h"
#include "GUI/BsGUIManager.h"

#include "Serialization/BsBuiltinResourceLookup.h"
#include "Generated/BsBuiltinComponentLookup.generated.h"
#include "Generated/BsBuiltinReflectableTypesLookup.generated.h"

using namespace bs;
void EngineScriptLibrary::Initialize()
{
	Path engineAssemblyPath = GetEngineAssemblyPath();
	const String ASSEMBLY_ENTRY_POINT = "Program::Start";

#if B3D_IS_ENGINE
	MonoManager::StartUp();
	MonoAssembly& engineAssembly = MonoManager::Instance().LoadAssembly(engineAssemblyPath.ToString(), kEngineAssembly);
#endif

	PlayInEditor::StartUp();
	ScriptDebug::StartUp();
	ScriptObjectManager::StartUp();
	ManagedResourceManager::StartUp();
	ScriptAssemblyManager::StartUp();
	ScriptResourceManager::StartUp();
	ScriptGameObjectManager::StartUp();
	ScriptScene::StartUp();
	ScriptInput::StartUp();
	ScriptVirtualInput::StartUp();
	ScriptGUI::StartUp();

	mEngineTypeMappings.Resources = BuiltinResourceTypes::GetEntries();
	mEngineTypeMappings.Components = BuiltinComponent::GetEntries();
	mEngineTypeMappings.ReflectableObjects = BuiltinReflectableTypes::GetEntries();

	ScriptAssemblyManager::Instance().LoadAssemblyInfo(kEngineAssembly, mEngineTypeMappings);

#if B3D_IS_ENGINE
	engineAssembly.Invoke(ASSEMBLY_ENTRY_POINT);
#endif
}

void EngineScriptLibrary::Update()
{
	ScriptScene::Update();
	PlayInEditor::Instance().Update();
	ScriptObjectManager::Instance().Update();
	ScriptGUI::Update();
}

void EngineScriptLibrary::Reload()
{
#if B3D_IS_ENGINE
	Path engineAssemblyPath = GetEngineAssemblyPath();

	// Do a full refresh if we have already loaded script assemblies
	if(mScriptAssembliesLoaded)
	{
		Vector<AssemblyRefreshInfo> assemblies;
		assemblies.push_back(AssemblyRefreshInfo(kEngineAssembly, &engineAssemblyPath, &mEngineTypeMappings));

		Path gameAssemblyPath = GetGameAssemblyPath();
		if(FileSystem::Exists(gameAssemblyPath))
			assemblies.push_back(AssemblyRefreshInfo(kScriptGameAssembly, &gameAssemblyPath, &BuiltinTypeMappings::kEmpty));

		ScriptObjectManager::Instance().RefreshAssemblies(assemblies);
	}
	else // Otherwise just additively load them
	{
		Path gameAssemblyPath = GetGameAssemblyPath();
		if(FileSystem::Exists(gameAssemblyPath))
		{
			MonoManager::Instance().LoadAssembly(gameAssemblyPath.ToString(), kScriptGameAssembly);
			ScriptAssemblyManager::Instance().LoadAssemblyInfo(kScriptGameAssembly, BuiltinTypeMappings());
		}

		mScriptAssembliesLoaded = true;
	}
#endif
}

void EngineScriptLibrary::Destroy()
{
	UnloadAssemblies();
	ShutdownModules();
}

void EngineScriptLibrary::UnloadAssemblies()
{
	ManagedResourceManager::Instance().Clear();
	MonoManager::Instance().UnloadScriptDomain();
	ScriptObjectManager::Instance().ProcessFinalizedObjects();
}

void EngineScriptLibrary::ShutdownModules()
{
	ScriptGUI::ShutDown();
	ScriptVirtualInput::ShutDown();
	ScriptInput::ShutDown();
	ScriptScene::ShutDown();
	ManagedResourceManager::ShutDown();

#if B3D_IS_ENGINE
	MonoManager::ShutDown();
#else
	MonoManager::Instance().unloadAll();
#endif

	ScriptGameObjectManager::ShutDown();
	ScriptResourceManager::ShutDown();
	ScriptAssemblyManager::ShutDown();
	ScriptObjectManager::ShutDown();
	ScriptDebug::ShutDown();
	PlayInEditor::ShutDown();

	// Make sure all GUI elements are actually destroyed
	GUIManager::Instance().ProcessDestroyQueue();
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
