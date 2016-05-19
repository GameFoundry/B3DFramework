//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsApplication.h"
#include "BsGUIManager.h"
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

namespace BansheeEngine
{
	START_UP_DESC createStartUpDesc(RENDER_WINDOW_DESC& primaryWindowDesc, const String& renderAPI, const String& renderer, 
		const String& audio, const Vector<String>& importers)
	{
		START_UP_DESC desc;
		desc.renderAPI = renderAPI;
		desc.renderer = renderer;
		desc.audio = audio;
		desc.physics = "BansheePhysX";
		desc.input = "BansheeOISInput";

		desc.primaryWindowDesc = primaryWindowDesc;
		desc.importers = importers;

		return desc;
	}

	Application::Application(RENDER_WINDOW_DESC primaryWindowDesc, RenderAPIPlugin renderAPI, RendererPlugin renderer, 
		AudioPlugin audio, const Vector<String>& importers)
		: CoreApplication(createStartUpDesc(primaryWindowDesc, getLibNameForRenderAPI(renderAPI)
		, getLibNameForRenderer(renderer), getLibNameForAudio(audio), importers)), mMonoPlugin(nullptr)
		, mSBansheeEnginePlugin(nullptr)
	{

	}

	Application::~Application()
	{
		// Cleanup any new objects queued for destruction by unloaded scripts
		CoreObjectManager::instance().clearDirty();
		gCoreThread().update();
		gCoreThread().submitAccessors(true);

		Cursor::shutDown();

		ShortcutManager::shutDown();
		GUIManager::shutDown();
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

	void Application::startUp(RENDER_WINDOW_DESC& primaryWindowDesc, RenderAPIPlugin renderAPI, 
		RendererPlugin renderer, AudioPlugin audio, const Vector<String>& importers)
	{
		CoreApplication::startUp<Application>(primaryWindowDesc, renderAPI, renderer, audio, importers);
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
		Path releaseAssemblyFolder = FileSystem::getWorkingDirectoryPath();
		releaseAssemblyFolder.append(Paths::getReleaseAssemblyPath());

		Path debugAssemblyFolder = FileSystem::getWorkingDirectoryPath();
		debugAssemblyFolder.append(Paths::getDebugAssemblyPath());

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

	String Application::getLibNameForRenderAPI(RenderAPIPlugin plugin)
	{
		static String DX11Name = "BansheeD3D11RenderAPI";
		static String DX9Name = "BansheeD3D9RenderAPI";
		static String OpenGLName = "BansheeGLRenderAPI";

		switch (plugin)
		{
		case RenderAPIPlugin::DX11:
			return DX11Name;
		case RenderAPIPlugin::DX9:
			return DX9Name;
		case RenderAPIPlugin::OpenGL:
			return OpenGLName;
		}

		return StringUtil::BLANK;
	}

	String Application::getLibNameForRenderer(RendererPlugin plugin)
	{
		static String DefaultName = "RenderBeast";

		switch (plugin)
		{
		case RendererPlugin::Default:
			return DefaultName;
		}
	
		return StringUtil::BLANK;
	}

	String Application::getLibNameForAudio(AudioPlugin plugin)
	{
		static String OpenAudioName = "BansheeOpenAudio";
		static String FMODName = "BansheeFMOD";

		switch (plugin)
		{
		case AudioPlugin::OpenAudio:
			return OpenAudioName;
		case AudioPlugin::FMOD:
			return FMODName;
		}

		return StringUtil::BLANK;
	}

	Application& gApplication()
	{
		return static_cast<Application&>(Application::instance());
	}
}