Renderer								{#renderer}
===============
[TOC]

Renderer is a system that processes all renderable objects in the scene, renders them, applies lighting and shadows, renders overlay like GUI and applies post processing effects. It is the system that determines how your game looks (together with custom materials you might specify). In Banshee the renderer is implemented as a plugin, so you may create your own and fully customize the look of your game. Banshee also comes with a default renderer called "RenderBeast".

In this manual we'll explain how to use a renderer to render objects in the scene, as well as how to implement your own renderer.

# Rendering scene objects {#renderer_a}
To render objects in the scene create a @ref BansheeEngine::SceneObject "SceneObject" onto which you then attach relevant @ref BansheeEngine::Component "Components". These @ref BansheeEngine::Component "Components" then register themselves with the renderer, which takes care of everything else. See the [game object](@ref gameObjects) manual for more information about scene objects and components.

There are three primary types of components used for rendering:
 - @ref BansheeEngine::CCamera "CCamera" - A camera used for viewing the scene.
 - @ref BansheeEngine::CRenderable "CRenderable" - An object to render.
 - @ref BansheeEngine::CLight "CLight" - A light to illuminate the objects with.
 
## Camera {#renderer_a_a}
@ref BansheeEngine::CCamera "Camera" component determines which portion of the scene is being rendered, onto which surface, and how. Before you create a camera you must first create a @ref BansheeEngine::RenderTarget "RenderTarget" onto which the camera will output the rendered image. See the [render targets](@ref renderTargets) manual for information on how to create render targets.

See the following example to create a camera:
~~~~~~~~~~~~~{.cpp}
SPtr<RenderTarget> rt = ...;

HSceneObject cameraSO = SceneObject::create("Camera");
HCamera camera = cameraSO->addComponent<CCamera>(rt);
~~~~~~~~~~~~~

The transform of the camera's parent scene object determines which portion of the scene does the camera see. You can position and rotate the camera using the standard @ref BansheeEngine::SceneObject "SceneObject" transform methods:
~~~~~~~~~~~~~{.cpp}
cameraSO->setPosition(Vector3(0, 0, 50)); // Move camera to (0, 0, 50)
cameraSO->lookAt(Vector3(0, 0, 0)); // Look towards origin
~~~~~~~~~~~~~

You may also adjust various other properties of the camera, including @ref BansheeEngine::ProjectionType "projection type", field of view, aspect ratio and similar using methods like @ref BansheeEngine::CCamera::setProjectionType "CCamera::setProjectionType", @ref BansheeEngine::CCamera::setHorzFOV "CCamera::setHorzFOV" and @ref BansheeEngine::CCamera::setAspectRatio "CCamera::setAspectRatio". See the @ref BansheeEngine::CCamera "Camera" API documentation for a full list of methods.

Each camera be assigned a priority via @ref BansheeEngine::CCamera::setPriority "CCamera::setPriority". The priority determines in which order are multiple cameras rendered. Higher priority means the camera will be rendered sooner. Be aware that render targets also have priority, and their priority takes precedance over camera's priority. Camera priority only matters if multiple cameras are rendering to the same render target.

Camera can also be assigned a layer via @ref BansheeEngine::CCamera::setLayers "CCamera::setLayers". The layer is a bitmask that can be used for filtering which objects does the camera render. As we'll see later, each renderable object can also be assigned a layer. Any object whose layer ORed with the camera's layer bitmask is non-zero will be rendered to the camera.

A variety of methods for converting between world, view, projection, clip and NDC spaces are also provided by the camera. For example see @ref BansheeEngine::CCamera::screenPointToRay "CCamera::screenPointToRay".

You may also retrieve the camera's projection and view matrices by calling @ref BansheeEngine::CCamera::getProjectionMatrix "CCamera::getProjectionMatrix" and @ref BansheeEngine::CCamera::getViewMatrix "CCamera::getViewMatrix" respectively.

## Renderable {#renderer_a_b}
Once the camera in the scene is set up, you can start adding object you wish to render. All such objects are represented with the @ref BansheeEngine::CRenderable "CRenderable" component. This is a fairly simple components that expects only a @ref BansheeEngine::Mesh "Mesh" and one or multiple @ref BansheeEngine::Material "Material(s)". Read the [mesh](@ref meshes) and [material](@ref materials) manual for more information how to create them.

See the following example to create a renderable object:
~~~~~~~~~~~~~{.cpp}
HMesh mesh = ...; // Create or import a mesh
HMaterial material = ...; // Create or import a material

HSceneObject renderableSO = SceneObject::create("Renderable");
HRenderable renderable = renderableSO->addComponent<CRenderable>();
renderable->setMesh(mesh);
renderable->setMaterial(material);
~~~~~~~~~~~~~

You can also optionally set multiple materials in case your mesh has multiple sub-meshes. In such case each material will be used for rendering one sub-mesh.

And as we have mentioned in the previous section, each renderable can also be assigned a layer by calling @ref BansheeEngine::CRenderable::setLayer "CRenderable::setLayer", which in combination with camera's layer can be used for filtering on which camera(s) should the object be visible.

## Light {#renderer_a_c}
Finally we can choose to add a light to the scene. A light will influence the lighting and shadows of renderable objects in its radius. Use the @ref BansheeEngine::CLight "CLight" component to set up a light. Light comes in three variants:
 - Point light that illuminates everything in a certain radius.
 - Spot light that illuminates everything within a certain angle and radius.
 - Directional light that illuminates everything at an infinite distance from a particular direction.
 
You can choose between light variants by calling @ref BansheeEngine::CLight::setType "CLight::setType". You can also set parameters like light color, intensity, range and others. See the @ref BansheeEngine::CLight "CLight" API documentation for a full list.

An example creating a point light:
~~~~~~~~~~~~~{.cpp}
HSceneObject lightSO = SceneObject::create("Light");
HLight light = lightSO->addComponent<CLight>();
light->setType(LightType::Point);
light->setRadius(100.0f);
light->setIntensity(1000.0f);
~~~~~~~~~~~~~

# Creating your own renderer {#renderer_b}
Renderer is the system that makes use of all the components we listed so far. Renderer isn't actually aware of scene objects and components, and instead operates on @ref BansheeEngine::Camera "Camera", @ref BansheeEngine::Renderable "Renderable" and @ref BansheeEngine::Light "Light" types directly. These are the internal objects that are managed by @ref BansheeEngine::CCamera "CCamera", @ref BansheeEngine::CRenderable "CRenderable" and @ref BansheeEngine::CLight "CLight", respectively. They provide the same interface as their component versions. They are also @ref BansheeEngine::CoreObject "core objects", meaning they have both a simulation and a core thread counterparts.

To create your own renderer you must implement the @ref BansheeEngine::Renderer "Renderer" interface. The implementation should iterate over all renderable objects, cameras and lights and perform rendering according to your own rules. At the end of rendering, every render target in every active camera should be filled with an image of the rendered scene. During rendering you must consider mesh and material provided by the renderable objects, and apply lighting according to light objects. You should use the low level @ref BansheeEngine::RenderAPI "RenderAPI" to render individual objects.

The renderer is mostly a core-thread system as then it has a more direct access to the @ref BansheeEngine::RenderAPI "RenderAPI", as well as various utility functionality we'll describe later. Read the [core thread](@ref coreThread) manual for more information about the core thread and core objects, and the [render API](@ref renderAPI) manual on how to use the low level rendering functionality.

The @ref BansheeEngine::Renderer "Renderer" interface requires you to implement the following methods:
 - @ref BansheeEngine::CoreRenderer::getName "Renderer::getName" - Returns a unique name of the renderer. This can be used by shader techniques to identify which renderer is active and when they should run, as described in the [material](@ref materials) manual.
 - @ref BansheeEngine::CoreRenderer::renderAll "Renderer::renderAll" - This is a method called from the simulation thread that executes the rendering. It is called once per frame. In this method you should queue your actual rendering method for execution on the core thread.
 - @ref BansheeEngine::CoreRenderer::notifyCameraAdded "Renderer::notifyCameraAdded" - Called on the core thread whenever a new @ref BansheeEngine::Camera "Camera" is created (i.e. when a @ref BansheeEngine::CCamera "CCamera" component is added to the scene). Also called when camera's properties change (camera is removed, then re-added).
 - @ref BansheeEngine::CoreRenderer::notifyCameraUpdated "Renderer::notifyCameraUpdated" - Called on the core thread whenever a @ref BansheeEngine::Camera "Camera's" position or rotation changes.
 - @ref BansheeEngine::CoreRenderer::notifyCameraRemoved "Renderer::notifyCameraRemoved" - Called on the core thread whenever a @ref BansheeEngine::Camera "Camera" is destroyed. Also called when camera's properties change (camera is removed, then re-added).
 - @ref BansheeEngine::Renderer::notifyRenderableAdded "Renderer::notifyRenderableAdded" - Called on the core thread whenever a new @ref BansheeEngine::Renderable "Renderable" is created (e.g. when a @ref BansheeEngine::CRenderable "CRenderable" component is added to the scene).
 - @ref BansheeEngine::Renderer::notifyRenderableUpdated "Renderer::notifyRenderableUpdated" - Called whenever @ref BansheeEngine::Renderable "Renderable" properties change, e.g. when a scene object a renderable is attached to moves.
 - @ref BansheeEngine::Renderer::notifyRenderableRemoved "Renderer::notifyRenderableRemoved" - Called whenever a @ref BansheeEngine::Renderable "Renderable" is destroyed.
 - @ref BansheeEngine::Renderer::notifyLightAdded "Renderer::notifyLightAdded" - Called whenever a new @ref BansheeEngine::Light "Light" is created (e.g. when a @ref BansheeEngine::CLight "CLight" component is added to the scene).
 - @ref BansheeEngine::Renderer::notifyLightUpdated "Renderer::notifyLightUpdated" - Called whenever @ref BansheeEngine::Light "Light" properties change, e.g. when a scene object a light is attached to moves.
 - @ref BansheeEngine::Renderer::notifyLightRemoved "Renderer::notifyLightRemoved" - Called whenever @ref BansheeEngine::Light "Light" is destroyed.
 
Implementing these methods should give you all the information you need to render your scene. You will have the required render target and projection/view matrices from the @ref BansheeEngine::Camera "Camera", mesh/material/world transform from @ref BansheeEngine::Renderable "Renderable" objects, and lighting information from @ref BansheeEngine::Light "Light". Use the information provided in the [render API](@ref renderAPI) manual to learn how to render using those objects.

Aside from rendering scene objects, the renderer should also take care of rendering everything else, like GUI, debug information and similar. This is handled in a general manner via callbacks. Every external system (like GUI) can register itself with the renderer by calling @ref BansheeEngine::CoreRenderer::registerRenderCallback "Renderer::registerRenderCallback". This method accepts a few parameters:
 - Pointer to a @ref BansheeEngine::Camera "Camera" for which the callback is valid. The renderer should call the callback when rendering onto that camera.
 - Index that indicated the priority. Callbacks with indices lower than zero should be executed before scene object rendering, and indices equal to or higher than zero should be executed after scene object rendering.
 - Callback to execute. Other systems will usually call @ref BansheeEngine::RenderAPI "RenderAPI" methods in such callback to perform rendering manually.
 - Overlay toggle. Overlay elements are rendered after all other objects (and non-overlay callbacks). They don't require a depth buffer, nor a multi-sampled render target for rendering (e.g. like GUI), which makes them cheaper to render.

Once a callback is registered, the renderer can access it via the `mRenderCallbacks` protected field.
 
While this is enough to create a custom renderer, a variety of useful utilities are provided to make rendering easier.

## Utilities {#renderer_b_a}
These systems aren't critical for renderer creation, but instead provide an easier way to perform commonly required functions.

### Render queue {#renderer_b_a_a}
@ref BansheeEngine::RenderQueue "RenderQueue" allows you to sort and group scene objects for rendering. For example transparent objects might need to be sorted back to front based on their distance from the camera. It is also often useful to group objects if they share the same material, to reduce state switching which can improve performance.

Use @ref BansheeEngine::RenderQueue::add "RenderQueue::add" to add new objects to the queue. It expects a @ref BansheeEngine::RenderableElement "RenderableElement" which you can create from information provided by @ref BansheeEngine::Renderable "Renderable" when @ref BansheeEngine::Renderer::notifyRenderableAdded "Renderer::notifyRenderableAdded" is called. Normally you wish to have a single @ref BansheeEngine::RenderableElement "RenderableElement" for each sub-mesh present in the renderable object's mesh.

Once all elements are in the queue, you can call @ref BansheeEngine::RenderQueue::setStateReduction "RenderQueue::setStateReduction" to select how to sort the objects:
 - @ref BansheeEngine::StateReduction::None "StateReduction::None" - There will be no sorting, based either by distance or material.
 - @ref BansheeEngine::StateReduction::Material "StateReduction::Material" - Elements will be sorted by material first, then by distance.
 - @ref BansheeEngine::StateReduction::Distance "StateReduction::Distance" - Elements will be sorted by distance first, then by material.
 
Once the state reduction mode is set call @ref BansheeEngine::RenderQueue::sort "RenderQueue::sort", and then @ref BansheeEngine::RenderQueue::getSortedElements "RenderQueue::getSortedElements" to retrieve a sorted list of render elements. The returned list contains a list of @ref BansheeEngine::RenderQueueElement "RenderQueueElement" which lets you know exactly which render element to render using which pass, and also tells you when a new pass needs to be applied (if multiple elements in a row use the same pass, it doesn't need to be re-applied).

For example:
~~~~~~~~~~~~~{.cpp}
Vector<RenderableElement*> elements = ...;

SPtr<RenderQueue> queue = bs_shared_ptr_new<RenderQueue>(StateReduction::Distance);
for(auto& element : elements)
{
	float distance = ...; // Calculate distance from element to camera, for sorting
	queue->add(element, distance);
}

queue->sort();
const Vector<RenderQueueElement>& sortedElements = queue->getSortedElements();
... render sorted elements ...
~~~~~~~~~~~~~

### Renderer material {#renderer_b_a_b}
Often the renderer needs to use special shaders for various effects (e.g. post-processing effects like FXAA). Unlike shaders and materials used by users, these shaders are built-in into the engine. Since we know they'll always be there we can make it easier for the renderer to load and use them by implementing the @ref BansheeEngine::RendererMaterial "RendererMaterial" interface:
~~~~~~~~~~~~~{.cpp}
// Set up a post-processing material
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	// Required macro pointing to the shader file
	RMAT_DEF("Downsample.bsl");

public:
	DownsampleMat()
	{
		// Retrieve material parameters, and perform other set-up
		mInputTexture = mMaterial->getParamTexture("gInputTex");
		mInvTexSize = mMaterial->getParamVec2("gInvTexSize");
	}

	// Set up parameters and render a full screen quad using the material
	void execute(const SPtr<TextureCore>& input)
	{
		// Actually assign parameters before rendering
		mInputTexture.set(input);
		
		const RenderTextureProperties& rtProps = target->getProperties();
		Vector2 invTextureSize(1.0f / rtProps.getWidth(), 1.0f / rtProps.getHeight());

		mParams.gInvTexSize.set(invTextureSize);
		
		... set up the render target ...
		... render using the material ...
	}
private:
	MaterialParamVec2Core mInvTexSize;
	MaterialParamTextureCore mInputTexture;
};

// Method defined in RMAT_DEF
void DirectionalLightMat::_initDefines(ShaderDefines& defines)
{
	// Set up optional defines to control shader compilation
	defines.set("ENABLE_HIGH_QUALITY", 1);
}

~~~~~~~~~~~~~

Renderer material implementation starts by deriving from @ref BansheeEngine::RendererMaterial<T> "RendererMaterial<T>". This is followed by a declaration of the @ref RMAT_DEF macro, which contains a path to the shader file. The shader file should be located in "Data/Raw/Engine/Shaders/" folder.

You must also implement `_initDefines` method, which allows you to modify the compilation environment. It can be empty if not required, but it is useful if your shader has different settings in the form of \#ifdef blocks, in which case different renderer materials can reference the same file, but yield different results depending on what is set in this method. Be aware that all built-in shaders are pre-processed by the @ref BansheeEngine::BuiltinResources "BuiltinResources" manager. If you are changing define options you should delete the "Data/Engine/Timestamp.asset" file, which will force the manager to rebuild all shaders (and actually apply the new defines).

Once these methods/macros are implemented, you can then instantiate your renderer material and access the @ref BansheeEngine::Material "Material" using the `mMaterial` field. Normally your material will have some parameters, which you'll want to retrieve in the constructor, and then set before rendering. 

@see @ref materials to learn how to work with materials in general.
@see @ref renderTargets to learn how to create and render to a render target.

### Parameter blocks {#renderer_b_a_c}
If you shader has data parameters (like float, int, bool) it is efficient to group them into blocks of parameters. These blocks are better known as uniform buffers in OpenGL, or constant buffers in DX11. An example of such a buffer in HLSL looks like this:
~~~~~~~~~~~~~{.cpp}
// Contains various parameters specific to the current camera
cbuffer PerCamera
{
	float3	 gViewDir;
	float3 	 gViewOrigin;
	float4x4 gMatViewProj;
	float4x4 gMatView;
	float4x4 gMatProj;
	float4x4 gMatInvProj;
	float4x4 gMatInvViewProj;			
}
~~~~~~~~~~~~~

While you could assign these parameters using the standard way as described in the [material](@ref materials) manual:
~~~~~~~~~~~~~{.cpp}
MaterialParamVec3Core viewDirParam = mMaterial->getParamVec3("gViewDir");
viewDirParam.set(Vector3(0.707.0, 0.707f, 0.0f));
// Or just simply:
// mMaterial->setVec3("gViewDir", Vector3(0.707.0, 0.707f, 0.0f));

... repeat for all other parameters
~~~~~~~~~~~~~

It is more efficient to assign them as a parameter block. Primarily because parameter blocks can be set once, and then shared between multiple materials. To set up a parameter block use the @ref BS_PARAM_BLOCK_BEGIN, @ref BS_PARAM_BLOCK_ENTRY and @ref BS_PARAM_BLOCK_END macros, like so:

~~~~~~~~~~~~~{.cpp}
BS_PARAM_BLOCK_BEGIN(PerCameraParamBlock)
	BS_PARAM_BLOCK_ENTRY(Vector3, gViewDir)
	BS_PARAM_BLOCK_ENTRY(Vector3, gViewOrigin)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatView)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatProj)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvProj)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvViewProj)
