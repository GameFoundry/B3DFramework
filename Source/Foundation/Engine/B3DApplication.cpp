//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DApplication.h"
#include "GUI/B3DGUIManager.h"
#include "2D/B3DSpriteManager.h"
#include "Resources/B3DBuiltinResources.h"
#include "Script/B3DScriptManager.h"
#include "Profiling/B3DProfilingManager.h"
#include "Input/B3DVirtualInput.h"
#include "Scene/B3DSceneManager.h"
#include "Scene/B3DSceneObject.h"
#include "Platform/B3DCursor.h"
#include "CoreObject/B3DRenderThread.h"
#include "FileSystem/B3DFileSystem.h"
#include "Resources/B3DPlainTextImporter.h"
#include "Importer/B3DImporter.h"
#include "GUI/B3DShortcutManager.h"
#include "CoreObject/B3DCoreObjectManager.h"
#include "Renderer/B3DRendererManager.h"
#include "Renderer/B3DRendererMaterialManager.h"
#include "Debug/B3DDebugDraw.h"
#include "Platform/B3DPlatform.h"
#include "Resources/B3DEngineShaderIncludeHandler.h"
#include "Resources/B3DResources.h"
#include "B3DEngineConfig.h"
#include "GUI/B3DProfilerOverlay.h"
#include "Components/B3DCamera.h"
#include "Scene/B3DSceneInstance.h"
#include "Text/B3DStockIcons.h"

using namespace b3d;

Application::Application(const ApplicationCreateInformation& desc)
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

	SceneManager::Instance().SetMainCameraRenderTarget(GetPrimaryWindow());
	DebugDraw::StartUp();

	StartUpScriptManager();
}

void Application::OnShutDown()
{
	// Need to clear all objects before I unload any plugins, as they
	// could have allocated parts or all of those objects.
	const UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>> allScenes = GetSceneManager().GetAllScenes();
	for(const auto& entry : allScenes)
	{
		const SPtr<SceneInstance>& scene = entry.second.lock();
		if(scene == nullptr)
			continue;

		scene->Clear(true);
	}

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

void Application::ShowProfilerOverlay(ProfilerOverlayType type, const HCamera& camera)
{
	const SPtr<SceneInstance>& scene = camera ? camera->SceneObject()->GetScene() : GetSceneManager().GetMainScene();
	const HCamera& overlayCamera = camera ? camera : scene->GetMainCamera();
	if(!overlayCamera.IsValid())
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

ApplicationCreateInformation Application::BuildCreateInformation(VideoMode videoMode, const String& title, bool fullscreen)
{
	ApplicationCreateInformation desc;

	// Set up default plugins
	desc.RenderApi = B3D_RENDER_BACKEND;
	desc.Renderer = B3D_RENDERER;
	desc.Audio = B3D_AUDIO_BACKEND;
	desc.Physics = B3D_PHYSICS_BACKEND;

	desc.Importers.push_back("bsfFreeImgImporter");
	desc.Importers.push_back("bsfFBXImporter");
	desc.Importers.push_back("bsfFontImporter");
	desc.Importers.push_back("bsfSL");

	desc.PrimaryWindow.VideoMode = videoMode;
	desc.PrimaryWindow.Fullscreen = fullscreen;
	desc.PrimaryWindow.Title = title;

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
