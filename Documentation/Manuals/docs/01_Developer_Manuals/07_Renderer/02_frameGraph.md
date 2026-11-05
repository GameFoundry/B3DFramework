---
title: Frame graph
---

A @b3d::render::FrameGraph describes a piece of GPU work as a list of passes and the resources they read and write. From those declarations the graph works out everything a command buffer would otherwise need from you: which barriers to issue and where, where temporary resources live in memory, and which passes can be skipped.

All rendering in the framework goes through frame graphs. The renderer builds one to render its views, and smaller systems such as mipmap generation or the font atlas build their own. Command buffers issue no barriers of their own, so unless you record barriers by hand (see the [barriers](../Low_Level_rendering/barriers) manual), a frame graph is how your GPU work gets correctly synchronized.

The graph is a render thread object, in the `b3d::render` namespace.

# Building a graph
A graph is used in three steps:
 - Declare resources and passes.
 - @b3d::render::FrameGraph::Compile culls passes, places temporary resources in memory and plans every barrier. No resources or passes can be added afterwards.
 - @b3d::render::FrameGraph::Execute records the passes on command buffers and submits them. Each pass's function runs here, not when the pass was added.

The example below renders into a temporary texture, then draws that texture into a window.

~~~~~~~~~~~~~{.cpp}
GpuWorkContext& gpuContext = GetRenderer()->GetGpuContext();

// The graph records with this context, and places its temporary resources with the context's allocator
FrameGraph graph(gpuContext, "MyGraph");

// The window already exists, so it is imported instead of created
graph.ImportRenderTarget(window);

// The texture only lives within this graph
const TShared<Texture> colorTexture = graph.CreateTexture("Color", colorInformation);

// First pass: clear the texture and draw into it
FrameGraphRasterPass drawPass;
drawPass.SetColorAttachment(0, colorTexture, FrameGraphAttachmentLoad::Clear);

const TShared<GpuParameterSet> drawParameters = gpuContext.GetParameterSetPool().Create(pipelineState->GetParameterLayout()->GetSet(0), 0);
drawPass.AddParameterSet(drawParameters);
drawParameters->SetSampledTexture("gMainTexture", surfaceTexture);

graph.AddRasterPass("Draw", std::move(drawPass), [drawParameters, pipelineState](FrameGraphPassContext& context)
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
	commandBuffer.SetGpuGraphicsPipelineState(pipelineState);
	commandBuffer.SetGpuParameterSet(drawParameters);
	commandBuffer.Draw(0, 3);
});

// Second pass: sample the texture and overwrite the window
BlitMat* const blitMaterial = BlitMat::GetVariation(1, true, false);

FrameGraphRasterPass blitPass;
blitPass.SetRenderTarget(window, FrameGraphAttachmentLoad::DontCare);

const TShared<GpuParameterSet> blitParameters = blitMaterial->Prepare(blitPass, colorTexture);

graph.AddRasterPass("Blit", std::move(blitPass), [blitMaterial, blitParameters, width, height](FrameGraphPassContext& context)
{
	blitMaterial->Execute(context, blitParameters, Area2(0.0f, 0.0f, (float)width, (float)height), false);
});

graph.Compile();
graph.Execute();

gpuDevice->PresentRenderWindow(window);
~~~~~~~~~~~~~

Nothing above orders the passes or transitions the texture. The graph sees that the second pass samples what the first pass wrote, and issues the barrier between them.

Passes that survive culling run in the order they were added. The graph does not reorder them.

A graph compiles and executes once. A compiled graph must be executed before it is destroyed; a graph that was never compiled can simply be destroyed. Only one graph may execute at a time, and only the thread that owns the graph's work context may use it.

@b3d::render::FrameGraph::AddExecuteCallback registers a function that runs at the end of **Execute()**, after the work is submitted. Use it for cleanup that the pass functions depend on, when the code that builds the passes doesn't also execute the graph.

# Resources
A pass can touch three kinds of resources.

