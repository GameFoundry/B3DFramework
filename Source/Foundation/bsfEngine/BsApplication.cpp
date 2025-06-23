//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
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
#include "CoreObject/BsRenderThread.h"
#include "FileSystem/BsFileSystem.h"
#include "Resources/BsPlainTextImporter.h"
#include "Importer/BsImporter.h"
#include "GUI/BsShortcutManager.h"
#include "CoreObject/BsCoreObjectManager.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRendererMaterialManager.h"
#include "Debug/BsDebugDraw.h"
#include "Platform/BsPlatform.h"
#include "Resources/BsEngineShaderIncludeHandler.h"
#include "Resources/BsResources.h"
#include "BsEngineConfig.h"
#include "GUI/BsProfilerOverlay.h"
#include "Text/BsStockIcons.h"

using namespace b3d;

Application::Application(const START_UP_DESC& desc)
	: CoreApplication(desc)
{}

Application::~Application()
{
	// Cleanup any new objects queued for destruction by unloaded scripts
	CoreObjectManager::Instance().SyncToRenderThread(true);
	GetRenderThread().PostCommand([]{}, "SyncToRenderThread() before shutdown", true);

	Cursor::ShutDown();

	GUIManager::ShutDown();
	SpriteManager::ShutDown();
	StockIcons::ShutDown();
	BuiltinResources::ShutDown();
	RendererMaterialManager::ShutDown();
	VirtualInput::ShutDown();
}

void Application::OnStartUp()
{
	CoreApplication::OnStartUp();

	PlainTextImporter* importer = B3DNew<PlainTextImporter>();
	Importer::Instance().RegisterAssetImporterInternal(importer);

	VirtualInput::StartUp();
	StockIcons::StartUp();
	BuiltinResources::StartUp();
	RendererMaterialManager::StartUp();
	RendererManager::Instance().Initialize(GetPrimaryGpuDevice());
	SceneManager::StartUp(); // Must be initialized after the renderer
	SpriteManager::StartUp();
	GUIManager::StartUp();
	ShortcutManager::StartUp();

	Cursor::StartUp();
	Cursor::Instance().SetCursor(CursorType::Arrow);
	Platform::SetIcon(BuiltinResources::Instance().GetFrameworkIcon());

	SceneManager::Instance().SetMainRenderTarget(GetPrimaryWindow());
	DebugDraw::StartUp();

	StartUpScriptManager();
}

void Application::OnShutDown()
{
	// Need to clear all objects before I unload any plugins, as they
	// could have allocated parts or all of those objects.
	SceneManager::Instance().ClearMainScene(true);

	// Resources too (Prefabs especially, since they hold the same data as a scene)
	Resources::Instance().UnloadAll();

	// Shut down before script manager as scripts could have registered shortcut callbacks
	ShortcutManager::ShutDown();

	ScriptManager::ShutDown();
	DebugDraw::ShutDown();

	SceneManager::ShutDown(); // Needs to trigger after ScriptManager, as script objects may still be referencing scene objects
	CoreApplication::OnShutDown();
}

void Application::PreUpdate()
{
	CoreApplication::PreUpdate();

	VirtualInput::Instance().UpdateInternal();

	if(mProfilerOverlay)
		mProfilerOverlay->Update();
}

void Application::PostUpdate()
{
	CoreApplication::PostUpdate();
	UpdateScriptManager();

	PROFILE_CALL(GUIManager::Instance().Update(), "GUI");
	DebugDraw::Instance().UpdateInternal();
}

void Application::ShowProfilerOverlay(ProfilerOverlayType type, const SPtr<Camera>& camera)
{
	const SPtr<Camera>& overlayCamera = camera ? camera : GetSceneManager().GetMainCamera();
	if(!overlayCamera)
		return;

	if(!mProfilerOverlay)
		mProfilerOverlay = B3DMakeShared<ProfilerOverlay>(overlayCamera);
	else
		mProfilerOverlay->SetTarget(overlayCamera);

	mProfilerOverlay->Show(type);
}

void Application::HideProfilerOverlay()
{
	if(mProfilerOverlay)
		mProfilerOverlay->Hide();

	mProfilerOverlay = nullptr;
}

void Application::StartUpRenderer()
{
	// Do nothing, we activate the renderer at a later stage
}

void Application::StartUpScriptManager()
{
	ScriptManager::StartUp();
}

void Application::UpdateScriptManager()
{
	ScriptManager::Instance().Update();
}

START_UP_DESC Application::BuildStartUpDesc(VideoMode videoMode, const String& title, bool fullscreen)
{
	START_UP_DESC desc;

	// Set up default plugins
	desc.RenderApi = B3D_RENDER_BACKEND;
	desc.Renderer = B3D_RENDERER;
	desc.Audio = B3D_AUDIO_BACKEND;
	desc.Physics = B3D_PHYSICS_BACKEND;

	desc.Importers.push_back("bsfFreeImgImporter");
	desc.Importers.push_back("bsfFBXImporter");
	desc.Importers.push_back("bsfFontImporter");
	desc.Importers.push_back("bsfSL");

	desc.PrimaryWindowDesc.VideoMode = videoMode;
	desc.PrimaryWindowDesc.Fullscreen = fullscreen;
	desc.PrimaryWindowDesc.Title = title;

	return desc;
}

SPtr<IShaderIncludeHandler> Application::GetShaderIncludeHandler() const
{
	return B3DMakeShared<EngineShaderIncludeHandler>();
}

namespace b3d
{
Application& GetApplication()
{
	return static_cast<Application&>(Application::Instance());
}
} // namespace b3d
