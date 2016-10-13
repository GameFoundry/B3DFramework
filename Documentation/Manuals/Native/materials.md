Materials									{#materials}
===============
[TOC]

A material controls how is an object rendered. In Banshee it is represented with @ref BansheeEngine::Material "Material" and @ref BansheeEngine::MaterialCore "MaterialCore" classes. Both of these provide almost equivalent functionality, but the former is for use on the simulation thread, and the latter is for use on the core thread. If you are confused by the dual nature of the objects, read the [core thread](@ref coreThread) manual. 

We'll focus on the simulation thread @ref BansheeEngine::Material "Material" throughout this manual, but aside from the `Core` suffix there are no major differences between the two interfaces.

Internally material contains a set of GPU programs, render states and GPU program parameters. GPU programs and render states are immutable and are controlled by the @ref BansheeEngine::Shader "Shader" assigned to the material. The parameters can be modified by the interface provided by the material, and this parameter access actually constitutes the bulk of the material's interface. Essentially materials can be thought of as shader "instances".

Since shaders are such intrical parts of materials, we'll explain how to create them before continuing with the discussion about materials.

# Shaders {#materials_a}
A shader is a group of GPU programs and render states that fully control the rendering of an object. Shader also exposes a set of parameters the user can change in order to control the rendering further (e.g. by providing different textures or transforms for each object).

To learn more about GPU programs and render states read the [GPU program](@ref gpuPrograms) and [render API](@ref renderAPI) manuals.

A shader can actually contain multiple sets of GPU programs and render states. Such set is called a @ref BansheeEngine::Technique "Technique", but only one technique can ever be active for rendering. Different techniques can be used for supporting different render API's or renderers (e.g. one technique for DirectX 11, other for OpenGL). Each shader can contain one or multiple techniques and the most appropriate technique will be chosen according to the active render API and renderer.

Each technique contains one or multiple @ref BansheeEngine::Pass "passes". Each pass contains a set of GPU programs and render states. When rendering using a certain technique each pass will be executed one after another. This allows you to render objects that require more complex rendering that requires multiple separate steps.

To summarize, the relationship between materials, shaders, techniques and passes is:
 - Material [contains one]-> Shader [contains one or multiple]-> Technique [contains one or multiple]-> Pass

## Pass {#materials_a_a}
Before we can create a shader we must first know how to create a @ref BansheeEngine::Pass "Pass". Pass can be created by filling out a @ref BansheeEngine::PASS_DESC "PASS_DESC" descriptor and passing it to @ref BansheeEngine::Pass::create "Pass::create". @ref BansheeEngine::PASS_DESC "PASS_DESC" is fairly simple and it expects a set of GPU programs and render states. For example:
~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgram> vertexProg = ...;
SPtr<GpuProgram> fragmentProg = ...;

SPtr<BlendState> blendState = ...;

// Create a pass with a vertex and fragment program, and a non-default blend state
PASS_DESC desc;
desc.vertexProgram = vertexProg;
desc.fragmentProg = fragmentProg;
desc.blendState = blendState;

SPtr<Pass> pass = Pass::create(desc);
~~~~~~~~~~~~~

[GPU program](@ref gpuPrograms) and [render API](@ref renderAPI) manuals teach you how to create GPU programs and render states. 

## Technique {#materials_a_b}
Now that we know how to create a pass, we can use one or multiple passes to initialize a @ref BansheeEngine::Technique "Technique". A technique is just a container for passes for a specific render API and renderer.

To create a technique call @ref BansheeEngine::Technique::create "Technique::create" and provide it with:
 - One or multiple passes
 - Supported render API name: Use built-in `RenderAPIAny` to signal that the technique works on any API, or use the render API identifier to signal that it only works on a specific one. By default supported identifiers are: `"VulkanRenderAPI"`, `"D3D11RenderAPI"`, `"GLRenderAPI"`, but more can be added via plugins. In general those identifiers are returned from @ref BansheeEngine::RenderAPICore::getName "RenderAPICore::getName". Most users should be okay by providing `RenderAPIAny`.
 - Supported renderer name: Use built-in `RendererAny` to signal that the technique works on any renderer, or use the renderer identifier to signal that it only works on a specific one. By default the only supported identifier is `"RenderBeast"`, but more can be added via plugins. In general those identifiers are returned from @ref BansheeEngine::Renderer::getName "Renderer::getName". Most users should be okay by providing `RendererAny`.
 - An optional list of tags that allows renderer to pick which technique to use when rendering objects. Can be left empty in most cases.

For example:
~~~~~~~~~~~~~{.cpp}
SPtr<Pass> pass = ...;

// Create a technique that supports any render API or renderer, with a single pass
SPtr<Technique> technique = Technique::create(RenderAPIAny, RendererAny, { pass });
~~~~~~~~~~~~~
 
## Creating a shader {#materials_a_c}
Now that we have a technique we can create our shader by calling @ref BansheeEngine::Shader::create "Shader::create", which expects a list of techniques, name of the shader and @ref BansheeEngine::TSHADER_DESC<T> "SHADER_DESC" as input.
	
@ref BansheeEngine::TSHADER_DESC<T> "SHADER_DESC" allows you to specify a set of optional parameters to control how the shader rendering works:
  - `queueSortType` - Controls how should objects rendered with this shader be sorted. Either front to back, back to front, or without sorting. This property can be used for the active renderer to properly render objects, as certain effects (like transparency) require their objects to be rendered in a specific order.
  - `queuePriority` - Controls when should objects rendered with this shader be rendered relative to other objects. Objects with higher priority will be rendered before ones with lower priority, and sorting due to `queueSortType` will never sort outside of priority groups. This can allow you to render a certain type of objects before all others (e.g. all opaque objects should be rendered before transparent ones).
  - `separablePasses` - An optimization hint to the renderer that can improve performance when turned on. Only relevant if the shader has techniques with multiple passes. When true the renderer will not necessarily execute passes right after another, but might render other objects in-between passes. This can reduce state switching as multiple objects can be rendered with a single pass, but is only relevant for algorithms that can handle such a process (most can't).
  - `flags` - Allows you to provide custom flags to the renderer. Interpretation of this value depends on the renderer used and by default its value is ignored.
	
@ref BansheeEngine::TSHADER_DESC<T> "SHADER_DESC" also expects you to provide a set of parameters accepted by the GPU programs used by the passes. We'll explain parameters later, but for now assume we're creating a shader with no parameters:
~~~~~~~~~~~~~{.cpp}
SPtr<Technique> technique = ...;
	
SHADER_DESC desc;
desc.queueSortType = QueueSortType::None;
desc.queuePriority = 0;
desc.separablePasses = false;
desc.flags = 0;

SPtr<Shader> shader = Shader::create("MyShader", desc, { technique });
~~~~~~~~~~~~~	
	
## Shader parameters {#materials_a_d}
Shader parameters allow you to change values of parameters in GPU programs. They are similar to GPU program parameters described in the [GPU program](@ref gpuPrograms) manual, but they set the values on all GPU programs on all passes, instead of working only on a single GPU program. Additionally they also support renderer semantics (see below).

Shader only defines the parameter interface (their name, type and description), but actual parameters can only be changed from a @ref BansheeEngine::Material "Material", as we'll show later. To create the parameter interface you must populate the @ref BansheeEngine::TSHADER_DESC<T> "SHADER_DESC" structure by calling one of the @ref BansheeEngine::TSHADER_DESC<T>::addParameter "SHADER_DESC::addParameter" overloads.

Parameters come in two variants:
 - Data - These are primitive types like float, int or bool. This includes their vector and array variants. Check @ref BansheeEngine::GpuParamDataType "GpuParamDataType" for a list of all data parameter types.
 - Object - These are object types like texture, buffer or sampler state. Check @ref BansheeEngine::GpuParamObjectType "GpuParamObjectType" for a list of all object parameter types.

Each parameter must have a name and a GPU variable it maps to. The name can be anything you like (as long as its unique) and it will be used for accessing the parameter from the @ref BansheeEngine::Material "Material". The GPU variable must be a name of the GPU variable defined in GPU program code. This is the name contained in @ref BansheeEngine::GpuParamDesc "GpuParamDesc" which is returned from @ref BansheeEngine::GpuProgram::getParamDesc "GpuProgram::getParamDesc". 

You must also provide the type of the parameter. This way the system can know how to distinguish parameters in case multiple GPU programs have the same-named GPU variable but with different type. For data parameters you can also provide the size of the array (in case the GPU variable is an array), or the size of a single element (only needed for structs).

An extended example from above:
~~~~~~~~~~~~~{.cpp}
SPtr<Technique> technique = ...;
	
SHADER_DESC desc;
desc.queueSortType = QueueSortType::None;
desc.queuePriority = 0;
desc.separablePasses = false;
desc.flags = 0;

// Add a 4x4 transform matrix data parameter
desc.addParameter("WorldTfrm", "WorldTfrm", GPDT_MATRIX_4X4);

// Add a texture parameter
desc.addParameter("AlbedoTex", "AlbedoTex", GPOT_TEXTURE2D);

SPtr<Shader> shader = Shader::create("MyShader", desc, { technique });
~~~~~~~~~~~~~

### Advanced parameters {#materials_a_d_a}
@ref BansheeEngine::SHADER_DESC::addParameter "SHADER_DESC::addParameter" also supports two additional arguments we didn't touch on in the previous section: renderer semantic and default value.

Renderer semantic allows you to give the parameter a unique name that can be recognized by the active renderer. The renderer can then use these semantics to automatically assign values to them while rendering. For example the "WVP" semantic might notify the renderer to populate this parameter with the world-view-projection matrix. This way the user is not responsible for setting such parameters manually.

The list of supported semantics depends on the currently active renderer. The default renderer is still in the implementation phase and supports no semantics. Read more about semantics in the [renderer](@ref renderer) manual.

The parameter default value allows you to provide a value that will be used for initializing the parameter when a @ref BansheeEngine::Material "Material" is initially constructed. For data parameters the default value is a provided as a raw block of memory, and for object parameters it can be a reference to a @ref BansheeEngine::Texture "Texture" or a @ref BansheeEngine::SamplerState "SamplerState".

An extended example from above with semantics and default values:
~~~~~~~~~~~~~{.cpp}
SPtr<Technique> technique = ...;
	
SHADER_DESC desc;
desc.queueSortType = QueueSortType::None;
desc.queuePriority = 0;
desc.separablePasses = false;
desc.flags = 0;

// Add a 4x4 transform matrix data parameter with a "W" semantic and identity matrix as default
desc.addParameter("WorldTfrm", "WorldTfrm", GPDT_MATRIX_4X4, "W", 1, 0, &Matrix4::Identity);

// Add a texture parameter with an "Albedo" semantic and a white texture as default
desc.addParameter("AlbedoTex", "AlbedoTex", GPOT_TEXTURE2D, Texture::White, "Albedo");

HShader shader = Shader::create("MyShader", desc, { technique });
~~~~~~~~~~~~~

## Banshee Shading Language {#materials_a_d_b}
Instead of creating shaders manually like we have described so far, Banshee provides a BSLFX language (Banshee Shading Language FX) that allows you to describe everything we talked about so far (GPU programs, states, parameter interface) in a single file which can then be imported into the engine like so:
~~~~~~~~~~~~~{.cpp}
HShader shader = gImporter().import<Shader>("myShader.bsl");
~~~~~~~~~~~~~

This is the preferred way of creating shaders, as its much simpler, concise and flexible. Learn about BSLFX syntax in the [BSLFX](@ref bslfx) manual.

## Saving/loading {#materials_a_d_c}
A shader is a @ref BansheeEngine::Resource "Resource" and can be saved/loaded like any other. See the [resource](@ref resources) manual.

# Creating a material {#materials_b}
Now that we finally have a shader we can create our material by calling @ref BansheeEngine::Material::create "Material::create" like so:
~~~~~~~~~~~~~{.cpp}
HShader shader = ...;
HMaterial material = Material::create(shader);
~~~~~~~~~~~~~

This will select the most appropriate technique from the shader and initialize the material with it. Once initialized you can read and write parameters from/to the material, and use it for rendering.

# Material parameters {#materials_c}
Material allows you to read and write parameters in two different ways:
 - By calling various set/get methods like @ref BansheeEngine::Material::getVec4 "Material::getVec4" or @ref BansheeEngine::Material::setVec4 "Material::setVec4" which read and write the parameter value directly.
 - By retrieving parameter handle by calling @ref BansheeEngine::Material::getParamVec4 "Material::getParamVec4" which returns a handle object @ref BansheeEngine::TMaterialDataParam<T, false> "MaterialParamVec4". This object can then be used by calling its @ref BansheeEngine::TMaterialDataParam<T, false>::get "MaterialParamVec4::get" and @ref BansheeEngine::TMaterialDataParam<T, false>::set "MaterialParamVec4::set" methods similar to above.
 
We have shown an example for a 4D vector but similarly named methods exist for other parameter types. The second method of accessing the parameters is prefered as it allows you to save the handle to the parameter, which avoids a potentially expensive name lookup every time we need to access it.

An example of writing a material value, both directly and with a handle:
~~~~~~~~~~~~~{.cpp}
HMaterial material = ...;

// Set parameter directly
material->setMat4("WorldTfrm", Matrix4::Identity);

// Get handle and set value indirectly
MaterialParamMat4 worldTfrmParam = material->getParamMat4("WorldTfrm");
worldTfrmParam.set(Matrix4::Identity);

// Set object parameter indirectly
MaterialParamTexture albedoParam = material->getParamTexture("AlbedoTex");
albedoParam.set(Texture::White);
~~~~~~~~~~~~~
# Rendering with material {#materials_d}
From the simulation thread you cannot use material to render manually (you must instead use GPU programs and states manually as described by the [render API](@ref renderAPI) manual). You are instead expected to set the material on a @ref BansheeEngine::Renderable "Renderable" object which will then be used for rendering automatically. Read the [renderer](@ref renderer) manual for more information.

Core thread gives you more flexibility and you can use @ref BansheeEngine::RendererUtility::setPass "RendererUtility::setPass" to bind a specific pass from a material to the pipeline, and @ref BansheeEngine::RendererUtility::setPassParams "RendererUtility::setPassParams" to bind material parameters for a specific pass. 

In order to retrieve a set of per-program @ref BansheeEngine::GpuParams "GpuParams" that can be used for binding directly to the pipeline, call @ref BansheeEngine::Material::createParamsSet "Material::createParamsSet", followed by @ref BansheeEngine::Material::updateParamsSet "Material::updateParamsSet". You are required to call @ref BansheeEngine::Material::updateParamsSet "Material::updateParamsSet" whenever material parameters change, in order to transfer the new data to @ref BansheeEngine::GpuParams "GpuParams".

After pass and pass parameters are bound you can follow them with draw calls as described in the [render API](@ref renderAPI) manual to render objects manually. 

# Saving/loading {#materials_e}
A material is a @ref BansheeEngine::Resource "Resource" and can be saved/loaded like any other. See the [resource](@ref resources) manual.