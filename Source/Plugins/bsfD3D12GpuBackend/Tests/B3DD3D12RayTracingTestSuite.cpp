//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12RayTracingTestSuite.h"
#include "B3DD3D12BarrierUtility.h"
#include "B3DD3D12GpuAccelerationStructure.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuPipelineState.h"
#include "B3DD3D12GpuShaderBindingTable.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuShaderBindingTable.h"
#include "Material/B3DPass.h"
#include "Material/B3DShader.h"
#include "Material/B3DShaderRegistry.h"
#include "Material/B3DVariation.h"
#include "Resources/B3DBuiltinResources.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DVector3.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Returns the D3D12 device that tests run on, or null if there is none or it doesn't support acceleration structures. */
	D3D12GpuDevice* GetAccelerationStructureDevice()
	{
		GpuBackend& backend = GpuBackend::Instance();
		if(String(backend.GetBackendName()) != "bsfD3D12GpuBackend" || backend.GetDeviceCount() == 0)
			return nullptr;

		GpuDevice* const device = backend.GetDevice(0).get();
		if(!device->GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES))
			return nullptr;

		return static_cast<D3D12GpuDevice*>(device);
	}

	/** Returns the address of the native resource of @p buffer. */
	D3D12_GPU_VIRTUAL_ADDRESS GetAddress(const TShared<render::GpuBuffer>& buffer)
	{
		return static_cast<D3D12GpuBuffer&>(*buffer).GetD3D12Buffer()->GetGPUVirtualAddress();
	}

	/** Returns the native structure of @p accelerationStructure. */
	D3D12AccelerationStructure& GetNativeStructure(const GpuAccelerationStructure& accelerationStructure)
	{
		return *static_cast<const D3D12GpuAccelerationStructure&>(accelerationStructure).GetD3D12Resource();
	}
} // namespace

D3D12RayTracingTestSuite::D3D12RayTracingTestSuite()
	: TestSuite("D3D12RayTracingTestSuite")
{
	B3D_ADD_TEST(D3D12RayTracingTestSuite::TestAddresses)
	B3D_ADD_TEST(D3D12RayTracingTestSuite::TestInstanceLayout)
	B3D_ADD_TEST(D3D12RayTracingTestSuite::TestBarriers)
	B3D_ADD_TEST(D3D12RayTracingTestSuite::TestShaderBindingTable)
}

void D3D12RayTracingTestSuite::TestAddresses()
{
	D3D12GpuDevice* const device = GetAccelerationStructureDevice();
	if(device == nullptr)
		return;

	GetRenderThread().PostCommand([this, device]()
	{
		const u32 scratchAlignment = device->GetCapabilities().RayTracing.ScratchOffsetAlignment;
		static constexpr u32 kInstanceAlignment = 16;
		static constexpr u32 kAccelerationStructureAlignment = 256;

		// Odd sizes, so resources that share heaps don't start aligned by accident
		Vector<TShared<render::GpuBuffer>> buffers;
		Vector<TShared<GpuAccelerationStructure>> structures;
		for(u32 bufferIndex = 0; bufferIndex < 4; bufferIndex++)
		{
			const u32 elementCount = 3 + bufferIndex * 7;
			const TShared<render::GpuBuffer> scratch = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, elementCount,
				GpuBufferFlag::GpuWrite));
			const TShared<render::GpuBuffer> instances = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, elementCount,
				GpuBufferFlag::AccelerationStructureInput, GpuBufferMemory::Mapped));
			const TShared<GpuAccelerationStructure> structure = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
				GpuAccelerationStructureType::BottomLevel, elementCount * 256));
			B3D_TEST_ASSERT(scratch != nullptr && instances != nullptr && structure != nullptr)
			if(scratch == nullptr || instances == nullptr || structure == nullptr)
				return;

			B3D_TEST_ASSERT(GetAddress(scratch) != 0 && GetAddress(scratch) % scratchAlignment == 0)
			B3D_TEST_ASSERT(GetAddress(instances) != 0 && GetAddress(instances) % kInstanceAlignment == 0)

			// A structure is a buffer of its own, created for structures, which shaders read through a view
			const D3D12AccelerationStructure& nativeStructure = GetNativeStructure(*structure);
			B3D_TEST_ASSERT(nativeStructure.GetGPUVirtualAddress() != 0 && nativeStructure.GetGPUVirtualAddress() % kAccelerationStructureAlignment == 0)
			B3D_TEST_ASSERT(nativeStructure.GetGPUVirtualAddress() == nativeStructure.GetBuffer().GetGPUVirtualAddress())
			B3D_TEST_ASSERT(nativeStructure.GetSRVHandle().ptr != 0)
			B3D_TEST_ASSERT(structure->GetGpuResource() == &nativeStructure)

			const D3D12_RESOURCE_DESC description = nativeStructure.GetBuffer().GetD3D12Resource()->GetDesc();
			B3D_TEST_ASSERT(description.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER && description.Width >= elementCount * 256)
			B3D_TEST_ASSERT((description.Flags & D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE) != 0)
			B3D_TEST_ASSERT((description.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0)

			buffers.push_back(scratch);
			buffers.push_back(instances);
			structures.push_back(structure);
		}
	}, "D3D12RayTracingTestSuite::TestAddresses", true);
}