| Kind | Created by | Passes may | Lifetime |
|------|------------|------------|----------|
| Transient | @b3d::render::FrameGraph::CreateTexture, @b3d::render::FrameGraph::CreateBuffer | Read and write | Within the graph, unless extracted |
| Imported | **GpuDevice**, then @b3d::render::FrameGraph::ImportTexture, @b3d::render::FrameGraph::ImportBuffer or @b3d::render::FrameGraph::ImportRenderTarget | Read and write | Owned by the caller |
| Resting | **GpuDevice** | Only read | Owned by the caller |

## Transient resources
Create resources that are only needed within the graph as transients. Their memory is shared with other transients whose lifetimes don't overlap, and is reused between frames.

A transient's properties are valid immediately, and you can put it in parameter sets and attachments right away, but it has no memory until **Compile()**. Transients can only be accessed by the GPU, so they cannot be mapped or uploaded to from the CPU. Don't use one after the graph ends.

See the [transient resources](../Low_Level_rendering/transientResources) manual for how the memory is managed.

## Imported resources
Import any resource the graph did not create that a pass needs to write. Imported resources must be initialized and at rest when imported, and the graph returns them to rest at the end. A resource can only be a resource of one graph at a time, and importing the same resource twice into a graph has no further effect.

Render targets are imported as a whole with **ImportRenderTarget()**, which is the only way to render into a window. The surfaces of a window can only be accessed as attachments.

## Resting resources
Every other resource is at rest, which means it can be read from any shader stage without a barrier. Passes may bind and read these without any declaration, but never write them. Long-lived data such as meshes, material textures and uniform buffers are normally used this way. Binding a resting resource in a slot that writes it reports an error telling you to import it.

Resources kept across frames, such as shadow maps or history buffers, are created on the **GpuDevice** by the system that keeps them, then imported into the graphs that write them.

## Keeping a transient after the graph
@b3d::render::FrameGraph::ExtractTexture and @b3d::render::FrameGraph::ExtractBuffer turn a transient into an ordinary resource. **Compile()** gives it its own memory, and **Execute()** returns it to rest. You can then use it like any other resource, and import it into later graphs.

~~~~~~~~~~~~~{.cpp}
const TShared<Texture> output = graph.CreateTexture("Output", outputInformation);
graph.ExtractTexture(output);

// ... add the passes that write the texture, compile and execute ...

mKeptTexture = output;
~~~~~~~~~~~~~

## Returning a resource to rest early
A resource normally returns to rest at the end of the graph. @b3d::render::FrameGraphPass::Export returns it to rest right after a chosen pass, so later passes of the same graph can read it without declaring it. They are then free to bind it through parameter sets that are not added to the pass. Later passes must not write it, and only imported and extracted resources can be exported.

~~~~~~~~~~~~~{.cpp}
FrameGraphComputePass pass;
pass.AddParameterSet(parameters);
parameters->SetStorageTexture("gOutput", importedTexture, TextureSurface());
pass.Export(importedTexture);

graph.AddComputePass("Generate", std::move(pass), fnExecute);
~~~~~~~~~~~~~

# Passes
There are three types of passes, added with @b3d::render::FrameGraph::AddRasterPass, @b3d::render::FrameGraph::AddComputePass and @b3d::render::FrameGraph::AddCopyPass. Each takes a name, a description of the pass and a function that records its commands. The name labels the pass in GPU captures and profiler samples.

## Declaring parameter sets
Raster and compute passes declare the resources they bind through parameter sets. Add each set to the pass with @b3d::render::FrameGraphPass::AddParameterSet, and assign its resources before you add the pass to the graph, because the graph reads the set at that point.

Each graph resource in a set counts as an access of the pass, determined by the slot it is bound to:
 - Uniform buffers and sampled textures are read.
 - Storage buffers and storage textures are read and written, unless the shader declares a read-only buffer.
 - The shader stages come from the slot.

A pass cannot access the same subresource in two layouts, such as sampling a texture while also using it as a storage texture. The graph reports it, and the pass must be split in two.

A set that holds no graph resources may be bound without being added.

## Pass functions
The function you provide runs during **Execute()**, after every pass has been added, and on a command buffer the graph provides through @b3d::render::FrameGraphPassContext::GetCommandBuffer. This has a few consequences:
 - Capture what the function needs by value. By the time it runs the code that added the pass has moved on, and may have changed the data it used.
 - Don't open or close render passes, and don't issue barriers. The graph does both.
 - The function of a culled pass does not run at all.

