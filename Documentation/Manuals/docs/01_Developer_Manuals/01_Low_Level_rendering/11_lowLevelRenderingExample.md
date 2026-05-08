---
title: Working example
---
A working example on how to use the low level rendering API, using most of the functionality described in these manuals, can be found in the *Framework/Examples/Source/LowLevelRendering* project provided with the source code.

The example demonstrates how to render a textured cube mesh using the low-level rendering API. Below is a breakdown of the key steps involved.

# Overview

The example performs the following operations:
- Creates GPU programs (vertex and fragment shaders)
- Creates a graphics pipeline state with blend and depth-stencil states
- Creates vertex and index buffers with appropriate vertex descriptions
- Creates textures and sampler states
- Sets up render targets
- Renders a textured cube every frame using command buffers
- Presents the result to the window

# Setup phase

During setup, all necessary rendering resources are initialized on the render thread (see @b3d::RenderThread for details on executing code on the render thread).

## Creating GPU programs

GPU programs are created using **GpuProgramCreateInformation** and the GPU device:

~~~~~~~~~~~~~{.cpp}
const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();

GpuProgramCreateInformation vertexProgramCreateInformation;
vertexProgramCreateInformation.Type = GPT_VERTEX_PROGRAM;
vertexProgramCreateInformation.EntryPoint = "main";
vertexProgramCreateInformation.Language = "hlsl"; // or "glsl" or "vksl"
vertexProgramCreateInformation.Source = vertexShaderSource;

TShared<GpuProgram> vertexProgram = gpuDevice->CreateGpuProgram(vertexProgramCreateInformation);

GpuProgramCreateInformation fragmentProgramCreateInformation;
fragmentProgramCreateInformation.Type = GPT_FRAGMENT_PROGRAM;
fragmentProgramCreateInformation.EntryPoint = "main";
fragmentProgramCreateInformation.Language = "hlsl";
fragmentProgramCreateInformation.Source = fragmentShaderSource;

TShared<GpuProgram> fragmentProgram = gpuDevice->CreateGpuProgram(fragmentProgramCreateInformation);
~~~~~~~~~~~~~

## Creating pipeline state

A graphics pipeline state is created with blend and depth-stencil states:

~~~~~~~~~~~~~{.cpp}
BlendStateInformation blendStateInformation;
blendStateInformation.RenderTargets[0].BlendEnable = true;
blendStateInformation.RenderTargets[0].RenderTargetWriteMask = 0b0111; // RGB, don't write to alpha
blendStateInformation.RenderTargets[0].ColorBlendOperation = BO_ADD;
blendStateInformation.RenderTargets[0].ColorSourceFactor = BF_SOURCE_ALPHA;
blendStateInformation.RenderTargets[0].ColorDestinationFactor = BF_INV_SOURCE_ALPHA;

DepthStencilStateInformation depthStencilStateInformation;
depthStencilStateInformation.DepthWriteEnable = false;
depthStencilStateInformation.DepthReadEnable = false;

GpuGraphicsPipelineStateInformation pipelineStateInformation;
pipelineStateInformation.BlendState = blendStateInformation;
pipelineStateInformation.DepthStencilState = depthStencilStateInformation;
pipelineStateInformation.VertexProgram = vertexProgram;
pipelineStateInformation.FragmentProgram = fragmentProgram;

TShared<GpuGraphicsPipelineState> pipelineState = gpuDevice->CreateGpuGraphicsPipelineState(pipelineStateInformation);
~~~~~~~~~~~~~

## Creating GPU parameter sets

GPU parameter sets are created from the pipeline state's parameter set layout (obtained via `GetParameterLayout()->GetSet()`) and will hold uniform buffers, textures, and samplers for a specific descriptor set:

~~~~~~~~~~~~~{.cpp}
TShared<GpuParameterSet> parameterSet = gpuDevice->CreateGpuParameterSet(pipelineState->GetParameterLayout()->GetSet(0), 0);
~~~~~~~~~~~~~

## Creating vertex description

A vertex description defines the layout of vertex data:

~~~~~~~~~~~~~{.cpp}
TInlineArray<VertexElement, 8> vertexElements;
vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));

