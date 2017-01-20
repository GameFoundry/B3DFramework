//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsApplication.h"
#include "BsCrashHandler.h"
#include "BsCoreThread.h"
#include "BsFileSerializer.h"
#include "BsGameSettings.h"
#include "BsFileSystem.h"
#include "BsResources.h"
#include "BsResourceManifest.h"
#include "BsPrefab.h"
#include "BsSceneObject.h"
#include "BsSceneManager.h"
#include "BsRenderAPI.h"
#include "BsGameResourceManager.h"
#include "BsEngineConfig.h"

void runApplication();

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <windows.h>

using namespace bs;

int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
{
	CrashHandler::startUp();

	__try
	{
		runApplication();
	}
	__except (gCrashHandler().reportCrash(GetExceptionInformation()))
	{
		PlatformUtility::terminate(true);
	}

	CrashHandler::shutDown();

	return 0;
}
#endif // End BS_PLATFORM

using namespace bs;

void runApplication()
{
	Path gameSettingsPath = Paths::getGameSettingsPath();

	FileDecoder fd(gameSettingsPath);
	SPtr<GameSettings> gameSettings = std::static_pointer_cast<GameSettings>(fd.decode());

	if (gameSettings == nullptr)
		gameSettings = bs_shared_ptr_new<GameSettings>();

	unsigned int resolutionWidth = 200;
	unsigned int resolutionHeight = 200;

	if (!gameSettings->fullscreen)
	{
		resolutionWidth = gameSettings->resolutionWidth;
		resolutionHeight = gameSettings->resolutionHeight;
	}

	START_UP_DESC startUpDesc;
	startUpDesc.renderAPI = BS_RENDER_API_MODULE;
	startUpDesc.renderer = BS_RENDERER_MODULE;
	startUpDesc.audio = BS_AUDIO_MODULE;
	startUpDesc.physics = BS_PHYSICS_MODULE;
	startUpDesc.input = BS_INPUT_MODULE;
	startUpDesc.scripting = true;

	startUpDesc.primaryWindowDesc.videoMode = VideoMode(resolutionWidth, resolutionHeight);
	startUpDesc.primaryWindowDesc.title = toString(gameSettings->titleBarText);
	startUpDesc.primaryWindowDesc.fullscreen = false;
	startUpDesc.primaryWindowDesc.hidden = gameSettings->fullscreen;
	startUpDesc.primaryWindowDesc.depthBuffer = false;

	Application::startUp(startUpDesc);

	// Note: What if script tries to load resources during startup? The manifest nor the mapping wont be set up yet.
	Path resourcesPath = Paths::getGameResourcesPath();
	Path resourceMappingPath = resourcesPath + GAME_RESOURCE_MAPPING_NAME;

	FileDecoder mappingFd(resourceMappingPath);
	SPtr<ResourceMapping> resMapping = std::static_pointer_cast<ResourceMapping>(mappingFd.decode());

	GameResourceManager::instance().setMapping(resMapping);

	if (gameSettings->fullscreen)
	{
		if (gameSettings->useDesktopResolution)
		{
			const VideoModeInfo& videoModeInfo = RenderAPI::getVideoModeInfo();
			const VideoOutputInfo& primaryMonitorInfo = videoModeInfo.getOutputInfo(0);
			const VideoMode& selectedVideoMode = primaryMonitorInfo.getDesktopVideoMode();

			SPtr<RenderWindow> window = gApplication().getPrimaryWindow();
			window->setFullscreen(selectedVideoMode);

			resolutionWidth = selectedVideoMode.getWidth();
			resolutionHeight = selectedVideoMode.getHeight();
		}
		else
		{
			resolutionWidth = gameSettings->resolutionWidth;
			resolutionHeight = gameSettings->resolutionHeight;

			VideoMode videoMode(resolutionWidth, resolutionHeight);

			SPtr<RenderWindow> window = gApplication().getPrimaryWindow();
			window->show();
			window->setFullscreen(videoMode);
		}
	}

	gameSettings->useDesktopResolution = false; // Not relevant after first startup

	// TODO - Save full video mode
	gameSettings->resolutionWidth = resolutionWidth;
	gameSettings->resolutionHeight = resolutionHeight;

	FileEncoder fe(gameSettingsPath);
	fe.encode(gameSettings.get());

	Path resourceManifestPath = resourcesPath + GAME_RESOURCE_MANIFEST_NAME;

	SPtr<ResourceManifest> manifest;
	if (FileSystem::exists(resourceManifestPath))
	{
		Path resourceRoot = resourcesPath;
		resourceRoot.makeParent(); // Remove /Resources entry, as we expect all resources to be relative to that path

		manifest = ResourceManifest::load(resourceManifestPath, resourceRoot);

		gResources().registerResourceManifest(manifest);
	}

	{
		HPrefab mainScene = static_resource_cast<Prefab>(gResources().loadFromUUID(gameSettings->mainSceneUUID, 
			false, ResourceLoadFlag::LoadDependencies));
		if (mainScene.isLoaded(false))
		{
			HSceneObject root = mainScene->instantiate();
			HSceneObject oldRoot = gSceneManager().getRootNode();

			gSceneManager()._setRootNode(root);
			oldRoot->destroy();
		}
	}

	Application::instance().runMainLoop();
	Application::shutDown();
}