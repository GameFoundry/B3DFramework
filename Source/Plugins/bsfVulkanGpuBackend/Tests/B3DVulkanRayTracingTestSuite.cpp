//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanRayTracingTestSuite.h"
#include "B3DVulkanGpuAccelerationStructure.h"
#include "B3DVulkanGpuBuffer.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuPipelineState.h"
#include "B3DVulkanGpuShaderBindingTable.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"
#include "GpuBackend/B3DGpuBackend.h"
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
	/** Returns the Vulkan device that tests run on, or null if there is none or it doesn't support acceleration structures. */
	TShared<VulkanGpuDevice> GetAccelerationStructureDevice()
	{
		GpuBackend& backend = GpuBackend::Instance();
		if(backend.GetDeviceCount() == 0)
			return nullptr;

		const TShared<GpuDevice> device = backend.GetDevice(0);
		if(!device->GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES))
			return nullptr;

		return std::static_pointer_cast<VulkanGpuDevice>(device);
	}
} // namespace

VulkanRayTracingTestSuite::VulkanRayTracingTestSuite()
	: TestSuite("VulkanRayTracingTestSuite")
{
	B3D_ADD_TEST(VulkanRayTracingTestSuite::TestBufferUsage)
	B3D_ADD_TEST(VulkanRayTracingTestSuite::TestDeviceAddresses)
	B3D_ADD_TEST(VulkanRayTracingTestSuite::TestInstanceLayout)
	B3D_ADD_TEST(VulkanRayTracingTestSuite::TestShaderBindingTable)
}

void VulkanRayTracingTestSuite::TestBufferUsage()
{
	const TShared<VulkanGpuDevice> device = GetAccelerationStructureDevice();
	if(device == nullptr)
		return;

	const auto fnGetUsage = [&device](const GpuBufferCreateInformation& information)
	{
		return VulkanGpuBuffer::GetVkBufferUsageFlags(*device, information);
	};

	static constexpr VkBufferUsageFlags kBuildInput = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	static constexpr VkBufferUsageFlags kDeviceAddress = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	static constexpr VkBufferUsageFlags kStorage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
	const GpuBufferFlags inputFlags = GpuBufferFlag::AccelerationStructureInput;

	// Builds read their inputs through device addresses
	const GpuBufferCreateInformation inputs[] =
	{
		GpuBufferCreateInformation::CreateVertex(12, 3, inputFlags, GpuBufferMemory::Mapped),
		GpuBufferCreateInformation::CreateIndex(IT_32BIT, 3, inputFlags, GpuBufferMemory::Mapped),
		GpuBufferCreateInformation::CreateStructuredStorage(64, 4, inputFlags, GpuBufferMemory::Mapped)
	};

	for(const GpuBufferCreateInformation& information : inputs)
	{
		const VkBufferUsageFlags usage = fnGetUsage(information);
		B3D_TEST_ASSERT((usage & (kBuildInput | kDeviceAddress)) == (kBuildInput | kDeviceAddress) && (usage & kStorage) == 0)
	}

	// Buffers that can be scratch memory have a device address, but are not build inputs
	const VkBufferUsageFlags scratchUsage = fnGetUsage(GpuBufferCreateInformation::CreateStructuredStorage(4, 64, GpuBufferFlag::GpuWrite));
	B3D_TEST_ASSERT((scratchUsage & kDeviceAddress) != 0 && (scratchUsage & (kBuildInput | kStorage)) == 0)

	// Other buffers have neither
	const GpuBufferCreateInformation others[] =
	{
		GpuBufferCreateInformation::CreateVertex(12, 3, GpuBufferFlag::None, GpuBufferMemory::Mapped),
		GpuBufferCreateInformation::CreateIndex(IT_32BIT, 3),
		GpuBufferCreateInformation::CreateStructuredStorage(4, 64),
		GpuBufferCreateInformation::CreateStructuredStorage(4, 64, GpuBufferFlag::GpuWrite, GpuBufferMemory::Mapped),
		GpuBufferCreateInformation::CreateSimpleStorage(BF_32X1U, 64, GpuBufferFlag::GpuWrite),
		GpuBufferCreateInformation::CreateUniform(256)
	};

	for(const GpuBufferCreateInformation& information : others)
		B3D_TEST_ASSERT((fnGetUsage(information) & (kBuildInput | kDeviceAddress | kStorage)) == 0)
}

