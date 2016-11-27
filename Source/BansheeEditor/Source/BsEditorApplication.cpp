//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorApplication.h"
#include "BsEditorWindowManager.h"
#include "BsEditorWidgetManager.h"
#include "BsMainEditorWindow.h"
#include "BsRenderWindow.h"
#include "BsBuiltinEditorResources.h"
#include "BsUndoRedo.h"
#include "BsFileSerializer.h"
#include "BsFileSystem.h"
#include "BsEditorWidgetLayout.h"
#include "BsScenePicking.h"
#include "BsSelection.h"
#include "BsGizmoManager.h"
#include "BsCodeEditor.h"
#include "BsBuildManager.h"
#include "BsScriptCodeImporter.h"
#include "BsEditorShaderIncludeHandler.h"
#include "BsDropDownWindowManager.h"
#include "BsProjectLibrary.h"
#include "BsProjectSettings.h"
#include "BsEditorSettings.h"
#include "BsScriptManager.h"
#include "BsImporter.h"
#include "BsVirtualInput.h"
#include "BsResources.h"
#include "BsCoreSceneManager.h"
#include "BsSplashScreen.h"
#include "BsDynLib.h"
#include "BsSceneManager.h"

namespace bs
{
	const Path EditorApplication::WIDGET_LAYOUT_PATH = PROJECT_INTERNAL_DIR + L"Layout.asset";
	const Path EditorApplication::BUILD_DATA_PATH = PROJECT_INTERNAL_DIR + L"BuildData.asset";
	const Path EditorApplication::PROJECT_SETTINGS_PATH = PROJECT_INTERNAL_DIR + L"Settings.asset";

	START_UP_DESC createStartupDesc()
	{
		START_UP_DESC startUpDesc;
		startUpDesc.renderAPI = BS_RENDER_API_MODULE;
		startUpDesc.renderer = BS_RENDERER_MODULE;
		startUpDesc.audio = BS_AUDIO_MODULE;
		startUpDesc.physics = BS_PHYSICS_MODULE;
		startUpDesc.input = BS_INPUT_MODULE;

		startUpDesc.primaryWindowDesc.videoMode = VideoMode(1920, 1080);
		startUpDesc.primaryWindowDesc.title = "BansheeEditor";
		startUpDesc.primaryWindowDesc.fullscreen = false;
		startUpDesc.primaryWindowDesc.border = WindowBorder::None;
		startUpDesc.primaryWindowDesc.hideUntilSwap = true;
		startUpDesc.primaryWindowDesc.depthBuffer = false;

		startUpDesc.importers.push_back("BansheeFreeImgImporter");
		startUpDesc.importers.push_back("BansheeFBXImporter");
		startUpDesc.importers.push_back("BansheeFontImporter");
		startUpDesc.importers.push_back("BansheeSL");

		return startUpDesc;
	}

	Path getEditorSettingsPath()
	{
		return Paths::getRuntimeDataPath() + L"Settings.asset";
	}

	EditorApplication::EditorApplication()
		:Application(createStartupDesc()), mIsProjectLoaded(false), mSBansheeEditorPlugin(nullptr)
	{

	}

	EditorApplication::~EditorApplication()
	{
		ProjectLibrary::shutDown();
		BuiltinEditorResources::shutDown();
	}

	void EditorApplication::onStartUp()
	{
		Application::onStartUp();
		SplashScreen::show();

		// In editor we render game on a separate surface, handled in Game window
		SceneManager::instance().setMainRenderTarget(nullptr);

		loadEditorSettings();
		mProjectSettings = bs_shared_ptr_new<ProjectSettings>();

		BuiltinEditorResources::startUp();

		{
			auto inputConfig = VirtualInput::instance().getConfiguration();

			inputConfig->registerButton("Copy", BC_C, ButtonModifier::Ctrl);
			inputConfig->registerButton("Cut", BC_X, ButtonModifier::Ctrl);
			inputConfig->registerButton("Paste", BC_V, ButtonModifier::Ctrl);
		}

		ScriptCodeImporter* scriptCodeImporter = bs_new<ScriptCodeImporter>();
		Importer::instance()._registerAssetImporter(scriptCodeImporter);

		ProjectLibrary::startUp();

		UndoRedo::startUp();
		EditorWindowManager::startUp();
		EditorWidgetManager::startUp();
		DropDownWindowManager::startUp();

		ScenePicking::startUp();
		Selection::startUp();
		GizmoManager::startUp();
		BuildManager::startUp();
		CodeEditorManager::startUp();

		MainEditorWindow::create(getPrimaryWindow());
		ScriptManager::instance().initialize();
	}

