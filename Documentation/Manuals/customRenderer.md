Creating a renderer plugin							{#customRenderer}
===============
[TOC]

If your project requires a very specific form of rendering you might decide you want to write your own renderer from scratch. In bs::f renderers are built as plugins, and this manual will show you how to create one. This manual can also be useful if trying to understand how the renderer works, even if you are not implementing your own.

# Components and the renderer {#renderer_a}
We've already shown how to render scene objects. You create a **SceneObject** on which you then attach components such as **CCamera**, **CRenderable** or **CLight**. These components will then register themselves with the renderer, when takes care of everything else rendering-wise.

Renderer isn't actually aware of scene objects and components, and instead operates on lower level objects owned by those components. For example **CCamera** component internally owns a @ref bs::Camera "Camera" object, and **CRenderable** component internally owns a @ref bs::Renderable "Renderable" object. As a general rule the non-component versions are named the same, without the "C" prefix. Their interface is very similar to their component versions.

All such lower level objects used by the renderer are also **CoreObject**%s, meaning they have both a simulation and a core thread counterparts. 
 
# Renderer plugin interface {#renderer_b}
To create your own renderer you must implement the @ref bs::ct::Renderer "ct::Renderer" interface. Renderer executes on the core thread, although there are a few simulation thread methods we'll note specifically.

## Scene state notifications {#renderer_b_a}
Majority of the renderer interface consists of methods that notify the renderer when certain objects are added, updated or removed from the scene. The types of objects that the renderer cares about are:
 - @ref bs::Camera "Camera"
 - @ref bs::Renderable "Renderable"
 - @ref bs::Light "Light"
 - @ref bs::ReflectionProbe "ReflectionProbe"
 - @ref bs::Skybox "Skybox"
 - @ref bs::LightProbeVolume "LightProbeVolume"
 - @ref bs::ParticleSystem "ParticleSystem"
 - @ref bs::Decal "Decal"
  
Whenever such objects are created, destroyed or some property on them is updated, one of the following methods is called:
 - @ref bs::ct::Renderer::notifyCameraAdded "ct::Renderer::notifyCameraAdded()" - Called when a new **Camera** is created (e.g. when a **CCamera** component is added to the scene).
 - @ref bs::ct::Renderer::notifyCameraUpdated "ct::Renderer::notifyCameraUpdated()" - Called when **Camera** position, rotation or scale changes.
 - @ref bs::ct::Renderer::notifyCameraRemoved "ct::Renderer::notifyCameraRemoved()" - Called when a **Camera** is destroyed.
 - @ref bs::ct::Renderer::notifyRenderableAdded "ct::Renderer::notifyRenderableAdded()" - Called when a new **Renderable** is created (e.g. when a **CRenderable** component is added to the scene).
 - @ref bs::ct::Renderer::notifyRenderableUpdated "ct::Renderer::notifyRenderableUpdated()" - Called when **Renderable** position, rotation or scale changes.
 - @ref bs::ct::Renderer::notifyRenderableRemoved "ct::Renderer::notifyRenderableRemoved()" - Called when a **Renderable** is destroyed.
 - @ref bs::ct::Renderer::notifyLightAdded "ct::Renderer::notifyLightAdded()" - Called when a new **Light** is created (e.g. when a **CLight** component is added to the scene).
 - @ref bs::ct::Renderer::notifyLightUpdated "ct::Renderer::notifyLightUpdated()" - Called when **Light** position, rotation or scale changes.
 - @ref bs::ct::Renderer::notifyLightRemoved "ct::Renderer::notifyLightRemoved()" - Called when a **Light** is destroyed.
 - @ref bs::ct::Renderer::notifyReflectionProbeAdded "ct::Renderer::notifyReflectionProbeAdded()" - Called when a new **ReflectionProbe** is created (e.g. when a **CReflectionProbe** component is added to the scene).
 - @ref bs::ct::Renderer::notifyReflectionProbeUpdated "ct::Renderer::notifyReflectionProbeUpdated()" - Called when **ReflectionProbe** position, rotation or scale changes.
 - @ref bs::ct::Renderer::notifyReflectionProbeRemoved "ct::Renderer::notifyReflectionProbeRemoved()" - Called when a **ReflectionProbe** is destroyed.
 - @ref bs::ct::Renderer::notifySkyboxAdded "ct::Renderer::notifySkyboxAdded()" - Called when a new **Skybox** is created (e.g. when a **CSkybox** component is added to the scene).
 - @ref bs::ct::Renderer::notifySkyboxRemoved "ct::Renderer::notifySkyboxRemoved()" - Called when a **Skybox** is destroyed. 
 - @ref bs::ct::Renderer::notifyLightProbeVolumeAdded "ct::Renderer::notifyLightProbeVolumeAdded()" - Called when a new **LightProbeVolume** is created (e.g. when a **CLightProbeVolume** component is added to the scene).
 - @ref bs::ct::Renderer::notifyLightProbeVolumeUpdated "ct::Renderer::notifyLightProbeVolumeUpdated()" - Called when probes are added or modified in a **LightProbeVolume**. 
 - @ref bs::ct::Renderer::notifyLightProbeVolumeRemoved "ct::Renderer::notifyLightProbeVolumeRemoved()" - Called when a **LightProbeVolume** is destroyed.
 - @ref bs::ct::Renderer::notifyParticleSystemAdded "ct::Renderer::notifyParticleSystemAdded()" - Called when a new **ParticleSystem** is created (e.g. when a **CParticleSystem** component is added to the scene).
 - @ref bs::ct::Renderer::notifyParticleSystemUpdated "ct::Renderer::notifyParticleSystemUpdated()" - Called when particle system transform changes or when its settings are modified. 
 - @ref bs::ct::Renderer::notifyParticleSystemRemoved "ct::Renderer::notifyParticleSystemRemoved()" - Called when a **ParticleSystem** is destroyed.
 - @ref bs::ct::Renderer::notifyDecalAdded "ct::Renderer::notifyDecalAdded()" - Called when a new **Decal** is created (e.g. when a **CDecal** component is added to the scene).
 - @ref bs::ct::Renderer::notifyDecalUpdated "ct::Renderer::notifyDecalUpdated()" - Called when decal position, rotation or scale changes.
 - @ref bs::ct::Renderer::notifyDecalRemoved "ct::Renderer::notifyDecalRemoved()" - Called when a **Decal** is destroyed.
 
Your renderer implementation can choose to implement some or all of those methods. By implementing these methods your renderer implementation is expected to keep track of the scene state, and then use that scene state for rendering. For example most renderers will at least need to keep track of all active cameras and renderable objects.
 
Note that *Added/*Removed methods don't only get called on object creation/destruction. If some major property of the object changes (e.g. a mesh or a material on a **Renderable**), the system will call **ct::Renderer::notifyRenderableRemoved()** followed by a call to **ct::Renderer::notifyRenderableAdded()**.
 
## Rendering {#renderer_b_b}
Aside from keeping track of the state of the scene your renderer must also implement @ref bs::ct::Renderer::renderAll "ct::Renderer::renderAll()". This method will be called every frame and it is the starting point for all rendering. Note that this method gets called from the simulation thread, and you are expected to manually launch rendering on the core thread.

~~~~~~~~~~~~~{.cpp}
class MyRenderer : public Renderer
{
	// ... other renderer methods
	
	void renderAll() 
	{
		// ... do any sim thread operations if required ...
		
		// Queue rendering
		gCoreThread().queueCommand(std::bind(&MyRenderer::renderAllCore, this));
	}
	
	void renderAllCore()
	{
		// ... iterate over all cameras and renderables, call RenderAPI and other low-level rendering methods to actually render something ...
	}
};
~~~~~~~~~~~~~

The implementation of your rendering method should iterate over all renderable objects, cameras, lights or other provided objects (depending on what kind of rendering you wish to do). The rendering happens through the low-level rendering API as described in earlier manuals. At the end of rendering, every render target in every active camera should be filled with an image of the rendered scene. During rendering you should consider mesh and material set on renderable objects, and optionally apply lighting, special or post-processing effects as needed. 

## Name {#renderer_b_c}
You are required to give your renderer a name by overriding @ref bs::ct::Renderer::getName "ct::Renderer::getName()". 

~~~~~~~~~~~~~{.cpp}
class MyRenderer : public Renderer
{
	const StringID& MyRenderer::getName() const
	{
		static StringID name = "MyRenderer";
		return name;
	}

	// ... other renderer methods
};
~~~~~~~~~~~~~

At this point your renderer is ready for use, but there is still various optional functionality to cover.

## Extensions {#renderer_b_d}
We talked about how to implement renderer extensions in the previous chapter. But if you are implementing your own renderer you need to process those extensions during the rendering process. You may also choose to ignore extensions and not render them at all.

All registered extensions are part of the **Renderer::mCallbacks** field. You can choose to iterate over them and execute them as needed.

~~~~~~~~~~~~~{.cpp}
class MyRenderer : public Renderer
{
	// ... other renderer methods

	// Performs rendering for a single camera, on the core thread
	void render(const SPtr<Camera>& camera)
	{
		// Render pre-base pass extensions
		auto iter = mCallbacks.begin();
		while (iter != mCallbacks.end())
		{
			RendererExtension* extension = *iter;
			if (extension->getLocation() != RenderLocation::PreBasePass)
				break;

			if (extension->check(*camera))
				extension->render(*camera);

			++iter;
		}
		
		// ... do normal rendering and optionally call extensions registered for other locations ...
	}
};
~~~~~~~~~~~~~

# Utilities {#renderer_c}
While what we have shown so far is enough to create a custom renderer, there are also a variety of utilities that can help out in the process. These systems aren't critical for renderer creation, but instead provide an easier way to perform commonly required functions.

## RendererUtility {#renderer_c_a}
@ref bs::ct::RendererUtility "ct::RendererUtility" provides some commonly required functionality for rendering. For the most part it provides methods that are wrappers around various **RenderAPI** methods described previously. It can be accessed globally through @ref bs::ct::gRendererUtility() "ct::gRendererUtility()" and the relevant methods are:
 - @ref bs::ct::RendererUtility::setPass "ct::RendererUtility::setPass()" - Binds a pass from a specific **Material** for rendering. Any further draw calls will be rendered using this pass.
 - @ref bs::ct::RendererUtility::setPassParams "ct::RendererUtility::setPassParams()" - Binds parameters (textures, samplers, etc.) from a **Material**, in the form of **GpuParamsSet**. Any further draw calls will be rendered using these parameters.
 - @ref bs::ct::RendererUtility::draw "ct::RendererUtility::draw()" - Draws a specific sub-mesh of the provided **ct::Mesh**, using the currently bound pass.
 - @ref bs::ct::RendererUtility::blit "ct::RendererUtility::blit()" - Copies the contents of the provided texture into the currently bound render target.
 - @ref bs::ct::RendererUtility::drawScreenQuad "ct::RendererUtility::drawScreenQuad()" - Draws a quad covering the screen using the currently bound pass.

~~~~~~~~~~~~~{.cpp}
SPtr<Material> material = ...;
SPtr<Mesh> mesh = ...;
SPtr<GpuParamsSet> paramsSet = material->createParamsSet();

gRendererUtility().setPass(material);
... set material parameters as normal ...
gRendererUtility().setPassParams(paramsSet);
gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
~~~~~~~~~~~~~

## Render queue {#renderer_c_b}
@ref bs::ct::RenderQueue "ct::RenderQueue" allows you to sort and group scene objects for rendering. For example transparent objects might need to be sorted back to front based on their distance from the camera. It is also often useful to group objects if they share the same material, to reduce state switching which can improve performance.

Use @ref bs::ct::RenderQueue::add "ct::RenderQueue::add()" to add new objects to the queue. It expects a @ref bs::ct::RenderableElement "ct::RenderableElement" which you can create from information provided by **Renderable** when **ct::Renderer::notifyRenderableAdded()** is called. Normally you wish to have a single **ct::RenderableElement** for each sub-mesh present in the renderable object's mesh.

Once all elements are in the queue, you can call @ref bs::ct::RenderQueue::setStateReduction "ct::RenderQueue::setStateReduction()" to select how to sort the objects:
 - @ref bs::ct::StateReduction::None "ct::StateReduction::None" - Elements will be sorted by distance but no state reduction by material will occurr.
 - @ref bs::ct::StateReduction::Material "ct::StateReduction::Material" - Elements will be sorted by material first, then by distance.
 - @ref bs::ct::StateReduction::Distance "ct::StateReduction::Distance" - Elements will be sorted by distance first, then by material.
 
Once the state reduction mode is set call @ref bs::ct::RenderQueue::sort "ct::RenderQueue::sort()", and then @ref bs::ct::RenderQueue::getSortedElements "ct::RenderQueue::getSortedElements()" to retrieve a sorted list of render elements. The returned list contains a list of @ref bs::ct::RenderQueueElement "ct::RenderQueueElement" which lets you know exactly which render element to render using which pass, and also tells you when a new pass needs to be applied.

For example:
~~~~~~~~~~~~~{.cpp}
Vector<RenderableElement*> elements = ...; // Fill this up from a list of renderables

SPtr<RenderQueue> queue = bs_shared_ptr_new<RenderQueue>(StateReduction::Distance);
for(auto& element : elements)
{
	float distance = ...; // Calculate distance from element to camera, for sorting
	queue->add(element, distance);
}

queue->sort();
const Vector<RenderQueueElement>& sortedElements = queue->getSortedElements();
... render sorted elements using the low level rendering API ...
~~~~~~~~~~~~~

## Renderer material {#renderer_c_c}
Often the renderer needs to use special shaders for various effects (e.g. resolving lighting for a deferred renderer or post-processing effects like FXAA). Unlike shaders and materials used by renderable objects, these shaders are built into the engine. Since we know they'll always be there we can make it easier for the renderer to load and use them by implementing the @ref bs::ct::RendererMaterial "ct::RendererMaterial<T>" interface. 

The template parameter must be name of your material implementation class. The class must contain a @ref RMAT_DEF macro which contains the filename of the shader the renderer material uses. The shader file should be present in the "Data/Raw/Engine/Shaders/" folder. 

~~~~~~~~~~~~~{.cpp}
// Set up a post-processing material that downsamples some texture
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	// Required macro pointing to the shader file
	RMAT_DEF("Downsample.bsl");
};
~~~~~~~~~~~~~

Once defined the renderer material can be accessed through the static @ref bs::ct::RendererMaterial::get<T>() "ct::RendererMaterial::get<T>()" method.

~~~~~~~~~~~~~{.cpp}
DownsampleMat* renderMat = DownsampleMat::get():
~~~~~~~~~~~~~

Once retrieved the object will contain the instance of the shader in the path you provided. Internally the material will provide you with a reference to either a graphics or compute pipeline state as *mGfxPipeline* and *mComputePipeline*, depending on the type of shader that was loaded. It will also provide you with **GpuParams** in the *mParams* field.

When the material is first created you will likely want to add a constructor in which you look up any necessary parameters the material might require, so they can be set more easily when rendering.
~~~~~~~~~~~~~{.cpp}
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	RMAT_DEF("Downsample.bsl");

public:
	DownsampleMat()
	{
		// Retrieve material parameters, and optionally perform other set-up
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	GpuParamTexture mInputTexture;
};
~~~~~~~~~~~~~

Often you can also create a method that binds the parameters, binds the material and executes the material all in one. This way external code doesn't need to do anything but to call it.

~~~~~~~~~~~~~{.cpp}
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	// ... other DownsampleMat code ...
	
	// Set up parameters and render a full screen quad using the material
	void execute(const SPtr<Texture>& input)
	{
		// Assign parameters before rendering
		mInputTexture.set(input);
		
		bind();
		gRendererUtility().drawScreenQuad();
	}

	// ... other DownsampleMat code ...
};
~~~~~~~~~~~~~

Note that a helper method @ref bs::ct::RendererMaterial::bind() "ct::RendererMaterial::bind()" is provided, which will bind both the GPU pipeline and parameters.

~~~~~~~~~~~~~{.cpp}
// External code wanting to run the material
SPtr<Texture> inputTex = ...;

DownsampleMat* renderMat = DownsampleMat::get():
renderMat->execute(inputTex);
~~~~~~~~~~~~~

### Variations {#renderer_c_c_a}

If your BSL file contains shader variations, then you can call @ref bs::ct::RendererMaterial::get<T>(const ShaderVariation&) "ct::RendererMaterial::get<T>(const ShaderVariation&)" to retrieve a specific variation. Variations were explained in more detail in the BSL manual.

~~~~~~~~~~~~~{.cpp}
// External code wanting to run a specific variation of the material
SPtr<Texture> inputTex = ...;

// Get the variation that has HIGH_QUALITY define enabled
ShaderVariation variation = ShaderVariation({
	ShaderVariation::Param("HIGH_QUALITY", true)
});

DownsampleMat* renderMat = DownsampleMat::get(variation):
renderMat->execute(inputTex);
~~~~~~~~~~~~~

Normally you will want to handle creation of various @ref bs::ShaderVariations "ShaderVariations" structures through a templated method, like so:

~~~~~~~~~~~~~{.cpp}
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	template<bool highQuality>
	static const ShaderVariation& getVariation()
	{
		static ShaderVariation variation = ShaderVariation({
			ShaderVariation::Param("HIGH_QUALITY", highQuality)
		});

		return variation;
	}

	// ... other DownsampleMat code ...
};
~~~~~~~~~~~~~

Then you can also add a static **getVariation** method to hide these internals from the caller.

~~~~~~~~~~~~~{.cpp}
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	// ... other DownsampleMat code ...
	
public:
	static DownsampleMat* getVariation(bool highQuality)
	{
		if(highQuality)
			return get(getVariation<true>());
			
		return get(getVariation<false>());
	}
};
~~~~~~~~~~~~~

Now the calling code can simply retrieve the variation it requires.

~~~~~~~~~~~~~{.cpp}
// External code wanting to run the high quality version of the material
SPtr<Texture> inputTex = ...;

DownsampleMat* renderMat = DownsampleMat::getVariation(true):
renderMat->execute(inputTex);
~~~~~~~~~~~~~

### Defines {#renderer_c_c_b}

Sometimes you wish to be able to dynamically control defines that are used to compile the shader code. This is particularily useful if you want to make sure your C++ code and shader code use the same value. To do this you need to create your material using the @ref RMAT_DEF_CUSTOMIZED macro, instead of **RMAT_DEF**. It has the exact same signature as **RMAT_DEF** but it provides an *_initDefines* method you must implement.

The method receives a @ref bs::ShaderDefines "ShaderDefines" object which you can then populate with relevant values. Those values will then be used when compiling the shader.

~~~~~~~~~~~~~{.cpp}
constxpr static UINT32 TILE_WIDTH = 8;
constxpr static UINT32 TILE_HEIGHT = 8;
constxpr static UINT32 PIXELS_PER_THREAD = 4;

void IrradianceComputeSHMat::_initDefines(ShaderDefines& defines)
{
	defines.set("TILE_WIDTH", TILE_WIDTH);
	defines.set("TILE_HEIGHT", TILE_HEIGHT);
	defines.set("PIXELS_PER_THREAD", PIXELS_PER_THREAD);
}
~~~~~~~~~~~~~

> All builtin shaders are cached. The system will automatically pick up any changes to shaders in *Data/Raw/Engine* folder and rebuild the cache when needed. However if you are changing defines as above you must manually force the system to rebuild by modifying the BSL file in *Data/Raw/Engine* folder.

## Parameter blocks {#renderer_c_d}
In the @ref gpuPrograms manual we talked about parameter block buffers, represented by **GpuParamBlockBuffer** class. These blocks are used for group data parameters (such as float, int or bool) into blocks that can then be efficiently bound to the pipeline. They are better known as uniform buffers in OpenGL/Vulkan, or constant buffers in DX11. 

An example of such a buffer in HLSL looks like this:
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

Such parameter block buffers are primarily useful when you need to share the same data between multiple materials. Instead of accessing parametes individually through **Material** or **GpuParams**, you would instead create a **GpuParamBlockBuffer** object, populate it, and then bind to **Material** or **GpuParams**.

When we talked about them earlier we have shown how to manually create a **GpuParamBlockBuffer** object and write to it by reading the **GpuParamDesc** object of the **GpuProgram**. This is cumbersome and requires a lot of boilerplate code. A simpler way of creating and populating a parameter block is to use @ref BS_PARAM_BLOCK_BEGIN, @ref BS_PARAM_BLOCK_ENTRY and @ref BS_PARAM_BLOCK_END macros. You simply define the parameter block structure using these macros in C++, to match the structure in HLSL/GLSL code.

~~~~~~~~~~~~~{.cpp}
BS_PARAM_BLOCK_BEGIN(PerCameraParamBlockDef)
	BS_PARAM_BLOCK_ENTRY(Vector3, gViewDir)
	BS_PARAM_BLOCK_ENTRY(Vector3, gViewOrigin)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatView)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatProj)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvProj)
	BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvViewProj)
BS_PARAM_BLOCK_END
~~~~~~~~~~~~~

Once your parameter block definition is created, you can instantiate a parameter block buffer, assign values to it, and assign the blocks to materials, like so:
~~~~~~~~~~~~~{.cpp}
PerCameraParamBlockDef def; // Normally you want to make this global so it's instantiated only once

// Instantiates a new parameter block from the definition
SPtr<GpuParamBlockBuffer> paramBlock = def.createBuffer(); 

// Assign a value to the gViewDir parameter of the parameter block
def.gViewDir.set(paramBlock, Vector3(0.707f, 0.707f, 0.0f));
... set other parameters in block ...

// Assign the parameter block to the material (optionally, assign to GpuParams if using them directly)
SPtr<Material> material = ...;
material->setParamBlockBuffer("PerCamera", paramBlock);

... render using the material ...
~~~~~~~~~~~~~

Blocks are often used with renderer materials we described in the previous section, although we didn't use one in that example.

Note that by using this approach you lose all the error checking normally performed by **Material** or **GpuParams** when you are assigning parameters individually. You must make sure that the layout in C++ matches the layout in the GPU program. In case of GLSL you must also specify `layout(std140)` keyword to ensure its layout is compatible with C++ struct layout. You must also make sure that variable names match the names in the GPU program code.

## Renderer semantics {#renderer_c_e}
Renderer semantics allow user created shaders to request that certain parameters in a GPU program are populated by the renderer. They can be specified when defining shader parameters.

For example the user might request a "VP" semantic, which could be recognized by the renderer that the shader requests a view-projection matrix. Such a matrix is not something that the user should have to assign to the material himself. The renderer can choose to parse material parameters looking for supported semantics, and assign their values. 

Ultimately whether the renderer chooses to parse the semantics or not is up to the renderer. Currently the default *RenderBeast* renderer does not make use of any semantics and instead maps parameters directly by using their name.

The semantics for each parameter can be accessed through the **Shader** object, which renderer needs to iterate through manually.

~~~~~~~~~~~~~{.cpp}
StringID RPS_ViewProjTfrm = "VP"; // Define semantic identifier

SPtr<Material> material = ...;
SPtr<Shader> shader = material->getShader();
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

## GpuResourcePool {#renderer_c_f}
Although you can create textures and buffers manually as described in the low level rendering API manual, @ref bs::ct::GpuResourcePool "ct::GpuResourcePool" provides a simpler and more efficient way of doing it. It will keep alive any referenced textures and buffers, so that other systems may re-use them if their size/formats match. This can improve performance when using many temporary/intermediary render textures (like in post-processing) or load-store buffers.

To request a render texture, first populate the @ref bs::ct::POOLED_RENDER_TEXTURE_DESC "ct::POOLED_RENDER_TEXTURE_DESC" descriptor, by calling any of @ref bs::ct::POOLED_RENDER_TEXTURE_DESC::create2D "ct::POOLED_RENDER_TEXTURE_DESC::create2D()", @ref bs::ct::POOLED_RENDER_TEXTURE_DESC::create3D "ct::POOLED_RENDER_TEXTURE_DESC::create3D()" or @ref bs::ct::POOLED_RENDER_TEXTURE_DESC::createCube "ct::POOLED_RENDER_TEXTURE_DESC::createCube()".

To request a buffer, populate the @ref bs::ct::POOLED_STORAGE_BUFFER_DESC "ct::POOLED_STORAGE_BUFFER_DESC" descriptor by calling either @ref bs::ct::POOLED_STORAGE_BUFFER_DESC::createStandard "ct::POOLED_STORAGE_BUFFER_DESC::createStandard()" or @ref bs::ct::POOLED_STORAGE_BUFFER_DESC::createStructured "ct::POOLED_STORAGE_BUFFER_DESC::createStructured()".

Then call @ref bs::ct::GpuResourcePool::get "ct::GpuResourcePool::get()" with the provided descriptor. This will either create a new render texture/buffer, or return one from the pool. The returned object is @ref bs::ct::PooledRenderTexture "ct::PooledRenderTexture" for textures and @ref bs::ct::PooledStorageBuffer  "ct::PooledStorageBuffer" for buffers.

Once you are done using the texture or buffer, call @ref bs::ct::GpuResourcePool::release "ct::GpuResourcePool::release()" to return the object to the pool, and make it available for other systems. If you plan on using this object again, make sure to keep a reference to the **ct::PooledRenderTexture** / **ct::PooledStorageBuffer** object. This will prevent the pool from fully destroying the object so it may be reused.

~~~~~~~~~~~~~{.cpp}
// An example creating a pooled render texture
POOLED_RENDER_TEXTURE_DESC desc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R8G8B8A8, 1024, 1024, TU_RENDERTARGET);
SPtr<PooledRenderTexture> pooledRT = GpuResourcePool::instance().get(desc);

RenderAPI::instance().setRenderTarget(pooledRT->renderTexture);
... render to target ...
GpuResourcePool::instance().release(pooledRT);
// Keep a reference to pooledRT if we plan on re-using it, then next time just call get() using the same descriptor
~~~~~~~~~~~~~

## Renderer options {#renderer_c_g}
You can customize your rendering at runtime by implementing the @ref bs::ct::RendererOptions "ct::RendererOptions" class. Your **ct::RendererOptions** implementation can then be assigned to the renderer by calling @ref bs::ct::Renderer::setOptions "ct::Renderer::setOptions()", and accessed within the renderer via the **Renderer::mOptions** field. No default options are provided and it's up to your renderer to decide what it requires.

Be aware that options are set from the simulation thread, and if you want to use them on the core thread you need to either properly synchronize the access, or send a copy of the options to the core thread.