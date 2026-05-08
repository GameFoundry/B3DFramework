---
title: Creating a renderer plugin
---

If your project requires a very specific form of rendering you might decide you want to write your own renderer from scratch. In the framework renderers are built as plugins, and this manual will show you how to create one. This manual can also be useful if trying to understand how the renderer works, even if you are not implementing your own.

# Components and the renderer
We've already shown how to render scene objects. You create a **SceneObject** on which you then attach components such as **Camera**, **Renderable** or **Light**. These components will then register themselves with the renderer, which takes care of everything else rendering-wise.

# Renderer plugin interface
To create your own renderer you must implement the @b3d::render::Renderer interface. Renderer executes on the render thread, although there are a few simulation thread methods we'll note specifically.

## Scene state management
The renderer receives scene state through the @b3d::render::RendererScene object. The RendererScene contains data about various types of objects in the scene, such as:
 - @b3d::Camera
 - @b3d::Renderable
 - @b3d::Light
 - @b3d::ReflectionProbe
 - @b3d::Skybox
 - @b3d::LightProbeVolume
 - @b3d::ParticleSystem
 - @b3d::Decal

These objects use two different mechanisms for synchronizing data to the renderer:

### RendererObjectStorage
**Renderable**, **Light**, **Decal**, **ReflectionProbe** and **ParticleSystem** objects use @b3d::RendererObjectStorage. This system maintains packed data arrays on the render thread and synchronizes changes from the main thread in batches each frame, for best performance. Each object allocates a persistent @b3d::RendererId on the main thread, which is resolved to a packed array index on the render thread. Synchronization happens via paired `SyncRead()` (main thread) and `SyncWrite()` (render thread) calls. These object storages can be accessed through the **RendererScene**.

### Direct registration
**Camera**, **Skybox** and **LightProbeVolume** use a simpler registration approach where the renderer is notified directly when objects are added, updated or removed:
 - @b3d::render::RendererScene::RegisterCamera - Registers a new **Camera** in the scene.
 - @b3d::render::RendererScene::UpdateCamera - Updates information about a previously registered **Camera**.
 - @b3d::render::RendererScene::UnregisterCamera - Removes a **Camera** from the scene.
 - @b3d::render::RendererScene::RegisterLightProbeVolume - Registers a new **LightProbeVolume** in the scene.
 - @b3d::render::RendererScene::UpdateLightProbeVolume - Updates information about a previously registered **LightProbeVolume**.
 - @b3d::render::RendererScene::UnregisterLightProbeVolume - Removes a **LightProbeVolume** from the scene.
 - @b3d::render::RendererScene::RegisterSkybox - Registers a new **Skybox** in the scene.
 - @b3d::render::RendererScene::UnregisterSkybox - Removes a **Skybox** from the scene.

By implementing these methods your renderer implementation is expected to keep track of the scene state, and then use that scene state for rendering. For example most renderers will at least need to keep track of all active cameras and renderable objects.
 
## Rendering
Aside from keeping track of the state of the scene your renderer must also implement @b3d::render::Renderer::RenderAll. This method will be called every frame and it is the starting point for all rendering. Note that this method gets called from the simulation thread, and you are expected to manually launch rendering on the render thread.

~~~~~~~~~~~~~{.cpp}
class MyRenderer : public Renderer
{
	// ... other renderer methods
	
	void RenderAll() 
	{
		// ... do any sim thread operations if required ...
		
		// Queue rendering
		GetRenderThread().PostCommand([this]() { RenderAllCore(); });
	}
	
	void RenderAllCore()
	{
		// ... iterate over all cameras and renderables, call GpuCommandBuffer and other low-level rendering methods to actually render something ...
	}
};
~~~~~~~~~~~~~

The implementation of your rendering method should iterate over all renderable objects, cameras, lights or other provided objects (depending on what kind of rendering you wish to do). The rendering happens through the low-level rendering API as described in earlier manuals. At the end of rendering, every render target in every active camera should be filled with an image of the rendered scene. During rendering you should consider mesh and material set on renderable objects, and optionally apply lighting, special or post-processing effects as needed. 

## Name
You are required to give your renderer a name by overriding @b3d::render::Renderer::GetName. 

~~~~~~~~~~~~~{.cpp}
class MyRenderer : public Renderer
{
	const StringID& GetName() const
	{
		static StringID name = "MyRenderer";
		return name;
	}

