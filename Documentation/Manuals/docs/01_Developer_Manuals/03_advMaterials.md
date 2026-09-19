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
TShared<Texture> someTexture = ...;

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

TShared<Pass> pass = Pass::create(desc);
~~~~~~~~~~~~~

The descriptors for GPU programs and non-programmable states are filled out as described in the low-level rendering API manual.

## Creating a variation
Now that we know how to create a pass, we can use one or multiple passes to initialize a **Variation**. A variation is just a container for one or multiple passes.

To create a variation call @b3d::Variation::Create and provide it with:
 - Shading language name - This should be "HLSL" or "GLSL". The engine will not use this variation unless this language is supported by the current render API. 
 - Array containing one or multiple passes
 
For example:
~~~~~~~~~~~~~{.cpp}
TShared<Pass> pass = ...;

// Create a variation that uses HLSL and has a single pass
TShared<Variation> variation = Variation::Create("HLSL", { pass });
~~~~~~~~~~~~~
  
## Creating a shader

@b3d::ShaderCreateInformation contains the shader's variations and a shared @b3d::ShaderDescription. The description holds material parameters, variation declarations, and render settings:

- @b3d::ShaderDescription::QueueSortType controls object sorting.
- @b3d::ShaderDescription::QueuePriority places higher-priority objects before lower-priority objects.
- @b3d::ShaderDescription::SeparablePasses allows other objects to render between the shader's passes.

~~~~~~~~~~~~~{.cpp}
ShaderCreateInformation createInformation;
createInformation.Variations.push_back(variation);
createInformation.Description->QueueSortType = QueueSortType::None;

HShader shader = Shader::Create("MyShader", createInformation);
~~~~~~~~~~~~~

Finish configuring the description before creating the shader; do not modify it afterward.

## Shader parameters

Shader parameters expose GPU program variables through **Material**. Setting a material parameter updates its matching variables across the passes in the selected variation.

Register parameters through **createInformation.Description->Parameters**, using @b3d::ShaderParameterDescription::AddParameter. Parameters come in two forms:

- @b3d::ShaderDataParameterInformation for scalar, vector, matrix, array and structure values. See @b3d::GpuDataParameterType for supported types.
- @b3d::ShaderObjectParameterInformation for textures, buffers and samplers. See @b3d::GpuParameterObjectType for supported types.

For each parameter, provide a unique material-facing name, the name of the variable in the GPU program, and its type. For arrays, also set **ArraySize** to match the declaration in the GPU program.

~~~~~~~~~~~~~{.cpp}
ShaderCreateInformation createInformation;
createInformation.Variations.push_back(variation);

ShaderParameterDescription& parameters = *createInformation.Description->Parameters;
parameters.AddParameter(ShaderDataParameterInformation("Roughness", "roughness", GPDT_FLOAT1));
parameters.AddParameter(ShaderObjectParameterInformation("Albedo", "albedoTexture", GPOT_TEXTURE2D));

HShader shader = Shader::Create("MyShader", createInformation);
~~~~~~~~~~~~~

Variations used with **Material** must expose the same parameter names, types and array sizes. Parameters marked `[internal]` in BSL may differ between variations. **RendererMaterial** and direct low-level rendering do not require a common material parameter interface.

### Renderer semantics

A renderer semantic is an optional tag identifying a parameter's purpose to the renderer. The renderer can use it to supply a value automatically, such as an object's world transform. Pass the semantic as the fourth argument to the parameter information constructor:

~~~~~~~~~~~~~{.cpp}
// Assumes the active renderer recognizes "W" as the world-transform semantic.
parameters.AddParameter(ShaderDataParameterInformation("WorldTransform", "worldTransform", GPDT_MATRIX_4X4, StringID("W")));
~~~~~~~~~~~~~

Supported semantics and the required parameter types depend on the active renderer; **"W"** is only an example, not a built-in guarantee. Leave renderer-managed parameters to the renderer instead of assigning them through **Material**. Omit the semantic, or use **StringID::kNone**, for parameters you set yourself.

### Default values

The optional second argument to **AddParameter()** supplies the initial value used when a **Material** is created with the shader. You can override it afterward using the material's parameter setters.

- Data parameters accept a **TArrayView<const u8>** containing the complete value, including all array elements. The bytes are copied by **AddParameter()**, so the source value need not remain alive afterward.
- Texture parameters accept **ShaderDefaultTextureType::White**, **Black**, **Normal** or **None**. To use another texture, assign it to the material with **SetTexture()**.
- Sampler parameters accept a **SamplerStateCreateInformation** describing the default sampler settings.

