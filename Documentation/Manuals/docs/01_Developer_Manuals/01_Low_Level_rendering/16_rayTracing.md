---
title: Ray tracing
---

Ray tracing lets GPU programs trace rays through a scene and find what they hit. The scene is stored in acceleration structures, which the GPU builds from your geometry. Programs then trace rays against them in one of two ways:
 - **Inline tracing** - A program of any type, such as a compute or fragment program, traces a ray and gets the hit back as a return value.
 - **Ray tracing pipelines** - A ray generation program starts rays, and the GPU runs other programs of the pipeline for what each ray hits or misses. A shader binding table picks which programs run.

This manual covers the C++ side. The shader side, including the BSL functions for inline tracing and the ray tracing entry points, is described in the [BSL](../../User_Manuals/Custom_Materials/bsl#ray-tracing) manual.

The *Framework/Examples/Source/RayTracing* project shows everything on this page working together. It traces one scene inline (`ExampleRayTracingInline.bsl`) and through a ray tracing pipeline (`ExampleRayTracingPipeline.bsl`), with both shaders in *Framework/Examples/Data/Shaders*.

# Capabilities
Ray tracing support is split into three device capabilities. Check them with @b3d::GpuDeviceCapabilities::HasCapability:
 - @b3d::RSC_ACCELERATION_STRUCTURES - The device creates and builds acceleration structures. Required by everything else on this page.
 - @b3d::RSC_RAY_QUERY - Programs can trace rays inline.
 - @b3d::RSC_RAY_TRACING_PIPELINE - The device runs ray tracing programs, pipelines and shader binding tables, and @b3d::render::GpuCommandBuffer::DispatchRays.

@b3d::GpuDeviceCapabilities::RayTracing holds the limits and layout requirements, as @b3d::GpuRayTracingCapabilities. Among others, it reports the size of an instance, the alignment of scratch memory, the maximum recursion depth, whether pipelines can contain callable programs (**SupportsCallablePrograms**), and whether top level structures can be read through the bindless heap (**SupportsBindlessAccelerationStructures**).

~~~~~~~~~~~~~{.cpp}
const GpuDeviceCapabilities& capabilities = gpuDevice->GetCapabilities();

const bool supportsInlineTracing = capabilities.HasCapability(RSC_ACCELERATION_STRUCTURES) && capabilities.HasCapability(RSC_RAY_QUERY);
const bool supportsPipelines = capabilities.HasCapability(RSC_ACCELERATION_STRUCTURES) && capabilities.HasCapability(RSC_RAY_TRACING_PIPELINE);
~~~~~~~~~~~~~

The `gpu.RayTracing` config variable turns all three off, as if the GPU had no ray tracing support. Meshes then create no ray tracing geometry, whatever their flags. It can only be set before the application starts: in `engine.ini`, on the command line (`--gpu.RayTracing=false`), or from code before start up.

# Acceleration structures
An acceleration structure is a @b3d::render::GpuAccelerationStructure. It comes in two levels, as @b3d::GpuAccelerationStructureType:
 - **BottomLevel** - Holds geometry: triangles, in vertex and index buffers.
 - **TopLevel** - Holds instances. Each instance places a bottom level structure in the scene with a transform. Rays are traced against top level structures.

A structure is created in three steps. Describe its contents with @b3d::GpuAccelerationStructureInputs, ask the device how much memory the structure and its build need with @b3d::GpuDevice::GetAccelerationStructureSizes, then create the structure with @b3d::GpuDevice::CreateGpuAccelerationStructure. The new structure has no contents until it is built on a command buffer.

## Bottom level structures
Each geometry of a bottom level structure is a @b3d::GpuRayTracingGeometry: a list of triangles in a vertex buffer, and optionally an index buffer. Positions must be @b3d::VET_FLOAT3. The buffers must be created with @b3d::GpuBufferFlag::AccelerationStructureInput, so builds can read them.

~~~~~~~~~~~~~{.cpp}
TShared<GpuBuffer> vertexBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(sizeof(Vector3), vertexCount,
	GpuBufferFlag::AccelerationStructureInput));