	// ... other renderer methods
};
~~~~~~~~~~~~~

At this point your renderer is ready for use, but there is still various optional functionality to cover.

## Extensions
We talked about how to implement renderer extensions in the previous chapter. But if you are implementing your own renderer you need to process those extensions during the rendering process. You may also choose to ignore extensions and not render them at all.

All registered extensions are part of the **Renderer::mRendererExtensions** field. You can choose to iterate over them and execute them as needed.

~~~~~~~~~~~~~{.cpp}
class MyRenderer : public Renderer
{
	// ... other renderer methods

	// Performs rendering for a single camera, on the render thread
	void Render(const TShared<Camera>& camera)
	{
		// Render pre-base pass extensions
		auto iter = mRendererExtensions.begin();
		while (iter != mRendererExtensions.end())
		{
			RendererExtension* extension = *iter;
			if (extension->GetLocation() != RenderLocation::PreBasePass)
				break;

			if (extension->Check(*camera))
				extension->Render(*camera, viewContext);

			++iter;
		}
		
		// ... do normal rendering and optionally call extensions registered for other locations ...
	}
};
~~~~~~~~~~~~~

# Utilities
While what we have shown so far is enough to create a custom renderer, there are also a variety of utilities that can help out in the process. These systems aren't critical for renderer creation, but instead provide an easier way to perform commonly required functions.

## RendererUtility
@b3d::render::RendererUtility provides some commonly required functionality for rendering. For the most part it provides methods that are wrappers around various **GpuCommandBuffer** methods described previously. It can be accessed globally through @b3d::render::GetRendererUtility() and the relevant methods are:
 - @b3d::render::RendererUtility::SetPass - Binds a pass from a specific **Material** for rendering. Any further draw calls will be rendered using this pass.
 - @b3d::render::RendererUtility::SetPassParams - Binds parameters (textures, samplers, etc.) from a **Material**, in the form of **MaterialParameterAdapter**. Any further draw calls will be rendered using these parameters.
 - @b3d::render::RendererUtility::Draw - Draws a specific sub-mesh of the provided **render::Mesh**, using the currently bound pass.
 - @b3d::render::RendererUtility::Blit - Copies the contents of the provided texture into the currently bound render target.
 - @b3d::render::RendererUtility::DrawScreenQuad - Draws a quad covering the screen using the currently bound pass.

~~~~~~~~~~~~~{.cpp}
TShared<Material> material = ...;
TShared<Mesh> mesh = ...;
TShared<MaterialParameterAdapter> paramsSet = material->CreateParameterAdapter();

GetRendererUtility().SetPass(cmdBuffer, material);
... set material parameters as normal ...
GetRendererUtility().SetPassParams(cmdBuffer, paramsSet);
GetRendererUtility().Draw(cmdBuffer, mesh, mesh->GetProperties().GetSubMesh(0));
~~~~~~~~~~~~~

## Render queue
@b3d::render::RenderQueue allows you to sort and group scene objects for rendering. For example transparent objects might need to be sorted back to front based on their distance from the camera. It is also often useful to group objects if they share the same material, to reduce state switching which can improve performance.

Use @b3d::render::RenderQueue::Add to add new objects to the queue. It expects a @b3d::render::DrawCommand which you can create from information provided by the object storages in **RendererScene**. Normally you wish to have a single **render::DrawCommand** for each sub-mesh present in the renderable object's mesh.

Once all elements are in the queue, you can call @b3d::render::RenderQueue::SetStateReduction to select how to sort the objects:
 - @b3d::render::StateReduction::None - Elements will be sorted by distance but no state reduction by material will occurr.
 - @b3d::render::StateReduction::Material - Elements will be sorted by material first, then by distance.
 - @b3d::render::StateReduction::Distance - Elements will be sorted by distance first, then by material.
 
Once the state reduction mode is set call @b3d::render::RenderQueue::Sort, and then @b3d::render::RenderQueue::GetSortedEntries to retrieve a sorted list of render elements. The returned list contains a list of @b3d::render::RenderQueueEntry which lets you know exactly which render element to render using which pass, and also tells you when a new pass needs to be applied.

For example:
~~~~~~~~~~~~~{.cpp}
Vector<RenderableElement*> elements = ...; // Fill this up from a list of renderables

