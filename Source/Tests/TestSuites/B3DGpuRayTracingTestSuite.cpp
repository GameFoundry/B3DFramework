//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuRayTracingTestSuite.h"
#include "Utility/B3DGpuBarrierTestUtility.h"
#include "Utility/B3DUnitTestGpuPipelines.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuBarriers.h"
#include "GpuBackend/B3DGpuBarrierValidator.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuWorkContext.h"

using namespace b3d;
using namespace b3d::render;

GpuRayTracingTestSuite::GpuRayTracingTestSuite()
	: TestSuite("GpuRayTracingTestSuite")
{
	B3D_ADD_TEST(GpuRayTracingTestSuite::TestAccelerationStructureBuffers)
}

void GpuRayTracingTestSuite::TestAccelerationStructureBuffers()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	static constexpr u32 kSize = 4096;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const GpuBufferCreateInformation bottomLevelInformation = GpuBufferCreateInformation::CreateAccelerationStructure(GpuAccelerationStructureType::BottomLevel, kSize);
	const GpuBufferCreateInformation topLevelInformation = GpuBufferCreateInformation::CreateAccelerationStructure(GpuAccelerationStructureType::TopLevel, kSize);
	const GpuBufferFlags inputFlags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AccelerationStructureInput;

	if(!device->GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES))
	{
		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot create an acceleration structure buffer, the GPU device doesn't support acceleration structures.");
			B3D_TEST_ASSERT(device->CreateGpuBuffer(bottomLevelInformation) == nullptr)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot create an acceleration structure input buffer, the GPU device doesn't support acceleration structures.");
			B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(12, 3, inputFlags)) == nullptr)
		}

		return;
	}

	// Builds read vertex, index and structured storage buffers only
	{
		LoggingScope logs(*this);
		logs.ExpectError("An acceleration structure input buffer must be a vertex, index or structured storage buffer.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, 4, inputFlags)) == nullptr)
	}

	{
		LoggingScope logs(*this);
		logs.ExpectError("An acceleration structure input buffer must be a vertex, index or structured storage buffer.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(64, GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AccelerationStructureInput)) == nullptr)
	}

	{
		LoggingScope logs(*this);
		logs.ExpectError("An acceleration structure buffer must have a size.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateAccelerationStructure(GpuAccelerationStructureType::BottomLevel, 0)) == nullptr)
	}

	{
		LoggingScope logs(*this);
		logs.ExpectError("An acceleration structure buffer must be created with GpuBufferFlag::StoreOnGPU and no other flag.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateAccelerationStructure(GpuAccelerationStructureType::BottomLevel, kSize, GpuBufferFlag::StoreOnCPUWithGPUAccess)) == nullptr)
	}

	{
		LoggingScope logs(*this);
		logs.ExpectError("An acceleration structure buffer must be created with GpuBufferFlag::StoreOnGPU and no other flag.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateAccelerationStructure(GpuAccelerationStructureType::BottomLevel, kSize, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU)) == nullptr)
	}

	{
		LoggingScope logs(*this);
		logs.ExpectError("Acceleration structure buffers are created on the render thread");
		B3D_TEST_ASSERT(b3d::GpuBuffer::Create(bottomLevelInformation) == nullptr)
	}

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuComputePipelineState> pipeline = CreateTestByteAddressBufferAccessPipeline(*device);
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device, &pipeline, &bottomLevelInformation, &topLevelInformation, inputFlags]()
	{
		// Build inputs: geometry in vertex and index buffers, instances in a structured buffer the CPU or a shader writes
		const TShared<render::GpuBuffer> inputVertices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(12, 3, inputFlags));
		const TShared<render::GpuBuffer> inputIndices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateIndex(IT_16BIT, 3, inputFlags));
		const TShared<render::GpuBuffer> inputInstances = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(64, 4, inputFlags | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
		const TShared<render::GpuBuffer> mappedInstances = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(64, 4, GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AccelerationStructureInput));
		B3D_TEST_ASSERT(inputVertices != nullptr && inputIndices != nullptr && inputInstances != nullptr && mappedInstances != nullptr)
		B3D_TEST_ASSERT(inputVertices == nullptr || inputVertices->GetInformation().Flags.IsSet(GpuBufferFlag::AccelerationStructureInput))

		const TShared<render::GpuBuffer> bottomLevel = device->CreateGpuBuffer(bottomLevelInformation);
		const TShared<render::GpuBuffer> topLevel = device->CreateGpuBuffer(topLevelInformation);
		B3D_TEST_ASSERT(bottomLevel != nullptr && topLevel != nullptr)
		if(bottomLevel == nullptr || topLevel == nullptr || inputVertices == nullptr)
			return;

		bottomLevel->SetName("BottomLevel");
		topLevel->SetName("TopLevel");
		inputVertices->SetName("InputVertices");

		B3D_TEST_ASSERT(bottomLevel->GetTotalSize() == kSize && topLevel->GetTotalSize() == kSize)
		B3D_TEST_ASSERT(bottomLevel->GetInformation().AccelerationStructure.Type == GpuAccelerationStructureType::BottomLevel)
		B3D_TEST_ASSERT(topLevel->GetInformation().AccelerationStructure.Type == GpuAccelerationStructureType::TopLevel)
		B3D_TEST_ASSERT(bottomLevel->GetMappedMemory() == nullptr && bottomLevel->GetBindlessIndex() == kGpuBindlessInvalidIndex)

		// The buffer rests readable where rays are traced against it and where builds read it, and nowhere else
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*bottomLevel, GQT_GRAPHICS);
		B3D_TEST_ASSERT(restingState.Access == GpuAccessFlag::Read)
		B3D_TEST_ASSERT(restingState.Stages == (GpuBackendUtility::GetAccelerationStructureStageFlags() & GpuBackendUtility::GetQueueStageFlags(GQT_GRAPHICS)))
		B3D_TEST_ASSERT(restingState.Stages.IsSet(GpuStageFlag::AccelerationStructureBuild) && restingState.Stages.IsSet(GpuStageFlag::ComputeShaderAccelerationStructure))
		B3D_TEST_ASSERT(!restingState.Stages.IsSetAny(GpuStageFlag::Transfer | GpuStageFlag::Host | GpuStageFlag::VertexInputAttributes | GpuStageFlag::ComputeShaderUniform |
			GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::AccelerationStructureBuildInput))

		// Other buffers rest readable as build inputs, but not where rays are traced
		const GpuAccessState inputRestingState = GpuBackendUtility::GetRestingState(*inputVertices, GQT_GRAPHICS);
		B3D_TEST_ASSERT(inputRestingState.Stages.IsSet(GpuStageFlag::AccelerationStructureBuildInput))
		B3D_TEST_ASSERT(!inputRestingState.Stages.IsSetAny(GpuStageFlag::AllShaderAccelerationStructure | GpuStageFlag::AccelerationStructureBuild))

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuParameterSet> parameterSet = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);

		{
			LoggingScope logs(*this);
			logs.ExpectError("An acceleration structure buffer cannot be bound as a storage buffer.");
			parameterSet->SetStorageBuffer("Input", topLevel);
		}

		// A build writes the buffer, and returns it to rest
		const GpuAccessState buildWrite(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Write);
		const GpuBufferBarrier buildBarrier(bottomLevel, restingState, buildWrite);
		const GpuBufferBarrier restBarrier(bottomLevel, buildWrite, restingState);

		const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Acceleration structure buffer barriers"));

		commands->IssueBarriers(buildBarrier);
		commands->IssueBarriers(restBarrier);
		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();

#if B3D_BUILD_TYPE_DEVELOPMENT
		{
			LoggingScope logs(*this);
			logs.ExpectError("The host cannot read or write acceleration structure buffer 'BottomLevel'.");

			const u32 word = 0;
			GpuBufferUtility::Write(*context, bottomLevel, 0, sizeof(word), &word);
		}

		{
			GpuBarrierValidator validator;
			validator.BeginCommandBuffer(GQT_GRAPHICS);

			validator.ValidateBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&buildBarrier, 1)), GpuBarrierPhase::Full, nullptr);
			validator.ValidateBufferAccess(*bottomLevel, GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Write);
			validator.ValidateBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&restBarrier, 1)), GpuBarrierPhase::Full, nullptr);
			validator.ValidateBufferAccess(*bottomLevel, GpuStageFlag::ComputeShaderAccelerationStructure, GpuAccessFlag::Read);
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Only an acceleration structure buffer can be accessed by rays traced against it. Buffer: 'InputVertices'.");

			GpuBarrierValidator validator;
			validator.BeginCommandBuffer(GQT_GRAPHICS);
			validator.ValidateBufferAccess(*inputVertices, GpuStageFlag::RayTracingShaderAccelerationStructure, GpuAccessFlag::Read);
		}

		const String stageError = "An acceleration structure buffer can only be accessed by rays traced against it and by acceleration structure builds.";
		{
			LoggingScope logs(*this);
			logs.ExpectError(stageError + " Buffer: 'TopLevel'.");

			GpuBarrierValidator validator;
			validator.BeginCommandBuffer(GQT_GRAPHICS);
			validator.ValidateBufferAccess(*topLevel, GpuStageFlag::Transfer, GpuAccessFlag::Read);
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError(stageError + " Buffer: 'TopLevel'.");

			const GpuBufferBarrier copyBarrier(topLevel, restingState, GpuAccessState(GpuStageFlag::Transfer, GpuAccessFlag::Write));

			GpuBarrierValidator validator;
			validator.BeginCommandBuffer(GQT_GRAPHICS);
			validator.ValidateBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&copyBarrier, 1)), GpuBarrierPhase::Full, nullptr);
		}
#endif
	}, "GpuRayTracingTestSuite::TestAccelerationStructureBuffers", true);
}
