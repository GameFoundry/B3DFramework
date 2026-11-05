---
title: Working example
---
A working example on how to use the low level rendering API, using most of the functionality described in these manuals, can be found in the *Framework/Examples/Source/LowLevelRendering* project provided with the source code.

The example demonstrates how to render a textured cube mesh using the low-level rendering API. Below is a breakdown of the key steps involved.

# Overview

The example performs the following operations:
- Creates GPU programs and a graphics pipeline state from a BSL shader's pass
- Creates vertex and index buffers with appropriate vertex descriptions
- Creates textures and sampler states
- Describes the textures it renders into
- Renders a textured cube every frame through a frame graph
- Presents the result to the window

# Setup phase

During setup, all necessary rendering resources are initialized on the render thread (see @b3d::RenderThread for details on executing code on the render thread).

## Creating GPU programs and pipeline state

The GPU programs and the states they render with come from a shader that ships with the example. Its compiled pass provides everything needed to create the pipeline state through the low-level API:

~~~~~~~~~~~~~{.cpp}
const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

// Grab the shader's first variation compatible with the active GPU backend, and ensure it is compiled
const TShared<Variation> shaderVariation = boxShader->GetCompatibleVariations()[0];
if(!shaderVariation->IsCompiled())
	shaderVariation->Compile().BlockUntilComplete();

const PassInformation& passInformation = shaderVariation->GetPass(0)->GetInformation();

GpuGraphicsPipelineStateCreateInformation pipelineCreateInformation;
pipelineCreateInformation.VertexProgram = gpuDevice->CreateGpuProgram(passInformation.VertexProgramCreateInformation);
pipelineCreateInformation.FragmentProgram = gpuDevice->CreateGpuProgram(passInformation.FragmentProgramCreateInformation);
pipelineCreateInformation.BlendState = passInformation.BlendStateInformation;
pipelineCreateInformation.RasterizerState = passInformation.RasterizerStateInformation;
pipelineCreateInformation.DepthStencilState = passInformation.DepthStencilStateInformation;

TShared<GpuGraphicsPipelineState> pipelineState = gpuDevice->CreateGpuGraphicsPipelineState(pipelineCreateInformation);
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

// Write vertex data (the renderer's work context drives the render-thread upload)
GpuBufferUtility::Write(render::GetRenderer()->GetGpuContext(), vertexBuffer, 0, vertexStride * numVertices, vertexData, GpuBufferWriteFlag::Discard);

// Create index buffer
u32 numIndices = 36;

GpuBufferCreateInformation indexBufferCreateInformation;
indexBufferCreateInformation.Type = GpuBufferType::Index;
indexBufferCreateInformation.Index.Count = numIndices;
indexBufferCreateInformation.Index.Type = IT_32BIT;

TShared<GpuBuffer> indexBuffer = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

// Write index data
GpuBufferUtility::Write(render::GetRenderer()->GetGpuContext(), indexBuffer, 0, numIndices * sizeof(u32), indexData, GpuBufferWriteFlag::Discard);
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

## Describing the render textures

The cube renders into a color and a depth texture. The textures are only needed within a frame, so setup only describes them, and the frame graph creates them every frame in memory it reuses between frames:

~~~~~~~~~~~~~{.cpp}
TextureCreateInformation colorInformation;
colorInformation.Width = 1280;
colorInformation.Height = 720;
colorInformation.Format = PF_RGBA8;
colorInformation.Usage = TextureUsageFlag::RenderTarget;
colorInformation.ClearColor = Color::kBlue;

TextureCreateInformation depthInformation;
depthInformation.Width = 1280;
depthInformation.Height = 720;
depthInformation.Format = PF_D32;
depthInformation.Usage = TextureUsageFlag::DepthStencil;
~~~~~~~~~~~~~

# Render phase

Every frame, the example describes its GPU work as a [frame graph](../Renderer/frameGraph): a pass that renders the cube into the two textures, and a pass that draws the color texture into the window. The graph records every barrier between the passes.

## Setting up uniform buffers

Uniform data is written to a GPU buffer, which is later bound to the parameters:

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
~~~~~~~~~~~~~

## Creating the frame graph

The graph records with the render thread's work context. The window is a resource the graph did not create, so it is imported, while the two textures are created by the graph and only live within it:

~~~~~~~~~~~~~{.cpp}
GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();

FrameGraph frameGraph(gpuContext, "Low level rendering");
frameGraph.ImportRenderTarget(renderWindow);

const TShared<Texture> colorTexture = frameGraph.CreateTexture("Box color", colorInformation);
const TShared<Texture> depthTexture = frameGraph.CreateTexture("Box depth", depthInformation);
~~~~~~~~~~~~~

## Describing the cube pass

The pass clears both textures to their clear values and renders into them:

~~~~~~~~~~~~~{.cpp}
FrameGraphRasterPass boxPass;
boxPass.SetColorAttachment(0, colorTexture, FrameGraphAttachmentLoad::Clear);
boxPass.SetDepthStencilAttachment(depthTexture, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear);
~~~~~~~~~~~~~

## Creating GPU parameter sets

GPU parameter sets are allocated from the render thread's work context, using its parameter set pool. The pipeline state's parameter set layout (obtained via `GetParameterLayout()->GetSet()`) selects which descriptor set the parameter set holds uniform buffers, textures, and samplers for. Adding the set to the pass tells the graph what the pass binds:

~~~~~~~~~~~~~{.cpp}
const TShared<GpuParameterSet> boxParameters = gpuContext.GetParameterSetPool().Create(pipelineState->GetParameterLayout()->GetSet(0), 0);
boxPass.AddParameterSet(boxParameters);

boxParameters->SetUniformBuffer("Params", uniformBuffer);
boxParameters->SetSampledTexture("gMainTexture", surfaceTexture);
boxParameters->SetSamplerState("gMainTexSamp", surfaceSampler);
~~~~~~~~~~~~~

The surface texture and the uniform buffer are not resources of the graph. They are at rest, which allows the pass to read them with no further declaration.

## Recording the cube pass

The function added with the pass runs when the graph records it, after the pass's barriers and with the attachments bound. It binds the pipeline state, the geometry and the parameter set, then issues an indexed draw call:

~~~~~~~~~~~~~{.cpp}
frameGraph.AddRasterPass("Box", std::move(boxPass), [boxParameters, pipelineState, vertexBuffer, indexBuffer, vertexDescription, numIndices, numVertices](FrameGraphPassContext& passContext) mutable
{
	GpuCommandBuffer& commandBuffer = passContext.GetCommandBuffer();

	commandBuffer.SetGpuGraphicsPipelineState(pipelineState);

	commandBuffer.SetVertexBuffers(0, &vertexBuffer, 1);
	commandBuffer.SetIndexBuffer(indexBuffer);
	commandBuffer.SetVertexDescription(vertexDescription);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	commandBuffer.SetGpuParameterSet(boxParameters);

	commandBuffer.DrawIndexed(0, numIndices, 0, numVertices, 1, 0);
});
~~~~~~~~~~~~~

## Drawing to the window

Since the rendering was done to an offscreen texture, the result must be drawn into the window before presenting. A second pass draws a full-screen quad that samples the color texture, using @b3d::render::BlitMat. It overwrites the whole window, so the window's previous contents are not loaded. The material's `Prepare()` creates a parameter set for the pass and adds it to the pass, which is how the graph knows that this pass reads what the previous pass wrote:

~~~~~~~~~~~~~{.cpp}
BlitMat* const blitMaterial = BlitMat::GetVariation(1, true, false);

FrameGraphRasterPass blitPass;
blitPass.SetRenderTarget(renderWindow, FrameGraphAttachmentLoad::DontCare);

const TShared<GpuParameterSet> blitParameters = blitMaterial->Prepare(blitPass, colorTexture);

frameGraph.AddRasterPass("Blit to window", std::move(blitPass), [blitMaterial, blitParameters](FrameGraphPassContext& passContext)
{
	blitMaterial->Execute(passContext, blitParameters, Area2(0.0f, 0.0f, 1280.0f, 720.0f), false);
});
~~~~~~~~~~~~~

## Submitting and presenting

Finally, the graph places the textures in memory and plans the barriers between the passes, then records and submits the passes. This leaves the window ready to be presented:

~~~~~~~~~~~~~{.cpp}
frameGraph.Compile();
frameGraph.Execute();

gpuDevice->PresentRenderWindow(renderWindow);
~~~~~~~~~~~~~