TShared<RenderQueue> queue = B3DMakeShared<RenderQueue>(StateReduction::Distance);
for(auto& element : elements)
{
	float distance = ...; // Calculate distance from element to camera, for sorting
	queue->Add(element, distance);
}

queue->Sort();
const Vector<RenderQueueElement>& sortedElements = queue->GetSortedEntries();
... render sorted elements using the low level rendering API ...
~~~~~~~~~~~~~

## Renderer material
Often the renderer needs to use special shaders for various effects (e.g. resolving lighting for a deferred renderer or post-processing effects like FXAA). Unlike shaders and materials used by renderable objects, these shaders are built into the engine. Since we know they'll always be there we can make it easier for the renderer to load and use them by implementing the @b3d::render::RendererMaterial interface. 

The template parameter must be name of your material implementation class. The class must contain a @RMAT_DEF macro which contains the filename of the shader the renderer material uses. The shader file should be present in the "Data/Raw/Engine/Shaders/" folder. 

~~~~~~~~~~~~~{.cpp}
// Set up a post-processing material that downsamples some texture
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	// Required macro pointing to the shader file
	RMAT_DEF("Downsample.bsl");
};
~~~~~~~~~~~~~

Once defined the renderer material can be accessed through the static @b3d::render::RendererMaterial::get<T>() method.

~~~~~~~~~~~~~{.cpp}
DownsampleMat* renderMat = DownsampleMat::Get();
~~~~~~~~~~~~~

Once retrieved the object will contain the instance of the shader in the path you provided. Internally the material will provide you with a reference to either a graphics or compute pipeline state as *mGfxPipeline* and *mComputePipeline*, depending on the type of shader that was loaded. It will also provide you with **GpuParameterSet** in the *mParams* field.

When the material is first created you will likely want to add a constructor in which you look up any necessary parameters the material might require, so they can be set more easily when rendering.
~~~~~~~~~~~~~{.cpp}
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	RMAT_DEF("Downsample.bsl");

public:
	DownsampleMat()
	{
		// Retrieve material parameters, and optionally perform other set-up
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
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
	void execute(GpuCommandBuffer& cmdBuffer, const TShared<Texture>& input)
	{
		// Assign parameters before rendering
		mInputTexture.Set(input);
		
		Bind(cmdBuffer);
		GetRendererUtility().DrawScreenQuad(cmdBuffer);
	}

	// ... other DownsampleMat code ...
};
~~~~~~~~~~~~~

Note that a helper method @b3d::render::RendererMaterial::Bind() is provided, which will bind both the GPU pipeline and parameters.

~~~~~~~~~~~~~{.cpp}
// External code wanting to run the material
TShared<Texture> inputTex = ...;

DownsampleMat* renderMat = DownsampleMat::Get();
renderMat->Execute(inputTex);
~~~~~~~~~~~~~

### Variations

If your BSL file contains shader variations, then you can call @b3d::render::RendererMaterial::Get<T>(const ShaderVariationParameters&) to retrieve a specific variation. Variations were explained in more detail in the BSL manual.

~~~~~~~~~~~~~{.cpp}
// External code wanting to run a specific variation of the material
TShared<Texture> inputTex = ...;

// Get the variation that has HIGH_QUALITY define enabled
ShaderVariation variation = ShaderVariation({
	ShaderVariation::Param("HIGH_QUALITY", true)
});

DownsampleMat* renderMat = DownsampleMat::Get(variation);
renderMat->Execute(inputTex);
~~~~~~~~~~~~~

Normally you will want to handle creation of various @b3d::ShaderVariations structures through a templated method, like so:

~~~~~~~~~~~~~{.cpp}
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	template<bool highQuality>
	static const ShaderVariationParameters& GetVariation()
	{
		static ShaderVariationParameters variation = ShaderVariationParameters({
			ShaderVariationParameter("HIGH_QUALITY", highQuality)
		});

		return variation;
	}

	// ... other DownsampleMat code ...
};
~~~~~~~~~~~~~

Then you can also add a static **GetVariation** method to hide these internals from the caller.

~~~~~~~~~~~~~{.cpp}
class DownsampleMat : public RendererMaterial<DownsampleMat>
{
	// ... other DownsampleMat code ...
	
public:
	static DownsampleMat* GetVariation(bool highQuality)
	{
		if(highQuality)
			return Get(GetVariation<true>());
			
		return Get(GetVariation<false>());
	}
};
~~~~~~~~~~~~~

