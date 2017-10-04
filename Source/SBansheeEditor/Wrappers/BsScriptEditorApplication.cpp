//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptEditorApplication.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsEditorApplication.h"
#include "Library/BsProjectLibrary.h"
#include "Library/BsProjectResourceMeta.h"
#include "Scene/BsPrefab.h"
#include "Scene/BsSceneManager.h"
#include "EditorWindow/BsEditorWindowManager.h"
#include "EditorWindow/BsMainEditorWindow.h"
#include "GUI/BsGUIStatusBar.h"
#include "Wrappers/BsScriptEditorTestSuite.h"
#include "Testing/BsTestOutput.h"
#include "Script/BsScriptManager.h"
#include "GUI/BsGUIMenuBar.h"
#include "BsPlayInEditorManager.h"
#include "Platform/BsPlatform.h"
#include "BsScriptResourceManager.h"
#include "FileSystem/BsFileSystem.h"
#include "Wrappers/BsScriptPrefab.h"

#include "BsScriptRenderTexture.generated.h"

namespace bs
{
	bool ScriptEditorApplication::mRequestProjectLoad = false;
	bool ScriptEditorApplication::mRequestAssemblyReload = false;
	Path ScriptEditorApplication::mProjectLoadPath;
	HEvent ScriptEditorApplication::OnStatusBarClickedConn;

	ScriptEditorApplication::OnProjectLoadedThunkDef ScriptEditorApplication::onProjectLoadedThunk;
	ScriptEditorApplication::OnStatusBarClickedThunkDef ScriptEditorApplication::onStatusBarClickedThunk;

	ScriptEditorApplication::ScriptEditorApplication(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptEditorApplication::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_SetStatusScene", (void*)&ScriptEditorApplication::internal_SetStatusScene);
		metaData.scriptClass->addInternalCall("Internal_SetStatusProject", (void*)&ScriptEditorApplication::internal_SetStatusProject);
		metaData.scriptClass->addInternalCall("Internal_SetStatusCompiling", (void*)&ScriptEditorApplication::internal_SetStatusCompiling);
		metaData.scriptClass->addInternalCall("Internal_GetProjectPath", (void*)&ScriptEditorApplication::internal_GetProjectPath);
		metaData.scriptClass->addInternalCall("Internal_GetProjectName", (void*)&ScriptEditorApplication::internal_GetProjectName);
		metaData.scriptClass->addInternalCall("Internal_GetProjectLoaded", (void*)&ScriptEditorApplication::internal_GetProjectLoaded);
		metaData.scriptClass->addInternalCall("Internal_GetCompilerPath", (void*)&ScriptEditorApplication::internal_GetCompilerPath);
		metaData.scriptClass->addInternalCall("Internal_GetBuiltinReleaseAssemblyPath", (void*)&ScriptEditorApplication::internal_GetBuiltinReleaseAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetBuiltinDebugAssemblyPath", (void*)&ScriptEditorApplication::internal_GetBuiltinDebugAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetScriptAssemblyPath", (void*)&ScriptEditorApplication::internal_GetScriptAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetFrameworkAssemblyPath", (void*)&ScriptEditorApplication::internal_GetFrameworkAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetEngineAssemblyName", (void*)&ScriptEditorApplication::internal_GetEngineAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetEditorAssemblyName", (void*)&ScriptEditorApplication::internal_GetEditorAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetScriptGameAssemblyName", (void*)&ScriptEditorApplication::internal_GetScriptGameAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetScriptEditorAssemblyName", (void*)&ScriptEditorApplication::internal_GetScriptEditorAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_SaveScene", (void*)&ScriptEditorApplication::internal_SaveScene);
		metaData.scriptClass->addInternalCall("Internal_IsValidProject", (void*)&ScriptEditorApplication::internal_IsValidProject);
		metaData.scriptClass->addInternalCall("Internal_SaveProject", (void*)&ScriptEditorApplication::internal_SaveProject);
		metaData.scriptClass->addInternalCall("Internal_LoadProject", (void*)&ScriptEditorApplication::internal_LoadProject);
		metaData.scriptClass->addInternalCall("Internal_UnloadProject", (void*)&ScriptEditorApplication::internal_UnloadProject);
		metaData.scriptClass->addInternalCall("Internal_CreateProject", (void*)&ScriptEditorApplication::internal_CreateProject);
		metaData.scriptClass->addInternalCall("Internal_ReloadAssemblies", (void*)&ScriptEditorApplication::internal_ReloadAssemblies);
		metaData.scriptClass->addInternalCall("Internal_OpenFolder", (void*) &ScriptEditorApplication::internal_OpenFolder);
		metaData.scriptClass->addInternalCall("Internal_RunUnitTests", (void*)&ScriptEditorApplication::internal_RunUnitTests);
		metaData.scriptClass->addInternalCall("Internal_Quit", (void*)&ScriptEditorApplication::internal_Quit);
		metaData.scriptClass->addInternalCall("Internal_ToggleToolbarItem", (void*)&ScriptEditorApplication::internal_ToggleToolbarItem);
		metaData.scriptClass->addInternalCall("Internal_GetIsPlaying", (void*)&ScriptEditorApplication::internal_GetIsPlaying);
		metaData.scriptClass->addInternalCall("Internal_SetIsPlaying", (void*)&ScriptEditorApplication::internal_SetIsPlaying);
		metaData.scriptClass->addInternalCall("Internal_GetIsPaused", (void*)&ScriptEditorApplication::internal_GetIsPaused);
		metaData.scriptClass->addInternalCall("Internal_SetIsPaused", (void*)&ScriptEditorApplication::internal_SetIsPaused);
		metaData.scriptClass->addInternalCall("Internal_FrameStep", (void*)&ScriptEditorApplication::internal_FrameStep);
		metaData.scriptClass->addInternalCall("Internal_SetMainRenderTarget", (void*)&ScriptEditorApplication::internal_SetMainRenderTarget);
		metaData.scriptClass->addInternalCall("Internal_HasFocus", (void*)&ScriptEditorApplication::internal_HasFocus);

		onProjectLoadedThunk = (OnProjectLoadedThunkDef)metaData.scriptClass->getMethod("Internal_OnProjectLoaded")->getThunk();
		onStatusBarClickedThunk = (OnStatusBarClickedThunkDef)metaData.scriptClass->getMethod("Internal_OnStatusBarClicked")->getThunk();
	}