BS_PARAM_BLOCK_END
~~~~~~~~~~~~~

By using this approach you lose all the error checking normally performed by @ref BansheeEngine::Material "Material". You must make sure that the layout in C++ matches the layout in the GPU program. In case of GLSL you must also specify `layout(std140)` keyword to ensure its layout is compatible with C++ struct layout. You must also make sure that variable names match the names in the GPU program.

Once your parameter block is created, you can instantiate it, assign values to it, and assign the blocks to materials, like so:
~~~~~~~~~~~~~{.cpp}
PerCameraParamBlock block; // Instantiate block, normally you'd want to store this
block.gViewDir.set(Vector3(0.707.0, 0.707f, 0.0f));
... set other parameters in block ...

SPtr<MaterialCore> material = ...;
material->setParamBlockBuffer("PerCamera", block.getBuffer());

... render using the material ...
~~~~~~~~~~~~~

Blocks are often used with renderer materials we described in the previous section, although we didn't use one in that example.

### Renderer semantics {#renderer_b_a_d}
Renderer semantics allow user created shaders to request that certain parameters in a GPU program are populated by the renderer. See the [materials](@ref materials) and [BSLFX](@ref bslfx) manuals on how to specify such semantics. 

For example the user might request a "VP" semantic, which could be recognized by the renderer that the shader requests a view-projection matrix. Such a matrix is not something that the user should have to assign to the material himself. The renderer can choose to parse material parameters looking for supported semantics, and assign their values.