Data from the frame allocator that a pass function reads must stay allocated until **Execute()** returns, so keep the @b3d::FrameAllocatorScope around both the building and the execution of the graph.

## Raster passes
A @b3d::render::FrameGraphRasterPass draws into attachments. The graph opens the render pass before the function runs, and closes it after.

Attach textures with @b3d::render::FrameGraphRasterPass::SetColorAttachment and @b3d::render::FrameGraphRasterPass::SetDepthStencilAttachment, which build a render target for you. To render into a render target you imported, such as a window, use @b3d::render::FrameGraphRasterPass::SetRenderTarget instead.

~~~~~~~~~~~~~{.cpp}
FrameGraphRasterPass pass;

// Render into mip 2 of the texture
pass.SetColorAttachment(0, colorTexture, FrameGraphAttachmentLoad::Clear, 2);
pass.SetDepthStencilAttachment(depthTexture, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear, 2);
~~~~~~~~~~~~~

@b3d::render::FrameGraphAttachmentLoad determines what happens with the existing contents of the attachment:
 - **Load** - The pass reads the existing contents, for example to blend onto them.
 - **Clear** - The pass clears the attachment to the clear value of its texture or render target.
 - **DontCare** - The pass overwrites every pixel it reads, and the existing contents are discarded.

Pick the cheapest one that is correct. If you **Load** contents that no earlier pass wrote, the graph drops the load and development builds log a warning, because the contents are undefined. Transients have no contents at the start of the graph, while imported resources hold whatever they held before it.

@b3d::render::FrameGraphDepthStencilMode determines how the pass accesses the depth-stencil attachment:
 - **Write** - Depth and stencil tests may write the attachment.
 - **ReadOnly** - Depth and stencil tests only read the attachment, and the pass may also sample it.
 - **DepthReadOnly** - Depth tests only read the depth aspect, which the pass may also sample, and stencil tests may write the stencil aspect.

The graph sets no viewport, so a pass that draws sets one on the command buffer first with @b3d::render::GpuCommandBuffer::SetViewport.

## Compute passes
A @b3d::render::FrameGraphComputePass dispatches compute work. The pass below reads one buffer and writes another.

~~~~~~~~~~~~~{.cpp}
const TShared<GpuBuffer> output = graph.CreateBuffer("Output", outputBufferInformation);

FrameGraphComputePass pass;
const TShared<GpuParameterSet> parameters = gpuContext.GetParameterSetPool().Create(computePipeline->GetParameterLayout()->GetSet(0), 0);
pass.AddParameterSet(parameters);
parameters->SetStorageBuffer("gInput", input);
parameters->SetStorageBuffer("gOutput", output);

graph.AddComputePass("Process", std::move(pass), [parameters, computePipeline](FrameGraphPassContext& context)
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	commandBuffer.SetGpuComputePipelineState(computePipeline);
	commandBuffer.SetGpuParameterSet(parameters);
	commandBuffer.DispatchCompute(8, 8);
});
~~~~~~~~~~~~~

## Copy passes
A @b3d::render::FrameGraphCopyPass copies between resources with transfer operations, such as @b3d::render::GpuCommandBuffer::CopyBufferToBuffer or @b3d::render::GpuCommandBuffer::BlitTexture. Copies bind no parameter sets, so they declare what they access with @b3d::render::FrameGraphCopyPass::AddSource and @b3d::render::FrameGraphCopyPass::AddDestination.

Both sources and destinations must be resources of the graph, so import any resource that isn't one, even if the pass only reads it.

~~~~~~~~~~~~~{.cpp}
graph.ImportBuffer(source);
graph.ImportBuffer(destination);

FrameGraphCopyPass pass;
pass.AddSource(source);
pass.AddDestination(destination);

graph.AddCopyPass("Copy", std::move(pass), [source, destination, byteCount](FrameGraphPassContext& context)
{
	context.GetCommandBuffer().CopyBufferToBuffer(source, destination, 0, 0, byteCount);
});
~~~~~~~~~~~~~

