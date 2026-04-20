---
title: Advanced materials
---

We have already talked about how to create materials, assign them parameters and bind them to a **Renderable** component. In this section we'll show an advanced way to assign material parameters, how to use materials for rendering directly (without using **Renderable** component) and how to create your own shaders without the use of BSL. 

# Material parameters
Previously we have shown to how to set **Material** parameters by calling methods like @b3d::Material::SetTexture, @b3d::Material::SetFloat, @b3d::Material::SetColor, @b3d::Material::SetVec4 and similar.

As an alternative you can also set materials through material parameter handles. Once a material handle is retrieved it allows you to set material parameters much more efficiently than by calling the methods above directly. 

To retrieve the handles call any of the following methods, depending on material parameter type:
 - @b3d::render::Material::GetParamTexture - Outputs a @b3d::TMaterialParamTexture<Core> handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamLoadStoreTexture - Outputs a @b3d::TMaterialParamLoadStoreTexture<Core> handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamBuffer - Outputs a @b3d::TMaterialParamBuffer<Core> handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamSamplerState - Outputs a @b3d::TMaterialParamSampState<Core> handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamFloat - Outputs a @MaterialParamFloat handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamVec2 - Outputs a @MaterialParamVec2 handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamVec3 - Outputs a @MaterialParamVec3 handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamVec4 - Outputs a @MaterialParamVec4 handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamColor - Outputs a @MaterialParamColor handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamMat3 - Outputs a @MaterialParamMat3 handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamMat4 - Outputs a @MaterialParamMat4 handle that can be used for reading & writing the parameter value.
 - @b3d::render::Material::GetParamFloatCurve - Outputs a @MaterialParamFloatCurve handle that can be used for assigning an animation curve to a *float* parameter. This can be used as an alternative to **render::Material::GetParamFloat()** in that the value will now be animated over the range of the curve, instead of being just a static value.
 - @b3d::render::Material::GetParamColorGradient - Outputs a @b3d::TMaterialColorGradientParam<Core> handle that can be used for assigning a color gradient to a *Color* parameter. This can be used as an alternative to **render::Material::GetParamColor()** in that the value will now be animated over the range of the gradient, instead of being just a static value.
 - @b3d::render::Material::GetParamSpriteTexture - Outputs a @b3d::TMaterialParamSpriteTexture<Core> handle that can be used for assigning a sprite texture to a *texture* parameter. This can be used as an alternative to **render::Material::GetParamTexture()** in that the sprite texture can be animated while a normal texture is always static.
 
Handles provide **Set()** and **Get()** methods that can be used for writing and reading the parameter values. 
 
~~~~~~~~~~~~~{.cpp}
HMaterial material = ...;

MaterialParamMat4 myMatParam;
MaterialParamTexture myTextureParam;

myMatParam = material->GetParamMat4("vertProjMatrix");
myTextureParam = material->GetParamTexture("mainTexture");

Matrix4 viewProjMat = ...;
SPtr<Texture> someTexture = ...;

myMatParam.Set(viewProjMat);
myTextureParam.Set(someTexture);
~~~~~~~~~~~~~ 
 
Material handles are very similar as **GpuParameterSet** handles we talked about earlier. There are two major differences:
 - **GpuParameterSet** handles will only set the parameter value for a specific **GpuProgram**, while material handles will set the values for all **GpuProgram**%s that map to that handle.
 - **Material** parameters need to be explicitly defined in the **Shader** (shown below). **Material** parameters always map to one or multiple **GpuProgram** parameters.

# Creating a shader manually
So far when we wanted to create a shader we would create a BSL file which would then be imported, creating a @b3d::Shader. But you can also create shaders manually by explicitly providing HLSL/GLSL code for **GpuProgram**%s and non-programmable states. Most of the things outlined in this section are performed by BSL compiler internally when a **Shader** is imported.