For example:
~~~~~~~~~~~~~{.cpp}
StringID RPS_ViewProjTfrm = "VP"; // Define semantic identifier

SPtr<MaterialCore> material = ...;
SPtr<ShaderCore> shader = material->getShader();
auto& dataParams = shader->getDataParams();
for (auto& entry : texParams)
{
	if (entry.second.rendererSemantic == RPS_ViewProjTfrm)
	{
		// Found it, assign some value to the parameter
		mMaterial->setMat4(entry.second.name, Matrix4::IDENTITY);
		break;
	}
}
~~~~~~~~~~~~~

You can choose to implement semantics in your renderer, but they aren't required. You can just as easily require the user to use specific parameter names, and then use those names directly (like we used in previous examples in this manual).

### RendererUtility {#renderer_b_a_e}

@ref BansheeEngine::RendererUtility "RendererUtility" provides some commonly required functionality for rendering. For the most part these are just wrappers around @ref BansheeEngine::RenderAPI "RenderAPI" methods, so you don't have to set things like vertex/index buffers manually, as described in [render API](@ref renderAPI) manual.
 - @ref BansheeEngine::RendererUtility::setPass "RendererUtility::setPass" - Binds a pass from a specific @ref BansheeEngine::Material "Material" for rendering. Any further draw calls will be rendered using this pass.
 - @ref BansheeEngine::RendererUtility::setPassParams "RendererUtility::setPassParams" - Binds parameters (textures, samplers, etc.) from a @ref BansheeEngine::Material "Material". Any further draw calls will be rendered using these parameters.
 - @ref BansheeEngine::RendererUtility::draw "RendererUtility::draw" - Draws a specific sub-mesh of the provided @ref BansheeEngine::MeshCore "Mesh", using the currently bound pass.
 - @ref BansheeEngine::RendererUtility::blit "RendererUtility::blit" - Copies the contents of the provided texture into the currently bound render target.
 - @ref BansheeEngine::RendererUtility::drawScreenQuad "RendererUtility::drawScreenQuad" - Draws a quad using the currently bound pass.
 