void D3D12RayTracingTestSuite::TestInstanceLayout()
{
	D3D12GpuDevice* const device = GetAccelerationStructureDevice();
	if(device == nullptr)
		return;

	GetRenderThread().PostCommand([this, device]()
	{
		B3D_TEST_ASSERT(device->GetCapabilities().RayTracing.InstanceSize == sizeof(D3D12_RAYTRACING_INSTANCE_DESC))

		const TShared<GpuAccelerationStructure> bottomLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::BottomLevel, 4096));
		B3D_TEST_ASSERT(bottomLevel != nullptr)
		if(bottomLevel == nullptr)
			return;

		Matrix4 transform = Matrix4::Translation(Vector3(1.0f, 2.0f, 3.0f));
		transform[0][1] = 4.0f;
		transform[2][0] = 5.0f;

		GpuRayTracingInstance instances[2] = { GpuRayTracingInstance(bottomLevel.get(), transform, 0x123456), GpuRayTracingInstance(bottomLevel.get(), Matrix4::kIdentity) };
		instances[0].HitGroupIndexOffset = 0x654321;
		instances[0].Mask = 0x5A;
		instances[0].Flags = GpuRayTracingInstanceFlag::DisableTriangleCulling | GpuRayTracingInstanceFlag::ForceOpaque;
		instances[1].Flags = GpuRayTracingInstanceFlag::FlipTriangleFacing | GpuRayTracingInstanceFlag::ForceNonOpaque;

		D3D12_RAYTRACING_INSTANCE_DESC written[2];
		B3D_TEST_ASSERT(device->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(instances, 2), written))

		// Rows of the transform, without the last one
		for(u32 row = 0; row < 3; row++)
		{
			for(u32 column = 0; column < 4; column++)
				B3D_TEST_ASSERT(written[0].Transform[row][column] == transform[row][column])
		}

		B3D_TEST_ASSERT(written[0].InstanceID == 0x123456 && written[0].InstanceMask == 0x5A && written[0].InstanceContributionToHitGroupIndex == 0x654321)

		// Front faces are counterclockwise for instances that keep the engine's facing
		B3D_TEST_ASSERT(written[0].Flags == (D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE | D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE |
			D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE))
		B3D_TEST_ASSERT(written[0].AccelerationStructure == GetNativeStructure(*bottomLevel).GetGPUVirtualAddress())

		B3D_TEST_ASSERT(written[1].InstanceID == 0 && written[1].InstanceMask == 0xFF && written[1].InstanceContributionToHitGroupIndex == 0)
		B3D_TEST_ASSERT(written[1].Flags == D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE)
		B3D_TEST_ASSERT(written[1].Transform[0][0] == 1.0f && written[1].Transform[0][3] == 0.0f && written[1].Transform[2][2] == 1.0f)
	}, "D3D12RayTracingTestSuite::TestInstanceLayout", true);
}