Each shader definition contains two things:
 - A list of parameters, with a mapping of each parameter to one or multiple variables in a GPU program
 - One or multiple @b3d::Variation%s. Each variation is essentially a fully fledged shader of its own. Variations are chosen by the renderer depending on the context. For example some variations only support the DirectX backend, while others only the Vulkan backend. Different variations will also exist for different shader variations (e.g. a high- and low-end version of the same shader).
  - Each variation contains one or multiple @b3d::Pass%es. A pass is a set of GPU programs and non-programmable states. When rendering using a certain variation each pass will be executed one after another. This allows you to render objects that require more complex rendering that requires multiple separate steps - althrough in practice most variations have only a single pass.

To summarize, the relationship between materials, shaders, variations and passes is:
 - **Material** [contains one]-> **Shader** [contains one or multiple]-> **Variation** [contains one or multiple]-> **Pass**
 
Optionally, the shader definition can also contain a set of @b3d::SubShader objects. As explained previously sub-shaders are a set of variations that are used to override specific renderer behaviour. 
 
## Creating a pass
A **Pass** can be created by filling out a @b3d::PASS_DESC descriptor and passing it to @b3d::Pass::create method. **PASS_DESC** is fairly simple and it expects a set of GPU program and non-programmable state descriptors.

~~~~~~~~~~~~~{.cpp}
GPU_PROGRAM_DESC vertexProgDesc = ...;
GPU_PROGRAM_DESC fragmentProgDesc = ...;

BLEND_STATE_DESC blendStateDesc = ...;

// Create a pass with a vertex and fragment program, and a non-default blend state
PASS_DESC desc;
desc.vertexProgram = vertexProgDesc;
desc.fragmentProg = fragmentProgDesc;
desc.blendState = blendStateDesc;

SPtr<Pass> pass = Pass::create(desc);
~~~~~~~~~~~~~

The descriptors for GPU programs and non-programmable states are filled out as described in the low-level rendering API manual.

## Creating a variation
Now that we know how to create a pass, we can use one or multiple passes to initialize a **Variation**. A variation is just a container for one or multiple passes.

To create a variation call @b3d::Variation::Create and provide it with:
 - Shading language name - This should be "HLSL" or "GLSL". The engine will not use this variation unless this language is supported by the current render API. 
 - Array containing one or multiple passes
 
For example:
~~~~~~~~~~~~~{.cpp}
SPtr<Pass> pass = ...;

// Create a variation that uses HLSL and has a single pass
SPtr<Variation> variation = Variation::Create("HLSL", { pass });
~~~~~~~~~~~~~
  
## Creating a shader
Now that we have a variation we can create the shader by calling @b3d::Shader::Create, which expects the name of the shader and a @b3d::TSHADER_DESC<T> structure as input.
	