# Culling
**Compile()** culls every pass whose writes nothing observes. A write is observed if a later surviving pass reads it, if the resource outlives the graph (it is imported or extracted), or if the resource is exported after the pass. Transients used only by culled passes never get memory.

A pass that writes nothing the graph knows about is culled too, along with its side effects. If a pass only exists for what it does outside the graph, make its output an imported resource.

A write also stops being observed once a later pass discards the contents, by clearing an attachment or by loading it with **DontCare**. An earlier pass that wrote only those contents is culled, even if the resource is imported.

Surviving raster passes also skip storing attachments that nothing observes afterwards. This is common for depth buffers that aren't used after the pass.

To check what happened, call @b3d::render::FrameGraph::IsPassCulled, or read @b3d::render::FrameGraph::GetStatistics. Both are valid after **Compile()**. The @b3d::render::FrameGraphStatistics counts passes, culled passes, discarded attachments, barriers and submissions.

# Materials
Renderer materials (see the [renderer plugin](../Renderer/customRenderer) manual) split their work the same way the graph does. By convention a material has three methods:
 - `Initialize()` caches its parameter handles, with the `Get*Parameter()` methods, or the `TryGet*Parameter()` methods for parameters that may be absent.
 - `Prepare(pass, ...)` runs when the pass is declared. It creates a parameter set with @b3d::render::RendererMaterialBase::CreateGpuParameterSet, which also adds the set to the pass, assigns the resources, and returns the set.
 - `Execute(context, set, ...) const` runs in the pass function. It binds the material and the set, and records the work. It is `const` because pass functions may run on several threads, so it must not change material state. Anything `Prepare()` computes goes into the set or the function's captures.

~~~~~~~~~~~~~{.cpp}
class DownsampleMaterial : public RendererMaterial<DownsampleMaterial>
{
	RMAT_DEF("Downsample.bsl")

public:
	void Initialize() override
	{
		mParameterLayout->GetStorageBufferParameter("gInput", mInputParameter);
		mParameterLayout->GetStorageBufferParameter("gOutput", mOutputParameter);
	}

	TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<GpuBuffer>& input, const TShared<GpuBuffer>& output)
	{
		const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);
		mInputParameter.Set(set, input);
		mOutputParameter.Set(set, output);

		return set;
	}

	void Execute(FrameGraphPassContext& context, const TShared<GpuParameterSet>& set, const Vector2I& outputSize) const
	{
		GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();

		Bind(commandBuffer, set);
		commandBuffer.DispatchCompute((u32)Math::DivideAndRoundUp(outputSize.X, 8), (u32)Math::DivideAndRoundUp(outputSize.Y, 8));
	}

private:
	GpuParameterStorageBuffer mInputParameter;
	GpuParameterStorageBuffer mOutputParameter;
};
~~~~~~~~~~~~~

Using the material takes one call to each:

~~~~~~~~~~~~~{.cpp}
DownsampleMaterial* const material = DownsampleMaterial::Get();

FrameGraphComputePass pass;
const TShared<GpuParameterSet> set = material->Prepare(pass, input, output);

graph.AddComputePass("Downsample", std::move(pass), [material, set, outputSize](FrameGraphPassContext& context)
{
	material->Execute(context, set, outputSize);
});
~~~~~~~~~~~~~

# Reading results back
@b3d::render::TextureUtility::ReadAsync and @b3d::render::GpuBufferUtility::ReadAsync have overloads that take a graph. They add a copy pass to it, so the resource must be a resource of the graph, and the call must come before **Compile()**.

~~~~~~~~~~~~~{.cpp}
TAsyncOp<TShared<PixelData>> readOp = TextureUtility::ReadAsync(graph, texture);

graph.Compile();
graph.Execute();
~~~~~~~~~~~~~

To run your own code when a pass's work finishes, connect to @b3d::render::GpuCommandBuffer::OnDidComplete from the pass function. It runs on the thread that owns the command buffer. Also connect to @b3d::render::GpuCommandBuffer::OnDestroyed if something waits on the result, since a command buffer that is never submitted never completes.