void VulkanRayTracingTestSuite::TestDeviceAddresses()
{
	const TShared<VulkanGpuDevice> device = GetAccelerationStructureDevice();
	if(device == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device]()
	{
		const u32 scratchAlignment = device->GetCapabilities().RayTracing.ScratchOffsetAlignment;
		static constexpr u32 kInstanceAlignment = 16;
		static constexpr u32 kAccelerationStructureAlignment = 256;

		// Odd sizes, so resources that share memory blocks don't start aligned by accident
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

			const VulkanBuffer& scratchResource = *static_cast<VulkanGpuBuffer&>(*scratch).GetVulkanResource();
			const VulkanBuffer& instanceResource = *static_cast<VulkanGpuBuffer&>(*instances).GetVulkanResource();
			const VulkanAccelerationStructure& structureResource = *static_cast<VulkanGpuAccelerationStructure&>(*structure).GetVulkanResource();

			B3D_TEST_ASSERT(scratchResource.GetDeviceAddress() != 0 && scratchResource.GetDeviceAddress() % scratchAlignment == 0)
			B3D_TEST_ASSERT(instanceResource.GetDeviceAddress() != 0 && instanceResource.GetDeviceAddress() % kInstanceAlignment == 0)
			B3D_TEST_ASSERT(structureResource.GetDeviceAddress() != 0 && structureResource.GetDeviceAddress() % kAccelerationStructureAlignment == 0)

			B3D_TEST_ASSERT(structureResource.GetVulkanHandle() != VK_NULL_HANDLE && structureResource.GetBuffer().GetVulkanHandle() != VK_NULL_HANDLE)
			B3D_TEST_ASSERT(structure->GetGpuResource() == &structureResource)

			buffers.push_back(scratch);
			buffers.push_back(instances);
			structures.push_back(structure);
		}

		// Buffers that builds don't access have no device address
		const TShared<render::GpuBuffer> plain = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, 16));
		B3D_TEST_ASSERT(plain != nullptr)
		if(plain != nullptr)
		{
			const VulkanBuffer& plainResource = *static_cast<VulkanGpuBuffer&>(*plain).GetVulkanResource();
			B3D_TEST_ASSERT(plainResource.GetDeviceAddress() == 0)
		}
	}, "VulkanRayTracingTestSuite::TestDeviceAddresses", true);
}

void VulkanRayTracingTestSuite::TestInstanceLayout()
{
	const TShared<VulkanGpuDevice> device = GetAccelerationStructureDevice();
	if(device == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device]()
	{
		B3D_TEST_ASSERT(device->GetCapabilities().RayTracing.InstanceSize == sizeof(VkAccelerationStructureInstanceKHR))

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

		VkAccelerationStructureInstanceKHR written[2];
		B3D_TEST_ASSERT(device->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(instances, 2), written))

		// Rows of the transform, without the last one
		for(u32 row = 0; row < 3; row++)
		{
			for(u32 column = 0; column < 4; column++)
				B3D_TEST_ASSERT(written[0].transform.matrix[row][column] == transform[row][column])
		}

		B3D_TEST_ASSERT(written[0].instanceCustomIndex == 0x123456 && written[0].mask == 0x5A && written[0].instanceShaderBindingTableRecordOffset == 0x654321)
		// Vulkan flips the facing of instances that keep the engine's facing
		B3D_TEST_ASSERT(written[0].flags == (VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR | VK_GEOMETRY_INSTANCE_TRIANGLE_FLIP_FACING_BIT_KHR |
			VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR))
		B3D_TEST_ASSERT(written[0].accelerationStructureReference == static_cast<VulkanGpuAccelerationStructure&>(*bottomLevel).GetVulkanResource()->GetDeviceAddress())

		B3D_TEST_ASSERT(written[1].instanceCustomIndex == 0 && written[1].mask == 0xFF && written[1].instanceShaderBindingTableRecordOffset == 0)
		B3D_TEST_ASSERT(written[1].flags == VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR)
		B3D_TEST_ASSERT(written[1].transform.matrix[0][0] == 1.0f && written[1].transform.matrix[0][3] == 0.0f && written[1].transform.matrix[2][2] == 1.0f)
	}, "VulkanRayTracingTestSuite::TestInstanceLayout", true);
}