TShared<GpuBuffer> indexBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateIndex(IT_32BIT, indexCount,
	GpuBufferFlag::AccelerationStructureInput));

// ... write the vertices and indices ...

const GpuRayTracingGeometry geometry(vertexBuffer, sizeof(Vector3), vertexCount, indexBuffer, indexCount, IT_32BIT);
const GpuAccelerationStructureInputs bottomLevelInputs(TArrayView<const GpuRayTracingGeometry>(&geometry, 1),
	GpuAccelerationStructureBuildFlag::PreferFastTrace);

GpuAccelerationStructureSizes bottomLevelSizes;
gpuDevice->GetAccelerationStructureSizes(bottomLevelInputs, bottomLevelSizes);

TShared<GpuAccelerationStructure> bottomLevel = gpuDevice->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
	GpuAccelerationStructureType::BottomLevel, bottomLevelSizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "Box"));
~~~~~~~~~~~~~

Rays report the position of a geometry in the list as its geometry index. **VertexOffset** and **IndexOffset** select a range of a shared buffer.

Geometries are opaque by default (@b3d::GpuRayTracingGeometryFlag::Opaque), so hits on them are accepted without running any hit programs or callbacks. Clear the flag for geometry that needs an alpha test, or force it per instance as described below.

@b3d::GpuAccelerationStructureBuildFlag trades the build time against the speed of tracing and the memory use. **PreferFastTrace** suits geometry that rarely changes, and **PreferFastBuild** geometry that is rebuilt often.

## Instances and top level structures
A top level structure reads its instances from a structured storage buffer, in a layout the device defines. Describe each instance with a @b3d::GpuRayTracingInstance, and write them into the buffer with @b3d::GpuDevice::WriteRayTracingInstances. Each instance takes @b3d::GpuRayTracingCapabilities::InstanceSize bytes.

~~~~~~~~~~~~~{.cpp}
const GpuRayTracingCapabilities& rayTracing = gpuDevice->GetCapabilities().RayTracing;

GpuRayTracingInstance instances[2];
instances[0] = GpuRayTracingInstance(bottomLevel.get(), Matrix4::Translation(Vector3(-2.0f, 0.0f, 0.0f)), 0);
instances[1] = GpuRayTracingInstance(bottomLevel.get(), Matrix4::Translation(Vector3(2.0f, 0.0f, 0.0f)), 1);
instances[1].Flags = GpuRayTracingInstanceFlag::ForceNonOpaque;

TShared<GpuBuffer> instanceBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(rayTracing.InstanceSize, 2,
	GpuBufferFlag::AccelerationStructureInput, GpuBufferMemory::Mapped));

{
	const GpuBufferMappedScope mapping = instanceBuffer->Map(GpuMapOption::Write);
	gpuDevice->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(instances, 2), mapping.GetMappedMemory());
}

const GpuAccelerationStructureInputs topLevelInputs(instanceBuffer, 2);

GpuAccelerationStructureSizes topLevelSizes;
gpuDevice->GetAccelerationStructureSizes(topLevelInputs, topLevelSizes);

TShared<GpuAccelerationStructure> topLevel = gpuDevice->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
	GpuAccelerationStructureType::TopLevel, topLevelSizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "Scene"));
~~~~~~~~~~~~~

An instance has these members:
 - **Transform** - Affine transform from the space of the bottom level structure to the space of the scene.
 - **InstanceId** - A 24-bit value that shaders read as `InstanceID()`, or as `InstanceID` of a hit. Use it to index data of your own for the instance, such as its material.
 - **HitGroupIndexOffset** - Added to the index of the hit group record that runs for hits on the instance. See shader binding tables below.
 - **Mask** - Rays only hit the instance if this and their instance mask have a bit in common.
 - **Flags** - @b3d::GpuRayTracingInstanceFlag, which can disable triangle culling, flip the facing of triangles, or treat every geometry of the instance as opaque (**ForceOpaque**) or as not opaque (**ForceNonOpaque**).
 - **BottomLevel** - The bottom level structure the instance places.

A top level structure with zero instances is valid, and rays traced against it hit nothing.

