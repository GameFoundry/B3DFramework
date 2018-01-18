//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsApplication.h"
#include "GUI/BsGUIManager.h"
#include "2D/BsSpriteManager.h"
#include "Resources/BsBuiltinResources.h"
#include "Script/BsScriptManager.h"
#include "Profiling/BsProfilingManager.h"
#include "Input/BsVirtualInput.h"
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "Platform/BsCursor.h"
#include "CoreThread/BsCoreThread.h"
#include "FileSystem/BsFileSystem.h"
#include "Resources/BsPlainTextImporter.h"
#include "Importer/BsImporter.h"
#include "GUI/BsShortcutManager.h"
#include "CoreThread/BsCoreObjectManager.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRendererMaterialManager.h"
#include "Debug/BsDebugDraw.h"
#include "Platform/BsPlatform.h"
#include "Resources/BsEngineShaderIncludeHandler.h"
#include "Resources/BsResources.h"
#include "BsEngineConfig.h"

namespace bs
{
	Application::Application(const START_UP_DESC& desc)
		: CoreApplication(desc), mMonoPlugin(nullptr), mSBansheeEnginePlugin(nullptr)
	{

	}

	Application::~Application()
	{
		// Cleanup any new objects queued for destruction by unloaded scripts
		CoreObjectManager::instance().syncToCore();
		gCoreThread().update();
		gCoreThread().submitAll(true);

		Cursor::shutDown();

		GUIManager::shutDown();
		SpriteManager::shutDown();
		BuiltinResources::shutDown();
		RendererMaterialManager::shutDown();
		VirtualInput::shutDown();
	}

	void Application::onStartUp()
	{
		CoreApplication::onStartUp();

		PlainTextImporter* importer = bs_new<PlainTextImporter>();
		Importer::instance()._registerAssetImporter(importer);

		VirtualInput::startUp();
		BuiltinResources::startUp();
		RendererMaterialManager::startUp();
		RendererManager::instance().initialize();
		SpriteManager::startUp();
		GUIManager::startUp();
		ShortcutManager::startUp();

		Cursor::startUp();
		Cursor::instance().setCursor(CursorType::Arrow);
		Platform::setIcon(BuiltinResources::instance().getBansheeIcon());

		SceneManager::instance().setMainRenderTarget(getPrimaryWindow());
		DebugDraw::startUp();

		ScriptManager::startUp();

		if(mStartUpDesc.scripting)
			loadScriptSystem();
	}

	void Application::onShutDown()
	{
		// Need to clear all objects before I unload any plugins, as they
		// could have allocated parts or all of those objects.
		SceneManager::instance().clearScene(true);

		// Resources too (Prefabs especially, since they hold the same data as a scene)
		Resources::instance().unloadAll();

		// Shut down before script manager as scripts could have registered shortcut callbacks
		ShortcutManager::shutDown();

		ScriptManager::shutDown();
		DebugDraw::shutDown();

		if (mStartUpDesc.scripting)
			unloadScriptSystem();

		CoreApplication::onShutDown();
	}

	void Application::preUpdate()
	{
		CoreApplication::preUpdate();

		VirtualInput::instance()._update();
	}

	void Application::postUpdate()
	{
		CoreApplication::postUpdate();

		PROFILE_CALL(GUIManager::instance().update(), "GUI");
		DebugDraw::instance()._update();
	}

	void Application::loadScriptSystem()
	{
		loadPlugin("BansheeMono", &mMonoPlugin);
		loadPlugin("SBansheeEngine", &mSBansheeEnginePlugin); 

		ScriptManager::instance().initialize();
	}

	void Application::unloadScriptSystem()
	{
		// These plugins must be unloaded before any other script plugins, because
		// they will cause finalizers to trigger and various modules those finalizers
		// might reference must still be active
		if(mSBansheeEnginePlugin != nullptr)
			unloadPlugin(mSBansheeEnginePlugin);

		if(mMonoPlugin != nullptr)
			unloadPlugin(mMonoPlugin);
	}

	void Application::startUpRenderer()
	{
		// Do nothing, we activate the renderer at a later stage
	}

	START_UP_DESC Application::buildStartUpDesc(VideoMode videoMode, const String& title, bool fullscreen)
	{
		START_UP_DESC desc;

		// Set up default plugins
		desc.renderAPI = BS_RENDER_API_MODULE;
		desc.renderer = BS_RENDERER_MODULE;
		desc.audio = BS_AUDIO_MODULE;
		desc.physics = BS_PHYSICS_MODULE;
		desc.scripting = false;

		desc.importers.push_back("BansheeFreeImgImporter");
		desc.importers.push_back("BansheeFBXImporter");
		desc.importers.push_back("BansheeFontImporter");
		desc.importers.push_back("BansheeSL");

		desc.primaryWindowDesc.videoMode = videoMode;
		desc.primaryWindowDesc.fullscreen = fullscreen;
		desc.primaryWindowDesc.title = title;

		return desc;
	}

	Path Application::getEngineAssemblyPath() const
	{
		Path assemblyPath = getBuiltinAssemblyFolder();
		assemblyPath.append(toWString(String(ENGINE_ASSEMBLY)) + L".dll");
		
		return assemblyPath;
	}

	Path Application::getGameAssemblyPath() const
	{
		Path assemblyPath = getScriptAssemblyFolder();
		assemblyPath.append(toWString(String(SCRIPT_GAME_ASSEMBLY)) + L".dll");

		return assemblyPath;
	}

	Path Application::getBuiltinAssemblyFolder() const
	{
		Path releaseAssemblyFolder = Paths::getReleaseAssemblyPath();
		Path debugAssemblyFolder = Paths::getDebugAssemblyPath();

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

	Path Application::getScriptAssemblyFolder() const
	{
		return getBuiltinAssemblyFolder();
	}

	SPtr<IShaderIncludeHandler> Application::getShaderIncludeHandler() const
	{
		return bs_shared_ptr_new<EngineShaderIncludeHandler>();
	}

	Application& gApplication()
	{
		return static_cast<Application&>(Application::instance());
	}
}
