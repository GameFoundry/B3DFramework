GPU programs									{#gpuPrograms}
===============
[TOC]

GPU programs are programmable parts of the GPU pipeline, in other literature often called shaders (Banshee uses the word shader for a higher level concept, see the [material](@ref materials) manual).

GPU programs in Banshee are represented with the @ref BansheeEngine::GpuProgram "GpuProgram" and @ref BansheeEngine::GpuProgramCore "GpuProgramCore" classes. Both of these provide almost equivalent functionality, but the former is for use on the simulation thread, and the latter is for use on the core thread. If you are confused by the dual nature of the objects, read the [core thread](@ref coreThread) manual. 

In this manual we will focus on the simulation thread implementation, and then note the differences in the core thread version at the end.

There are six types of GPU programs: vertex, hull (tesselation control), geometry, domain (tesselation evaluation), fragment (pixel) and compute programs. Each is used for a different purpose but has the same interface. We assume the user is familiar with the GPU pipeline and what the different programs types do. 

Although GPU programs can be used on their own (which we will cover in this manual), they are more commonly used as parts of @ref BansheeEngine::Shader "Shaders". Read the [material](@ref materials) manual for more information.

# Creating GPU programs {#gpuPrograms_a}
To create a GPU program call @ref BansheeEngine::GpuProgram::create() "GpuProgram::create". This method expects the following parameters:
 - Source code of the GPU program. This should be in a language supported by the current render API (e.g. HLSL for DirectX, GLSL for OpenGL).
 - Name of the entry point into the GPU program. This is the name of the function that will be called when the program is ran. Must be "main" for OpenGL.
 - Language the source code is written in. This can be "hlsl" or "glsl" by default, but more languages can be added by extensions.
 - @ref BansheeEngine::GpuProgramType "Type" of the GPU program (vertex, fragment, etc.).
 - @ref BansheeEngine::GpuProgramProfile "Profile" of the GPU program. Determines what functionality does the underyling hardware support. Nowadays it is safe to always set this to profile 5.0 unless creating programs for very old hardware.
 
For example if we wanted to create a HLSL fragment program (HLSL source not shown):
~~~~~~~~~~~~~{.cpp}
String hlslSource = "...";

SPtr<GpuProgram> myProgram = GpuProgram::create(hlslSource, "main", "hlsl", GPT_FRAGMENT_PROGRAM, GPP_FS_5_0);
~~~~~~~~~~~~~ 
 
Once the GPU program has been created it is not guaranteed to be usable. The compilation of the provided source code could have failed, which you can check by calling @ref BansheeEngine::GpuProgram::isCompiled() "GpuProgram::isCompiled", and retrieve the error message by calling @ref BansheeEngine::GpuProgram::getCompileErrorMessage() "GpuProgram::getCompileErrorMessage". Be aware that both of these methods are only valid after the core thread has initialized the object. You can ensure this by calling @ref BansheeEngine::CoreObject::blockUntilCoreInitialized "GpuProgram::blockUntilCoreInitialized" but be aware this will block the calling thread which can result in a significant performance impact.

# GPU program parameters {#gpuPrograms_b}
Once the GPU program has been compiled you can access information about its parameters (constant variables, or uniforms in DirectX/OpenGL lingo). Use @ref BansheeEngine::GpuProgram::getParamDesc "GpuProgram::getParamDesc" to retrieve a structure containing all GPU parameters, including primitive parameters, textures, samplers, buffers and parameter buffers (constant/uniform buffers in DirectX/OpenGL lingo). 

You generally don't need to use this information directly, but you provide it when creating a @ref BansheeEngine::GpuParams "GpuParams" object.

## GpuParams {#gpuPrograms_b_a}
@ref BansheeEngine::GpuParams "GpuParams" is a container for all parameters required by a set of GPU programs (one per stage). It allows you to set primitive/texture/sampler/buffer parameters used by the GPU programs, which it stores in an internal buffer. You can then bind it to the pipeline and it will be used by the bound GPU programs.

For example to assign a texture and a 2D vector as input to the program we created earlier:
~~~~~~~~~~~~~{.cpp}
GPU_PARAMS_DESC desc;
desc.fragmentParams = myProgram->getParamDesc();
... usually you want to set other stages as well ...

SPtr<GpuParams> params = GpuParams::create(desc);

// Retrieve GPU param handles we can then read/write to
GpuParamVec2 myVectorParam;
GpuParamTexture myTextureParam;

params->getParam(GPT_FRAGMENT_PROGRAM, "myVector", myVectorParam); // Assuming "myVector" is the variable name in the program source code
params->getTextureParam(GPT_FRAGMENT_PROGRAM, "myTexture", myTextureParam); // Assuming "myTexture" is the variable name in the program source code

myVectorParam.set(Vector2(1, 2));
myTextureParam.set(myTexture); // Assuming we created "myTexture" earlier.
~~~~~~~~~~~~~

As you can see we must first retrieve a handle to the parameter, and then we can use that handle for reading/writing to the parameter. You can store those handles for easier access to the parameters, as looking them up by using the parameter name can be relatively slow.

See the [render API manual](@ref renderAPI) for more information about how to set and bind GPU program parameters.

# Using GPU programs for rendering {#gpuPrograms_c}
You can bind a GPU program to the pipeline by assigning it to a @ref BansheeEngine::GpuPipelineState "GpuPipelineState" and calling @ref BansheeEngine::RenderAPI::setGraphicsPipeline "RenderAPI::setGraphicsPipeline". Or alternatively if using compute programs you can bind by calling @ref BansheeEngine::RenderAPI::setComputePipeline "RenderAPI::setComputePipeline". Any subsequent draw/dispatch calls will then use the bound GPU programs. 

You can bind parameters for use in the GPU program by calling @ref BansheeEngine::RenderAPICore::setGpuParams "RenderAPICore::setGpuParams" which accepts the @ref BansheeEngine::GpuParamsCore "GpuParamsCore" object we described above.

Much more detailed information about rendering is provided in the [render API manual](@ref renderAPI).

# Core thread GPU programs {#gpuPrograms_e}
So far we have only talked about the simulation thread @ref BansheeEngine::GpuProgram "GpuProgram" but have ignored the core thread @ref BansheeEngine::GpuProgramCore "GpuProgramCore". The functionality between the two is mostly the same, with the major difference being that operations performed on the core thread version are immediate. So calls to @ref BansheeEngine::GpuProgramCore::isCompiled() "GpuProgramCore::isCompiled" and @ref BansheeEngine::GpuProgramCore::getCompileErrorMessage() "GpuProgramCore::getCompileErrorMessage" don't require any waiting.

Addtionally the core thread object can provide information about vertex input declaration. Vertex input declaration is only available for vertex shaders, and it defines in what format does the GPU program expect vertices to be in. This can be useful for setting up your meshes in the valid format. Use @ref BansheeEngine::GpuProgramCore::getInputDeclaration "GpuProgramCore::getInputDeclaration" to retrieve the @ref BansheeEngine::VertexDeclarationCore "VertexDeclaration". Check out the [mesh](@ref meshes) manual for more information on how to use vertex declarations.

# OpenGL specifics {#gpuPrograms_f}
When creating vertex inputs for a GPU program written in GLSL a set of built-in variables are provided:
 - bs_position - Vertex position
 - bs_normal - Vertex normal
 - bs_tangent - Vertex tangent
 - bs_bitangent - Vertex bitangent
 - bs_texcoord - Vertex UV
 - bs_color - Vertex color
 - bs_blendweights - Blend weights used for skinning
 - bs_blendindices - Blend indices used for skinning
 
You can append 0-8 to the names to receive more than one element of the same name. For HLSL the standard HLSL semantics are used instead. Actual types of these elements, as well as the data stored by them doesn't need to match the names and it's up to the user to provide whatever data he needs.