## Building
@b3d::render::GpuCommandBuffer::BuildAccelerationStructures records one or more builds, each described by a @b3d::GpuAccelerationStructureBuild. Builds are recorded on a graphics or compute command buffer, outside of a render pass. A build overwrites the previous contents of its destination.

Each build needs scratch memory while it runs: a structured storage buffer created with @b3d::GpuBufferFlag::GpuWrite, with at least **BuildScratchSize** bytes from the size query after the build's offset. Builds recorded in one call must use separate ranges of scratch memory, at offsets aligned to @b3d::GpuRayTracingCapabilities::ScratchOffsetAlignment.

A top level build lists every bottom level structure its instances reference in @b3d::GpuAccelerationStructureBuild::BottomLevels. The top level structure keeps them alive until it is built again or destroyed.

Builds of one call run in any order and may overlap, so build bottom level structures and the top level structures that reference them in separate calls, with a barrier between them. Rebuilding a bottom level structure requires rebuilding every top level structure that references it.

Command buffers issue no barriers of their own (see the [barriers](../Low_Level_rendering/barriers) manual). A build accesses its destination, its scratch memory and the bottom level structures of a top level structure in @b3d::render::GpuStageFlag::AccelerationStructureBuild. It reads vertices, indices and instances in @b3d::render::GpuStageFlag::AccelerationStructureBuildInput, which is part of the resting state of buffers, so input buffers need no barriers.

~~~~~~~~~~~~~{.cpp}
const u32 scratchSize = std::max(bottomLevelSizes.BuildScratchSize, topLevelSizes.BuildScratchSize);
TShared<GpuBuffer> scratchBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, (scratchSize + 3) / 4,
	GpuBufferFlag::GpuWrite));

const GpuQueueType queueType = commandBuffer->GetQueueType();
const GpuAccessState buildWrite(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Write);
const GpuAccessState scratchAccess(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Read | GpuAccessFlag::Write);
const GpuAccessState structureRestingState = GpuBackendUtility::GetRestingState(*bottomLevel, queueType);
const GpuAccessState scratchRestingState = GpuBackendUtility::GetRestingState(*scratchBuffer, queueType);

// Build the bottom level structure
const GpuBufferBarrier scratchBarrier(scratchBuffer, scratchRestingState, scratchAccess);
const GpuAccelerationStructureBarrier bottomLevelBarrier(bottomLevel, structureRestingState, buildWrite);
commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchBarrier, 1), {}, {},
	TArrayView<const GpuAccelerationStructureBarrier>(&bottomLevelBarrier, 1)));

const GpuAccelerationStructureBuild bottomLevelBuild(bottomLevelInputs, bottomLevel, scratchBuffer);
commandBuffer->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&bottomLevelBuild, 1));

// The top level build reads the bottom level structure, and reuses the scratch memory
const GpuBufferBarrier scratchReuseBarrier(scratchBuffer, scratchAccess, scratchAccess);
const GpuAccelerationStructureBarrier topLevelBarriers[] =
{
	GpuAccelerationStructureBarrier(bottomLevel, buildWrite, structureRestingState),
	GpuAccelerationStructureBarrier(topLevel, structureRestingState, buildWrite)
};

commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchReuseBarrier, 1), {}, {},
	TArrayView<const GpuAccelerationStructureBarrier>(topLevelBarriers, 2)));

GpuAccelerationStructureBuild topLevelBuild(topLevelInputs, topLevel, scratchBuffer);
topLevelBuild.BottomLevels = TArrayView<const TShared<GpuAccelerationStructure>>(&bottomLevel, 1);
commandBuffer->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&topLevelBuild, 1));

// Return everything to rest
const GpuBufferBarrier scratchRestBarrier(scratchBuffer, scratchAccess, scratchRestingState);
const GpuAccelerationStructureBarrier topLevelRestBarrier(topLevel, buildWrite, structureRestingState);
commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchRestBarrier, 1), {}, {},
	TArrayView<const GpuAccelerationStructureBarrier>(&topLevelRestBarrier, 1)));
~~~~~~~~~~~~~

Most code builds structures in a [frame graph](../Renderer/frameGraph) instead, which plans these barriers itself. See the frame graph section below.