Now the calling code can simply retrieve the variation it requires.

~~~~~~~~~~~~~{.cpp}
// External code wanting to run the high quality version of the material
TShared<Texture> inputTex = ...;

DownsampleMat* renderMat = DownsampleMat::GetVariation(true);
renderMat->Execute(inputTex);
~~~~~~~~~~~~~

### Defines

Sometimes you wish to be able to dynamically control defines that are used to compile the shader code. This is particularily useful if you want to make sure your C++ code and shader code use the same value. To do this you need to create your material using the @RMAT_DEF_CUSTOMIZED macro, instead of **RMAT_DEF**. It has the exact same signature as **RMAT_DEF** but it provides an *InitDefinesInternal* method you must implement.

The method receives a @b3d::ShaderDefines object which you can then populate with relevant values. Those values will then be used when compiling the shader.

~~~~~~~~~~~~~{.cpp}
constexpr static u32 TILE_WIDTH = 8;
constexpr static u32 TILE_HEIGHT = 8;
constexpr static u32 PIXELS_PER_THREAD = 4;

void IrradianceComputeSHMat::InitDefinesInternal(ShaderDefines& defines)
{
	defines.Set("TILE_WIDTH", TILE_WIDTH);
	defines.Set("TILE_HEIGHT", TILE_HEIGHT);
	defines.Set("PIXELS_PER_THREAD", PIXELS_PER_THREAD);
}
~~~~~~~~~~~~~

> All builtin shaders are cached. The system will automatically pick up any changes to shaders in *Data/Raw/Engine* folder and rebuild the cache when needed. However if you are changing defines as above you must manually force the system to rebuild by modifying the BSL file in *Data/Raw/Engine* folder.

## Uniform buffer definitions
In the [GPU programs](../Low_Level_rendering/gpuPrograms) manual we talked about uniform buffers, represented by **GpuBuffer** objects with uniform buffer usage. These buffers are used to group data parameters (such as float, int or bool) into blocks that can then be efficiently bound to the pipeline. They are better known as uniform buffers in OpenGL/Vulkan, or constant buffers in DX11.

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

Such uniform buffers are primarily useful when you need to share the same data between multiple materials. Instead of accessing parameters individually through **Material** or **GpuParams**, you would instead create a **GpuBuffer** object, populate it, and then bind to **Material** or **GpuParams**.

When we talked about them earlier we have shown how to manually create a **GpuBuffer** object and write to it by reading the **GpuParamDesc** object of the **GpuProgram**. This is cumbersome and requires a lot of boilerplate code. A simpler way of creating and populating a uniform buffer is to use @B3D_UNIFORM_BUFFER_BEGIN, @B3D_UNIFORM_BUFFER_MEMBER and @B3D_UNIFORM_BUFFER_END macros. You simply define the uniform buffer structure using these macros in C++, to match the structure in HLSL/GLSL code.

~~~~~~~~~~~~~{.cpp}
B3D_UNIFORM_BUFFER_BEGIN(PerCameraUniformBufferDef)
	B3D_UNIFORM_BUFFER_MEMBER(Vector3, gViewDir)
	B3D_UNIFORM_BUFFER_MEMBER(Vector3, gViewOrigin)
	B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatViewProj)
	B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatView)
	B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatProj)
	B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatInvProj)
	B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatInvViewProj)
B3D_UNIFORM_BUFFER_END
~~~~~~~~~~~~~

Once your uniform buffer definition is created, you can instantiate a uniform buffer, assign values to it, and assign the buffer to materials, like so:
~~~~~~~~~~~~~{.cpp}
PerCameraUniformBufferDef def; // Normally you want to make this global so it's instantiated only once

// Instantiates a new uniform buffer from the definition
TShared<GpuBuffer> uniformBuffer = def.CreateBuffer();

// Assign a value to the gViewDir parameter of the uniform buffer
def.gViewDir.Set(uniformBuffer, Vector3(0.707f, 0.707f, 0.0f));
... set other parameters in buffer ...

// Assign the uniform buffer to the material (optionally, assign to GpuParams if using them directly)
TShared<Material> material = ...;
material->SetUniformBuffer("PerCamera", uniformBuffer);

... render using the material ...
~~~~~~~~~~~~~