void D3D12RayTracingTestSuite::TestBarriers()
{
	D3D12GpuDevice* const device = GetAccelerationStructureDevice();
	if(device == nullptr)
		return;

	ComPtr<ID3D12InfoQueue> infoQueue;
	if(FAILED(device->GetD3D12Device()->QueryInterface(IID_PPV_ARGS(&infoQueue))))
		return;

	GetRenderThread().PostCommand([this, device, &infoQueue]()
	{
		ID3D12Device* const nativeDevice = device->GetD3D12Device();

		ComPtr<ID3D12CommandAllocator> allocator;
		ComPtr<ID3D12GraphicsCommandList> baseCommandList;
		ComPtr<ID3D12GraphicsCommandList7> commandList;
		B3D_TEST_ASSERT(SUCCEEDED(nativeDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
		B3D_TEST_ASSERT(SUCCEEDED(nativeDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&baseCommandList))))
		B3D_TEST_ASSERT(SUCCEEDED(baseCommandList.As(&commandList)))
		if(commandList == nullptr)
			return;

		const TShared<GpuAccelerationStructure> structure = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::BottomLevel, 4096));
		const TShared<render::GpuBuffer> scratch = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, 1024, GpuBufferFlag::GpuWrite));
		B3D_TEST_ASSERT(structure != nullptr && scratch != nullptr)
		if(structure == nullptr || scratch == nullptr)
			return;

		ID3D12Resource* const structureResource = GetNativeStructure(*structure).GetBuffer().GetD3D12Resource();
		ID3D12Resource* const scratchResource = static_cast<D3D12GpuBuffer&>(*scratch).GetD3D12Buffer()->GetD3D12Resource();

		// Returns the messages the debug layer stored since @p firstMessage
		const auto fnGetMessages = [&infoQueue](u64 firstMessage)
		{
			String messages;
			const u64 messageCount = infoQueue->GetNumStoredMessages();
			for(u64 messageIndex = firstMessage; messageIndex < messageCount; messageIndex++)
			{
				SIZE_T messageLength = 0;
				if(FAILED(infoQueue->GetMessage(messageIndex, nullptr, &messageLength)) || messageLength == 0)
					continue;

				Vector<u8> storage(messageLength);
				D3D12_MESSAGE* const message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
				if(SUCCEEDED(infoQueue->GetMessage(messageIndex, message, &messageLength)) && message->Severity <= D3D12_MESSAGE_SEVERITY_WARNING)
					messages += String(message->pDescription) + "\n";
			}

			return messages;
		};

		const auto fnRecord = [&commandList](const D3D12_BUFFER_BARRIER& barrier)
		{
			D3D12_BARRIER_GROUP group;
			group.Type = D3D12_BARRIER_TYPE_BUFFER;
			group.NumBarriers = 1;
			group.pBufferBarriers = &barrier;
			commandList->Barrier(1, &group);
		};

		// Barriers the engine records around builds and traces, mapped as RecordBarriers() maps them
		const GpuStageFlags supportedStages = device->GetSupportedStageFlags();
		const auto fnGetScope = [supportedStages](const GpuAccessState& source, const GpuAccessState& destination)
		{
			return GpuBarrierScope(D3D12BarrierUtility::RemoveUnsupportedStages(source.Stages, supportedStages), source.Access,
				D3D12BarrierUtility::RemoveUnsupportedStages(destination.Stages, supportedStages), destination.Access);
		};

		const GpuAccessState build(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Write);
		const GpuAccessState buildRead(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Read);
		const GpuAccessState buildReadWrite(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Read | GpuAccessFlag::Write);
		const GpuAccessState structureResting = GpuBackendUtility::GetRestingState(*structure, GQT_GRAPHICS);
		const GpuAccessState scratchResting = GpuBackendUtility::GetRestingState(*scratch, GQT_GRAPHICS);

		struct EngineBarrier
		{
			const char* Name;
			bool IsAccelerationStructure;
			GpuAccessState Source;
			GpuAccessState Destination;
		};

		const EngineBarrier engineBarriers[] =
		{
			{ "build -> compute trace", true, build, GpuAccessState(GpuStageFlag::ComputeShaderAccelerationStructure, GpuAccessFlag::Read) },
			{ "build -> ray tracing trace", true, build, GpuAccessState(GpuStageFlag::RayTracingShaderAccelerationStructure, GpuAccessFlag::Read) },
			{ "build -> vertex and fragment trace", true, build, GpuAccessState(GpuStageFlag::VertexShaderAccelerationStructure |
				GpuStageFlag::FragmentShaderAccelerationStructure, GpuAccessFlag::Read) },
			{ "bottom level build -> top level build", true, build, buildRead },
			{ "build -> resting", true, build, structureResting },
			{ "resting -> build", true, structureResting, build },
			{ "compute trace -> build", true, GpuAccessState(GpuStageFlag::ComputeShaderAccelerationStructure, GpuAccessFlag::Read), build },
			{ "scratch build -> build", false, buildReadWrite, buildReadWrite },
			{ "scratch resting -> build", false, scratchResting, buildReadWrite },
			{ "scratch build -> resting", false, buildReadWrite, scratchResting },
		};

		for(const EngineBarrier& engineBarrier : engineBarriers)
		{
			// Stages the device doesn't support, like ray tracing shaders without ray tracing pipelines, have no barriers
			const GpuBarrierScope scope = fnGetScope(engineBarrier.Source, engineBarrier.Destination);
			if(!scope.IsValid())
				continue;
			const D3D12_BUFFER_BARRIER barrier = engineBarrier.IsAccelerationStructure
				? D3D12BarrierUtility::GetAccelerationStructureBarrier(structureResource, scope)
				: D3D12BarrierUtility::GetBufferBarrier(scratchResource, scope);

			const u64 firstMessage = infoQueue->GetNumStoredMessages();
			fnRecord(barrier);

			const String messages = fnGetMessages(firstMessage);
			B3D_TEST_ASSERT_MSG(messages.empty(), StringUtility::Format("'{0}' (sync {1} -> {2}, access {3} -> {4}): {5}", engineBarrier.Name,
				(u32)barrier.SyncBefore, (u32)barrier.SyncAfter, (u32)barrier.AccessBefore, (u32)barrier.AccessAfter, messages))
		}

		// Structures are only accessed as structures, and scratch memory as unordered access
		const D3D12_BUFFER_BARRIER traceBarrier = D3D12BarrierUtility::GetAccelerationStructureBarrier(structureResource, fnGetScope(build,
			GpuAccessState(GpuStageFlag::ComputeShaderAccelerationStructure, GpuAccessFlag::Read)));
		B3D_TEST_ASSERT(traceBarrier.AccessBefore == D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_WRITE)
		B3D_TEST_ASSERT(traceBarrier.AccessAfter == D3D12_BARRIER_ACCESS_RAYTRACING_ACCELERATION_STRUCTURE_READ)
		B3D_TEST_ASSERT(traceBarrier.SyncBefore == D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE && traceBarrier.SyncAfter == D3D12_BARRIER_SYNC_COMPUTE_SHADING)

		const D3D12_BUFFER_BARRIER scratchBarrier = D3D12BarrierUtility::GetBufferBarrier(scratchResource, fnGetScope(buildReadWrite, buildReadWrite));
		B3D_TEST_ASSERT(scratchBarrier.AccessBefore == D3D12_BARRIER_ACCESS_UNORDERED_ACCESS && scratchBarrier.AccessAfter == D3D12_BARRIER_ACCESS_UNORDERED_ACCESS)
		B3D_TEST_ASSERT(scratchBarrier.SyncBefore == D3D12_BARRIER_SYNC_BUILD_RAYTRACING_ACCELERATION_STRUCTURE)

		commandList->Close();
	}, "D3D12RayTracingTestSuite::TestBarriers", true);
}