# Binding a structure to a program
Programs declare a top level structure as a `RaytracingAccelerationStructure` parameter. Bind it to a parameter set with @b3d::render::GpuParameterSet::SetAccelerationStructure, as you would bind a texture. Only top level structures can be bound.

~~~~~~~~~~~~~{.cpp}
parameterSet->SetAccelerationStructure("gScene", topLevel);
~~~~~~~~~~~~~

On devices with @b3d::GpuRayTracingCapabilities::SupportsBindlessAccelerationStructures, a top level structure created with @b3d::GpuAccelerationStructureFlag::Bindless can also be read through the bindless resource heap, at the index @b3d::render::GpuAccelerationStructure::GetBindlessIndex returns. See the [bindless resources](../Low_Level_rendering/bindlessResources) manual.

~~~~~~~~~~~~~{.hlsl}
RaytracingAccelerationStructure scene = ResourceDescriptorHeap[SceneIndex];
~~~~~~~~~~~~~

# Inline tracing
Inline tracing needs no special pipeline. A program that traces rays inline, for example a compute program that calls **RayTraceClosest** or **RayTraceAny**, runs in an ordinary compute or graphics pipeline, on devices with @b3d::RSC_RAY_QUERY. Its shader side, including callbacks for alpha testing, is described in the [BSL](../../User_Manuals/Custom_Materials/bsl#inline-ray-tracing) manual.

~~~~~~~~~~~~~{.hlsl}
RaytracingAccelerationStructure gScene;
RWTexture2D<float4> gOutput;

[numthreads(8, 8, 1)]
void csmain(uint3 threadId : SV_DispatchThreadID)
{
	RayDesc ray = GetCameraRay(threadId.xy);

	RayHit hit = RayTraceClosest(gScene, RAY_FLAG_NONE, 0xFF, ray);
	gOutput[threadId.xy] = hit.IsHit ? float4(1.0f, 1.0f, 1.0f, 1.0f) : float4(0.0f, 0.0f, 0.0f, 1.0f);
}
~~~~~~~~~~~~~

On the C++ side this is a normal compute dispatch (see the [compute](../Low_Level_rendering/compute) manual), with the structure bound in a parameter set:

~~~~~~~~~~~~~{.cpp}
GpuComputePipelineStateCreateInformation pipelineInformation;
pipelineInformation.Program = gpuDevice->CreateGpuProgram(pass->GetGpuProgramCreateInformation(GPT_COMPUTE_PROGRAM));
TShared<GpuComputePipelineState> pipeline = gpuDevice->CreateGpuComputePipelineState(pipelineInformation);

TShared<GpuParameterSet> parameterSet = gpuContext.GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
parameterSet->SetAccelerationStructure("gScene", topLevel);
parameterSet->SetStorageTexture("gOutput", outputTexture, TextureSurface::kComplete);

commandBuffer->SetGpuComputePipelineState(pipeline);
commandBuffer->SetGpuParameterSet(parameterSet);
commandBuffer->DispatchCompute((width + 7) / 8, (height + 7) / 8);
~~~~~~~~~~~~~

# Ray tracing pipelines
A @b3d::GpuRayTracingPipelineState holds the programs a ray tracing dispatch can run. Create it with @b3d::GpuDevice::CreateGpuRayTracingPipelineState, from a @b3d::GpuRayTracingPipelineStateCreateInformation:
 - **RayGenerationPrograms** - Programs that start the rays of a dispatch. At least one.
 - **MissPrograms** - Programs that run when a ray hits nothing.
 - **HitGroups** - Programs that run when a ray hits geometry, as @b3d::GpuRayTracingHitGroup.
 - **CallablePrograms** - Programs that other programs call with `CallShader()`. Require @b3d::GpuRayTracingCapabilities::SupportsCallablePrograms.
 - **MaximumRecursionDepth** - Largest number of nested `TraceRay()` calls. 1 if only ray generation programs trace rays.

Each program is one entry point. A BSL pass with ray tracing entry points compiles into one program per entry point, listed in @b3d::PassInformation::Programs. Find a program by its entry point name, and create it on the device:

~~~~~~~~~~~~~{.cpp}
TShared<GpuProgram> CreateProgram(GpuDevice& gpuDevice, const Pass& pass, const char* entryPoint)
{
	for(const GpuProgramCreateInformation& programInformation : pass.GetInformation().Programs)
	{
		if(programInformation.EntryPoint == entryPoint)
			return gpuDevice.CreateGpuProgram(programInformation);
	}

	return nullptr;
}
~~~~~~~~~~~~~

A hit group combines up to three programs, each of them optional, but at least one must be set:
 - **ClosestHitProgram** - Runs for the closest hit of a ray.
 - **AnyHitProgram** - Decides whether a candidate hit on geometry that isn't opaque is accepted. Without one, every candidate is accepted.
 - **IntersectionProgram** - Intersects a ray with procedural geometry. Null for triangles.

~~~~~~~~~~~~~{.cpp}
GpuRayTracingPipelineStateCreateInformation pipelineInformation;
pipelineInformation.RayGenerationPrograms.push_back(CreateProgram(*gpuDevice, *pass, "TraceScene"));
pipelineInformation.MissPrograms.push_back(CreateProgram(*gpuDevice, *pass, "MissPrimary"));
pipelineInformation.MissPrograms.push_back(CreateProgram(*gpuDevice, *pass, "MissShadow"));

// Hit group 0 shades opaque surfaces, 1 shades alpha tested surfaces, and 2 only alpha tests shadow rays
const TShared<GpuProgram> shadeHit = CreateProgram(*gpuDevice, *pass, "ShadeHit");
pipelineInformation.HitGroups.push_back(GpuRayTracingHitGroup(shadeHit));
pipelineInformation.HitGroups.push_back(GpuRayTracingHitGroup(shadeHit, CreateProgram(*gpuDevice, *pass, "AlphaTestPrimary")));
pipelineInformation.HitGroups.push_back(GpuRayTracingHitGroup(nullptr, CreateProgram(*gpuDevice, *pass, "AlphaTestShadow")));

TShared<GpuRayTracingPipelineState> pipeline = gpuDevice->CreateGpuRayTracingPipelineState(pipelineInformation);
~~~~~~~~~~~~~

A pipeline has one parameter interface, shared by all of its programs. Every program must come from sources that declare the same parameters (see @b3d::GpuProgram::GetParameterLayoutHash), so the programs of one BSL pass always work together, while programs of different shaders usually can't be mixed. @b3d::GpuRayTracingPipelineState::GetParameterLayout returns the layout to create parameter sets from.

# Shader binding tables
A @b3d::render::GpuShaderBindingTable picks which programs of a pipeline run for which rays. Create it with @b3d::GpuDevice::CreateGpuShaderBindingTable from a @b3d::GpuShaderBindingTableCreateInformation, which holds the pipeline and four lists of records. Each record is an index into the matching list of the pipeline:
 - **RayGenerationRecords** - @b3d::render::GpuCommandBuffer::DispatchRays picks one of them.
 - **MissRecords** - The `MissShaderIndex` argument of `TraceRay()` picks one of them.
 - **HitGroupRecords** - A hit on geometry runs the record at the instance's **HitGroupIndexOffset**, plus the `RayContributionToHitGroupIndex` argument of `TraceRay()`, plus the geometry index times the `MultiplierForGeometryContributionToHitGroupIndex` argument of `TraceRay()`. A record can be @b3d::kGpuShaderBindingTableEmptyRecord, in which case no program runs for the hit.
 - **CallableRecords** - The index `CallShader()` is called with picks one of them.

Records only refer to programs. They carry no data of their own, so put per-instance data in a structured buffer, or in the bindless heap, and index it with the instance id.

This table belongs to a pipeline that traces two types of rays: primary rays pass 0 as their contribution and miss index, and shadow rays pass 1. Opaque instances have a hit group offset of 0, and alpha tested instances an offset of 2.

~~~~~~~~~~~~~{.cpp}
GpuShaderBindingTableCreateInformation tableInformation;
tableInformation.Pipeline = pipeline;
tableInformation.RayGenerationRecords = { 0 };
tableInformation.MissRecords = { 0, 1 };

// Opaque primary, opaque shadow, alpha tested primary, alpha tested shadow. Shadow rays on opaque instances run no program.
tableInformation.HitGroupRecords = { 0, kGpuShaderBindingTableEmptyRecord, 1, 2 };

TShared<GpuShaderBindingTable> table = gpuDevice->CreateGpuShaderBindingTable(tableInformation);
~~~~~~~~~~~~~

## Dispatching rays
@b3d::render::GpuCommandBuffer::DispatchRays runs a ray generation record of a table once for every point of a grid, usually once per pixel. It binds the table's pipeline, and uses the parameter sets and push constants set on the command buffer, as a compute dispatch does. Record it on a graphics or compute command buffer, outside of a render pass.

~~~~~~~~~~~~~{.cpp}
TShared<GpuParameterSet> parameterSet = gpuContext.GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
parameterSet->SetAccelerationStructure("gScene", topLevel);
parameterSet->SetStorageTexture("gOutput", outputTexture, TextureSurface::kComplete);

commandBuffer->SetGpuParameterSet(parameterSet);
commandBuffer->DispatchRays(table, 0, width, height);
~~~~~~~~~~~~~

# Frame graphs
In a [frame graph](../Renderer/frameGraph), builds go in a @b3d::render::FrameGraphAccelerationStructurePass, added with @b3d::render::FrameGraph::AddAccelerationStructurePass. Acceleration structures can only be imported into a graph, with @b3d::render::FrameGraph::ImportAccelerationStructure. The pattern is:
 1. Import the structures the graph builds.
 2. Create the scratch buffer as a transient of the graph.
 3. Declare each build with @b3d::render::FrameGraphAccelerationStructurePass::AddBuild, and record it in the pass function.
 4. Export the built structures from the pass with @b3d::render::FrameGraphPass::Export.

**AddBuild()** declares what the build accesses. The destination is written and its contents are discarded, the scratch buffer is read and written, and the bottom level structures and the vertex, index and instance buffers are read. The destination and the scratch buffer must be resources of the graph. The objects it only reads are declared if they are resources of the graph, and read at rest otherwise.

~~~~~~~~~~~~~{.cpp}
graph.ImportAccelerationStructure(topLevel);

const TShared<GpuBuffer> scratchBuffer = graph.CreateBuffer("Scene scratch",
	GpuBufferCreateInformation::CreateStructuredStorage(4, (topLevelSizes.BuildScratchSize + 3) / 4, GpuBufferFlag::GpuWrite));

GpuAccelerationStructureBuild build(topLevelInputs, topLevel, scratchBuffer);
build.BottomLevels = TArrayView<const TShared<GpuAccelerationStructure>>(mBottomLevels.data(), (u32)mBottomLevels.size());

FrameGraphAccelerationStructurePass pass;
pass.AddBuild(build);
pass.Export(topLevel);

graph.AddAccelerationStructurePass("Build scene", std::move(pass), [build](FrameGraphPassContext& context)
{
	context.GetCommandBuffer().BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&build, 1));
});
~~~~~~~~~~~~~