For per-frame data that changes every frame, you should use transient allocations instead:
~~~~~~~~~~~~~{.cpp}
// Allocate a transient buffer (valid for the current frame only)
GpuBufferSuballocation transient = def.AllocateTransient();

// Set parameters directly on the transient allocation
def.gViewDir.Set(transient, Vector3(0.707f, 0.707f, 0.0f));
... set other parameters ...

// Bind the transient allocation to the material
material->SetUniformBuffer("PerCamera", transient);

... render using the material ...
// No need to manually manage frame lifetimes - automatically recycled
~~~~~~~~~~~~~

Uniform buffer definitions are often used with renderer materials we described in the previous section, although we didn't use one in that example.

Note that by using this approach you lose all the error checking normally performed by **Material** or **GpuParams** when you are assigning parameters individually. You must make sure that the layout in C++ matches the layout in the GPU program. In case of GLSL you must also specify `layout(std140)` keyword to ensure its layout is compatible with C++ struct layout. You must also make sure that variable names match the names in the GPU program code.

## Renderer semantics
Renderer semantics allow user created shaders to request that certain parameters in a GPU program are populated by the renderer. They can be specified when defining shader parameters.

For example the user might request a "VP" semantic, which could be recognized by the renderer that the shader requests a view-projection matrix. Such a matrix is not something that the user should have to assign to the material himself. The renderer can choose to parse material parameters looking for supported semantics, and assign their values. 

Ultimately whether the renderer chooses to parse the semantics or not is up to the renderer. Currently the default *RenderBeast* renderer does not make use of any semantics and instead maps parameters directly by using their name.

The semantics for each parameter can be accessed through the **Shader** object, which renderer needs to iterate through manually.

~~~~~~~~~~~~~{.cpp}
StringID RPS_ViewProjTfrm = "VP"; // Define semantic identifier

TShared<Material> material = ...;
TShared<Shader> shader = material->GetShader();
auto& dataParams = shader->GetDataParams();
for (auto& entry : texParams)
{
	if (entry.second.rendererSemantic == RPS_ViewProjTfrm)
	{
		// Found it, assign some value to the parameter
		mMaterial->SetMat4(entry.second.name, Matrix4::kIdentity);
		break;
	}
}
~~~~~~~~~~~~~

## GpuResourcePool
Although you can create textures and buffers manually as described in the low level rendering API manual, @b3d::render::GpuResourcePool provides a simpler and more efficient way of doing it. It will keep alive any referenced textures and buffers, so that other systems may re-use them if their size/formats match. This can improve performance when using many temporary/intermediary render textures (like in post-processing) or load-store buffers.

To request a render texture, first populate the @b3d::render::PooledRenderTextureCreateInformation descriptor, by calling any of @b3d::render::PooledRenderTextureCreateInformation::Create2D, @b3d::render::PooledRenderTextureCreateInformation::Create3D or @b3d::render::PooledRenderTextureCreateInformation::CreateCube.

To request a buffer, populate the @b3d::render::POOLED_STORAGE_BUFFER_DESC descriptor by calling either @b3d::render::POOLED_STORAGE_BUFFER_DESC::createStandard or @b3d::render::POOLED_STORAGE_BUFFER_DESC::createStructured.

Then call @b3d::render::GpuResourcePool::Get with the provided descriptor. This will either create a new render texture/buffer, or return one from the pool. The returned object is @b3d::render::PooledRenderTexture for textures and @b3d::render::PooledStorageBuffer for buffers.

Once you are done using the texture or buffer, it will be automatically returned to the pool when your reference goes out of scope. If you plan on using this object again, make sure to keep a reference to the **render::PooledRenderTexture** / **render::PooledStorageBuffer** object. This will prevent the pool from fully destroying the object so it may be reused.

~~~~~~~~~~~~~{.cpp}
// An example creating a pooled render texture
PooledRenderTextureCreateInformation desc = PooledRenderTextureCreateInformation::Create2D(PF_R8G8B8A8, 1024, 1024, TextureUsageFlag::RenderTarget);
TShared<PooledRenderTexture> pooledRT = GpuResourcePool::Instance().Get(desc);

GpuCommandBuffer& commandBuffer = ...;
commandBuffer.BeginRenderPass(RenderPassCreateInformation(pooledRT->RenderTexture));
... render to target ...
commandBuffer.EndRenderPass();
// Keep a reference to pooledRT if we plan on re-using it, then next time just call Get() using the same descriptor
~~~~~~~~~~~~~