TShared<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(vertexElements);
~~~~~~~~~~~~~

## Creating vertex and index buffers

Vertex and index buffers are created using **GpuBufferCreateInformation**:

~~~~~~~~~~~~~{.cpp}
u32 vertexStride = vertexDescription->GetVertexStride();
u32 numVertices = 24;

GpuBufferCreateInformation vertexBufferCreateInformation;
vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
vertexBufferCreateInformation.Vertex.Count = numVertices;
vertexBufferCreateInformation.Vertex.ElementSize = vertexStride;

TShared<GpuBuffer> vertexBuffer = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

// Write vertex data
GpuBufferUtility::Write(vertexBuffer, 0, vertexStride * numVertices, vertexData, GpuBufferWriteFlag::Discard);

// Create index buffer
u32 numIndices = 36;

GpuBufferCreateInformation indexBufferCreateInformation;
indexBufferCreateInformation.Type = GpuBufferType::Index;
indexBufferCreateInformation.Index.Count = numIndices;
indexBufferCreateInformation.Index.Type = IT_32BIT;

TShared<GpuBuffer> indexBuffer = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

// Write index data
GpuBufferUtility::Write(indexBuffer, 0, numIndices * sizeof(u32), indexData, GpuBufferWriteFlag::Discard);
~~~~~~~~~~~~~

## Creating textures and samplers

A texture is created from pixel data, and a sampler state is created to control texture filtering:

~~~~~~~~~~~~~{.cpp}
TShared<PixelData> pixelData = PixelData::Create(2, 2, 1, PF_RGBA8);
pixelData->SetColorAt(Color::kWhite, 0, 0);
pixelData->SetColorAt(Color::kBlack, 1, 0);
pixelData->SetColorAt(Color::kWhite, 1, 1);
pixelData->SetColorAt(Color::kBlack, 0, 1);

TShared<Texture> surfaceTexture = gpuDevice->CreateTexture(pixelData);

SamplerStateCreateInformation samplerStateCreateInformation;
samplerStateCreateInformation.MinFilter = FO_POINT;
samplerStateCreateInformation.MagFilter = FO_POINT;

TShared<SamplerState> surfaceSampler = gpuDevice->FindOrCreateSamplerState(samplerStateCreateInformation);
~~~~~~~~~~~~~

## Creating render targets

Render targets are created with color and depth attachments:

~~~~~~~~~~~~~{.cpp}
TextureCreateInformation colorAttachmentInformation;
colorAttachmentInformation.Width = 1280;
colorAttachmentInformation.Height = 720;
colorAttachmentInformation.Format = PF_RGBA8;
colorAttachmentInformation.Usage = TextureUsageFlag::RenderTarget;

TShared<Texture> colorAttachment = gpuDevice->CreateTexture(colorAttachmentInformation);

TextureCreateInformation depthAttachmentInformation;
depthAttachmentInformation.Width = 1280;
depthAttachmentInformation.Height = 720;
depthAttachmentInformation.Format = PF_D32;
depthAttachmentInformation.Usage = TextureUsageFlag::DepthStencil;

TShared<Texture> depthAttachment = gpuDevice->CreateTexture(depthAttachmentInformation);

RenderTextureCreateInformation renderTextureInformation;
renderTextureInformation.ColorSurfaces[0].Texture = colorAttachment;
renderTextureInformation.DepthStencilSurface.Texture = depthAttachment;

TShared<RenderTexture> renderTarget = RenderTexture::Create(renderTextureInformation);
~~~~~~~~~~~~~

# Render phase

Every frame, the following rendering operations are performed using a command buffer.

## Creating command buffer

Command buffers are obtained from the command buffer pool:

~~~~~~~~~~~~~{.cpp}
const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
const TShared<GpuCommandBufferPool>& commandBufferPool = RendererManager::Instance().GetActive()->GetCommandBufferPool();

TShared<GpuCommandBuffer> commandBuffer = commandBufferPool->Create(GpuCommandBufferCreateInformation::Create("LowLevelRendering"));
~~~~~~~~~~~~~