The pass function records the build during **Execute()**, so everything the build's views refer to, such as the geometries of a bottom level build and the list of bottom level structures above, must stay alive until the graph executes.

Exporting the structure returns it to rest right after the pass, so later passes, and work after the graph, can trace rays against it. Later passes that bind it in a parameter set added to the pass declare the read automatically, so a structure that is not exported is still read correctly by the graph's own passes.

Tracing goes in a @b3d::render::FrameGraphComputePass, both for inline tracing and for **DispatchRays()**:

~~~~~~~~~~~~~{.cpp}
FrameGraphComputePass tracePass;
tracePass.AddParameterSet(parameterSet);

graph.AddComputePass("Trace", std::move(tracePass), [parameterSet, table, width, height](FrameGraphPassContext& context)
{
	GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();
	commandBuffer.SetGpuParameterSet(parameterSet);
	commandBuffer.DispatchRays(table, 0, width, height);
});
~~~~~~~~~~~~~

# Meshes and scenes
Most of the time you don't need to describe geometry and instances yourself. Two helpers build acceleration structures from meshes, and add the passes that build them to a frame graph.

## Mesh geometry
A mesh created with @b3d::MeshFlag::SupportsRayTracing, which is part of the default flags, can be used as ray tracing geometry. The flag only takes effect if the mesh isn't **MeshFlag::Dynamic**, its vertices and indices are in the formats listed below, and the device has @b3d::RSC_ACCELERATION_STRUCTURES. Its vertex and index buffers are then created with @b3d::GpuBufferFlag::AccelerationStructureInput, and also with @b3d::GpuBufferFlag::Bindless if the device has bindless heaps. @b3d::render::Mesh::SupportsRayTracing reports whether this happened.