	void EditorApplication::onShutDown()
	{
		unloadProject();

		CodeEditorManager::shutDown();
		BuildManager::shutDown();
		GizmoManager::shutDown();
		Selection::shutDown();
		ScenePicking::shutDown();

		saveEditorSettings();

		DropDownWindowManager::shutDown();
		EditorWidgetManager::shutDown();
		EditorWindowManager::shutDown();
		UndoRedo::shutDown();

		Application::onShutDown();
	}

	void EditorApplication::loadScriptSystem()
	{
		loadPlugin("BansheeMono", &mMonoPlugin);
		loadPlugin("SBansheeEngine", &mSBansheeEnginePlugin);
		loadPlugin("SBansheeEditor", &mSBansheeEditorPlugin);
	}

	void EditorApplication::startUp()
	{
		CoreApplication::startUp<EditorApplication>();
	}

	void EditorApplication::preUpdate()
	{
		Application::preUpdate();

		EditorWidgetManager::instance().update();
		DropDownWindowManager::instance().update();
	}

	void EditorApplication::postUpdate()
	{
		// Call update on editor widgets before parent's postUpdate because the parent will render the GUI and we need
		// to ensure editor widget's GUI is updated.
		EditorWindowManager::instance().update();

		Application::postUpdate();

		SplashScreen::hide();
		setFPSLimit(mEditorSettings->getFPSLimit());
	}

	void EditorApplication::quitRequested()
	{
		typedef void(*QuitRequestedFunc)();

		QuitRequestedFunc quitRequestedCall = (QuitRequestedFunc)mSBansheeEditorPlugin->getSymbol("quitRequested");

		if (quitRequestedCall != nullptr)
			quitRequestedCall();
	}

	Path EditorApplication::getEditorAssemblyPath() const
	{
		Path assemblyPath = getBuiltinAssemblyFolder();
		assemblyPath.append(toWString(EDITOR_ASSEMBLY) + L".dll");

		return assemblyPath;
	}

	Path EditorApplication::getEditorScriptAssemblyPath() const
	{
		Path assemblyPath = getScriptAssemblyFolder();
		assemblyPath.append(toWString(SCRIPT_EDITOR_ASSEMBLY) + L".dll");

		return assemblyPath;
	}

	Path EditorApplication::getScriptAssemblyFolder() const
	{
		if (!isProjectLoaded())
			return Path::BLANK;

		Path assemblyFolder = getProjectPath();
		assemblyFolder.append(INTERNAL_ASSEMBLY_PATH);

		return assemblyFolder;
	}

	void EditorApplication::saveProject()
	{
		if (!isProjectLoaded())
			return;

		Path buildDataPath = getProjectPath();
		buildDataPath.append(BUILD_DATA_PATH);

		BuildManager::instance().save(buildDataPath);
		saveWidgetLayout(EditorWidgetManager::instance().getLayout());
		saveEditorSettings();
		saveProjectSettings();

		gProjectLibrary().saveLibrary();
	}

	void EditorApplication::unloadProject()
	{
		if (!isProjectLoaded())
			return;

		saveProject();

		mProjectSettings = bs_shared_ptr_new<ProjectSettings>();
		BuildManager::instance().clear();
		UndoRedo::instance().clear();

		EditorWidgetManager::instance().closeAll();
		gProjectLibrary().unloadLibrary();
		Resources::instance().unloadAllUnused();
		gCoreSceneManager().clearScene();

		mProjectPath = Path::BLANK;
		mProjectName = StringUtil::WBLANK;
		mIsProjectLoaded = false;
	}

	void EditorApplication::loadProject(const Path& projectPath)
	{
		unloadProject();

		mProjectPath = projectPath;
		mProjectName = projectPath.getWTail();
		mIsProjectLoaded = true;

		loadProjectSettings();

		Path buildDataPath = getProjectPath();
		buildDataPath.append(BUILD_DATA_PATH);

		BuildManager::instance().load(buildDataPath);
		gProjectLibrary().loadLibrary();

		// Do this before restoring windows to ensure types are loaded
		ScriptManager::instance().reload();
		
		SPtr<EditorWidgetLayout> layout = loadWidgetLayout();
		if (layout != nullptr)
			EditorWidgetManager::instance().setLayout(layout);
	}