~~~~~~~~~~~~~{.cpp}
graph.AddCopyPass("Readback", std::move(pass), [operation, source, stagingBuffer, byteCount](FrameGraphPassContext& context)
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	commandBuffer.CopyBufferToBuffer(source, stagingBuffer, 0, 0, byteCount);

	commandBuffer.OnDidComplete.Connect([operation, stagingBuffer]() mutable
	{
		const GpuBufferMappedScope mapping = stagingBuffer->Map(GpuMapOption::Read);
		// ... read the mapped memory, then complete the operation ...
		operation.CompleteOperation(true);
	});

	commandBuffer.OnDestroyed.Connect([operation](bool isSubmitted) mutable
	{
		if(!isSubmitted)
			operation.CompleteOperation(false);
	});
});
~~~~~~~~~~~~~

> Completion is only processed at the end of a frame, and when you wait for the GPU. Code that blocks on a result, such as an importer running without frames, must wait itself or it will hang.

To wait, either call @b3d::GpuDevice::WaitUntilIdle after **Execute()**, or wait only for the work of this graph by signaling a fence from it. The fence is signaled by the last submission of the graph, which is always on the first graphics queue.

~~~~~~~~~~~~~{.cpp}
const TShared<GpuTimelineFence> fence = gpuDevice->CreateTimelineFence();
const u64 fenceValue = 1;

graph.AddSignalFence(fence, fenceValue);
graph.Compile();
graph.Execute();

gpuDevice->GetQueue(GQT_GRAPHICS, 0)->WaitUntilFence(fence, fenceValue);
~~~~~~~~~~~~~

# Where graphs come from
Most code does not create the graph, but adds passes to one it is given:
 - The renderer builds one graph for the scene each time it renders its views, with all views adding their passes to it.
 - [Renderer extensions](../Renderer/rendererExtensions) receive that graph through @b3d::render::RendererViewContext::Graph.
 - Tasks created with @b3d::render::RendererTask::Create receive a graph through their worker function. The renderer compiles and executes it, along with the work of other tasks of the same frame.

~~~~~~~~~~~~~{.cpp}
auto fnFilter = [source, output](FrameGraph& graph)
{
	// ... add passes to the graph ...

	// True if the task is complete, or false to run it again next frame
	return true;
};

const TShared<RendererTask> task = RendererTask::Create("Filter", fnFilter);
GetRenderer()->AddTask(task);
~~~~~~~~~~~~~

Work that runs outside of the frame creates a graph of its own, on the render thread with the renderer's work context, or on another thread with its own [work context](../Low_Level_rendering/gpuWorkContext). Mipmap generation (`Image/B3DGenerateMipmap.cpp`), texture compression (`Image/B3DTextureCompressor.cpp`) and the font atlas (`Text/B3DFont.cpp`) are examples.

Only one graph may execute at a time, because the submissions of a graph must reach the queues without another graph's submissions between them. A pass function therefore cannot execute another graph. Development builds report overlapping executions as an error.

# Async compute
Compute and copy passes created with @b3d::render::FrameGraphPassFlag::AsyncCompute prefer the async compute queue, so they can overlap graphics work.

~~~~~~~~~~~~~{.cpp}
FrameGraphComputePass pass(FrameGraphPassFlag::AsyncCompute);
~~~~~~~~~~~~~

The flag is a preference. The pass runs on the graphics queue if the device has no compute queue, or if `gpu.FrameGraphAsyncCompute` is disabled. Raster passes always run on the graphics queue, and the graph reports the flag as an error on them.

The rest of the graph needs no changes. The graph cuts its passes into several submissions, plans the waits between the queues, and transfers resources between them with split barriers. It always ends on the graphics queue, waiting on the compute queue. Passes still run in the order they were added on each queue.