@b3d::render::Mesh::GetOrCreateRayTracingGeometry returns the mesh's @b3d::render::RayTracingGeometry, creating it on first use. It is a bottom level structure built from the mesh's own vertex and index buffers, so only the structure itself takes new memory. The call returns null if the mesh doesn't support ray tracing, its vertices or indices aren't in the formats below, or it has no triangle list sub-mesh.

Ray tracing shaders read mesh data in fixed formats, so the mesh must provide them:
 - Positions: **VET_FLOAT3**. Required.
 - Normals: **VET_UBYTE4_NORM**, each component stored as `N * 0.5 + 0.5`. Optional.
 - Tangents: **VET_UBYTE4_NORM**, stored the same way, with the handedness of the bitangent in W. Optional.
 - First texture coordinates: **VET_FLOAT2**. Optional.
 - Indices: **IT_32BIT**. Required.

@b3d::render::RayTracingGeometry::kPositionType and the constants next to it list these formats. They are the formats @b3d::RendererMeshData produces, so imported meshes have them, and @b3d::RendererMeshData::Convert converts other mesh data to them (except 16-bit indices). @b3d::render::RayTracingGeometry::IsSupportedFormat checks a vertex description and index type against them. A mesh created with **SupportsRayTracing** whose data is in other formats logs a warning and doesn't support ray tracing, so create meshes that rays never hit without the flag.