void D3D12RayTracingTestSuite::TestShaderBindingTable()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	if(!device->GetCapabilities().HasCapability(RSC_RAY_TRACING_PIPELINE))
		return;

	// The programs of the RayTracingPipeline test shader, by entry point
	const TShared<Shader> shader = ShaderRegistry::Instance().GetOrCompileShader<false>(Path("Tests/RayTracingPipeline.bsl"), BuiltinResources::kBuiltinShaderCachePrefix,
		ShaderDefines());
	B3D_TEST_ASSERT(shader != nullptr)
	if(shader == nullptr)
		return;

	const Vector<TShared<Variation>> variations = shader->GetCompatibleVariations();
	B3D_TEST_ASSERT(variations.size() == 1)
	if(variations.size() != 1)
		return;

	if(!variations[0]->IsCompiled())
		variations[0]->Compile().BlockUntilComplete();

	B3D_TEST_ASSERT(variations[0]->IsCompiled() && variations[0]->GetPassCount() == 1)
	if(!variations[0]->IsCompiled() || variations[0]->GetPassCount() != 1)
		return;

	UnorderedMap<String, TShared<GpuProgram>> programs;
	for(const GpuProgramCreateInformation& programInformation : variations[0]->GetPass(0)->GetInformation().Programs)
		programs[programInformation.EntryPoint] = device->CreateGpuProgram(programInformation);

	GpuRayTracingPipelineStateCreateInformation pipelineInformation;
	pipelineInformation.RayGenerationPrograms.push_back(programs["TraceTestRays"]);
	pipelineInformation.RayGenerationPrograms.push_back(programs["WriteIndices"]);
	pipelineInformation.MissPrograms.push_back(programs["MissA"]);
	pipelineInformation.HitGroups.push_back(GpuRayTracingHitGroup(programs["ClosestHitA"]));
	pipelineInformation.HitGroups.push_back(GpuRayTracingHitGroup(programs["ClosestHitB"]));
	pipelineInformation.HitGroups.push_back(GpuRayTracingHitGroup(programs["ClosestHitC"]));

	const TShared<GpuRayTracingPipelineState> pipeline = device->CreateGpuRayTracingPipelineState(pipelineInformation);
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	// Three hit group records, the middle one empty, and no callable records
	GpuShaderBindingTableCreateInformation tableInformation;
	tableInformation.Pipeline = pipeline;
	tableInformation.RayGenerationRecords.push_back(0);
	tableInformation.RayGenerationRecords.push_back(1);
	tableInformation.MissRecords.push_back(0);
	tableInformation.HitGroupRecords.push_back(0);
	tableInformation.HitGroupRecords.push_back(kGpuShaderBindingTableEmptyRecord);
	tableInformation.HitGroupRecords.push_back(2);

	TShared<GpuShaderBindingTable> table;
	{
		// The first table of the process warns when the CPU can't write the GPU's memory.
		LoggingScope logs(*this);
		logs.IgnoreWarning("Shader binding tables are in system memory");
		table = device->CreateGpuShaderBindingTable(tableInformation);
	}

	B3D_TEST_ASSERT(table != nullptr)
	if(table == nullptr)
		return;

	const D3D12GpuShaderBindingTable& d3d12Table = static_cast<const D3D12GpuShaderBindingTable&>(*table);
	const D3D12GpuRayTracingPipelineState& d3d12Pipeline = static_cast<const D3D12GpuRayTracingPipelineState&>(*pipeline);
	B3D_TEST_ASSERT(d3d12Table.GetBuffer() != nullptr)
	if(d3d12Table.GetBuffer() == nullptr)
		return;

	// Every region starts at a multiple of 64 bytes. Records are 32 bytes apart, and ray generation records 64, because a dispatch passes
	// one of them as a table of its own.
	const D3D12_GPU_VIRTUAL_ADDRESS tableAddress = d3d12Table.GetBuffer()->GetGPUVirtualAddress();
	const auto fnCheckRegion = [this, tableAddress](const GpuShaderBindingTableRegion& region, u32 recordCount, u32 stride, const char* name)
	{
		const String message = StringUtility::Format("{0} region: offset {1}, stride {2}, size {3}.", name, region.Offset, region.Stride, region.Size);
		B3D_TEST_ASSERT_MSG((tableAddress + region.Offset) % D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT == 0, message)
		B3D_TEST_ASSERT_MSG(region.Stride == stride && region.Size == stride * recordCount, message)
	};

	fnCheckRegion(d3d12Table.GetRayGenerationRegion(), 2, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, "Ray generation");
	fnCheckRegion(d3d12Table.GetMissRegion(), 1, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, "Miss");
	fnCheckRegion(d3d12Table.GetHitGroupRegion(), 3, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, "Hit group");

	const GpuShaderBindingTableRegion& callableRegion = d3d12Table.GetCallableRegion();
	B3D_TEST_ASSERT(callableRegion.Offset == 0 && callableRegion.Stride == 0 && callableRegion.Size == 0)

	// Records hold the identifiers of their groups, numbered ray generation, miss, then hit groups. The empty record is zeros. The table
	// stays mapped.
	const u8* const tableData = static_cast<const u8*>(d3d12Table.GetBuffer()->GetMappedData());
	B3D_TEST_ASSERT(tableData != nullptr)
	if(tableData == nullptr)
		return;

	const auto fnGetRecord = [tableData](const GpuShaderBindingTableRegion& region, u32 recordIndex)
	{
		return tableData + region.Offset + recordIndex * region.Stride;
	};

	const auto fnGetIdentifier = [&d3d12Pipeline](u32 groupIndex)
	{
		const TArrayView<const u8> identifier = d3d12Pipeline.GetShaderGroupHandle(groupIndex);
		return identifier.size() == D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES ? identifier.data() : nullptr;
	};

	for(u32 groupIndex = 0; groupIndex < 6; groupIndex++)
		B3D_TEST_ASSERT(fnGetIdentifier(groupIndex) != nullptr)

	if(fnGetIdentifier(5) == nullptr)
		return;

	const u32 identifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	const GpuShaderBindingTableRegion& hitGroupRegion = d3d12Table.GetHitGroupRegion();
	const u8 zeros[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES] = {};
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(d3d12Table.GetRayGenerationRegion(), 0), fnGetIdentifier(0), identifierSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(d3d12Table.GetRayGenerationRegion(), 1), fnGetIdentifier(1), identifierSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(d3d12Table.GetMissRegion(), 0), fnGetIdentifier(2), identifierSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(hitGroupRegion, 0), fnGetIdentifier(3), identifierSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(hitGroupRegion, 1), zeros, identifierSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(hitGroupRegion, 2), fnGetIdentifier(5), identifierSize) == 0)

	// Identifiers of different groups differ
	B3D_TEST_ASSERT(std::memcmp(fnGetIdentifier(3), fnGetIdentifier(4), identifierSize) != 0)
}