void VulkanRayTracingTestSuite::TestShaderBindingTable()
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

	const VulkanGpuShaderBindingTable& vulkanTable = static_cast<const VulkanGpuShaderBindingTable&>(*table);
	const VulkanGpuRayTracingPipelineState& vulkanPipeline = static_cast<const VulkanGpuRayTracingPipelineState&>(*pipeline);
	const GpuRayTracingCapabilities& rayTracing = device->GetCapabilities().RayTracing;

	// Every region starts at the table alignment, with records the record alignment apart
	const auto fnCheckRegion = [this, &rayTracing](const VkStridedDeviceAddressRegionKHR& region, u32 recordCount, const char* name)
	{
		const String message = StringUtility::Format("{0} region: address {1}, stride {2}, size {3}.", name, region.deviceAddress, region.stride, region.size);
		B3D_TEST_ASSERT_MSG(region.deviceAddress != 0 && region.deviceAddress % rayTracing.ShaderTableAlignment == 0, message)
		B3D_TEST_ASSERT_MSG(region.stride >= rayTracing.ShaderGroupHandleSize && region.stride % rayTracing.ShaderRecordAlignment == 0, message)
		B3D_TEST_ASSERT_MSG(region.stride <= rayTracing.MaximumShaderRecordStride && region.size == region.stride * recordCount, message)
	};

	for(u32 recordIndex = 0; recordIndex < 2; recordIndex++)
	{
		// A dispatch passes one ray generation record, whose size must equal its stride
		const VkStridedDeviceAddressRegionKHR region = vulkanTable.GetRayGenerationRegion(recordIndex);
		fnCheckRegion(region, 1, "Ray generation");
	}

	fnCheckRegion(vulkanTable.GetMissRegion(), 1, "Miss");
	fnCheckRegion(vulkanTable.GetHitGroupRegion(), 3, "Hit group");

	const VkStridedDeviceAddressRegionKHR& callableRegion = vulkanTable.GetCallableRegion();
	B3D_TEST_ASSERT(callableRegion.deviceAddress == 0 && callableRegion.stride == 0 && callableRegion.size == 0)

	// Records hold the handles of their groups, numbered ray generation, miss, then hit groups. The empty record is zeros. The table is in
	// host-visible memory, which stays mapped.
	const VulkanBuffer& buffer = *vulkanTable.GetBuffer();
	const VkDeviceAddress tableAddress = buffer.GetDeviceAddress();
	const VkStridedDeviceAddressRegionKHR& hitGroupRegion = vulkanTable.GetHitGroupRegion();
	const u8* const tableData = static_cast<const u8*>(buffer.GetMappedMemory());
	B3D_TEST_ASSERT(tableData != nullptr)
	if(tableData == nullptr)
		return;

	const auto fnGetRecord = [tableData, tableAddress](const VkStridedDeviceAddressRegionKHR& region, u32 recordIndex)
	{
		return tableData + (region.deviceAddress - tableAddress) + recordIndex * region.stride;
	};

	const u32 handleSize = rayTracing.ShaderGroupHandleSize;
	const TArrayView<const u8> firstHitGroupHandle = vulkanPipeline.GetShaderGroupHandle(3);
	const TArrayView<const u8> lastHitGroupHandle = vulkanPipeline.GetShaderGroupHandle(5);
	B3D_TEST_ASSERT(firstHitGroupHandle.size() == handleSize && lastHitGroupHandle.size() == handleSize)

	const Vector<u8> zeros(handleSize, 0);
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(vulkanTable.GetRayGenerationRegion(1), 0), vulkanPipeline.GetShaderGroupHandle(1).data(), handleSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(vulkanTable.GetMissRegion(), 0), vulkanPipeline.GetShaderGroupHandle(2).data(), handleSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(hitGroupRegion, 0), firstHitGroupHandle.data(), handleSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(hitGroupRegion, 1), zeros.data(), handleSize) == 0)
	B3D_TEST_ASSERT(std::memcmp(fnGetRecord(hitGroupRegion, 2), lastHitGroupHandle.data(), handleSize) == 0)
}