The geometry has one geometry per triangle list sub-mesh:
 - @b3d::render::RayTracingGeometry::GetSubMeshGeometryIndices maps each sub-mesh to the geometry index rays report for it, or `~0u` for sub-meshes that aren't triangle lists.
 - @b3d::render::RayTracingGeometry::GetGeometryLayouts returns a @b3d::render::RayTracingGeometryLayout per geometry, in geometry index order. It tells shaders where the geometry's indices are, and where its positions, normals, tangents and first texture coordinates are, as bindless buffer indices with offsets and strides.
 - Writing new data to the mesh marks the geometry as not built (@b3d::render::RayTracingGeometry::IsBuilt).
 - @b3d::render::RayTracingGeometry::Build adds a pass to a frame graph that builds the structure, and exports it.

The geometry is built opaque. Instances that need any hit programs or **AnyHit** callbacks, such as alpha tested ones, must set **GpuRayTracingInstanceFlag::ForceNonOpaque**.

## Scenes
@b3d::render::RayTracingScene manages a top level structure of mesh geometry instances. Each instance is a @b3d::render::RayTracingGeometryInstance, with the same members as **GpuRayTracingInstance**, except that it refers to a **RayTracingGeometry** instead of a bottom level structure.
 - @b3d::render::RayTracingScene::AddInstance adds an instance and returns a slot that identifies it.
 - @b3d::render::RayTracingScene::UpdateInstance, @b3d::render::RayTracingScene::SetInstanceTransform and @b3d::render::RayTracingScene::RemoveInstance change or remove the instance in a slot.
 - @b3d::render::RayTracingScene::GetInstanceCount returns the number of instances.
 - @b3d::render::RayTracingScene::Build adds the passes that bring the structures up to date, as described below.
 - @b3d::render::RayTracingScene::GetAccelerationStructure returns the top level structure.

**Build()** adds up to two passes to the graph. The first builds the bottom level structures of the geometries that aren't built, all in one pass. The second rebuilds the top level structure if anything changed since the last build, and exports it. In a frame where nothing changed it adds no passes.