Use @ref BansheeEngine::gRendererUtility "gRendererUtility" to access the @ref BansheeEngine::RendererUtility "RendererUtility" more easily.
 
Binding a material and rendering using @ref BansheeEngine::RendererUtility "RendererUtility":
~~~~~~~~~~~~~{.cpp}
SPtr<MaterialCore> material = ...;
SPtr<MeshCore> mesh = ...;

gRendererUtility().setPass(material);
... set material parameters ...
gRendererUtility().setPassParams(material);
gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
~~~~~~~~~~~~~
 
### RenderTexturePool {#renderer_b_a_f}
Although you can create render textures manually as described in the [render target](@ref renderTargets) manual, @ref BansheeEngine::RenderTexturePool "RenderTexturePool" provides a simpler and more efficient way of doing it. It will keep alive any referenced render textures, so that other systems may re-use them if their size/formats match. This can improve performance when using many temporary/intermediary render textures (like in post-processing).

To request a render texture, first populate the @ref BansheeEngine::POOLED_RENDER_TEXTURE_DESC "POOLED_RENDER_TEXTURE_DESC" descriptor, by calling either @ref BansheeEngine::POOLED_RENDER_TEXTURE_DESC::create2D "POOLED_RENDER_TEXTURE_DESC::create2D", @ref BansheeEngine::POOLED_RENDER_TEXTURE_DESC::create3D "POOLED_RENDER_TEXTURE_DESC::create3D" or @ref BansheeEngine::POOLED_RENDER_TEXTURE_DESC::createCube "POOLED_RENDER_TEXTURE_DESC::createCube".

