//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptEditorApplication.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsEditorApplication.h"
#include "BsProjectLibrary.h"
#include "BsProjectResourceMeta.h"
#include "BsPrefab.h"
#include "BsSceneManager.h"
#include "BsEditorWindowManager.h"
#include "BsMainEditorWindow.h"
#include "BsGUIStatusBar.h"
#include "BsScriptEditorTestSuite.h"
#include "BsTestOutput.h"
#include "BsScriptManager.h"
#include "BsGUIMenuBar.h"
#include "BsPlayInEditorManager.h"
#include "BsScriptRenderTarget.h"
#include "BsScriptResourceManager.h"
#include "BsFileSystem.h"
#include "BsScriptPrefab.h"

namespace BansheeEngine
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
		metaData.scriptClass->addInternalCall("Internal_SetStatusScene", &ScriptEditorApplication::internal_SetStatusScene);
		metaData.scriptClass->addInternalCall("Internal_SetStatusProject", &ScriptEditorApplication::internal_SetStatusProject);
		metaData.scriptClass->addInternalCall("Internal_SetStatusCompiling", &ScriptEditorApplication::internal_SetStatusCompiling);
		metaData.scriptClass->addInternalCall("Internal_GetProjectPath", &ScriptEditorApplication::internal_GetProjectPath);
		metaData.scriptClass->addInternalCall("Internal_GetProjectName", &ScriptEditorApplication::internal_GetProjectName);
		metaData.scriptClass->addInternalCall("Internal_GetProjectLoaded", &ScriptEditorApplication::internal_GetProjectLoaded);
		metaData.scriptClass->addInternalCall("Internal_GetCompilerPath", &ScriptEditorApplication::internal_GetCompilerPath);
		metaData.scriptClass->addInternalCall("Internal_GetBuiltinReleaseAssemblyPath", &ScriptEditorApplication::internal_GetBuiltinReleaseAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetBuiltinDebugAssemblyPath", &ScriptEditorApplication::internal_GetBuiltinDebugAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetScriptAssemblyPath", &ScriptEditorApplication::internal_GetScriptAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetFrameworkAssemblyPath", &ScriptEditorApplication::internal_GetFrameworkAssemblyPath);
		metaData.scriptClass->addInternalCall("Internal_GetEngineAssemblyName", &ScriptEditorApplication::internal_GetEngineAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetEditorAssemblyName", &ScriptEditorApplication::internal_GetEditorAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetScriptGameAssemblyName", &ScriptEditorApplication::internal_GetScriptGameAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_GetScriptEditorAssemblyName", &ScriptEditorApplication::internal_GetScriptEditorAssemblyName);
		metaData.scriptClass->addInternalCall("Internal_SaveScene", &ScriptEditorApplication::internal_SaveScene);
		metaData.scriptClass->addInternalCall("Internal_IsValidProject", &ScriptEditorApplication::internal_IsValidProject);
		metaData.scriptClass->addInternalCall("Internal_SaveProject", &ScriptEditorApplication::internal_SaveProject);
		metaData.scriptClass->addInternalCall("Internal_LoadProject", &ScriptEditorApplication::internal_LoadProject);
		metaData.scriptClass->addInternalCall("Internal_UnloadProject", &ScriptEditorApplication::internal_UnloadProject);
		metaData.scriptClass->addInternalCall("Internal_CreateProject", &ScriptEditorApplication::internal_CreateProject);
		metaData.scriptClass->addInternalCall("Internal_ReloadAssemblies", &ScriptEditorApplication::internal_ReloadAssemblies);
		metaData.scriptClass->addInternalCall("Internal_OpenExternally", &ScriptEditorApplication::internal_OpenExternally);
		metaData.scriptClass->addInternalCall("Internal_RunUnitTests", &ScriptEditorApplication::internal_RunUnitTests);
		metaData.scriptClass->addInternalCall("Internal_Quit", &ScriptEditorApplication::internal_Quit);
		metaData.scriptClass->addInternalCall("Internal_ToggleToolbarItem", &ScriptEditorApplication::internal_ToggleToolbarItem);
		metaData.scriptClass->addInternalCall("Internal_GetIsPlaying", &ScriptEditorApplication::internal_GetIsPlaying);
		metaData.scriptClass->addInternalCall("Internal_SetIsPlaying", &ScriptEditorApplication::internal_SetIsPlaying);
		metaData.scriptClass->addInternalCall("Internal_GetIsPaused", &ScriptEditorApplication::internal_GetIsPaused);
		metaData.scriptClass->addInternalCall("Internal_SetIsPaused", &ScriptEditorApplication::internal_SetIsPaused);
		metaData.scriptClass->addInternalCall("Internal_FrameStep", &ScriptEditorApplication::internal_FrameStep);
		metaData.scriptClass->addInternalCall("Internal_SetMainRenderTarget", &ScriptEditorApplication::internal_SetMainRenderTarget);
		metaData.scriptClass->addInternalCall("Internal_HasFocus", &ScriptEditorApplication::internal_HasFocus);

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

		ScriptPrefab* scriptPrefab;
		ScriptResourceManager::instance().getScriptResource(scene, &scriptPrefab, true);

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

	void ScriptEditorApplication::internal_OpenExternally(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);

		PlatformUtility::open(nativePath);
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
			SceneManager::instance().setMainRenderTarget(renderTarget->getNativeValue());
	}

	bool ScriptEditorApplication::internal_HasFocus()
	{
		return EditorWindowManager::instance().hasFocus();
	}
}
