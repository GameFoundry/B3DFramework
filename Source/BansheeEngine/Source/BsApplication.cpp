//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsApplication.h"
#include "BsGUIManager.h"
#include "BsSpriteManager.h"
#include "BsBuiltinResources.h"
#include "BsScriptManager.h"
#include "BsProfilingManager.h"
#include "BsVirtualInput.h"
#include "BsSceneManager.h"
#include "BsSceneObject.h"
#include "BsCursor.h"
#include "BsCoreThread.h"
#include "BsFileSystem.h"
#include "BsPlainTextImporter.h"
#include "BsImporter.h"
#include "BsShortcutManager.h"
#include "BsCoreObjectManager.h"
#include "BsRendererManager.h"
#include "BsRendererMaterialManager.h"
#include "BsPlatform.h"
#include "BsEngineShaderIncludeHandler.h"

namespace bs
{
	Application::Application(const START_UP_DESC& desc)
		: CoreApplication(desc), mMonoPlugin(nullptr), mSBansheeEnginePlugin(nullptr)
	{

	}

	Application::~Application()
	{
		// Cleanup any new objects queued for destruction by unloaded scripts
		CoreObjectManager::instance().syncToCore(gCoreAccessor());
		gCoreThread().update();
		gCoreThread().submitAccessors(true);

		Cursor::shutDown();

		ShortcutManager::shutDown();
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

		ScriptManager::startUp();
		loadScriptSystem();
	}

	void Application::onShutDown()
	{
		// Need to clear all objects before I unload any plugins, as they
		// could have allocated parts or all of those objects.
		SceneManager::instance().clearScene(true);

		ScriptManager::shutDown();
		unloadScriptSystem();

		CoreApplication::onShutDown();
	}

	void Application::startUp(VideoMode videoMode, const String& title, bool fullscreen,
						std::function<void()> updateCallback)
	{
		START_UP_DESC desc;

		// Set up default plugins
		desc.renderAPI = BS_RENDER_API_MODULE;
		desc.renderer = BS_RENDERER_MODULE;
		desc.audio = BS_AUDIO_MODULE;
		desc.physics = BS_PHYSICS_MODULE;
		desc.input = BS_INPUT_MODULE;

		desc.importers.push_back("BansheeFreeImgImporter");
		desc.importers.push_back("BansheeFBXImporter");
		desc.importers.push_back("BansheeFontImporter");
		desc.importers.push_back("BansheeSL");

		desc.primaryWindowDesc.videoMode = videoMode;
		desc.primaryWindowDesc.fullscreen = fullscreen;
		desc.primaryWindowDesc.title = title;

		desc.updateCallback = updateCallback;

		startUp(desc);
	}

	void Application::startUp(const START_UP_DESC& desc)
	{
		CoreApplication::startUp<Application>(desc);
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
		unloadPlugin(mSBansheeEnginePlugin);
		unloadPlugin(mMonoPlugin);
	}

	void Application::startUpRenderer()
	{
		// Do nothing, we activate the renderer at a later stage
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