	void ScriptEditorApplication::startUp()
	{
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		OnStatusBarClickedConn = mainWindow->getStatusBar().onMessageClicked.connect(&ScriptEditorApplication::onStatusBarClicked);
	}

	void ScriptEditorApplication::shutDown()
	{
		OnStatusBarClickedConn.disconnect();
	}

	void ScriptEditorApplication::update()
	{
		// Project load must be delayed when requested from managed code because it
		// triggers managed assembly reload, and that can't be performed when called
		// from the Mono thread.
		if (mRequestProjectLoad)
		{
			gEditorApplication().loadProject(mProjectLoadPath);

			mRequestProjectLoad = false;
			mRequestAssemblyReload = false;
			MonoUtil::invokeThunk(onProjectLoadedThunk);
		}
		else if (mRequestAssemblyReload)
		{
			ScriptManager::instance().reload();
			mRequestAssemblyReload = false;
		}
	}

	void ScriptEditorApplication::onStatusBarClicked()
	{
		MonoUtil::invokeThunk(onStatusBarClickedThunk);
	}

	void ScriptEditorApplication::internal_SetStatusScene(MonoString* name, bool modified)
	{
		WString nativeScene = MonoUtil::monoToWString(name);

		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		mainWindow->getStatusBar().setScene(nativeScene, modified);
	}

	void ScriptEditorApplication::internal_SetStatusProject(bool modified)
	{
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();

		if (gEditorApplication().isProjectLoaded())
			mainWindow->getStatusBar().setProject(gEditorApplication().getProjectName(), modified);
		else
			mainWindow->getStatusBar().setProject(L"None", false);
	}

	void ScriptEditorApplication::internal_SetStatusCompiling(bool compiling)
	{
		MainEditorWindow* mainWindow = EditorWindowManager::instance().getMainWindow();
		mainWindow->getStatusBar().setIsCompiling(compiling);
	}