Then call @ref BansheeEngine::RenderTexturePool::get "RenderTexturePool::get" which will either create a new render texture, or return one from the pool. The returned object is @ref BansheeEngine::PooledRenderTexture "PooledRenderTexture" from which you can access the actual render texture.

Once you are done using the texture, call @ref BansheeEngine::RenderTexturePool::release "RenderTexturePool::release" to return the texture to the pool, and make it available for other systems. If you plan on using this texture again, make sure to keep a reference to the @ref BansheeEngine::PooledRenderTexture "PooledRenderTexture". This will prevent the pool from fully destroying the texture so it may be reused.

For example:
~~~~~~~~~~~~~{.cpp}
POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8G8B8A8, 1024, 1024);
SPtr<PooledRenderTexture> pooledRT = RenderTexturePool::instance().get(desc);

RenderAPICore::instance().setRenderTarget(pooledRT->renderTexture);
... render to target ...
RenderTexturePool::instance().release(pooledRT);
// Keep a reference to pooledRT if we plan on re-using it, then next time just call get() using the same descriptor
~~~~~~~~~~~~~

### Renderer options {#renderer_b_a_g}
You can customize your rendering at runtime by implementing the @ref BansheeEngine::CoreRendererOptions "RendererOptions" class. Your @ref BansheeEngine::CoreRendererOptions "RendererOptions" implementation can then be assigned to the renderer by calling @ref BansheeEngine::CoreRenderer::setOptions "Renderer::setOptions", and accessed within the renderer via the `mOptions` field. No default options are provided and it's up to your renderer to decide what it requires.

Be aware that options are set from the simulation thread, and if you want to use them on the core thread to either properly synchronize the access, or send a copy of the options to the core thread.