	void EditorApplication::createProject(const Path& path)
	{
		Path resourceDir = Path::combine(path, ProjectLibrary::RESOURCES_DIR);
		Path internalResourcesDir = Path::combine(path, ProjectLibrary::INTERNAL_RESOURCES_DIR);

		if (!FileSystem::exists(resourceDir))
			FileSystem::createDir(resourceDir);

		if (!FileSystem::exists(internalResourcesDir))
			FileSystem::createDir(internalResourcesDir);

		saveDefaultWidgetLayout(path);
	}

	bool EditorApplication::isValidProjectPath(const Path& path)
	{
		if (!path.isAbsolute())
			return false;

		if (!FileSystem::isDirectory(path))
			return false;

		Path resourceDir = Path::combine(path, ProjectLibrary::RESOURCES_DIR);
		if (!FileSystem::exists(resourceDir))
			return false;

		return true;
	}

	SPtr<EditorWidgetLayout> EditorApplication::loadWidgetLayout()
	{
		Path layoutPath = getProjectPath();
		layoutPath.append(WIDGET_LAYOUT_PATH);

		if (!FileSystem::exists(layoutPath))
			saveDefaultWidgetLayout(getProjectPath());

		if(FileSystem::exists(layoutPath))
		{
			FileDecoder fs(layoutPath);
			return std::static_pointer_cast<EditorWidgetLayout>(fs.decode());
		}

		return nullptr;
	}

	void EditorApplication::saveWidgetLayout(const SPtr<EditorWidgetLayout>& layout)
	{
		Path layoutPath = getProjectPath();
		layoutPath.append(WIDGET_LAYOUT_PATH);

		FileEncoder fs(layoutPath);
		fs.encode(layout.get());
	}

	void EditorApplication::saveDefaultWidgetLayout(const Path& folder)
	{
		Path internalResourcesDir = Path::combine(folder, ProjectLibrary::INTERNAL_RESOURCES_DIR);

		if (!FileSystem::exists(internalResourcesDir))
			FileSystem::createDir(internalResourcesDir);

		Path defaultLayoutPath = BuiltinEditorResources::getDefaultWidgetLayoutPath();

		if (FileSystem::exists(defaultLayoutPath))
		{
			Path projectLayoutPath = Path::combine(folder, WIDGET_LAYOUT_PATH);
			FileSystem::copy(defaultLayoutPath, projectLayoutPath, false);
		}
	}

	void EditorApplication::loadEditorSettings()
	{
		Path settingsPath = getEditorSettingsPath();

		if (FileSystem::exists(settingsPath))
		{
			FileDecoder fs(settingsPath);
			mEditorSettings = std::static_pointer_cast<EditorSettings>(fs.decode());
		}
		
		if (mEditorSettings == nullptr)
			mEditorSettings = bs_shared_ptr_new<EditorSettings>();
	}

	void EditorApplication::saveEditorSettings()
	{
		if (mEditorSettings == nullptr)
			return;

		Path settingsPath = getEditorSettingsPath();

		FileEncoder fs(settingsPath);
		fs.encode(mEditorSettings.get());
	}

	void EditorApplication::loadProjectSettings()
	{
		if (isProjectLoaded())
		{
			Path absoluteDataPath = getProjectPath();
			absoluteDataPath.append(PROJECT_SETTINGS_PATH);

			if (FileSystem::exists(absoluteDataPath))
			{
				FileDecoder fs(absoluteDataPath);
				mProjectSettings = std::static_pointer_cast<ProjectSettings>(fs.decode());
			}
		}

		if (mProjectSettings == nullptr)
			mProjectSettings = bs_shared_ptr_new<ProjectSettings>();
	}

	void EditorApplication::saveProjectSettings()
	{
		if (mProjectSettings == nullptr || !isProjectLoaded())
			return;

		Path absoluteDataPath = getProjectPath();
		absoluteDataPath.append(PROJECT_SETTINGS_PATH);

		FileEncoder fs(absoluteDataPath);
		fs.encode(mProjectSettings.get());
	}

	SPtr<IShaderIncludeHandler> EditorApplication::getShaderIncludeHandler() const
	{
		return bs_shared_ptr_new<EditorShaderIncludeHandler>();
	}

	EditorApplication& gEditorApplication()
	{
		return static_cast<EditorApplication&>(EditorApplication::instance());
	}
}