## Setting up uniform buffers

Uniform data is written to a GPU buffer and bound to the parameters:

~~~~~~~~~~~~~{.cpp}
struct UniformBlock
{
    Matrix4 MatrixWorldViewProjection;
    Color Tint;
};

UniformBlock uniformBlock;
uniformBlock.MatrixWorldViewProjection = CalculateWorldViewProjectionMatrix();
uniformBlock.Tint = Color(1.0f, 1.0f, 1.0f, 0.5f);

TShared<GpuBuffer> uniformBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(sizeof(UniformBlock)));
{
	GpuBufferMappedScope mappedScope = uniformBuffer->Map(GpuMapOption::Write);
	memcpy(mappedScope.GetMappedMemory(), &uniformBlock, sizeof(uniformBlock));
}

parameterSet->SetUniformBuffer("Params", uniformBuffer);
~~~~~~~~~~~~~

## Binding textures and samplers

Textures and samplers are bound to the GPU parameter set:

~~~~~~~~~~~~~{.cpp}
parameterSet->SetSampledTexture("gMainTexture", surfaceTexture);
parameterSet->SetSamplerState("gMainTexture", surfaceSampler);
~~~~~~~~~~~~~

## Beginning render pass

A render pass is started with the render target and the target is cleared:

~~~~~~~~~~~~~{.cpp}
render::RenderPassCreateInformation renderPassInformation(renderTarget);
renderPassInformation.ClearMask = RT_COLOR_ALL | RT_DEPTH;
renderPassInformation.ClearColor = Color::kBlue;
renderPassInformation.Parameters.Add(parameterSet);
commandBuffer->BeginRenderPass(renderPassInformation);
~~~~~~~~~~~~~

## Binding pipeline and geometry

The pipeline state, vertex buffers, index buffer, and vertex description are bound:

~~~~~~~~~~~~~{.cpp}
commandBuffer->SetGpuGraphicsPipelineState(pipelineState);
commandBuffer->SetVertexBuffers(0, &vertexBuffer, 1);
commandBuffer->SetIndexBuffer(indexBuffer);
commandBuffer->SetVertexDescription(vertexDescription);
commandBuffer->SetDrawOperation(DOT_TRIANGLE_LIST);
~~~~~~~~~~~~~

## Binding GPU parameter sets

GPU parameter sets containing uniform buffers, textures, and samplers are bound:

~~~~~~~~~~~~~{.cpp}
commandBuffer->SetGpuParameterSet(parameterSet);
~~~~~~~~~~~~~

## Drawing

An indexed draw call is issued:

~~~~~~~~~~~~~{.cpp}
commandBuffer->DrawIndexed(0, numIndices, 0, numVertices, 1, 0);
~~~~~~~~~~~~~

## Ending render pass

The render pass is ended after all draw calls have been issued:

~~~~~~~~~~~~~{.cpp}
commandBuffer->EndRenderPass();
~~~~~~~~~~~~~

## Blitting to the window

Since the rendering was done to an offscreen render target, the result must be copied (blitted) to the render window's back buffer before presenting. Use @b3d::render::RendererUtility::Blit with a @b3d::render::BlitInformation to copy the color attachment to the render window. The static helper @b3d::render::BlitInformation::BlitColor creates a blit descriptor with commonly used settings (no blending, no filtering):

~~~~~~~~~~~~~{.cpp}
// Get the color attachment from the offscreen render target
TShared<Texture> colorTexture = renderTarget->GetColorTexture(0);

// Blit the offscreen color attachment to the render window
TShared<RenderWindow> renderWindow = ...;
GetRendererUtility().Blit(*commandBuffer, BlitInformation::BlitColor(colorTexture, renderWindow));
~~~~~~~~~~~~~

## Submitting and presenting

Finally, the command buffer is submitted for GPU execution and the render window is presented:

~~~~~~~~~~~~~{.cpp}
// Submit the command buffer for GPU execution
gpuDevice->SubmitCommandBuffer(commandBuffer);

// Present the rendered result to the window
gpuDevice->PresentRenderWindow(renderWindow);
~~~~~~~~~~~~~
