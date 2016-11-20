GPU programs									{#gpuPrograms}
===============
[TOC]

GPU programs are programmable parts of the GPU pipeline, in other literature often called shaders (Banshee uses the word shader for a higher level concept, see the [material](@ref materials) manual).

GPU programs in Banshee are represented with the @ref bs::GpuProgram "GpuProgram" and @ref bs::GpuProgramCore "GpuProgramCore" classes. Both of these provide almost equivalent functionality, but the former is for use on the simulation thread, and the latter is for use on the core thread. If you are confused by the dual nature of the objects, read the [core thread](@ref coreThread) manual. 

In this manual we will focus on the simulation thread implementation, and then note the differences in the core thread version at the end.

There are six types of GPU programs: vertex, hull (tesselation control), geometry, domain (tesselation evaluation), fragment (pixel) and compute programs. Each is used for a different purpose but has the same interface. We assume the user is familiar with the GPU pipeline and what the different programs types do. 

Although GPU programs can be used on their own (which we will cover in this manual), they are more commonly used as parts of @ref bs::Shader "Shaders". Read the [material](@ref materials) manual for more information.

# Creating GPU programs {#gpuPrograms_a}
To create a GPU program call @ref bs::GpuProgram::create() "GpuProgram::create". This method expects the following parameters:
 - Source code of the GPU program. This should be in a language supported by the current render API (e.g. HLSL for DirectX, GLSL for OpenGL).
 - Name of the entry point into the GPU program. This is the name of the function that will be called when the program is ran. Must be "main" for OpenGL.
 - Language the source code is written in. This can be "hlsl" or "glsl" by default, but more languages can be added by extensions.
 - @ref bs::GpuProgramType "Type" of the GPU program (vertex, fragment, etc.).
 - @ref bs::GpuProgramProfile "Profile" of the GPU program. Determines what functionality does the underyling hardware support. Nowadays it is safe to always set this to profile 5.0 unless creating programs for very old hardware.
 
For example if we wanted to create a HLSL fragment program (HLSL source not shown):
~~~~~~~~~~~~~{.cpp}
String hlslSource = "...";

GPU_PROGRAM_DESC desc;
desc.type = GPT_FRAGMENT_PROGRAM;
desc.source = hlslSource;
desc.entryPoint = "main";
desc.language = "hlsl";
desc.profile = GPP_FS_5_0;

SPtr<GpuProgram> myProgram = GpuProgram::create(desc);
~~~~~~~~~~~~~ 
 
Once the GPU program has been created it is not guaranteed to be usable. The compilation of the provided source code could have failed, which you can check by calling @ref bs::GpuProgram::isCompiled() "GpuProgram::isCompiled", and retrieve the error message by calling @ref bs::GpuProgram::getCompileErrorMessage() "GpuProgram::getCompileErrorMessage". Be aware that both of these methods are only valid after the core thread has initialized the object. You can ensure this by calling @ref bs::CoreObject::blockUntilCoreInitialized "GpuProgram::blockUntilCoreInitialized" but be aware this will block the calling thread which can result in a significant performance impact.

# Using GPU programs for rendering {#gpuPrograms_b}
To use a GPU program in a draw or dispatch call, you must first create a GPU pipeline object using the relevant GPU programs.

There are two types of pipeline objects: @ref bs::GraphicsPipelineState "GraphicsPipelineState" and @ref bs::ComputePipelineState "ComputePipelineState".

Example to create a graphics pipeline:
~~~~~~~~~~~~~{.cpp}
PIPELINE_STATE_CORE_DESC desc;
desc.vertexProgram = ...
desc.fragmentProgram = ...;
desc.geometryProgram = ...;
desc.hullProgram = ...;
desc.domainProgram = ...;

SPtr<GraphicsPipelineStateCore> graphicsPipeline = GraphicsPipelineStateCore::create(desc);
~~~~~~~~~~~~~

Example to create a compute pipeline:
~~~~~~~~~~~~~{.cpp}
SPtr<GpuProgramCore> computeProgram = ...;
SPtr<ComputePipelineStateCore> computePipeline = ComputePipelineStateCore::create(computeProgram);
~~~~~~~~~~~~~

Once created the pipeline can be bound for rendering by calling @ref bs::RenderAPICore::setGraphicsPipeline "RenderAPICore::setGraphicsPipeline" or @ref bs::RenderAPI::setComputePipeline "RenderAPI::setComputePipeline".

~~~~~~~~~~~~~{.cpp}
// Bind pipeline for use (continued from above)

RenderAPICore& rapi = RenderAPICore::instance();
rapi.setGraphicsPipeline(graphicsPipeline);
// Or: rapi.setComputePipeline(computePipeline);
~~~~~~~~~~~~~

Once pipeline state is bound, any subsequent draw/dispatch calls will then use the GPU programs attached to that state. 

Much more detailed information about pipelines and rendering is provided in the [render API manual](@ref renderAPI).

# GPU program parameters {#gpuPrograms_c}
Although you can use a GPU program without any parameters, most will require some additional data in order to perform their operations. Program parameters represent data that is static throughout a single GPU program execution (e.g. a draw call). For example, when drawing a 3D object you will usually want to provide a projection matrix that transforms the object from 3D to 2D, according to the camera the user is viewing the object through.

You can access information about GPU program parameters by calling @ref bs::GpuProgram::getParamDesc "GpuProgram::getParamDesc". This will return a structure containing information about all GPU parameters used by that GPU program. This includes primitives (int, float, etc.), textures, samplers, buffers and parameter buffers (constant/uniform buffers in DirectX/OpenGL lingo). 

You generally don't need to use this information directly. It is instead automatically parsed when you create a GPU pipeline. Once you have a pipeline you can use it to create a *GpuParams* object that allows you to assign values to all parameters of a specific pipeline.

## GpuParams {#gpuPrograms_b_a}
@ref bs::GpuParams "GpuParams" is a container for all parameters required by a single GPU pipeline (graphics or compute). It allows you to set primitive/texture/sampler/buffer parameters used by the GPU programs, which it stores in an internal buffer. You can then bind it before executing at draw/dispatch call, and the assigned parameters will be used by GPU programs in the current pipeline.

For example to assign a texture and a 2D vector as input to the program we created earlier:
~~~~~~~~~~~~~{.cpp}
SPtr<GraphicsPipelineState> graphicsPipeline = ...;
SPtr<GpuParams> params = GpuParams::create(graphicsPipeline);

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

# Core thread GPU programs {#gpuPrograms_e}
So far we have only talked about the simulation thread @ref bs::GpuProgram "GpuProgram" but have ignored the core thread @ref bs::GpuProgramCore "GpuProgramCore". The functionality between the two is mostly the same, with the major difference being that operations performed on the core thread version are immediate. So calls to @ref bs::GpuProgramCore::isCompiled() "GpuProgramCore::isCompiled" and @ref bs::GpuProgramCore::getCompileErrorMessage() "GpuProgramCore::getCompileErrorMessage" don't require any waiting.

Addtionally the core thread object can provide information about vertex input declaration. Vertex input declaration is only available for vertex shaders, and it defines in what format does the GPU program expect vertices to be in. This can be useful for setting up your meshes in the valid format. Use @ref bs::GpuProgramCore::getInputDeclaration "GpuProgramCore::getInputDeclaration" to retrieve the @ref bs::VertexDeclarationCore "VertexDeclaration". Check out the [mesh](@ref meshes) manual for more information on how to use vertex declarations.

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