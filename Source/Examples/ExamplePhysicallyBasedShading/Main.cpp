//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
// Engine includes
#include "BsApplication.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "Importer/BsImporter.h"
#include "Importer/BsTextureImportOptions.h"
#include "Importer/BsMeshImportOptions.h"
#include "Material/BsMaterial.h"
#include "Input/BsVirtualInput.h"
#include "Components/BsCCamera.h"
#include "Components/BsCRenderable.h"
#include "Components/BsCLight.h"
#include "Components/BsCSkybox.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"
#include "Scene/BsSceneObject.h"
#include "BsEngineConfig.h"

// Example includes
#include "CameraFlyer.h"
#include "ObjectRotator.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <windows.h>
#endif

namespace bs
{
	struct Assets;

	UINT32 windowResWidth = 1280;
	UINT32 windowResHeight = 720;

	/** Imports all of our assets and prepares GameObject that handle the example logic. */
	void setUpExample();

	/** Import mesh & textures used by the example. */
	void loadAssets(Assets& assets);

	/** Imports a mesh at the provided path and optionally scales it. */
	HMesh loadMesh(const Path& path, float scale = 1.0f);
	
	/** 
	 * Imports a texture at the provided path. Textures not in sRGB space (e.g. normal maps) need to be specially marked by
	 * setting 'isSRGB' to false. Also allows for conversion of texture to cubemap by setting the 'isCubemap' parameter.
	 * If the data should be imported in a floating point format, specify 'isHDR' to true.
	 */
	HTexture loadTexture(const Path& path, bool isSRGB = true, bool isCubemap = false, bool isHDR = false);
	
	/** Create a material used by our example model. */
	void createMaterial(Assets& assets);

	/** Set up example scene objects. */
	void setUp3DScene(const Assets& assets);

	/** Set up input configuration and callbacks. */
	void setUpInput();

	/** Toggles the primary window between full-screen and windowed mode. */
	void toggleFullscreen();

	/** Called whenever the main render window is resized. */
	void renderWindowResized();

	/** Called when the selected video mode changes in the video mode list box. */
	void videoModeChanged(UINT32 idx, bool enabled);

	/** Triggered whenever a virtual button is released. */
	void buttonUp(const VirtualButton& button, UINT32 deviceIdx);
}

using namespace bs;

/** Main entry point into the application. */
#if BS_PLATFORM == BS_PLATFORM_WIN32
int CALLBACK WinMain(
	_In_  HINSTANCE hInstance,
	_In_  HINSTANCE hPrevInstance,
	_In_  LPSTR lpCmdLine,
	_In_  int nCmdShow
	)
#else
int main()
#endif
{
	// Descriptor used for initializing the engine
	START_UP_DESC startUpDesc;

	// Use default values as specified by the build system
	startUpDesc.renderAPI = BS_RENDER_API_MODULE;
	startUpDesc.renderer = BS_RENDERER_MODULE;
	startUpDesc.audio = BS_AUDIO_MODULE;
	startUpDesc.physics = BS_PHYSICS_MODULE;

	// Descriptor used for initializing the primary application window.
	startUpDesc.primaryWindowDesc.videoMode = VideoMode(windowResWidth, windowResHeight);
	startUpDesc.primaryWindowDesc.title = "Banshee Example App";
	startUpDesc.primaryWindowDesc.fullscreen = false;
	startUpDesc.primaryWindowDesc.depthBuffer = false;

	// List of importer plugins we plan on using for importing various resources
	startUpDesc.importers.push_back("BansheeFreeImgImporter"); // For importing textures
	startUpDesc.importers.push_back("BansheeFBXImporter"); // For importing meshes
	startUpDesc.importers.push_back("BansheeFontImporter"); // For importing fonts
	startUpDesc.importers.push_back("BansheeSL"); // For importing shaders

	// Initializes the application with systems and primary window as defined above
	Application::startUp(startUpDesc);

	// Imports all of ours assets and prepares GameObjects that handle the example logic.
	setUpExample();
	
	// Runs the main loop that does most of the work. This method will exit when user closes the main
	// window or exits in some other way.
	Application::instance().runMainLoop();

	Application::shutDown();

	return 0;
}

namespace bs
{
	Path dataPath = Paths::getRuntimeDataPath();
	Path exampleModelPath = dataPath + "Examples/Pistol/Pistol01.fbx";
	Path exampleAlbedoTexPath = dataPath + "Examples/Pistol/Pistol_DFS.png";
	Path exampleNormalsTexPath = dataPath + "Examples/Pistol/Pistol_NM.png";
	Path exampleRoughnessTexPath = dataPath + "Examples/Pistol/Pistol_RGH.png";
	Path exampleMetalnessTexPath = dataPath + "Examples/Pistol/Pistol_MTL.png";
    Path exampleSkyCubemapPath = dataPath + "Examples/Environments/PaperMill_E_3k.hdr";

	HCamera sceneCamera;

