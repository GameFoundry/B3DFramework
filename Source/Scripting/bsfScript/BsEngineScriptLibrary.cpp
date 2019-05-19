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
#include "Resources/BsGameResourceManager.h"
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

namespace bs
{
	void EngineScriptLibrary::initialize()
	{
		Path engineAssemblyPath = getEngineAssemblyPath();
		const String ASSEMBLY_ENTRY_POINT = "Program::Start";

#if BS_IS_BANSHEE3D
		MonoManager::startUp();
		MonoAssembly& engineAssembly = MonoManager::instance().loadAssembly(engineAssemblyPath.toString(), ENGINE_ASSEMBLY);
#endif

		PlayInEditor::startUp();
		ScriptDebug::startUp();
		GameResourceManager::startUp();
		ScriptObjectManager::startUp();
		ManagedResourceManager::startUp();
		ScriptAssemblyManager::startUp();
		ScriptResourceManager::startUp();
		ScriptGameObjectManager::startUp();
		ScriptScene::startUp();
		ScriptInput::startUp();
		ScriptVirtualInput::startUp();
		ScriptGUI::startUp();

		mEngineTypeMappings.resources = BuiltinResourceTypes::getEntries();
		mEngineTypeMappings.components = BuiltinComponent::getEntries();
		mEngineTypeMappings.reflectableObjects = BuiltinReflectableTypes::getEntries();

		ScriptAssemblyManager::instance().loadAssemblyInfo(ENGINE_ASSEMBLY, mEngineTypeMappings);

#if BS_IS_BANSHEE3D
		engineAssembly.invoke(ASSEMBLY_ENTRY_POINT);
#endif
	}

	void EngineScriptLibrary::update()
	{
		ScriptScene::update();
		PlayInEditor::instance().update();
		ScriptObjectManager::instance().update();
		ScriptGUI::update();
	}

	void EngineScriptLibrary::reload()
	{
#if BS_IS_BANSHEE3D
		Path engineAssemblyPath = getEngineAssemblyPath();

		// Do a full refresh if we have already loaded script assemblies
		if (mScriptAssembliesLoaded)
		{
			Vector<AssemblyRefreshInfo> assemblies;
			assemblies.push_back(AssemblyRefreshInfo(ENGINE_ASSEMBLY, &engineAssemblyPath, &mEngineTypeMappings));

			Path gameAssemblyPath = getGameAssemblyPath();
			if (FileSystem::exists(gameAssemblyPath))
				assemblies.push_back(AssemblyRefreshInfo(SCRIPT_GAME_ASSEMBLY, &gameAssemblyPath, &BuiltinTypeMappings::EMPTY));

			ScriptObjectManager::instance().refreshAssemblies(assemblies);
		}
		else // Otherwise just additively load them
		{
			Path gameAssemblyPath = getGameAssemblyPath();
			if (FileSystem::exists(gameAssemblyPath))
			{
				MonoManager::instance().loadAssembly(gameAssemblyPath.toString(), SCRIPT_GAME_ASSEMBLY);
				ScriptAssemblyManager::instance().loadAssemblyInfo(SCRIPT_GAME_ASSEMBLY, BuiltinTypeMappings());
			}

			mScriptAssembliesLoaded = true;
		}
#endif
	}

	void EngineScriptLibrary::destroy()
	{
		unloadAssemblies();
		shutdownModules();
	}

	void EngineScriptLibrary::unloadAssemblies()
	{
		ManagedResourceManager::instance().clear();
		MonoManager::instance().unloadScriptDomain();
		ScriptObjectManager::instance().processFinalizedObjects();
	}

	void EngineScriptLibrary::shutdownModules()
	{
		ScriptGUI::shutDown();
		ScriptVirtualInput::shutDown();
		ScriptInput::shutDown();
		ScriptScene::shutDown();
		ManagedResourceManager::shutDown();

#if BS_IS_BANSHEE3D
		MonoManager::shutDown();
#else
		MonoManager::instance().unloadAll();
#endif

		ScriptGameObjectManager::shutDown();
		ScriptResourceManager::shutDown();
		ScriptAssemblyManager::shutDown();
		ScriptObjectManager::shutDown();
		GameResourceManager::shutDown();
		ScriptDebug::shutDown();
		PlayInEditor::shutDown();

		// Make sure all GUI elements are actually destroyed
		GUIManager::instance().processDestroyQueue();
	}

	Path EngineScriptLibrary::getEngineAssemblyPath() const
	{
		Path assemblyPath = getBuiltinAssemblyFolder();
		assemblyPath.append(String(ENGINE_ASSEMBLY) + ".dll");

		return assemblyPath;
	}

#if BS_IS_BANSHEE3D
	Path EngineScriptLibrary::getGameAssemblyPath() const
	{
		Path assemblyPath = getScriptAssemblyFolder();
		assemblyPath.append(String(SCRIPT_GAME_ASSEMBLY) + ".dll");

		return assemblyPath;
	}
#endif

	Path EngineScriptLibrary::getBuiltinAssemblyFolder() const
	{
		Path releaseAssemblyFolder = getReleaseAssemblyPath();
		Path debugAssemblyFolder = getDebugAssemblyPath();

#if BS_DEBUG_MODE == 0
		if (FileSystem::exists(releaseAssemblyFolder))
			return releaseAssemblyFolder;

		return debugAssemblyFolder;
#else
		if (FileSystem::exists(debugAssemblyFolder))
			return debugAssemblyFolder;

		return releaseAssemblyFolder;
#endif
	}

	Path EngineScriptLibrary::getScriptAssemblyFolder() const
	{
		return getBuiltinAssemblyFolder();
	}

	const Path& EngineScriptLibrary::getReleaseAssemblyPath()
	{
		static Path path = Paths::findPath(Paths::RELEASE_ASSEMBLY_PATH);
		return path;
	}

	const Path& EngineScriptLibrary::getDebugAssemblyPath()
	{
		static Path path = Paths::findPath(Paths::DEBUG_ASSEMBLY_PATH);
		return path;
	}
}
