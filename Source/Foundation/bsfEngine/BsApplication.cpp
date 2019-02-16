//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
#include "GUI/BsProfilerOverlay.h"

namespace bs
{
	Application::Application(const START_UP_DESC& desc)
		: CoreApplication(desc)
	{ }

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
		Platform::setIcon(BuiltinResources::instance().getFrameworkIcon());

		SceneManager::instance().setMainRenderTarget(getPrimaryWindow());
		DebugDraw::startUp();

		startUpScriptManager();
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

		CoreApplication::onShutDown();
	}

	void Application::preUpdate()
	{
		CoreApplication::preUpdate();

		VirtualInput::instance()._update();

		if(mProfilerOverlay)
			mProfilerOverlay->update();
	}

	void Application::postUpdate()
	{
		CoreApplication::postUpdate();
		ScriptManager::instance().update();

		PROFILE_CALL(GUIManager::instance().update(), "GUI");
		DebugDraw::instance()._update();
	}

	void Application::showProfilerOverlay(ProfilerOverlayType type, const SPtr<Camera>& camera)
	{
		const SPtr<Camera>& overlayCamera = camera ? camera : gSceneManager().getMainCamera();
		if(!overlayCamera)
			return;

		if(!mProfilerOverlay)
			mProfilerOverlay = bs_shared_ptr_new<ProfilerOverlay>(overlayCamera);
		else
			mProfilerOverlay->setTarget(overlayCamera);

		mProfilerOverlay->show(type);
	}

	void Application::hideProfilerOverlay()
	{
		if(mProfilerOverlay)
			mProfilerOverlay->hide();

		mProfilerOverlay = nullptr;
	}

	void Application::startUpRenderer()
	{
		// Do nothing, we activate the renderer at a later stage
	}

	void Application::startUpScriptManager()
	{
		ScriptManager::startUp();
	}

	START_UP_DESC Application::buildStartUpDesc(VideoMode videoMode, const String& title, bool fullscreen)
	{
		START_UP_DESC desc;

		// Set up default plugins
		desc.renderAPI = BS_RENDER_API_MODULE;
		desc.renderer = BS_RENDERER_MODULE;
		desc.audio = BS_AUDIO_MODULE;
		desc.physics = BS_PHYSICS_MODULE;

		desc.importers.push_back("bsfFreeImgImporter");
		desc.importers.push_back("bsfFBXImporter");
		desc.importers.push_back("bsfFontImporter");
		desc.importers.push_back("bsfSL");

		desc.primaryWindowDesc.videoMode = videoMode;
		desc.primaryWindowDesc.fullscreen = fullscreen;
		desc.primaryWindowDesc.title = title;

		return desc;
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