@b3d::TSHADER_DESC<T> contains a list of variations to initialize the shader with, as well as a set of optional parameters to control how the shader rendering works:
  - @b3d::TSHADER_DESC<T>::QueueSortType - Controls how should objects rendered with this shader be sorted. Either front to back, back to front, or without sorting. This property can be used for the active renderer to properly render objects, as certain effects (like transparency) require their objects to be rendered in a specific order.
  - @b3d::TSHADER_DESC<T>::QueuePriority - Controls at what point should objects rendered with this shader be rendered relative to other objects. Objects with higher priority will be rendered before ones with lower priority, and sorting due to **SHADER_DESC::QueueSortType** will never sort outside of priority groups. This can allow you to render a certain type of objects before all others (e.g. all opaque objects should be rendered before transparent ones).
  - @b3d::TSHADER_DESC<T>::SeparablePasses - An optimization hint to the renderer that can improve performance when turned on. Only relevant if the shader has variations with multiple passes. When true the renderer will not necessarily execute passes right after another, but might render other objects in-between passes. This can reduce state switching as multiple objects can be rendered with a single pass, but is only relevant for algorithms that can handle such a process (most can't).

~~~~~~~~~~~~~{.cpp}
SPtr<Variation> variation = ...;
	
SHADER_DESC desc;
desc.queueSortType = QueueSortType::None;
desc.queuePriority = 0;
desc.separablePasses = false;
desc.variations = { variation };

SPtr<Shader> shader = Shader::Create("MyShader", desc);
~~~~~~~~~~~~~ 
  
## Shader parameters
Shader parameters allow you to change values of parameters in GPU programs through the **Material** interface. They are similar to GPU program parameters described earlier, but they set the values on all GPU programs on all passes in the active variation, instead of doing so only on a single GPU program. Additionally they also support renderer semantics (see below).

To create the parameter interface you must populate the **SHADER_DESC** structure by calling one of the @b3d::TSHADER_DESC<T>::AddParameter overloads.

Parameters come in two variants:
 - Data - These are primitive types like float, int or bool. This includes their vector and array variants. Check @b3d::GpuDataParameterType for a list of all data parameter types. In **SHADER_DESC** they are represented with the @b3d::SHADER_DATA_PARAM_DESC type.
 - Object - These are object types like texture, buffer or sampler state. Check @b3d::GpuParameterObjectType for a list of all object parameter types. In **SHADER_DESC** they are represented with the @b3d::SHADER_OBJECT_PARAM_DESC type.

For each parameter you must specify:
 - Its name. This will be the name accessible through **Material**. It can be anything you like, as long as it is unique.
 - Name of the GPU variable it maps to. This must be a variable defined in source code of one or multiple GPU programs used in the shader (across all variations/passes).
 - Type of the GPU variable, as described above.

~~~~~~~~~~~~~{.cpp}
// Extended example from above
SPtr<Variation> variation = ...;
	
SHADER_DESC desc;
desc.queueSortType = QueueSortType::None;
desc.queuePriority = 0;
desc.separablePasses = false;
desc.variations = { variation };

// Add a 4x4 transform matrix data parameter
desc.AddParameter(SHADER_DATA_PARAM_DESC("WorldTfrm", "WorldTfrm", GPDT_MATRIX_4X4));

// Add a texture parameter
desc.AddParameter(SHADER_OBJECT_PARAM_DESC("AlbedoTex", "AlbedoTex", GPOT_TEXTURE2D));

SPtr<Shader> shader = Shader::Create("MyShader", desc);
~~~~~~~~~~~~~

### Advanced parameters
When adding parameters you can also specify two additional properties we didn't touch on in the previous section: renderer semantic and default value.

Renderer semantic allows you to give the parameter a unique tag that can be recognized by the active renderer. The renderer can then use these semantics to automatically assign values to them while rendering. For example the "WVP" semantic might notify the renderer to populate this parameter with the world-view-projection matrix. This way the user is not responsible for setting such parameters manually. The actual semantics supported depend on the active renderer. If provided and renderer doesn't support a semantic, it will be ignored. We'll talk more on how to access semantics in the renderer manual.

The parameter default value allows you to provide a value that will be used for initializing the parameter when a **Material** is initially constructed. For data parameters the default value is a provided as a raw block of memory, and for object parameters it can be a reference to a **Texture** or a **SamplerState**.

~~~~~~~~~~~~~{.cpp}
// An extended example from above with semantics and default values:
SPtr<Variation> variation = ...;
	
SHADER_DESC desc;
desc.queueSortType = QueueSortType::None;
desc.queuePriority = 0;
desc.separablePasses = false;
desc.variations = { variation };

// Add a 4x4 transform matrix data parameter with a "W" semantic and identity matrix as default
desc.AddParameter(SHADER_DATA_PARAM_DESC("WorldTfrm", "WorldTfrm", GPDT_MATRIX_4X4, "W"), &Matrix4::kIdentity);

// Add a texture parameter with an "Albedo" semantic and a white texture as default
desc.AddParameter(SHADER_OBJECT_PARAM_DESC("AlbedoTex", "AlbedoTex", GPOT_TEXTURE2D, "Albedo"), Texture::kWhite);

HShader shader = Shader::Create("MyShader", desc);
~~~~~~~~~~~~~  
  
# Manually rendering using the material
In an earlier manual we have shown how to render using a **Material** by attaching it to a **Renderable** component and letting the renderer do the rest. You can however render using the material completely manually, using the low-level rendering API.

Material is a **CoreObject** which means it also has a render-thread interface accessible through @b3d::Material::GetRenderProxy(). The interface is the same as the non-core interface we have described so far.

## Binding material
**Material** cannot be bound directly to the low level rendering API. You must instead manually retrieve a pipeline state for one of its passes.

You can retrieve a specific pass from a material by calling @b3d::Material::GetPass(). The method expects an index of a variation and an index of a pass. To get the number of supported variations call @b3d::Material::GetVariationCount(), and to get the number of passes for a specific variation call @b3d::Material::GetPassCount() with a specific variation index.

Once you have a **Pass** you can retrieve from it either a **GraphicsPipelineState** or a **ComputePipelineState** by calling @b3d::Pass::GetGraphicsPipelineState() and @b3d::Pass::GetComputePipelineState(), respectively. Those can then be bound for rendering as shown in the low level rendering API manual.

~~~~~~~~~~~~~{.cpp}
SPtr<Material> material = ...;

u32 passIndex = 0;
u32 variationIndex = 0;
SPtr<Pass> pass = material->GetPass(passIndex, variationIndex);

GpuCommandBuffer& commandBuffer = ...;
commandBuffer.SetGpuGraphicsPipelineState(pass->GetGraphicsPipelineState());
~~~~~~~~~~~~~

Alternatively you can use the helper methods @b3d::render::RendererUtility::SetPass or @b3d::render::RendererUtility::SetComputePass.

## Binding material parameters
In order to bind material parameters we need to somehow get access to a **GpuParameterSet** object from the material. This is done through an intermediate class @b3d::MaterialParameterAdapter, created by a call to @b3d::Material::CreateParameterAdapter(), which as a parameter takes a variation index.

**GpuParameterSet** for a specific pass can then be retrieved by calling @b3d::MaterialParameterAdapter::GetGpuParameterSet() with the pass index and set index. They can then be bound as described in the low level render API manual.

~~~~~~~~~~~~~{.cpp}
SPtr<Material> material = ...;

u32 passIndex = 0;
u32 variationIndex = 0;
SPtr<MaterialParameterAdapter> parameterAdapter = material->CreateParameterAdapter(variationIndex);

GpuCommandBuffer& commandBuffer = ...;
commandBuffer.SetGpuParameterSet(parameterAdapter->GetGpuParameterSet(passIndex, 0));
~~~~~~~~~~~~~

Note that creation of a **MaterialParameterAdapter** object is expensive, and the intent is that it will be created once (or just a few times) per material. **MaterialParameterAdapter** contains a completely separate storage from the **Material** it was created from, therefore whenever material parameters are updated you must transfer its contents into **GpuParameterSet** by calling @b3d::MaterialParameterAdapter::Update().

~~~~~~~~~~~~~{.cpp}
SPtr<MaterialParameterAdapter> parameterAdapter = material->CreateParameterAdapter(variationIndex);

// ...update some parameters on the material...

// Transfer the updated data
parameterAdapter->Update(material);
~~~~~~~~~~~~~

In case your material contains any animated properties like animation curves, color gradients or sprite textures, you can also provide a `time` parameter to **MaterialParameterAdapter::Update()**, which determines the point at which to sample animated properties.

~~~~~~~~~~~~~{.cpp}
// Sample animated properties at 0.5 seconds into the animation
parameterAdapter->Update(material, 0.5f);
~~~~~~~~~~~~~

Once both the material's pipeline state and parameters are bound, you can then proceed to render as normally, as described in the low-level rendering manual.