A few rules apply to resources used on both queues:
 - An async pass can only read a resting texture if the texture can be read on several queues at once, as determined by @b3d::GpuBackendUtility::IsConcurrentlyReadable. That is true of textures that are sampleable only, or created with **TextureUsageFlag::AllowConcurrentQueueReads**. Other resting textures rest on the graphics queue and must be imported. Buffers have no such restriction. The check uses the pass's preference, so it behaves the same whether or not the pass actually moves.
 - A texture that rests on the graphics queue and is exported by an async pass returns to rest on the compute queue, and the graph moves it to the graphics queue at the first point where the graphics queue waits on the compute queue. Graphics passes after that point can read it without declaring it.
 - Each graph plans the order of its own submissions, but graphs submit their work in the order they execute, so execute separate graphs in the order their work must run.

The renderer currently records everything on the graphics queue. For working examples see the async compute tests in `B3DFrameGraphTestSuite.cpp`, such as **TestAsyncComputeProducer** and **TestAsyncComputeExport**.

# Parallel recording
A graph created with @b3d::render::FrameGraphFlag::RecordInParallel records its submissions on several threads, one task per submission. This only has an effect on graphs that have more than one submission, which in practice means graphs that use async compute. Submissions that use a window are recorded on the calling thread.

~~~~~~~~~~~~~{.cpp}
FrameGraph graph(gpuContext, "MyGraph", FrameGraphFlag::RecordInParallel);
~~~~~~~~~~~~~

The pass functions of such a graph run on several threads at once, which limits what they can do. They must not:
 - Use the work context of the graph.
 - Write parameter sets or buffers, such as with `Set*` calls or @b3d::render::TextureUtility::Write.
 - Create pipeline states, parameter layouts or materials.
 - Render to or copy from a window.
 - Discard-write, relocate or recreate resources.
 - Yield the fiber they run on.
 - Touch state shared with other passes without their own synchronization.

Only the thread that owns a parameter set may write it, so the graph flushes every set added to a pass before it starts recording. A set that a pass binds without adding it must be flushed by its owner with @b3d::render::GpuParameterSet::Flush before **Execute()**. Development builds report an error when a stale set is bound on a thread that doesn't own it. Graphs that record serially need none of this, since sets bound on their owner thread flush as they are bound.

A set holds one layout per sampled texture, so passes that add the same set must sample its textures in the same layouts: either all of them render into a texture they sample, or none do.

On D3D12, a set can only be bound with a pipeline whose layout of the set has the same bindings as the layout the set was created with. Otherwise the bind is skipped and an error is reported, in all builds and not only for parallel graphs.

The renderer records serially. See **TestParallelRecording** in `B3DFrameGraphTestSuite.cpp` for an example.

# Debugging
Development builds validate the barriers of every graph that records serially, including the resource accesses the pass functions make. Errors are logged and name the resource and the rule it broke, so a pass that accesses something it didn't declare shows up there. Graphs that record in parallel validate nothing, so enable `gpu.FrameGraphSerialRecording` to validate them too.

The following config variables, which you can set with arguments such as `--gpu.ValidateRestingStates=true`, help narrow down a problem:
 - `gpu.ValidateRestingStates` - Checks accesses of resources that no barrier declared against their resting state, which catches a resource that was not returned to rest. Development builds only, off by default.
 - `gpu.PoisonUnloadedAttachments` - Clears every surface a render pass neither loads nor clears to a value that stands out, so passes that rely on contents they did not load become visible. Development builds only, on by default in debug builds.
 - `gpu.FrameGraphAsyncCompute` - Disable to run every pass on the graphics queue.
 - `gpu.FrameGraphSerialRecording` - Enable to record parallel graphs serially. Development builds only.
 - `gpu.TransientAliasing` - Disable so that no two transients of a graph share memory. If this makes a corruption go away, a pass likely reads a transient it did not declare, or relies on contents that were overwritten.
 - `gpu.TransientResources` - Disable to place transients in persistent memory.

On Vulkan the validation layers provide an independent check of the synchronization. Debug builds with the validation layers enable them, and setting the environment variable `VK_VALIDATION_VALIDATE_SYNC=true` adds synchronization validation, as CI does.

The name of each pass labels its commands in frame captures, and each pass gets a GPU profiling sample. Passes can nest their own samples within it through @b3d::render::FrameGraphPassContext::GetProfiler, as described in the [GPU profiling](../Low_Level_rendering/gpuProfiling) manual.
