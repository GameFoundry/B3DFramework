//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
#include "BsPlayInEditorManager.h"
#include "Wrappers/BsScriptScene.h"
#include "GUI/BsGUIManager.h"

namespace bs
{
	EngineScriptLibrary::EngineScriptLibrary()
		:mScriptAssembliesLoaded(false)
	{ }

	void EngineScriptLibrary::initialize()
	{
		Path engineAssemblyPath = gApplication().getEngineAssemblyPath();
		const String ASSEMBLY_ENTRY_POINT = "Program::Start";

		MonoManager::startUp();
		MonoAssembly& bansheeEngineAssembly = MonoManager::instance().loadAssembly(engineAssemblyPath.toWString(), ENGINE_ASSEMBLY);

		PlayInEditorManager::startUp();
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

		ScriptAssemblyManager::instance().loadAssemblyInfo(ENGINE_ASSEMBLY);

		Path gameAssemblyPath = gApplication().getGameAssemblyPath();
		if (FileSystem::exists(gameAssemblyPath))
		{
			MonoManager::instance().loadAssembly(gameAssemblyPath.toWString(), SCRIPT_GAME_ASSEMBLY);
			ScriptAssemblyManager::instance().loadAssemblyInfo(SCRIPT_GAME_ASSEMBLY);
		}

		bansheeEngineAssembly.invoke(ASSEMBLY_ENTRY_POINT);
	}

	void EngineScriptLibrary::reload()
	{
		Path engineAssemblyPath = gApplication().getEngineAssemblyPath();
		Path gameAssemblyPath = gApplication().getGameAssemblyPath();

		// Do a full refresh if we have already loaded script assemblies
		if (mScriptAssembliesLoaded)
		{
			Vector<std::pair<String, Path>> assemblies;
			assemblies.push_back({ ENGINE_ASSEMBLY, engineAssemblyPath });

			if (FileSystem::exists(gameAssemblyPath))
				assemblies.push_back({ SCRIPT_GAME_ASSEMBLY, gameAssemblyPath });

			ScriptObjectManager::instance().refreshAssemblies(assemblies);
		}
		else // Otherwise just additively load them
		{
			MonoManager::instance().loadAssembly(engineAssemblyPath.toWString(), ENGINE_ASSEMBLY);
			ScriptAssemblyManager::instance().loadAssemblyInfo(ENGINE_ASSEMBLY);

			if (FileSystem::exists(gameAssemblyPath))
			{
				MonoManager::instance().loadAssembly(gameAssemblyPath.toWString(), SCRIPT_GAME_ASSEMBLY);
				ScriptAssemblyManager::instance().loadAssemblyInfo(SCRIPT_GAME_ASSEMBLY);
			}

			mScriptAssembliesLoaded = true;
		}
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
		MonoManager::shutDown();
		ScriptGameObjectManager::shutDown();
		ScriptResourceManager::shutDown();
		ScriptAssemblyManager::shutDown();
		ScriptObjectManager::shutDown();
		GameResourceManager::shutDown();
		ScriptDebug::shutDown();
		PlayInEditorManager::shutDown();

		// Make sure all GUI elements are actually destroyed
		GUIManager::instance().processDestroyQueue();
	}
}