	MonoString* ScriptEditorApplication::internal_GetProjectPath()
	{
		Path projectPath = gEditorApplication().getProjectPath();

		return MonoUtil::wstringToMono(projectPath.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetProjectName()
	{
		WString projectName = gEditorApplication().getProjectName();

		return MonoUtil::wstringToMono(projectName);
	}

	bool ScriptEditorApplication::internal_GetProjectLoaded()
	{
		return gEditorApplication().isProjectLoaded();
	}

	MonoString* ScriptEditorApplication::internal_GetCompilerPath()
	{
		Path compilerPath = MonoManager::instance().getCompilerPath();

		return MonoUtil::wstringToMono(compilerPath.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetBuiltinReleaseAssemblyPath()
	{
		Path releaseAssemblyFolder = Paths::getReleaseAssemblyPath();

		return MonoUtil::wstringToMono(releaseAssemblyFolder.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetBuiltinDebugAssemblyPath()
	{
		Path debugAssemblyFolder = Paths::getDebugAssemblyPath();

		return MonoUtil::wstringToMono(debugAssemblyFolder.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetScriptAssemblyPath()
	{
		Path assemblyFolder = gEditorApplication().getScriptAssemblyFolder();

		return MonoUtil::wstringToMono(assemblyFolder.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetFrameworkAssemblyPath()
	{
		Path assemblyFolder = MonoManager::instance().getFrameworkAssembliesFolder();

		return MonoUtil::wstringToMono(assemblyFolder.toWString());
	}

	MonoString* ScriptEditorApplication::internal_GetEngineAssemblyName()
	{
		return MonoUtil::wstringToMono(toWString(ENGINE_ASSEMBLY) + L".dll");
	}

	MonoString* ScriptEditorApplication::internal_GetEditorAssemblyName()
	{
		return MonoUtil::wstringToMono(toWString(EDITOR_ASSEMBLY) + L".dll");
	}

	MonoString* ScriptEditorApplication::internal_GetScriptGameAssemblyName()
	{
		return MonoUtil::wstringToMono(toWString(SCRIPT_GAME_ASSEMBLY) + L".dll");
	}

	MonoString* ScriptEditorApplication::internal_GetScriptEditorAssemblyName()
	{
		return MonoUtil::wstringToMono(toWString(SCRIPT_EDITOR_ASSEMBLY) + L".dll");
	}

	MonoObject* ScriptEditorApplication::internal_SaveScene(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);
		HSceneObject sceneRoot = gSceneManager().getRootNode();
		
		SPtr<ProjectResourceMeta> resMeta = gProjectLibrary().findResourceMeta(nativePath);
		HPrefab scene;
		if (resMeta != nullptr)
		{
			if (resMeta->getTypeID() != TID_Prefab)
				return nullptr;

			scene = static_resource_cast<Prefab>(gProjectLibrary().load(nativePath));
			scene->update(sceneRoot);

			gProjectLibrary().saveEntry(scene);
		}
		else
		{
			scene = Prefab::create(sceneRoot);
			gProjectLibrary().createEntry(scene, nativePath);
		}

		ScriptResourceBase* scriptPrefab = ScriptResourceManager::instance().getScriptResource(scene, true);
		return scriptPrefab->getManagedInstance();
	}

	bool ScriptEditorApplication::internal_IsValidProject(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);
		return gEditorApplication().isValidProjectPath(nativePath);
	}

	void ScriptEditorApplication::internal_SaveProject()
	{
		gEditorApplication().saveProject();
	}

	void ScriptEditorApplication::internal_LoadProject(MonoString* path)
	{
		mRequestProjectLoad = true;
		mProjectLoadPath = MonoUtil::monoToWString(path);
	}

	void ScriptEditorApplication::internal_UnloadProject()
	{
		gEditorApplication().unloadProject();
	}

	void ScriptEditorApplication::internal_CreateProject(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);

		gEditorApplication().createProject(nativePath);
	}

	void ScriptEditorApplication::internal_ReloadAssemblies()
	{
		mRequestAssemblyReload = true;
	}

	void ScriptEditorApplication::internal_OpenFolder(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);

		Platform::openFolder(nativePath);
	}

	void ScriptEditorApplication::internal_RunUnitTests()
	{
#if BS_DEBUG_MODE
		SPtr<TestSuite> testSuite = TestSuite::create<ScriptEditorTestSuite>();
		ExceptionTestOutput testOutput;
		testSuite->run(testOutput);
#endif
	}

	void ScriptEditorApplication::internal_Quit()
	{
		gApplication().stopMainLoop();
	}

	void ScriptEditorApplication::internal_ToggleToolbarItem(MonoString* name, bool on)
	{
		String nativeName = MonoUtil::monoToString(name);

		MainEditorWindow* editorWindow = EditorWindowManager::instance().getMainWindow();
		editorWindow->getMenuBar().toggleToolbarButton(nativeName, on);
	}

	bool ScriptEditorApplication::internal_GetIsPlaying()
	{
		return PlayInEditorManager::instance().getState() == PlayInEditorState::Playing;
	}

	void ScriptEditorApplication::internal_SetIsPlaying(bool value)
	{
		if (value)
			PlayInEditorManager::instance().setState(PlayInEditorState::Playing);
		else
			PlayInEditorManager::instance().setState(PlayInEditorState::Stopped);
	}

	bool ScriptEditorApplication::internal_GetIsPaused()
	{
		return PlayInEditorManager::instance().getState() == PlayInEditorState::Paused;
	}

	void ScriptEditorApplication::internal_SetIsPaused(bool value)
	{
		if (value)
			PlayInEditorManager::instance().setState(PlayInEditorState::Paused);
		else
		{
			bool isPaused = PlayInEditorManager::instance().getState() == PlayInEditorState::Paused;
			if (isPaused)
				PlayInEditorManager::instance().setState(PlayInEditorState::Playing);
		}
	}

	void ScriptEditorApplication::internal_FrameStep()
	{
		PlayInEditorManager::instance().frameStep();
	}

	void ScriptEditorApplication::internal_SetMainRenderTarget(ScriptRenderTarget* renderTarget)
	{
		if (renderTarget == nullptr)
			SceneManager::instance().setMainRenderTarget(nullptr);
		else
			SceneManager::instance().setMainRenderTarget(renderTarget->getInternal());
	}

	bool ScriptEditorApplication::internal_HasFocus()
	{
		return EditorWindowManager::instance().hasFocus();
	}
}