	/** Container for all resources used by the example. */
	struct Assets
	{
		HMesh exampleModel;
		HTexture exampleAlbedoTex;
		HTexture exampleNormalsTex;
		HTexture exampleRoughnessTex;
		HTexture exampleMetalnessTex;
		HTexture exampleSkyCubemap;
		HShader exampleShader;
		HMaterial exampleMaterial;
	};

	void setUpExample()
	{
		Assets assets;
		loadAssets(assets);
		createMaterial(assets);

		setUp3DScene(assets);
		setUpInput();
	}

	/**
	 * Load the required resources. First try to load a pre-processed version of the resources. If they don't exist import
	 * resources from the source formats into engine format, and save them for next time.
	 */
	void loadAssets(Assets& assets)
	{
		// Load an FBX mesh.
		assets.exampleModel = loadMesh(exampleModelPath, 10.0f);

        // Load textures
		assets.exampleAlbedoTex = loadTexture(exampleAlbedoTexPath);
		assets.exampleNormalsTex = loadTexture(exampleNormalsTexPath, false);
		assets.exampleRoughnessTex = loadTexture(exampleRoughnessTexPath, false);
		assets.exampleMetalnessTex = loadTexture(exampleMetalnessTexPath, false);
		assets.exampleSkyCubemap = loadTexture(exampleSkyCubemapPath, false, true, true);

		// Load the default physically based shader for rendering opaque objects
		assets.exampleShader = BuiltinResources::instance().getBuiltinShader(BuiltinShader::Standard);
	}
	
	HMesh loadMesh(const Path& path, float scale)
	{
		Path assetPath = path;
		assetPath.setExtension(path.getExtension() + ".asset");

		HMesh model = gResources().load<Mesh>(assetPath);
		if (model == nullptr) // Mesh file doesn't exist, import from the source file.
		{
			// When importing you may specify optional import options that control how is the asset imported.
			SPtr<ImportOptions> meshImportOptions = Importer::instance().createImportOptions(path);

			// rtti_is_of_type checks if the import options are of valid type, in case the provided path is pointing to a
			// non-mesh resource. This is similar to dynamic_cast but uses Banshee internal RTTI system for type checking.
			if (rtti_is_of_type<MeshImportOptions>(meshImportOptions))
			{
				MeshImportOptions* importOptions = static_cast<MeshImportOptions*>(meshImportOptions.get());

				importOptions->setImportScale(scale);
			}

			model = gImporter().import<Mesh>(path, meshImportOptions);

			// Save for later use, so we don't have to import on the next run.
			gResources().save(model, assetPath, true);
		}

		return model;
	}

	HTexture loadTexture(const Path& path, bool isSRGB, bool isCubemap, bool isHDR)
	{
		Path assetPath = path;
		assetPath.setExtension(path.getExtension() + ".asset");

		HTexture texture = gResources().load<Texture>(assetPath);
		if (texture == nullptr) // Texture file doesn't exist, import from the source file.
		{
			// When importing you may specify optional import options that control how is the asset imported.
			SPtr<ImportOptions> textureImportOptions = Importer::instance().createImportOptions(path);

			// rtti_is_of_type checks if the import options are of valid type, in case the provided path is pointing to a 
			// non-texture resource. This is similar to dynamic_cast but uses Banshee internal RTTI system for type checking.
			if (rtti_is_of_type<TextureImportOptions>(textureImportOptions))
			{
				TextureImportOptions* importOptions = static_cast<TextureImportOptions*>(textureImportOptions.get());

				// We want maximum number of mipmaps to be generated
				importOptions->setGenerateMipmaps(true);

				// If the texture is in sRGB space the system needs to know about it
				importOptions->setSRGB(isSRGB);

				// Ensures we can save the texture contents
				importOptions->setCPUCached(true);

                // Import as cubemap if needed
                importOptions->setIsCubemap(isCubemap);

                // If importing as cubemap, assume source is a panorama
                importOptions->setCubemapSourceType(CubemapSourceType::Cylindrical);

                // Importing using a HDR format if requested
                if (isHDR)
                    importOptions->setFormat(PF_RG11B10F);
			}

			// Import texture with specified import options
			texture = gImporter().import<Texture>(path, textureImportOptions);

			// Save for later use, so we don't have to import on the next run.
			gResources().save(texture, assetPath, true);
		}

		return texture;
	}	

	/** Create a material using the active shader, and assign the relevant textures to it. */
	void createMaterial(Assets& assets)
	{
		// Create a material with the active shader.
		HMaterial exampleMaterial = Material::create(assets.exampleShader);

        // Assign the four textures requires by the PBS shader
        exampleMaterial->setTexture("gAlbedoTex", assets.exampleAlbedoTex);
        exampleMaterial->setTexture("gNormalTex", assets.exampleNormalsTex);
        exampleMaterial->setTexture("gRoughnessTex", assets.exampleRoughnessTex);
        exampleMaterial->setTexture("gMetalnessTex", assets.exampleMetalnessTex);

		assets.exampleMaterial = exampleMaterial;
	}