## UniformBufferPools
@b3d::render::UniformBufferPools is a high-level manager used by the renderer to efficiently handle per-object uniform buffer allocations. It suballocates from large **GpuBuffer** objects via @b3d::render::GpuBufferPool, and shares **GpuParameterSet** instances across objects that happen to land in the same underlying buffer. At draw time only the dynamic buffer offset changes between draws, while the parameter set binding stays the same — minimizing descriptor set switches.

The pool supports different allocation types through @b3d::render::UniformBufferPools::PoolType:
 - @b3d::render::UniformBufferPools::RenderablePool - Per-object transform data only.
 - @b3d::render::UniformBufferPools::DecalPool - Per-object transform data plus decal-specific parameters.
 - @b3d::render::UniformBufferPools::GpuParticlesPool - Per-object transform data plus GPU particle simulation parameters.

### Configuration
Before use, you register pool configurations via @b3d::render::UniformBufferPools::RegisterType, then call @b3d::render::UniformBufferPools::Initialize. Each @b3d::render::UniformBufferPools::PoolConfiguration specifies the pool type, number of entries per underlying buffer, the buffer configurations (size, name, flags), and a **GpuPipelineParameterSetLayout**.

~~~~~~~~~~~~~{.cpp}
UniformBufferPools::PoolConfiguration config;
config.Type = UniformBufferPools::RenderablePool;
config.EntriesPerBuffer = 1024;
config.Layout = perObjectParameterSetLayout;
config.Buffers.Add({UniformBufferPools::PerObjectBuffer, "PerObject", perObjectBufferSize, GpuBufferFlag::StoreOnCPUWithGPUAccess});

uniformBufferPools.RegisterType(config);
uniformBufferPools.Initialize(gpuDevice);
~~~~~~~~~~~~~

### Allocating and releasing
@b3d::render::UniformBufferPools::Allocate returns an @b3d::render::UniformBufferPools::AllocationResult containing a handle, a shared **GpuParameterSet**, and the suballocations for each buffer type. When an object is removed, call @b3d::render::UniformBufferPools::Release with the handle.

~~~~~~~~~~~~~{.cpp}
// Allocate for a renderable
auto result = uniformBufferPools.Allocate();
renderState->PerObjectBufferAllocationHandle = result.Handle;
renderState->PerObjectParameterSet = result.ParameterSet;
renderState->PerObjectSuballocation = result.GetSuballocation(UniformBufferPools::PerObjectBuffer);

// Later, when the object is removed:
uniformBufferPools.Release(renderState->PerObjectBufferAllocationHandle);
~~~~~~~~~~~~~

### Updating data
Update methods write uniform data through a transient staging buffer and issue a copy to the persistent GPU-resident suballocation:
 - @b3d::render::UniformBufferPools::UpdatePerObjectBuffer - Writes world transform, inverse, no-scale transform, determinant sign, and layer.
 - @b3d::render::UniformBufferPools::UpdateDecalParamBuffer - Writes world-to-decal matrix, normal, tolerance, and other decal properties.
 - @b3d::render::UniformBufferPools::UpdateGpuParticlesParamBuffer - Writes color curve and size/frame-index curve offsets and scales.

At the end of each frame, call @b3d::render::UniformBufferPools::AdvanceFrame to recycle transient staging allocations.

### Draw-time binding
At draw time the shared parameter set is bound once, and only the dynamic offset changes per draw call to select the correct suballocation within the large buffer:

~~~~~~~~~~~~~{.cpp}
drawCommand.SharedPerObjectParameterSet = renderState->PerObjectParameterSet;
drawCommand.PerObjectBufferOffset = renderState->PerObjectSuballocation.GetSuballocationOffset();
~~~~~~~~~~~~~

## Renderer options
You can customize your rendering at runtime by implementing the @b3d::render::RendererOptions class. Your **render::RendererOptions** implementation can then be assigned to the renderer by calling @b3d::render::Renderer::SetOptions, and accessed within the renderer via the **Renderer::mOptions** field. No default options are provided and it's up to your renderer to decide what it requires.

Be aware that options are set from the simulation thread, and if you want to use them on the core thread you need to either properly synchronize the access, or send a copy of the options to the core thread.