To supply defaults in the earlier example, replace its parameter registration calls with:

~~~~~~~~~~~~~{.cpp}
// Register defaults before creating the shader.
const float roughness = 0.5f;
parameters.AddParameter(ShaderDataParameterInformation("Roughness", "roughness", GPDT_FLOAT1), TArrayView<const u8>((const u8*)&roughness, sizeof(roughness)));
parameters.AddParameter(ShaderObjectParameterInformation("Albedo", "albedoTexture", GPOT_TEXTURE2D), ShaderDefaultTextureType::White);

SamplerStateCreateInformation samplerInformation;
samplerInformation.MinFilter = FO_POINT;
samplerInformation.MagFilter = FO_POINT;
parameters.AddParameter(ShaderObjectParameterInformation("AlbedoSampler", "albedoSampler", GPOT_SAMPLER2D), samplerInformation);
~~~~~~~~~~~~~

# Manually rendering using the material
In an earlier manual we have shown how to render using a **Material** by attaching it to a **Renderable** component and letting the renderer do the rest. You can however render using the material completely manually, using the low-level rendering API.

Material is a **CoreObject** which means it also has a render-thread interface accessible through @b3d::Material::GetRenderProxy(). The interface is the same as the non-core interface we have described so far.

## Binding material
**Material** cannot be bound directly to the low level rendering API. You must instead manually retrieve a pipeline state for one of its passes.

You can retrieve a specific pass from a material by calling @b3d::Material::GetPass(). The method expects an index of a variation and an index of a pass. To get the number of supported variations call @b3d::Material::GetVariationCount(), and to get the number of passes for a specific variation call @b3d::Material::GetPassCount() with a specific variation index.

Once you have a **Pass** you can retrieve from it either a **GraphicsPipelineState** or a **ComputePipelineState** by calling @b3d::Pass::GetGraphicsPipelineState() and @b3d::Pass::GetComputePipelineState(), respectively. Those can then be bound for rendering as shown in the low level rendering API manual.

~~~~~~~~~~~~~{.cpp}
TShared<Material> material = ...;

u32 passIndex = 0;
u32 variationIndex = 0;
TShared<Pass> pass = material->GetPass(passIndex, variationIndex);

GpuCommandBuffer& commandBuffer = ...;
commandBuffer.SetGpuGraphicsPipelineState(pass->GetGraphicsPipelineState());
~~~~~~~~~~~~~

Alternatively you can use the helper methods @b3d::render::RendererUtility::SetPass or @b3d::render::RendererUtility::SetComputePass.

## Binding material parameters
In order to bind material parameters we need to somehow get access to a **GpuParameterSet** object from the material. This is done through an intermediate class @b3d::MaterialParameterAdapter, created by a call to @b3d::Material::CreateParameterAdapter(), which as a parameter takes a variation index.

Compile the selected variation and wait for completion before creating its adapter. Adapter creation never triggers compilation and returns null if the variation is uncompiled or its material interface is incompatible.

**GpuParameterSet** for a specific pass can then be retrieved by calling @b3d::MaterialParameterAdapter::GetGpuParameterSet() with the pass index and set index. They can then be bound as described in the low level render API manual.

~~~~~~~~~~~~~{.cpp}
TShared<Material> material = ...;

u32 passIndex = 0;
u32 variationIndex = 0;

const TAsyncOp<bool> compilation = material->GetVariation(variationIndex)->Compile();
compilation.BlockUntilComplete();

TShared<MaterialParameterAdapter> parameterAdapter = material->CreateParameterAdapter(variationIndex);
if(parameterAdapter == nullptr)
	return;

GpuCommandBuffer& commandBuffer = ...;
commandBuffer.SetGpuParameterSet(parameterAdapter->GetGpuParameterSet(passIndex, 0));
~~~~~~~~~~~~~

Note that creation of a **MaterialParameterAdapter** object is expensive, and the intent is that it will be created once (or just a few times) per material. **MaterialParameterAdapter** contains a completely separate storage from the **Material** it was created from, therefore whenever material parameters are updated you must transfer its contents into **GpuParameterSet** by calling @b3d::MaterialParameterAdapter::Update().

~~~~~~~~~~~~~{.cpp}
TShared<MaterialParameterAdapter> parameterAdapter = material->CreateParameterAdapter(variationIndex);

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