	/** Set up the 3D object used by the example, and the camera to view the world through. */
	void setUp3DScene(const Assets& assets)
	{
		/************************************************************************/
		/* 								SCENE OBJECT                      		*/
		/************************************************************************/

		// Now we create a scene object that has a position, orientation, scale and optionally
		// components to govern its logic. In this particular case we are creating a SceneObject
		// with a Renderable component which will render a mesh at the position of the scene object
		// with the provided material.

		// Create new scene object at (0, 0, 0)
		HSceneObject pistolSO = SceneObject::create("Pistol");
		
		// Attach the Renderable component and hook up the mesh we imported earlier,
		// and the material we created in the previous section.
		HRenderable renderable = pistolSO->addComponent<CRenderable>();
		renderable->setMesh(assets.exampleModel);
		renderable->setMaterial(assets.exampleMaterial);

		// Add a rotator component so we can rotate the object during runtime
		pistolSO->addComponent<ObjectRotator>();

        /************************************************************************/
        /* 									SKYBOX                       		*/
        /************************************************************************/

        // Add a skybox texture for sky reflections
        HSceneObject skyboxSO = SceneObject::create("Skybox");

        HSkybox skybox = skyboxSO->addComponent<CSkybox>();
        skybox->setTexture(assets.exampleSkyCubemap);

		/************************************************************************/
		/* 									CAMERA	                     		*/
		/************************************************************************/

		// In order something to render on screen we need at least one camera.

		// Like before, we create a new scene object at (0, 0, 0).
		HSceneObject sceneCameraSO = SceneObject::create("SceneCamera");

		// Get the primary render window we need for creating the camera. Additionally
		// hook up a callback so we are notified when user resizes the window.
		SPtr<RenderWindow> window = gApplication().getPrimaryWindow();
		window->onResized.connect(&renderWindowResized);

		// Add a Camera component that will output whatever it sees into that window 
		// (You could also use a render texture or another window you created).
		sceneCamera = sceneCameraSO->addComponent<CCamera>(window);

		// Set up camera component properties

		// Set closest distance that is visible. Anything below that is clipped.
		sceneCamera->setNearClipDistance(0.005f);

		// Set farthest distance that is visible. Anything above that is clipped.
		sceneCamera->setFarClipDistance(1000);

		// Set aspect ratio depending on the current resolution
		sceneCamera->setAspectRatio(windowResWidth / (float)windowResHeight);

        // Enable multi-sample anti-aliasing for better quality
        sceneCamera->setMSAACount(4);

		// Add a CameraFlyer component that allows us to move the camera. See CameraFlyer for more information.
		sceneCameraSO->addComponent<CameraFlyer>();

		// Position and orient the camera scene object
		sceneCameraSO->setPosition(Vector3(0.2f, 0.1f, 0.2f));
		sceneCameraSO->lookAt(Vector3(-0.1f, 0, 0));
	}

	/** Register mouse and keyboard inputs that will be used for controlling the camera. */
	void setUpInput()
	{
		// Register input configuration
		// Banshee allows you to use VirtualInput system which will map input device buttons
		// and axes to arbitrary names, which allows you to change input buttons without affecting
		// the code that uses it, since the code is only aware of the virtual names. 
		// If you want more direct input, see Input class.
		auto inputConfig = VirtualInput::instance().getConfiguration();

		// Camera controls for buttons (digital 0-1 input, e.g. keyboard or gamepad button)
		inputConfig->registerButton("Forward", BC_W);
		inputConfig->registerButton("Back", BC_S);
		inputConfig->registerButton("Left", BC_A);
		inputConfig->registerButton("Right", BC_D);
		inputConfig->registerButton("Forward", BC_UP);
		inputConfig->registerButton("Back", BC_BACK);
		inputConfig->registerButton("Left", BC_LEFT);
		inputConfig->registerButton("Right", BC_RIGHT);
		inputConfig->registerButton("FastMove", BC_LSHIFT);
		inputConfig->registerButton("RotateObj", BC_MOUSE_LEFT);
		inputConfig->registerButton("RotateCam", BC_MOUSE_RIGHT);

		// Camera controls for axes (analog input, e.g. mouse or gamepad thumbstick)
		// These return values in [-1.0, 1.0] range.
		inputConfig->registerAxis("Horizontal", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseX));
		inputConfig->registerAxis("Vertical", VIRTUAL_AXIS_DESC((UINT32)InputAxis::MouseY));
	}

	/** Callback triggered wheneve the user resizes the example window. */
	void renderWindowResized()
	{
		SPtr<RenderWindow> window = gApplication().getPrimaryWindow();
		const RenderWindowProperties& rwProps = window->getProperties();

		windowResWidth = rwProps.width;
		windowResHeight = rwProps.height;

		sceneCamera->setAspectRatio(rwProps.width / (float)rwProps.height);
	}
}