~~~~~~~~~~~~~{.cpp}
// Once
TShared<RayTracingScene> scene = B3DMakeShared<RayTracingScene>(*gpuDevice, "Scene");

RayTracingGeometryInstance instance(mesh->GetOrCreateRayTracingGeometry(), transform, objectIndex);
const u32 slot = scene->AddInstance(instance);

// Every frame
scene->SetInstanceTransform(slot, newTransform);
scene->Build(graph);

parameterSet->SetAccelerationStructure("gScene", scene->GetAccelerationStructure());
~~~~~~~~~~~~~

The rules:
 - Read the top level structure after **Build()**. The scene replaces it with a larger one when it outgrows it.
 - The scene and the geometries of its instances must stay alive until the graph executes, and the scene must not be built again before then.
 - The top level structure is bindless on devices with @b3d::GpuRayTracingCapabilities::SupportsBindlessAccelerationStructures.

## Reading mesh data in shaders
Hit shaders and inline tracing usually need more than the hit position, such as the normal and texture coordinates of the surface. The `RayTracingGeometryAccess` mixin reads them through the bindless heap, using the layouts of the geometries:
 - `RayTracingGeometryLayout` and `RayTracingVertexStream` structs, which match **RayTracingGeometryLayout** and **RayTracingVertexStream** in C++.
 - `uint3 LoadTriangleIndices(RayTracingGeometryLayout layout, uint primitiveIndex)` returns the vertex indices of a triangle, for the primitive index a ray reports.
 - `float3 InterpolatePosition(RayTracingGeometryLayout layout, uint3 indices, float2 barycentrics)` interpolates the object space position at the hit's barycentrics.
 - `float3 InterpolateNormal(...)` interpolates the object space normal, decoded to [-1, 1] but not normalized. Same parameters.
 - `float4 InterpolateTangent(...)` interpolates the object space tangent, decoded the same way, with the handedness in W. Same parameters.
 - `float2 InterpolateUv(...)` interpolates the first texture coordinates. Same parameters.

Normals, tangents and texture coordinates are optional. For a mesh without one, the stream points at a default value that every vertex shares, with a stride of 0, so shaders read it without checking: normal (0, 0, 1), tangent (1, 0, 0) with handedness 1, and texture coordinates (0, 0). These values live in one 16-byte buffer per device, which @b3d::render::RayTracingManager owns and every geometry shares.

Put the layouts of every geometry in a structured buffer, and give each instance the index of its first layout, through data indexed by its instance id:

~~~~~~~~~~~~~{.cpp}
Vector<RayTracingGeometryLayout> layouts;
for(const RayTracingGeometryLayout& layout : mesh->GetOrCreateRayTracingGeometry()->GetGeometryLayouts())
	layouts.push_back(layout);

TShared<GpuBuffer> layoutBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(RayTracingGeometryLayout),
	(u32)layouts.size(), GpuBufferFlag::None, GpuBufferMemory::Mapped));
~~~~~~~~~~~~~

~~~~~~~~~~~~~{.hlsl}
#include "$ENGINE$\RayTracingGeometryAccess.bslinc"

shader MyRayTracingShader
{
	mixin RayTracingGeometryAccess;

	code
	{
		struct ObjectData
		{
			uint FirstLayout;
		};

		StructuredBuffer<ObjectData> gObjects;
		StructuredBuffer<RayTracingGeometryLayout> gLayouts;

		// ... other parameters and entry points ...

		[shader("closesthit")]
		void ShadeHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
		{
			RayTracingGeometryLayout layout = gLayouts[gObjects[InstanceID()].FirstLayout + GeometryIndex()];
			uint3 indices = LoadTriangleIndices(layout, PrimitiveIndex());

			float3 objectNormal = InterpolateNormal(layout, indices, attributes.barycentrics);
			payload.Normal = normalize(mul(ObjectToWorld3x4(), float4(objectNormal, 0.0f)));
		}
	};
};
~~~~~~~~~~~~~

The mixin reads the heap with `NonUniformResourceIndex`, because the buffers can differ between threads. It requires a device with bindless heaps. Without them the buffer indices in the layouts are **kGpuBindlessInvalidIndex**.
