//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuRayTracingTestSuite.h"
#include "Utility/B3DGpuBarrierTestUtility.h"
#include "Utility/B3DUnitTestGpuPipelines.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"
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
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuShaderBindingTable.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "Material/B3DShaderReflection.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DVector3.h"

using namespace b3d;
using namespace b3d::render;

GpuRayTracingTestSuite::GpuRayTracingTestSuite()
	: TestSuite("GpuRayTracingTestSuite")
{
	B3D_ADD_TEST(GpuRayTracingTestSuite::TestAccelerationStructureCreation)
	B3D_ADD_TEST(GpuRayTracingTestSuite::TestAccelerationStructureQueries)
	B3D_ADD_TEST(GpuRayTracingTestSuite::TestInlineRayTracing)
	B3D_ADD_TEST(GpuRayTracingTestSuite::TestRayTracingPipelineCreation)
	B3D_ADD_TEST(GpuRayTracingTestSuite::TestRayTracingPipelineRecords)
}

void GpuRayTracingTestSuite::TestAccelerationStructureCreation()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	static constexpr u32 kSize = 4096;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const GpuBufferFlags inputFlags = GpuBufferFlag::AccelerationStructureInput;

	if(!device->GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES))
	{
		GetRenderThread().PostCommand([this, &device]()
		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot create acceleration structure 'BottomLevel', the GPU device doesn't support acceleration structures.");
			B3D_TEST_ASSERT(device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(GpuAccelerationStructureType::BottomLevel, kSize,
				GpuAccelerationStructureFlag::None, "BottomLevel")) == nullptr)
		}, "GpuRayTracingTestSuite::TestAccelerationStructureCreation", true);

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
		GpuBufferCreateInformation uniformInformation = GpuBufferCreateInformation::CreateUniform(64);
		uniformInformation.Flags = GpuBufferFlag::AccelerationStructureInput;
		B3D_TEST_ASSERT(device->CreateGpuBuffer(uniformInformation) == nullptr)
	}

	GetRenderThread().PostCommand([this, &device, inputFlags]()
	{
		{
			LoggingScope logs(*this);
			logs.ExpectError("Acceleration structure 'Empty' must have a size.");
			B3D_TEST_ASSERT(device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(GpuAccelerationStructureType::BottomLevel, 0,
				GpuAccelerationStructureFlag::None, "Empty")) == nullptr)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Acceleration structure 'BindlessBottomLevel' is bindless, which only a top level structure can be.");
			B3D_TEST_ASSERT(device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(GpuAccelerationStructureType::BottomLevel, kSize,
				GpuAccelerationStructureFlag::Bindless, "BindlessBottomLevel")) == nullptr)
		}

		// A bindless top level structure takes a heap slot on devices that support them
		{
			const GpuAccelerationStructureCreateInformation bindlessInformation(GpuAccelerationStructureType::TopLevel, kSize, GpuAccelerationStructureFlag::Bindless,
				"BindlessTopLevel");
			if(device->GetCapabilities().RayTracing.SupportsBindlessAccelerationStructures)
			{
				const TShared<GpuAccelerationStructure> bindlessTopLevel = device->CreateGpuAccelerationStructure(bindlessInformation);
				B3D_TEST_ASSERT(bindlessTopLevel != nullptr && bindlessTopLevel->GetBindlessIndex() != kGpuBindlessInvalidIndex)
				B3D_TEST_ASSERT(bindlessTopLevel == nullptr || bindlessTopLevel->GetFlags() == GpuAccelerationStructureFlag::Bindless)
			}
			else
			{
				LoggingScope logs(*this);
				logs.ExpectError("Cannot create bindless acceleration structure 'BindlessTopLevel', the GPU device doesn't support them.");
				B3D_TEST_ASSERT(device->CreateGpuAccelerationStructure(bindlessInformation) == nullptr)
			}
		}

		// Build inputs: geometry in vertex and index buffers, instances in a structured buffer the CPU or a shader writes
		const TShared<render::GpuBuffer> inputVertices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(12, 3, inputFlags));
		const TShared<render::GpuBuffer> inputIndices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateIndex(IT_16BIT, 3, inputFlags));
		const TShared<render::GpuBuffer> inputInstances = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(64, 4, inputFlags | GpuBufferFlag::GpuWrite));
		const TShared<render::GpuBuffer> mappedInstances = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(64, 4, GpuBufferFlag::AccelerationStructureInput, GpuBufferMemory::Mapped));
		B3D_TEST_ASSERT(inputVertices != nullptr && inputIndices != nullptr && inputInstances != nullptr && mappedInstances != nullptr)
		B3D_TEST_ASSERT(inputVertices == nullptr || inputVertices->GetInformation().Flags.IsSet(GpuBufferFlag::AccelerationStructureInput))

		const TShared<GpuAccelerationStructure> bottomLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(GpuAccelerationStructureType::BottomLevel,
			kSize, GpuAccelerationStructureFlag::None, "BottomLevel"));
		const TShared<GpuAccelerationStructure> topLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(GpuAccelerationStructureType::TopLevel,
			kSize, GpuAccelerationStructureFlag::None, "TopLevel"));
		B3D_TEST_ASSERT(bottomLevel != nullptr && topLevel != nullptr)
		if(bottomLevel == nullptr || topLevel == nullptr || inputVertices == nullptr)
			return;

		inputVertices->SetName("InputVertices");

		B3D_TEST_ASSERT(bottomLevel->GetType() == GpuAccelerationStructureType::BottomLevel && topLevel->GetType() == GpuAccelerationStructureType::TopLevel)
		B3D_TEST_ASSERT(bottomLevel->GetSize() == kSize && topLevel->GetSize() == kSize)
		B3D_TEST_ASSERT(bottomLevel->GetName() == "BottomLevel" && bottomLevel->GetFlags() == GpuAccelerationStructureFlag::None)
		B3D_TEST_ASSERT(bottomLevel->GetBindlessIndex() == kGpuBindlessInvalidIndex && bottomLevel->GetGpuResource() != nullptr)

		// The structure rests readable where rays are traced against it and where builds read it, and nowhere else
		const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*bottomLevel, GQT_GRAPHICS);
		B3D_TEST_ASSERT(restingState.Access == GpuAccessFlag::Read)
		B3D_TEST_ASSERT(restingState.Stages == (GpuBackendUtility::GetAccelerationStructureStageFlags() & GpuBackendUtility::GetQueueStageFlags(GQT_GRAPHICS)))
		B3D_TEST_ASSERT(restingState.Stages.IsSet(GpuStageFlag::AccelerationStructureBuild) && restingState.Stages.IsSet(GpuStageFlag::ComputeShaderAccelerationStructure))
		B3D_TEST_ASSERT(!restingState.Stages.IsSetAny(GpuStageFlag::Transfer | GpuStageFlag::Host | GpuStageFlag::VertexInputAttributes | GpuStageFlag::ComputeShaderUniform |
			GpuStageFlag::ComputeShaderNonUniform | GpuStageFlag::AccelerationStructureBuildInput))

		// Buffers rest readable as build inputs, but not where rays are traced
		const GpuAccessState inputRestingState = GpuBackendUtility::GetRestingState(*inputVertices, GQT_GRAPHICS);
		B3D_TEST_ASSERT(inputRestingState.Stages.IsSet(GpuStageFlag::AccelerationStructureBuildInput))
		B3D_TEST_ASSERT(!inputRestingState.Stages.IsSetAny(GpuStageFlag::AllShaderAccelerationStructure | GpuStageFlag::AccelerationStructureBuild))

		// A build writes the structure, and returns it to rest
		const GpuAccessState buildWrite(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Write);
		const GpuAccelerationStructureBarrier buildBarrier(bottomLevel, restingState, buildWrite);
		const GpuAccelerationStructureBarrier restBarrier(bottomLevel, buildWrite, restingState);

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Acceleration structure barriers"));

		commands->IssueBarriers(buildBarrier);
		commands->IssueBarriers(restBarrier);
		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();

#if B3D_BUILD_TYPE_DEVELOPMENT
		{
			GpuBarrierValidator validator;
			validator.BeginCommandBuffer(GQT_GRAPHICS);

			validator.ValidateBarriers(GpuBarriers(buildBarrier), GpuBarrierPhase::Full, nullptr);
			validator.ValidateAccelerationStructureAccess(*bottomLevel, GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Write);
			validator.ValidateBarriers(GpuBarriers(restBarrier), GpuBarrierPhase::Full, nullptr);
			validator.ValidateAccelerationStructureAccess(*bottomLevel, GpuStageFlag::ComputeShaderAccelerationStructure, GpuAccessFlag::Read);
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Only an acceleration structure can be accessed by rays traced against it. Buffer: 'InputVertices'.");

			GpuBarrierValidator validator;
			validator.BeginCommandBuffer(GQT_GRAPHICS);
			validator.ValidateBufferAccess(*inputVertices, GpuStageFlag::RayTracingShaderAccelerationStructure, GpuAccessFlag::Read);
		}

		const String stageError = "An acceleration structure can only be accessed by rays traced against it and by acceleration structure builds.";
		{
			LoggingScope logs(*this);
			logs.ExpectError(stageError + " Acceleration structure: 'TopLevel'.");

			GpuBarrierValidator validator;
			validator.BeginCommandBuffer(GQT_GRAPHICS);
			validator.ValidateAccelerationStructureAccess(*topLevel, GpuStageFlag::Transfer, GpuAccessFlag::Read);
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError(stageError + " Acceleration structure: 'TopLevel'.");

			const GpuAccelerationStructureBarrier copyBarrier(topLevel, restingState, GpuAccessState(GpuStageFlag::Transfer, GpuAccessFlag::Write));

			GpuBarrierValidator validator;
			validator.BeginCommandBuffer(GQT_GRAPHICS);
			validator.ValidateBarriers(GpuBarriers(copyBarrier), GpuBarrierPhase::Full, nullptr);
		}
#endif
	}, "GpuRayTracingTestSuite::TestAccelerationStructureCreation", true);
}

namespace
{
	/** Ray traced by the InlineRayTracing test shader, in the layout of its TestRay structure. */
	struct TestRay
	{
		TestRay() = default;

		/**
		 * @param	origin				Origin of the ray.
		 * @param	direction			Direction of the ray.
		 * @param	flags				Ray flags (RAY_FLAG_* values).
		 * @param	mask				Instance mask.
		 * @param	mode				0 to trace the closest hit, 1 any hit, 2 the closest hit with callbacks that reject every candidate.
		 * @param	maximumDistance		Distance past which hits are ignored.
		 */
		TestRay(const Vector3& origin, const Vector3& direction, u32 flags = 0, u32 mask = 0xFF, u32 mode = 0, float maximumDistance = 100.0f)
			: Origin(origin), Direction(direction), MaximumDistance(maximumDistance), Flags(flags), Mask(mask), Mode(mode)
		{ }

		Vector3 Origin = Vector3::kZero;
		float MinimumDistance = 0.0f;
		Vector3 Direction = Vector3::kZero;
		float MaximumDistance = 0.0f;
		u32 Flags = 0;
		u32 Mask = 0;
		u32 Mode = 0;
		u32 Padding = 0;
	};

	static_assert(sizeof(TestRay) == 48, "TestRay must match the layout of the shader's structure.");

	/** What the InlineRayTracing test shader reports for one ray, in the layout of its TestHit structure. */
	struct TestHit
	{
		float IsHit;
		float Distance;
		float Barycentrics[2];
		u32 InstanceIndex;
		u32 InstanceId;
		u32 GeometryIndex;
		u32 PrimitiveIndex;
		u32 FrontFace;
		u32 CallbackCount;
		u32 Padding[2];
	};

	static_assert(sizeof(TestHit) == 48, "TestHit must match the layout of the shader's structure.");

	/** What a TestRay is expected to hit. */
	struct ExpectedHit
	{
		/** A miss, after @p callbackCount any-hit callbacks. */
		explicit ExpectedHit(u32 callbackCount = 0)
			: CallbackCount(callbackCount)
		{ }

		/**
		 * @param	distance		Distance to the hit.
		 * @param	barycentricX	Weight of the triangle's second vertex.
		 * @param	barycentricY	Weight of the triangle's third vertex.
		 * @param	instanceIndex	Position of the instance in the top level structure.
		 * @param	instanceId		Id of the instance.
		 * @param	geometryIndex	Position of the geometry in the bottom level structure.
		 * @param	primitiveIndex	Triangle within the geometry.
		 * @param	frontFace		True if the ray hits the front face.
		 * @param	callbackCount	Number of any-hit callbacks.
		 */
		ExpectedHit(float distance, float barycentricX, float barycentricY, u32 instanceIndex, u32 instanceId, u32 geometryIndex, u32 primitiveIndex, bool frontFace,
			u32 callbackCount = 0)
			: IsHit(true), Distance(distance), BarycentricX(barycentricX), BarycentricY(barycentricY), InstanceIndex(instanceIndex), InstanceId(instanceId),
			GeometryIndex(geometryIndex), PrimitiveIndex(primitiveIndex), FrontFace(frontFace), CallbackCount(callbackCount)
		{ }

		bool IsHit = false;
		float Distance = 0.0f;
		float BarycentricX = 0.0f;
		float BarycentricY = 0.0f;
		u32 InstanceIndex = 0;
		u32 InstanceId = 0;
		u32 GeometryIndex = 0;
		u32 PrimitiveIndex = 0;
		bool FrontFace = false;
		u32 CallbackCount = 0;
	};

	/** Rounds @p value up to a multiple of @p alignment. */
	u32 AlignUp(u32 value, u32 alignment)
	{
		return (value + alignment - 1) / alignment * alignment;
	}
} // namespace

void GpuRayTracingTestSuite::TestAccelerationStructureQueries()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	GetRenderThread().PostCommand([this, &device]()
	{
		const GpuBufferFlags inputFlags = GpuBufferFlag::AccelerationStructureInput;
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Acceleration structure queries"));

		if(!device->GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES))
		{
			{
				LoggingScope logs(*this);
				logs.ExpectError("Cannot query acceleration structure sizes, the GPU device doesn't support acceleration structures.");

				GpuAccelerationStructureSizes sizes(1, 1);
				B3D_TEST_ASSERT(!device->GetAccelerationStructureSizes(GpuAccelerationStructureInputs(TShared<render::GpuBuffer>(), 0), sizes))
				B3D_TEST_ASSERT(sizes.AccelerationStructureSize == 0 && sizes.BuildScratchSize == 0)
			}

			{
				LoggingScope logs(*this);
				logs.ExpectError("Cannot write ray tracing instances, the GPU device doesn't support acceleration structures.");

				u8 instanceData[128] = {};
				B3D_TEST_ASSERT(!device->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(), instanceData))
			}

			{
				LoggingScope logs(*this);
				logs.ExpectError("Cannot build acceleration structures, the GPU device doesn't support them.");
				B3D_TEST_ASSERT(!commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>()))
			}

			context->SubmitCommandBuffer(commands);
			device->WaitUntilIdle();
			return;
		}

		const GpuRayTracingCapabilities& rayTracing = device->GetCapabilities().RayTracing;
		B3D_TEST_ASSERT(rayTracing.InstanceSize > 0 && rayTracing.ScratchOffsetAlignment > 0)

		const TShared<render::GpuBuffer> vertices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(12, 6, inputFlags, GpuBufferMemory::Mapped));
		const TShared<render::GpuBuffer> instances = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(rayTracing.InstanceSize, 2, inputFlags, GpuBufferMemory::Mapped));
		B3D_TEST_ASSERT(vertices != nullptr && instances != nullptr)
		if(vertices == nullptr || instances == nullptr)
			return;

		vertices->SetName("Vertices");

		const GpuRayTracingGeometry triangles(vertices, 12, 6);
		const GpuAccelerationStructureInputs bottomLevelInputs(TArrayView<const GpuRayTracingGeometry>(&triangles, 1));
		const GpuAccelerationStructureInputs topLevelInputs(instances, 2);

		GpuAccelerationStructureSizes bottomLevelSizes;
		GpuAccelerationStructureSizes topLevelSizes;
		B3D_TEST_ASSERT(device->GetAccelerationStructureSizes(bottomLevelInputs, bottomLevelSizes))
		B3D_TEST_ASSERT(device->GetAccelerationStructureSizes(topLevelInputs, topLevelSizes))
		B3D_TEST_ASSERT(bottomLevelSizes.AccelerationStructureSize > 0 && bottomLevelSizes.BuildScratchSize > 0)
		B3D_TEST_ASSERT(topLevelSizes.AccelerationStructureSize > 0 && topLevelSizes.BuildScratchSize > 0)

		// A top level structure without instances is valid, and needs memory too
		GpuAccelerationStructureSizes emptySizes;
		B3D_TEST_ASSERT(device->GetAccelerationStructureSizes(GpuAccelerationStructureInputs(TShared<render::GpuBuffer>(), 0), emptySizes))
		B3D_TEST_ASSERT(emptySizes.AccelerationStructureSize > 0)

		const TShared<GpuAccelerationStructure> bottomLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::BottomLevel, bottomLevelSizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "BottomLevel"));
		B3D_TEST_ASSERT(bottomLevel != nullptr)
		if(bottomLevel == nullptr)
			return;

		B3D_TEST_ASSERT(bottomLevel->GetType() == GpuAccelerationStructureType::BottomLevel && bottomLevel->GetSize() == bottomLevelSizes.AccelerationStructureSize)
		B3D_TEST_ASSERT(bottomLevel->GetBindlessIndex() == kGpuBindlessInvalidIndex)

		// Instances are written in the layout of the device, one InstanceSize apart
		{
			const GpuRayTracingInstance instance(bottomLevel.get(), Matrix4::kIdentity, 5);
			Vector<u8> instanceData(rayTracing.InstanceSize * 2, 0xCD);
			const GpuRayTracingInstance instancePair[] = { instance, instance };
			B3D_TEST_ASSERT(device->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(instancePair, 2), instanceData.data()))

			B3D_TEST_ASSERT(std::memcmp(instanceData.data(), instanceData.data() + rayTracing.InstanceSize, rayTracing.InstanceSize) == 0)
		}

#if B3D_BUILD_TYPE_DEVELOPMENT
		{
			LoggingScope logs(*this);
			logs.ExpectError("An acceleration structure cannot prefer both a fast trace and a fast build.");

			const GpuAccelerationStructureInputs inputs(TArrayView<const GpuRayTracingGeometry>(&triangles, 1),
				GpuAccelerationStructureBuildFlag::PreferFastTrace | GpuAccelerationStructureBuildFlag::PreferFastBuild);
			GpuAccelerationStructureSizes sizes;
			B3D_TEST_ASSERT(!device->GetAccelerationStructureSizes(inputs, sizes) && sizes.AccelerationStructureSize == 0)
		}

		{
			const TShared<render::GpuBuffer> plainVertices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(12, 3, GpuBufferFlag::None, GpuBufferMemory::Mapped));
			plainVertices->SetName("PlainVertices");

			LoggingScope logs(*this);
			logs.ExpectError("The vertex buffer 'PlainVertices' of an acceleration structure was not created with GpuBufferFlag::AccelerationStructureInput.");

			const GpuRayTracingGeometry geometry(plainVertices, 12, 3);
			GpuAccelerationStructureSizes sizes;
			B3D_TEST_ASSERT(!device->GetAccelerationStructureSizes(GpuAccelerationStructureInputs(TArrayView<const GpuRayTracingGeometry>(&geometry, 1)), sizes) && sizes.AccelerationStructureSize == 0)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Geometry 0 of an acceleration structure must have VET_FLOAT3 vertex positions.");

			GpuRayTracingGeometry geometry(vertices, 12, 6);
			geometry.VertexType = VET_FLOAT2;
			GpuAccelerationStructureSizes sizes;
			B3D_TEST_ASSERT(!device->GetAccelerationStructureSizes(GpuAccelerationStructureInputs(TArrayView<const GpuRayTracingGeometry>(&geometry, 1)), sizes) && sizes.AccelerationStructureSize == 0)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Geometry 0 of an acceleration structure has no index buffer, so its vertex count must be a multiple of 3.");

			const GpuRayTracingGeometry geometry(vertices, 12, 4);
			GpuAccelerationStructureSizes sizes;
			B3D_TEST_ASSERT(!device->GetAccelerationStructureSizes(GpuAccelerationStructureInputs(TArrayView<const GpuRayTracingGeometry>(&geometry, 1)), sizes) && sizes.AccelerationStructureSize == 0)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("The vertices of geometry 0 of an acceleration structure end past the end of buffer 'Vertices'.");

			GpuRayTracingGeometry geometry(vertices, 12, 6);
			geometry.VertexOffset = 12;
			GpuAccelerationStructureSizes sizes;
			B3D_TEST_ASSERT(!device->GetAccelerationStructureSizes(GpuAccelerationStructureInputs(TArrayView<const GpuRayTracingGeometry>(&geometry, 1)), sizes) && sizes.AccelerationStructureSize == 0)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError(StringUtility::Format("A top level acceleration structure has instance offset 4, which must be a multiple of {0}.", rayTracing.InstanceSize));

			GpuAccelerationStructureInputs inputs(instances, 1);
			inputs.InstanceOffset = 4;
			GpuAccelerationStructureSizes sizes;
			B3D_TEST_ASSERT(!device->GetAccelerationStructureSizes(inputs, sizes) && sizes.AccelerationStructureSize == 0)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Ray tracing instance 0 cannot force its geometry to be both opaque and non-opaque.");

			GpuRayTracingInstance instance(bottomLevel.get(), Matrix4::kIdentity);
			instance.Flags = GpuRayTracingInstanceFlag::ForceOpaque | GpuRayTracingInstanceFlag::ForceNonOpaque;
			Vector<u8> instanceData(rayTracing.InstanceSize);
			B3D_TEST_ASSERT(!device->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(&instance, 1), instanceData.data()))
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Ray tracing instance 0 must reference a bottom level acceleration structure.");

			const GpuRayTracingInstance instance(nullptr, Matrix4::kIdentity);
			Vector<u8> instanceData(rayTracing.InstanceSize);
			B3D_TEST_ASSERT(!device->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(&instance, 1), instanceData.data()))
		}

		// Builds are checked before anything is recorded
		const u32 scratchAlignment = rayTracing.ScratchOffsetAlignment;
		const u32 scratchSize = AlignUp(bottomLevelSizes.BuildScratchSize, scratchAlignment) * 2;
		const TShared<render::GpuBuffer> scratch = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, scratchSize / 4,
			GpuBufferFlag::GpuWrite));
		const TShared<GpuAccelerationStructure> topLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::TopLevel, topLevelSizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "TopLevel"));
		B3D_TEST_ASSERT(scratch != nullptr && topLevel != nullptr)
		if(scratch == nullptr || topLevel == nullptr)
			return;

		{
			LoggingScope logs(*this);
			logs.ExpectError("Build 0 builds acceleration structure 'TopLevel' from inputs of another level.");

			const GpuAccelerationStructureBuild build(bottomLevelInputs, topLevel, scratch);
			B3D_TEST_ASSERT(!commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&build, 1)))
		}

		if(scratchAlignment > 4)
		{
			LoggingScope logs(*this);
			logs.ExpectError(StringUtility::Format("The scratch offset 4 of build 0 must be a multiple of {0}.", scratchAlignment));

			const GpuAccelerationStructureBuild build(bottomLevelInputs, bottomLevel, scratch, 4);
			B3D_TEST_ASSERT(!commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&build, 1)))
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("The scratch buffer 'Vertices' of build 0 must be a structured storage buffer in GPU memory (GpuBufferMemory::Device), with GpuBufferFlag::GpuWrite.");

			const GpuAccelerationStructureBuild build(bottomLevelInputs, bottomLevel, vertices);
			B3D_TEST_ASSERT(!commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&build, 1)))
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Builds 0 and 1 build the same acceleration structure.");

			const GpuAccelerationStructureBuild builds[] =
			{
				GpuAccelerationStructureBuild(bottomLevelInputs, bottomLevel, scratch),
				GpuAccelerationStructureBuild(bottomLevelInputs, bottomLevel, scratch, AlignUp(bottomLevelSizes.BuildScratchSize, scratchAlignment))
			};

			B3D_TEST_ASSERT(!commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(builds, 2)))
		}

		{
			const TShared<GpuAccelerationStructure> otherBottomLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
				GpuAccelerationStructureType::BottomLevel, bottomLevelSizes.AccelerationStructureSize));

			LoggingScope logs(*this);
			logs.ExpectError("Builds 0 and 1 use overlapping scratch memory of buffer '" + scratch->GetName() + "'.");

			const GpuAccelerationStructureBuild builds[] =
			{
				GpuAccelerationStructureBuild(bottomLevelInputs, bottomLevel, scratch),
				GpuAccelerationStructureBuild(bottomLevelInputs, otherBottomLevel, scratch)
			};

			B3D_TEST_ASSERT(!commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(builds, 2)))
		}

		// Needs the top level build's scratch memory to fit after the bottom level build's
		const u32 topLevelScratchOffset = AlignUp(bottomLevelSizes.BuildScratchSize, scratchAlignment);
		if(topLevelScratchOffset + topLevelSizes.BuildScratchSize <= scratch->GetTotalSize())
		{
			LoggingScope logs(*this);
			logs.ExpectError("Builds 0 and 1 build a top level acceleration structure and a bottom level structure it references. Build them in separate calls, with a barrier "
				"between them.");

			GpuAccelerationStructureBuild topLevelBuild(topLevelInputs, topLevel, scratch, topLevelScratchOffset);
			topLevelBuild.BottomLevels = TArrayView<const TShared<GpuAccelerationStructure>>(&bottomLevel, 1);

			const GpuAccelerationStructureBuild builds[] = { GpuAccelerationStructureBuild(bottomLevelInputs, bottomLevel, scratch), topLevelBuild };
			B3D_TEST_ASSERT(!commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(builds, 2)))
		}

		{
			const TShared<GpuAccelerationStructure> smallBottomLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
				GpuAccelerationStructureType::BottomLevel, 256, GpuAccelerationStructureFlag::None, "Small"));

			if(bottomLevelSizes.AccelerationStructureSize > 256)
			{
				LoggingScope logs(*this);
				logs.ExpectError(StringUtility::Format("Build 0 needs {0} bytes, but acceleration structure 'Small' has 256.", bottomLevelSizes.AccelerationStructureSize));

				const GpuAccelerationStructureBuild build(bottomLevelInputs, smallBottomLevel, scratch);
				B3D_TEST_ASSERT(!commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&build, 1)))
			}
		}
#endif

		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();
	}, "GpuRayTracingTestSuite::TestAccelerationStructureQueries", true);
}

void GpuRayTracingTestSuite::TestInlineRayTracing()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const GpuDeviceCapabilities& capabilities = device->GetCapabilities();
	if(!capabilities.HasCapability(RSC_ACCELERATION_STRUCTURES) || !capabilities.HasCapability(RSC_RAY_QUERY))
		return;

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuComputePipelineState> pipeline = CreateTestInlineRayTracingPipeline(*device);
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	TShared<GpuComputePipelineState> bindlessPipeline;
	if(capabilities.RayTracing.SupportsBindlessAccelerationStructures && device->GetBindlessHeaps() != nullptr)
	{
		bindlessPipeline = CreateTestInlineRayTracingBindlessPipeline(*device);
		B3D_TEST_ASSERT(bindlessPipeline != nullptr)
	}

	GetRenderThread().PostCommand([this, &device, &pipeline, &bindlessPipeline]()
	{
		const GpuRayTracingCapabilities& rayTracing = device->GetCapabilities().RayTracing;
		const GpuBufferFlags inputFlags = GpuBufferFlag::AccelerationStructureInput;

		// One triangle at Z = 5, three vertices 16 bytes apart, followed by a quad of two indexed triangles at Z = 6, four vertices 12 bytes
		// apart
		static constexpr u32 kTriangleVerticesSize = 3 * 16;
		const float vertexData[] =
		{
			0.0f, 0.0f, 5.0f, 0.0f,		1.0f, 0.0f, 5.0f, 0.0f,		0.0f, 1.0f, 5.0f, 0.0f,
			2.0f, 0.0f, 6.0f,		3.0f, 0.0f, 6.0f,		2.0f, 1.0f, 6.0f,		3.0f, 1.0f, 6.0f
		};

		const u16 indexData[] = { 0, 1, 2, 1, 3, 2 };

		const TShared<render::GpuBuffer> vertices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(16, 6, inputFlags, GpuBufferMemory::Mapped));
		const TShared<render::GpuBuffer> indices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateIndex(IT_16BIT, 6, inputFlags, GpuBufferMemory::Mapped));
		B3D_TEST_ASSERT(vertices != nullptr && indices != nullptr)
		if(vertices == nullptr || indices == nullptr)
			return;

		// The host writes are made visible by the submission
		{
			const render::GpuBufferMappedScope vertexMapping = vertices->Map(GpuMapOption::Write);
			const render::GpuBufferMappedScope indexMapping = indices->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(vertexMapping.IsValid() && indexMapping.IsValid())
			if(!vertexMapping.IsValid() || !indexMapping.IsValid())
				return;

			std::memcpy(vertexMapping.GetMappedMemory(), vertexData, sizeof(vertexData));
			std::memcpy(indexMapping.GetMappedMemory(), indexData, sizeof(indexData));
		}

		GpuRayTracingGeometry quad(vertices, 12, 4, indices, 6, IT_16BIT);
		quad.VertexOffset = kTriangleVerticesSize;
		const GpuRayTracingGeometry opaqueGeometries[] = { GpuRayTracingGeometry(vertices, 16, 3), quad };

		GpuRayTracingGeometry nonOpaqueTriangle(vertices, 16, 3);
		nonOpaqueTriangle.Flags = GpuRayTracingGeometryFlag::None;

		const GpuAccelerationStructureInputs opaqueInputs(TArrayView<const GpuRayTracingGeometry>(opaqueGeometries, 2), GpuAccelerationStructureBuildFlag::PreferFastTrace);
		const GpuAccelerationStructureInputs nonOpaqueInputs(TArrayView<const GpuRayTracingGeometry>(&nonOpaqueTriangle, 1));
		GpuAccelerationStructureSizes opaqueSizes;
		GpuAccelerationStructureSizes nonOpaqueSizes;
		B3D_TEST_ASSERT(device->GetAccelerationStructureSizes(opaqueInputs, opaqueSizes) && device->GetAccelerationStructureSizes(nonOpaqueInputs, nonOpaqueSizes))

		TShared<GpuAccelerationStructure> opaqueBottomLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::BottomLevel, opaqueSizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "OpaqueBottomLevel"));
		TShared<GpuAccelerationStructure> nonOpaqueBottomLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::BottomLevel, nonOpaqueSizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "NonOpaqueBottomLevel"));
		B3D_TEST_ASSERT(opaqueBottomLevel != nullptr && nonOpaqueBottomLevel != nullptr)
		if(opaqueBottomLevel == nullptr || nonOpaqueBottomLevel == nullptr)
			return;

		// Instances 10 units apart along Y, each with its own mask bit or flags
		const auto fnCreateInstance = [](const TShared<GpuAccelerationStructure>& bottomLevel, float offsetY, u32 instanceId, u32 mask, GpuRayTracingInstanceFlags flags)
		{
			GpuRayTracingInstance instance(bottomLevel.get(), Matrix4::Translation(Vector3(0.0f, offsetY, 0.0f)), instanceId);
			instance.Mask = (u8)mask;
			instance.Flags = flags;
			return instance;
		};

		static constexpr u32 kInstanceCount = 6;
		const GpuRayTracingInstance instanceDescriptions[kInstanceCount] =
		{
			fnCreateInstance(opaqueBottomLevel, 0.0f, 7, 0x01, GpuRayTracingInstanceFlag::None),
			fnCreateInstance(opaqueBottomLevel, 10.0f, 9, 0x02, GpuRayTracingInstanceFlag::FlipTriangleFacing),
			fnCreateInstance(opaqueBottomLevel, 20.0f, 11, 0x04, GpuRayTracingInstanceFlag::DisableTriangleCulling),
			fnCreateInstance(nonOpaqueBottomLevel, 30.0f, 13, 0x08, GpuRayTracingInstanceFlag::None),
			fnCreateInstance(nonOpaqueBottomLevel, 40.0f, 15, 0x08, GpuRayTracingInstanceFlag::ForceOpaque),
			fnCreateInstance(opaqueBottomLevel, 50.0f, 17, 0x08, GpuRayTracingInstanceFlag::ForceNonOpaque)
		};

		const TShared<render::GpuBuffer> instances = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(rayTracing.InstanceSize, kInstanceCount, inputFlags, GpuBufferMemory::Mapped));
		B3D_TEST_ASSERT(instances != nullptr)
		if(instances == nullptr)
			return;

		{
			const render::GpuBufferMappedScope mapping = instances->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			B3D_TEST_ASSERT(device->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(instanceDescriptions, kInstanceCount), mapping.GetMappedMemory()))
		}

		// The structure the rays trace against comes from the heap too where the device can put it there
		const GpuAccelerationStructureInputs topLevelInputs(instances, kInstanceCount);
		const GpuAccelerationStructureInputs emptyInputs(TShared<render::GpuBuffer>(), 0);
		GpuAccelerationStructureSizes topLevelSizes;
		GpuAccelerationStructureSizes emptySizes;
		B3D_TEST_ASSERT(device->GetAccelerationStructureSizes(topLevelInputs, topLevelSizes) && device->GetAccelerationStructureSizes(emptyInputs, emptySizes))

		const GpuAccelerationStructureFlags topLevelFlags = bindlessPipeline != nullptr ? GpuAccelerationStructureFlag::Bindless : GpuAccelerationStructureFlag::None;
		const TShared<GpuAccelerationStructure> topLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::TopLevel, topLevelSizes.AccelerationStructureSize, topLevelFlags, "TopLevel"));
		const TShared<GpuAccelerationStructure> emptyTopLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::TopLevel, emptySizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "EmptyTopLevel"));
		B3D_TEST_ASSERT(topLevel != nullptr && emptyTopLevel != nullptr)
		if(topLevel == nullptr || emptyTopLevel == nullptr)
			return;

		B3D_TEST_ASSERT(topLevel->GetType() == GpuAccelerationStructureType::TopLevel && topLevel->GetSize() == topLevelSizes.AccelerationStructureSize)
		B3D_TEST_ASSERT((topLevel->GetBindlessIndex() != kGpuBindlessInvalidIndex) == (bindlessPipeline != nullptr))

		// The builds of one call use separate ranges of one scratch buffer, which the next call reuses
		const u32 scratchAlignment = rayTracing.ScratchOffsetAlignment;
		const u32 nonOpaqueScratchOffset = AlignUp(opaqueSizes.BuildScratchSize, scratchAlignment);
		const u32 emptyScratchOffset = AlignUp(topLevelSizes.BuildScratchSize, scratchAlignment);
		const u32 scratchSize = std::max(nonOpaqueScratchOffset + nonOpaqueSizes.BuildScratchSize, emptyScratchOffset + emptySizes.BuildScratchSize);
		const TShared<render::GpuBuffer> scratch = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, AlignUp(scratchSize, 4) / 4,
			GpuBufferFlag::GpuWrite));
		B3D_TEST_ASSERT(scratch != nullptr)
		if(scratch == nullptr)
			return;

		const GpuAccessState buildWrite(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Write);
		const GpuAccessState scratchAccess(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Read | GpuAccessFlag::Write);
		const GpuAccessState bottomLevelRest = GpuBackendUtility::GetRestingState(*opaqueBottomLevel, GQT_GRAPHICS);
		const GpuAccessState topLevelRest = GpuBackendUtility::GetRestingState(*topLevel, GQT_GRAPHICS);
		const GpuAccessState scratchRest = GpuBackendUtility::GetRestingState(*scratch, GQT_GRAPHICS);

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);

		{
			const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Acceleration structure builds"));

			const GpuAccelerationStructureBarrier bottomLevelBarriers[] =
			{
				GpuAccelerationStructureBarrier(opaqueBottomLevel, GpuAccessState(), buildWrite),
				GpuAccelerationStructureBarrier(nonOpaqueBottomLevel, GpuAccessState(), buildWrite)
			};

			const GpuBufferBarrier scratchBarrier(scratch, GpuAccessState(), scratchAccess);
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchBarrier, 1), {}, {},
				TArrayView<const GpuAccelerationStructureBarrier>(bottomLevelBarriers, 2)));

			const GpuAccelerationStructureBuild bottomLevelBuilds[] =
			{
				GpuAccelerationStructureBuild(opaqueInputs, opaqueBottomLevel, scratch),
				GpuAccelerationStructureBuild(nonOpaqueInputs, nonOpaqueBottomLevel, scratch, nonOpaqueScratchOffset)
			};

			B3D_TEST_ASSERT(commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(bottomLevelBuilds, 2)))

			const GpuAccelerationStructureBarrier topLevelBarriers[] =
			{
				GpuAccelerationStructureBarrier(opaqueBottomLevel, buildWrite, bottomLevelRest),
				GpuAccelerationStructureBarrier(nonOpaqueBottomLevel, buildWrite, bottomLevelRest),
				GpuAccelerationStructureBarrier(topLevel, GpuAccessState(), buildWrite),
				GpuAccelerationStructureBarrier(emptyTopLevel, GpuAccessState(), buildWrite)
			};

			const GpuBufferBarrier scratchReuseBarrier(scratch, scratchAccess, scratchAccess);
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchReuseBarrier, 1), {}, {},
				TArrayView<const GpuAccelerationStructureBarrier>(topLevelBarriers, 4)));

			const TShared<GpuAccelerationStructure> bottomLevels[] = { opaqueBottomLevel, nonOpaqueBottomLevel };
			GpuAccelerationStructureBuild topLevelBuild(topLevelInputs, topLevel, scratch);
			topLevelBuild.BottomLevels = TArrayView<const TShared<GpuAccelerationStructure>>(bottomLevels, 2);

			const GpuAccelerationStructureBuild topLevelBuilds[] = { topLevelBuild, GpuAccelerationStructureBuild(emptyInputs, emptyTopLevel, scratch, emptyScratchOffset) };
			B3D_TEST_ASSERT(commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(topLevelBuilds, 2)))

			const GpuAccelerationStructureBarrier restBarriers[] =
			{
				GpuAccelerationStructureBarrier(topLevel, buildWrite, topLevelRest),
				GpuAccelerationStructureBarrier(emptyTopLevel, buildWrite, topLevelRest)
			};

			const GpuBufferBarrier scratchRestBarrier(scratch, scratchAccess, scratchRest);
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchRestBarrier, 1), {}, {},
				TArrayView<const GpuAccelerationStructureBarrier>(restBarriers, 2)));
			context->SubmitCommandBuffer(commands);
			device->WaitUntilIdle();
		}

		// The top level structure keeps the bottom levels alive after everything else releases them
		opaqueBottomLevel = nullptr;
		nonOpaqueBottomLevel = nullptr;

		// A viewer at the origin looking along +Z sees the vertices of every triangle in clockwise order, since +X points to its left in the
		// right-handed world. Rasterization would show their front faces.
		static constexpr u32 kForceOpaque = 0x01;
		static constexpr u32 kCullBackFacingTriangles = 0x10;
		static constexpr u32 kCullFrontFacingTriangles = 0x20;
		const Vector3 forward(0.0f, 0.0f, 1.0f);
		const Vector3 backward(0.0f, 0.0f, -1.0f);

		const TestRay rays[] =
		{
			TestRay(Vector3(0.25f, 0.25f, 0.0f), forward),
			TestRay(Vector3(2.75f, 0.75f, 0.0f), forward),
			TestRay(Vector3(2.25f, 0.25f, 0.0f), forward),
			TestRay(Vector3(0.25f, 0.25f, 10.0f), backward),
			TestRay(Vector3(0.25f, 10.25f, 0.0f), forward),
			TestRay(Vector3(0.25f, 10.25f, 0.0f), forward, 0, 0x01),
			TestRay(Vector3(0.25f, 0.25f, 0.0f), forward, kCullBackFacingTriangles),
			TestRay(Vector3(0.25f, 0.25f, 0.0f), forward, kCullFrontFacingTriangles),
			TestRay(Vector3(0.25f, 10.25f, 0.0f), forward, kCullBackFacingTriangles),
			TestRay(Vector3(0.25f, 20.25f, 0.0f), forward, kCullFrontFacingTriangles),
			TestRay(Vector3(0.25f, 0.25f, 0.0f), forward, 0, 0xFF, 0, 4.0f),
			TestRay(Vector3(0.25f, 0.25f, 0.0f), forward, 0, 0xFF, 1),
			TestRay(Vector3(0.25f, 30.25f, 0.0f), forward, 0, 0x08, 2),
			TestRay(Vector3(0.25f, 30.25f, 0.0f), forward, 0, 0x08, 0),
			TestRay(Vector3(0.25f, 40.25f, 0.0f), forward, 0, 0x08, 2),
			TestRay(Vector3(0.25f, 50.25f, 0.0f), forward, 0, 0x08, 2),
			TestRay(Vector3(0.25f, 30.25f, 0.0f), forward, kForceOpaque, 0x08, 2)
		};

		const ExpectedHit expectedHits[] =
		{
			ExpectedHit(5.0f, 0.25f, 0.25f, 0, 7, 0, 0, true), // Closest hit on the triangle
			ExpectedHit(6.0f, 0.5f, 0.25f, 0, 7, 1, 1, true), // Second triangle of the indexed quad
			ExpectedHit(6.0f, 0.25f, 0.25f, 0, 7, 1, 0, true), // First triangle of the indexed quad
			ExpectedHit(5.0f, 0.25f, 0.25f, 0, 7, 0, 0, false), // Back face, seen from behind
			ExpectedHit(5.0f, 0.25f, 0.25f, 1, 9, 0, 0, false), // Translated instance with flipped facing
			ExpectedHit(), // Instance mask excludes the instance
			ExpectedHit(5.0f, 0.25f, 0.25f, 0, 7, 0, 0, true), // Culling back faces keeps the front face
			ExpectedHit(), // Culling front faces
			ExpectedHit(), // Culling back faces, which the flipped instance shows
			ExpectedHit(5.0f, 0.25f, 0.25f, 2, 11, 0, 0, true), // The instance disables culling
			ExpectedHit(), // The hit is past the maximum distance
			ExpectedHit(5.0f, 0.25f, 0.25f, 0, 7, 0, 0, true), // Any hit
			ExpectedHit(1), // The callback rejects the non-opaque triangle
			ExpectedHit(5.0f, 0.25f, 0.25f, 3, 13, 0, 0, true), // Without callbacks, non-opaque triangles are hits
			ExpectedHit(5.0f, 0.25f, 0.25f, 4, 15, 0, 0, true), // The instance forces the non-opaque triangle to be opaque
			ExpectedHit(1), // The instance forces the opaque triangle to be non-opaque, and the callback rejects it
			ExpectedHit(5.0f, 0.25f, 0.25f, 3, 13, 0, 0, true) // The ray forces the non-opaque triangle to be opaque
		};

		static constexpr u32 kRayCount = sizeof(rays) / sizeof(rays[0]);
		static_assert(kRayCount == sizeof(expectedHits) / sizeof(expectedHits[0]), "Every ray needs an expected hit.");

		const TShared<render::GpuBuffer> rayBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(TestRay), kRayCount,
			GpuBufferFlag::None, GpuBufferMemory::Mapped));
		const TShared<render::GpuBuffer> hitBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(TestHit), kRayCount,
			GpuBufferFlag::GpuWrite));
		const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kRayCount * sizeof(TestHit)));
		B3D_TEST_ASSERT(rayBuffer != nullptr && hitBuffer != nullptr && readback != nullptr)
		if(rayBuffer == nullptr || hitBuffer == nullptr || readback == nullptr)
			return;

		{
			const render::GpuBufferMappedScope mapping = rayBuffer->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			std::memcpy(mapping.GetMappedMemory(), rays, sizeof(rays));
		}

		const TShared<render::GpuParameterSet> parameterSet = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
		B3D_TEST_ASSERT(parameterSet->SetAccelerationStructure("Scene", topLevel))
		parameterSet->SetStorageBuffer("Rays", rayBuffer);
		parameterSet->SetStorageBuffer("Hits", hitBuffer);

		const GpuAccessState computeWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);
		GpuAccessState hitBufferState;
		GpuAccessState readbackState;

		// Traces every ray and returns what the rays hit
		const auto fnTrace = [&](const char* name)
		{
			Vector<TestHit> hits(kRayCount);

			const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create(name));
			commands->IssueBarriers(GpuBufferBarrier(hitBuffer, hitBufferState, computeWrite));
			commands->SetGpuComputePipelineState(pipeline);
			commands->SetGpuParameterSet(parameterSet);
			commands->DispatchCompute(kRayCount);

			const GpuBufferBarrier copyBarriers[] = { GpuBufferBarrier(hitBuffer, computeWrite, transferRead), GpuBufferBarrier(readback, readbackState, transferWrite) };
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(copyBarriers, 2)));
			commands->CopyBufferToBuffer(hitBuffer, readback, 0, 0, kRayCount * sizeof(TestHit));
			commands->IssueBarriers(GpuBufferBarrier(readback, transferWrite, hostRead));
			context->SubmitCommandBuffer(commands);
			device->WaitUntilIdle();

			hitBufferState = transferRead;
			readbackState = hostRead;

			const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(mapping.IsValid())
				std::memcpy(hits.data(), mapping.GetMappedMemory(), kRayCount * sizeof(TestHit));

			return hits;
		};

		const Vector<TestHit> hits = fnTrace("Inline ray tracing");
		static constexpr float kTolerance = 1e-4f;
		for(u32 rayIndex = 0; rayIndex < kRayCount; rayIndex++)
		{
			const TestHit& actual = hits[rayIndex];
			const ExpectedHit& expected = expectedHits[rayIndex];

			const String message = StringUtility::Format("Ray {0}: hit {1}, distance {2}, barycentrics ({3}, {4}), instance {5} (id {6}), geometry {7}, primitive {8}, "
				"front face {9}, callbacks {10}.", rayIndex, actual.IsHit, actual.Distance, actual.Barycentrics[0], actual.Barycentrics[1], actual.InstanceIndex,
				actual.InstanceId, actual.GeometryIndex, actual.PrimitiveIndex, actual.FrontFace, actual.CallbackCount);

			B3D_TEST_ASSERT_MSG((actual.IsHit != 0.0f) == expected.IsHit, message)
			B3D_TEST_ASSERT_MSG(actual.CallbackCount == expected.CallbackCount, message)
			if(!expected.IsHit)
				continue;

			B3D_TEST_ASSERT_MSG(std::abs(actual.Distance - expected.Distance) < kTolerance, message)
			B3D_TEST_ASSERT_MSG(std::abs(actual.Barycentrics[0] - expected.BarycentricX) < kTolerance && std::abs(actual.Barycentrics[1] - expected.BarycentricY) < kTolerance, message)
			B3D_TEST_ASSERT_MSG(actual.InstanceIndex == expected.InstanceIndex && actual.InstanceId == expected.InstanceId, message)
			B3D_TEST_ASSERT_MSG(actual.GeometryIndex == expected.GeometryIndex && actual.PrimitiveIndex == expected.PrimitiveIndex, message)
			B3D_TEST_ASSERT_MSG((actual.FrontFace != 0) == expected.FrontFace, message)
		}

		// A top level structure without instances is never hit
		B3D_TEST_ASSERT(parameterSet->SetAccelerationStructure("Scene", emptyTopLevel))
		const Vector<TestHit> emptyHits = fnTrace("Inline ray tracing against an empty structure");
		for(u32 rayIndex = 0; rayIndex < kRayCount; rayIndex++)
			B3D_TEST_ASSERT_MSG(emptyHits[rayIndex].IsHit == 0.0f, StringUtility::Format("Ray {0} hit an empty structure.", rayIndex))

		if(bindlessPipeline == nullptr)
			return;

		// The top level structure taken from the heap
		const TShared<render::GpuBuffer> bindlessHit = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4 * sizeof(float), 1,
			GpuBufferFlag::GpuWrite));
		const TShared<render::GpuBuffer> bindlessReadback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(4 * sizeof(float)));
		B3D_TEST_ASSERT(bindlessHit != nullptr && bindlessReadback != nullptr)
		if(bindlessHit == nullptr || bindlessReadback == nullptr)
			return;

		const TShared<render::GpuParameterSet> bindlessParameterSet = context->GetParameterSetPool().Create(bindlessPipeline->GetParameterLayout()->GetSet(0), 0);
		bindlessParameterSet->SetStorageBuffer("Hit", bindlessHit);

		const u32 sceneIndex = topLevel->GetBindlessIndex();
		const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Inline ray tracing through the heap"));
		commands->IssueBarriers(GpuBufferBarrier(bindlessHit, GpuAccessState(), computeWrite));
		commands->SetGpuComputePipelineState(bindlessPipeline);
		commands->SetGpuParameterSet(bindlessParameterSet);
		commands->SetPushConstants(0, sizeof(sceneIndex), &sceneIndex);
		commands->DispatchCompute(1);

		const GpuBufferBarrier copyBarriers[] = { GpuBufferBarrier(bindlessHit, computeWrite, transferRead), GpuBufferBarrier(bindlessReadback, GpuAccessState(), transferWrite) };
		commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(copyBarriers, 2)));
		commands->CopyBufferToBuffer(bindlessHit, bindlessReadback, 0, 0, 4 * sizeof(float));
		commands->IssueBarriers(GpuBufferBarrier(bindlessReadback, transferWrite, hostRead));
		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();

		const render::GpuBufferMappedScope mapping = bindlessReadback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		// Hit, distance, instance id and instance index of the ray that hits the instance with flipped facing
		const float* actual = static_cast<const float*>(mapping.GetMappedMemory());
		B3D_TEST_ASSERT_MSG(actual[0] == 1.0f && std::abs(actual[1] - 5.0f) < kTolerance && actual[2] == 9.0f && actual[3] == 1.0f,
			StringUtility::Format("Heap ray: hit {0}, distance {1}, instance id {2}, instance index {3}.", actual[0], actual[1], actual[2], actual[3]))
	}, "GpuRayTracingTestSuite::TestInlineRayTracing", true);
}

namespace
{
	/**
	 * Returns the pipeline of the RayTracingPipeline test shader: ray generation programs TraceTestRays and WriteIndices, miss programs MissA
	 * and MissB, and hit groups of ClosestHitA, ClosestHitB and ClosestHitC, in that order.
	 */
	GpuRayTracingPipelineStateCreateInformation GetTestPipelineInformation(UnorderedMap<String, TShared<GpuProgram>>& programs)
	{
		GpuRayTracingPipelineStateCreateInformation information;
		information.RayGenerationPrograms.push_back(programs["TraceTestRays"]);
		information.RayGenerationPrograms.push_back(programs["WriteIndices"]);
		information.MissPrograms.push_back(programs["MissA"]);
		information.MissPrograms.push_back(programs["MissB"]);
		information.HitGroups.push_back(GpuRayTracingHitGroup(programs["ClosestHitA"]));
		information.HitGroups.push_back(GpuRayTracingHitGroup(programs["ClosestHitB"]));
		information.HitGroups.push_back(GpuRayTracingHitGroup(programs["ClosestHitC"]));

		return information;
	}
} // namespace

void GpuRayTracingTestSuite::TestRayTracingPipelineCreation()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const GpuDeviceCapabilities& capabilities = device->GetCapabilities();
	if(!capabilities.HasCapability(RSC_RAY_TRACING_PIPELINE))
	{
		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot create a ray tracing pipeline, the GPU device doesn't support ray tracing pipelines.");
			B3D_TEST_ASSERT(device->CreateGpuRayTracingPipelineState(GpuRayTracingPipelineStateCreateInformation()) == nullptr)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot create a shader binding table, the GPU device doesn't support ray tracing pipelines.");
			B3D_TEST_ASSERT(device->CreateGpuShaderBindingTable(GpuShaderBindingTableCreateInformation()) == nullptr)
		}

		GetRenderThread().PostCommand([this, &device]()
		{
			const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
			const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
			const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Unsupported ray dispatch"));

			{
				LoggingScope logs(*this);
				logs.ExpectError("Cannot dispatch rays, the GPU device doesn't support ray tracing pipelines.");
				B3D_TEST_ASSERT(!commands->DispatchRays(nullptr, 0, 1, 1))
			}

			context->SubmitCommandBuffer(commands);
			device->WaitUntilIdle();
		}, "GpuRayTracingTestSuite::TestRayTracingPipelineCreation", true);

		return;
	}

	// Shaders are resources, which are created outside of the render thread
	UnorderedMap<String, TShared<GpuProgram>> programs = CreateTestRayTracingPrograms(*device, "RayTracingPipeline");
	UnorderedMap<String, TShared<GpuProgram>> otherPrograms = CreateTestRayTracingPrograms(*device, "RayTracingOtherParameters");
	bool areProgramsCreated = true;
	const auto fnCheckPrograms = [this, &areProgramsCreated](UnorderedMap<String, TShared<GpuProgram>>& shaderPrograms, std::initializer_list<const char*> entryPoints)
	{
		for(const char* entryPoint : entryPoints)
		{
			const auto found = shaderPrograms.find(entryPoint);
			const bool isCreated = found != shaderPrograms.end() && found->second != nullptr && found->second->IsCompiled();
			B3D_TEST_ASSERT_MSG(isCreated, StringUtility::Format("Program {0} wasn't created.", entryPoint))
			areProgramsCreated &= isCreated;
		}
	};

	fnCheckPrograms(programs, { "TraceTestRays", "WriteIndices", "MissA", "MissB", "ClosestHitA", "ClosestHitB", "ClosestHitC" });
	fnCheckPrograms(otherPrograms, { "WriteZero" });
	if(!areProgramsCreated)
		return;

	const GpuRayTracingPipelineStateCreateInformation validInformation = GetTestPipelineInformation(programs);

	// Pipeline creation errors, with the program each one names
	const auto fnExpectPipelineError = [this, &device](const GpuRayTracingPipelineStateCreateInformation& information, const String& error)
	{
		LoggingScope logs(*this);
		logs.ExpectError(error);
		B3D_TEST_ASSERT_MSG(device->CreateGpuRayTracingPipelineState(information) == nullptr, error)
	};

	{
		GpuRayTracingPipelineStateCreateInformation information = validInformation;
		information.RayGenerationPrograms.clear();
		fnExpectPipelineError(information, "A ray tracing pipeline needs at least one ray generation program.");
	}

	{
		GpuRayTracingPipelineStateCreateInformation information = validInformation;
		information.MissPrograms.push_back(nullptr);
		fnExpectPipelineError(information, "Miss program 2 of a ray tracing pipeline is null.");
	}

	{
		GpuRayTracingPipelineStateCreateInformation information = validInformation;
		information.RayGenerationPrograms.push_back(programs["MissA"]);
		fnExpectPipelineError(information, StringUtility::Format("Ray generation program 2 of a ray tracing pipeline, '{0}', is a Miss program.", programs["MissA"]->GetName()));
	}

	{
		GpuRayTracingPipelineStateCreateInformation information = validInformation;
		information.HitGroups[0].AnyHitProgram = programs["ClosestHitB"];
		fnExpectPipelineError(information, StringUtility::Format("Any hit program of hit group 0 of a ray tracing pipeline, '{0}', is a Closest Hit program.",
			programs["ClosestHitB"]->GetName()));
	}

	{
		GpuRayTracingPipelineStateCreateInformation information = validInformation;
		information.HitGroups.push_back(GpuRayTracingHitGroup());
		fnExpectPipelineError(information, "Hit group 3 of a ray tracing pipeline has no program.");
	}

	const u32 maximumRecursionDepth = capabilities.RayTracing.MaximumRecursionDepth;
	for(const u32 recursionDepth : { 0u, maximumRecursionDepth + 1 })
	{
		GpuRayTracingPipelineStateCreateInformation information = validInformation;
		information.MaximumRecursionDepth = recursionDepth;
		fnExpectPipelineError(information, StringUtility::Format("A ray tracing pipeline has maximum recursion depth {0}, which must be between 1 and {1}.",
			recursionDepth, maximumRecursionDepth));
	}

	// Cooked shaders can't have hit attributes over 32 bytes, so the program gets a copy of its reflection with larger attributes than the
	// device allows
	{
		UnorderedMap<String, GpuProgramCreateInformation> programInformation = GetTestRayTracingProgramInformation("RayTracingPipeline");
		GpuProgramCreateInformation& closestHitInformation = programInformation["ClosestHitA"];
		B3D_TEST_ASSERT(closestHitInformation.ShaderReflection != nullptr)
		if(closestHitInformation.ShaderReflection != nullptr)
		{
			const u32 hitAttributeSize = capabilities.RayTracing.MaximumHitAttributeSize + 4;
			const TShared<ShaderReflection> reflection = B3DMakeShared<ShaderReflection>(*closestHitInformation.ShaderReflection);
			reflection->EntryPoints[closestHitInformation.EntryPoint].HitAttributeSize = hitAttributeSize;
			closestHitInformation.ShaderReflection = reflection;

			const TShared<GpuProgram> largeAttributeProgram = device->CreateGpuProgram(closestHitInformation);
			GpuRayTracingPipelineStateCreateInformation information = validInformation;
			information.HitGroups[1].ClosestHitProgram = largeAttributeProgram;
			fnExpectPipelineError(information, StringUtility::Format("Closest hit program of hit group 1 of a ray tracing pipeline, '{0}', has {1} bytes of hit "
				"attributes. The maximum is {2}.", largeAttributeProgram->GetName(), hitAttributeSize, capabilities.RayTracing.MaximumHitAttributeSize));
		}
	}

	{
		GpuRayTracingPipelineStateCreateInformation information = validInformation;
		information.RayGenerationPrograms.push_back(otherPrograms["WriteZero"]);
		fnExpectPipelineError(information, StringUtility::Format("Ray generation program 2 of a ray tracing pipeline, '{0}', declares other parameters than '{1}'. "
			"Every program of a pipeline must declare the same parameters.", otherPrograms["WriteZero"]->GetName(), programs["TraceTestRays"]->GetName()));
	}

	// The pipeline takes the largest payload (one uint) and attributes (two barycentrics) of its programs
	const TShared<GpuRayTracingPipelineState> pipeline = device->CreateGpuRayTracingPipelineState(validInformation);
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	B3D_TEST_ASSERT(pipeline->GetParameterLayout() != nullptr)
	B3D_TEST_ASSERT_MSG(pipeline->GetMaximumPayloadSize() == 4 && pipeline->GetMaximumAttributeSize() == 8,
		StringUtility::Format("Payload {0} bytes, attributes {1} bytes.", pipeline->GetMaximumPayloadSize(), pipeline->GetMaximumAttributeSize()))

	// Shader binding table creation errors
	GpuShaderBindingTableCreateInformation validTable;
	validTable.Pipeline = pipeline;
	validTable.RayGenerationRecords.push_back(0);
	validTable.MissRecords.push_back(0);
	validTable.HitGroupRecords.push_back(0);
	validTable.HitGroupRecords.push_back(kGpuShaderBindingTableEmptyRecord);

	const auto fnExpectTableError = [this, &device](const GpuShaderBindingTableCreateInformation& information, const String& error)
	{
		LoggingScope logs(*this);
		logs.ExpectError(error);
		B3D_TEST_ASSERT_MSG(device->CreateGpuShaderBindingTable(information) == nullptr, error)
	};

	{
		GpuShaderBindingTableCreateInformation information = validTable;
		information.Pipeline = nullptr;
		fnExpectTableError(information, "A shader binding table needs a pipeline.");
	}

	{
		GpuShaderBindingTableCreateInformation information = validTable;
		information.RayGenerationRecords.clear();
		fnExpectTableError(information, "A shader binding table needs at least one ray generation record.");
	}

	{
		GpuShaderBindingTableCreateInformation information = validTable;
		information.RayGenerationRecords.push_back(2);
		fnExpectTableError(information, "Ray generation record 1 of a shader binding table refers to ray generation program 2, but the pipeline has 2.");
	}

	{
		GpuShaderBindingTableCreateInformation information = validTable;
		information.MissRecords.push_back(kGpuShaderBindingTableEmptyRecord);
		fnExpectTableError(information, StringUtility::Format("Miss record 1 of a shader binding table refers to miss program {0}, but the pipeline has 2.",
			kGpuShaderBindingTableEmptyRecord));
	}

	{
		GpuShaderBindingTableCreateInformation information = validTable;
		information.HitGroupRecords.push_back(3);
		fnExpectTableError(information, "Hit group record 2 of a shader binding table refers to hit group 3, but the pipeline has 3.");
	}

	{
		GpuShaderBindingTableCreateInformation information = validTable;
		information.CallableRecords.push_back(0);
		fnExpectTableError(information, "Callable record 0 of a shader binding table refers to callable program 0, but the pipeline has 0.");
	}

	TShared<render::GpuShaderBindingTable> table;
	{
		// The first table of the process warns when the CPU can't write the GPU's memory.
		LoggingScope logs(*this);
		logs.IgnoreWarning("Shader binding tables are in system memory");
		table = device->CreateGpuShaderBindingTable(validTable);
	}

	B3D_TEST_ASSERT(table != nullptr && table->GetPipeline() == pipeline)
	if(table == nullptr)
		return;

	// Dispatch errors, caught before anything is recorded
	GetRenderThread().PostCommand([this, &device, &table]()
	{
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Invalid ray dispatches"));

		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot dispatch rays without a shader binding table.");
			B3D_TEST_ASSERT(!commands->DispatchRays(nullptr, 0, 1, 1))
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot dispatch rays with ray generation record 1, the shader binding table has 1.");
			B3D_TEST_ASSERT(!commands->DispatchRays(table, 1, 1, 1))
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Cannot dispatch rays over an empty grid (4 x 0 x 1).");
			B3D_TEST_ASSERT(!commands->DispatchRays(table, 0, 4, 0))
		}

		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();
	}, "GpuRayTracingTestSuite::TestRayTracingPipelineCreation", true);
}

namespace
{
	/** Ray traced by the TraceTestRays program of the RayTracingPipeline test shader, in the layout of its TestRay structure. */
	struct PipelineTestRay
	{
		PipelineTestRay() = default;

		/**
		 * @param	origin					Origin of the ray, which travels along +Z.
		 * @param	hitGroupContribution	RayContributionToHitGroupIndex of TraceRay().
		 * @param	geometryMultiplier		MultiplierForGeometryContributionToHitGroupIndex of TraceRay().
		 * @param	missIndex				MissShaderIndex of TraceRay().
		 */
		PipelineTestRay(const Vector3& origin, u32 hitGroupContribution, u32 geometryMultiplier, u32 missIndex)
			: Origin(origin), Direction(0.0f, 0.0f, 1.0f), HitGroupContribution(hitGroupContribution), GeometryMultiplier(geometryMultiplier), MissIndex(missIndex)
		{ }

		Vector3 Origin = Vector3::kZero;
		float OriginPadding = 0.0f;
		Vector3 Direction = Vector3::kZero;
		float DirectionPadding = 0.0f;
		u32 HitGroupContribution = 0;
		u32 GeometryMultiplier = 0;
		u32 MissIndex = 0;
		u32 Padding = 0;
	};

	static_assert(sizeof(PipelineTestRay) == 48, "PipelineTestRay must match the layout of the shader's structure.");
} // namespace

void GpuRayTracingTestSuite::TestRayTracingPipelineRecords()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const GpuDeviceCapabilities& capabilities = device->GetCapabilities();
	if(!capabilities.HasCapability(RSC_ACCELERATION_STRUCTURES) || !capabilities.HasCapability(RSC_RAY_TRACING_PIPELINE))
		return;

	// Shaders are resources, which are created outside of the render thread
	UnorderedMap<String, TShared<GpuProgram>> programs = CreateTestRayTracingPrograms(*device, "RayTracingPipeline");
	B3D_TEST_ASSERT(programs.size() == 7)
	if(programs.size() != 7)
		return;

	const TShared<GpuRayTracingPipelineState> pipeline = device->CreateGpuRayTracingPipelineState(GetTestPipelineInformation(programs));
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	// Miss records in reverse order of the programs, and hit group records 0 to 3 run ClosestHitA, ClosestHitB, nothing and ClosestHitC
	GpuShaderBindingTableCreateInformation tableInformation;
	tableInformation.Pipeline = pipeline;
	tableInformation.RayGenerationRecords.push_back(0);
	tableInformation.RayGenerationRecords.push_back(1);
	tableInformation.MissRecords.push_back(1);
	tableInformation.MissRecords.push_back(0);
	tableInformation.HitGroupRecords.push_back(0);
	tableInformation.HitGroupRecords.push_back(1);
	tableInformation.HitGroupRecords.push_back(kGpuShaderBindingTableEmptyRecord);
	tableInformation.HitGroupRecords.push_back(2);

	TShared<render::GpuShaderBindingTable> table;
	{
		// The first table of the process warns when the CPU can't write the GPU's memory.
		LoggingScope logs(*this);
		logs.IgnoreWarning("Shader binding tables are in system memory");
		table = device->CreateGpuShaderBindingTable(tableInformation);
	}

	B3D_TEST_ASSERT(table != nullptr)
	if(table == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device, &pipeline, &table]()
	{
		const GpuRayTracingCapabilities& rayTracing = device->GetCapabilities().RayTracing;
		const GpuBufferFlags inputFlags = GpuBufferFlag::AccelerationStructureInput;

		// Geometry 0 is a triangle at X 0 to 1, and geometry 1 a triangle at X 2 to 3, both at Z = 5
		const float vertexData[] =
		{
			0.0f, 0.0f, 5.0f,		1.0f, 0.0f, 5.0f,		0.0f, 1.0f, 5.0f,
			2.0f, 0.0f, 5.0f,		3.0f, 0.0f, 5.0f,		2.0f, 1.0f, 5.0f
		};

		const TShared<render::GpuBuffer> vertices = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(12, 6, inputFlags, GpuBufferMemory::Mapped));
		B3D_TEST_ASSERT(vertices != nullptr)
		if(vertices == nullptr)
			return;

		{
			const render::GpuBufferMappedScope mapping = vertices->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			std::memcpy(mapping.GetMappedMemory(), vertexData, sizeof(vertexData));
		}

		GpuRayTracingGeometry secondTriangle(vertices, 12, 3);
		secondTriangle.VertexOffset = 3 * 12;
		const GpuRayTracingGeometry geometries[] = { GpuRayTracingGeometry(vertices, 12, 3), secondTriangle };
		const GpuAccelerationStructureInputs bottomLevelInputs(TArrayView<const GpuRayTracingGeometry>(geometries, 2));

		GpuAccelerationStructureSizes bottomLevelSizes;
		B3D_TEST_ASSERT(device->GetAccelerationStructureSizes(bottomLevelInputs, bottomLevelSizes))
		const TShared<GpuAccelerationStructure> bottomLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::BottomLevel, bottomLevelSizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "BottomLevel"));

		// Instance 0 at Y = 0 uses the hit group records from 0, and instance 1 at Y = 10 those from 1
		GpuRayTracingInstance instanceDescriptions[] =
		{
			GpuRayTracingInstance(bottomLevel.get(), Matrix4::kIdentity, 0),
			GpuRayTracingInstance(bottomLevel.get(), Matrix4::Translation(Vector3(0.0f, 10.0f, 0.0f)), 1)
		};

		instanceDescriptions[1].HitGroupIndexOffset = 1;

		const TShared<render::GpuBuffer> instances = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(rayTracing.InstanceSize, 2, inputFlags, GpuBufferMemory::Mapped));
		B3D_TEST_ASSERT(bottomLevel != nullptr && instances != nullptr)
		if(bottomLevel == nullptr || instances == nullptr)
			return;

		{
			const render::GpuBufferMappedScope mapping = instances->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			B3D_TEST_ASSERT(device->WriteRayTracingInstances(TArrayView<const GpuRayTracingInstance>(instanceDescriptions, 2), mapping.GetMappedMemory()))
		}

		const GpuAccelerationStructureInputs topLevelInputs(instances, 2);
		GpuAccelerationStructureSizes topLevelSizes;
		B3D_TEST_ASSERT(device->GetAccelerationStructureSizes(topLevelInputs, topLevelSizes))
		const TShared<GpuAccelerationStructure> topLevel = device->CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
			GpuAccelerationStructureType::TopLevel, topLevelSizes.AccelerationStructureSize, GpuAccelerationStructureFlag::None, "TopLevel"));

		const u32 scratchSize = std::max(bottomLevelSizes.BuildScratchSize, topLevelSizes.BuildScratchSize);
		const TShared<render::GpuBuffer> scratch = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, AlignUp(scratchSize, 4) / 4,
			GpuBufferFlag::GpuWrite));
		B3D_TEST_ASSERT(topLevel != nullptr && scratch != nullptr)
		if(topLevel == nullptr || scratch == nullptr)
			return;

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);

		// The bottom level build, then the top level build, both back to rest
		{
			const GpuAccessState buildWrite(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Write);
			const GpuAccessState scratchAccess(GpuStageFlag::AccelerationStructureBuild, GpuAccessFlag::Read | GpuAccessFlag::Write);
			const GpuAccessState structureRest = GpuBackendUtility::GetRestingState(*bottomLevel, GQT_GRAPHICS);
			const GpuAccessState scratchRest = GpuBackendUtility::GetRestingState(*scratch, GQT_GRAPHICS);

			const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create("Ray tracing pipeline builds"));
			const GpuBufferBarrier scratchBarrier(scratch, GpuAccessState(), scratchAccess);
			const GpuAccelerationStructureBarrier bottomLevelBarrier(bottomLevel, GpuAccessState(), buildWrite);
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchBarrier, 1), {}, {}, TArrayView<const GpuAccelerationStructureBarrier>(&bottomLevelBarrier, 1)));

			const GpuAccelerationStructureBuild bottomLevelBuild(bottomLevelInputs, bottomLevel, scratch);
			B3D_TEST_ASSERT(commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&bottomLevelBuild, 1)))

			const GpuBufferBarrier scratchReuseBarrier(scratch, scratchAccess, scratchAccess);
			const GpuAccelerationStructureBarrier topLevelBarriers[] =
			{
				GpuAccelerationStructureBarrier(bottomLevel, buildWrite, structureRest),
				GpuAccelerationStructureBarrier(topLevel, GpuAccessState(), buildWrite)
			};

			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchReuseBarrier, 1), {}, {}, TArrayView<const GpuAccelerationStructureBarrier>(topLevelBarriers, 2)));

			GpuAccelerationStructureBuild topLevelBuild(topLevelInputs, topLevel, scratch);
			topLevelBuild.BottomLevels = TArrayView<const TShared<GpuAccelerationStructure>>(&bottomLevel, 1);
			B3D_TEST_ASSERT(commands->BuildAccelerationStructures(TArrayView<const GpuAccelerationStructureBuild>(&topLevelBuild, 1)))

			const GpuBufferBarrier scratchRestBarrier(scratch, scratchAccess, scratchRest);
			const GpuAccelerationStructureBarrier topLevelRestBarrier(topLevel, buildWrite, structureRest);
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&scratchRestBarrier, 1), {}, {}, TArrayView<const GpuAccelerationStructureBarrier>(&topLevelRestBarrier, 1)));
			context->SubmitCommandBuffer(commands);
			device->WaitUntilIdle();
		}

		// The record a ray runs is the instance's offset, plus the ray's hit group contribution, plus the geometry index times the ray's
		// multiplier. A miss runs the record of the ray's miss index.
		const PipelineTestRay rays[] =
		{
			PipelineTestRay(Vector3(0.25f, 0.25f, 0.0f), 0, 0, 0),
			PipelineTestRay(Vector3(0.25f, 10.25f, 0.0f), 0, 0, 0),
			PipelineTestRay(Vector3(0.25f, 0.25f, 0.0f), 1, 0, 0),
			PipelineTestRay(Vector3(2.25f, 0.25f, 0.0f), 0, 2, 0),
			PipelineTestRay(Vector3(2.25f, 0.25f, 0.0f), 1, 2, 0),
			PipelineTestRay(Vector3(2.25f, 10.25f, 0.0f), 0, 2, 0),
			PipelineTestRay(Vector3(5.0f, 5.0f, 0.0f), 0, 0, 0),
			PipelineTestRay(Vector3(5.0f, 5.0f, 0.0f), 0, 0, 1)
		};

		const u32 expectedResults[] =
		{
			10, // Record 0 of instance 0: ClosestHitA
			11, // Instance 1 starts at record 1: ClosestHitB
			11, // The hit group contribution adds 1
			1, // Geometry 1 times 2 is the empty record 2, which runs nothing
			12, // Plus contribution 1 is record 3: ClosestHitC
			12, // Instance 1 plus geometry 1 times 2 is record 3 too
			101, // Miss record 0 runs MissB
			100 // Miss record 1 runs MissA
		};

		static constexpr u32 kRayCount = sizeof(rays) / sizeof(rays[0]);
		static_assert(kRayCount == sizeof(expectedResults) / sizeof(expectedResults[0]), "Every ray needs an expected result.");

		const TShared<render::GpuBuffer> rayBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(PipelineTestRay), kRayCount,
			GpuBufferFlag::None, GpuBufferMemory::Mapped));
		const TShared<render::GpuBuffer> resultBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kRayCount,
			GpuBufferFlag::GpuWrite));
		const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kRayCount * sizeof(u32)));
		B3D_TEST_ASSERT(rayBuffer != nullptr && resultBuffer != nullptr && readback != nullptr)
		if(rayBuffer == nullptr || resultBuffer == nullptr || readback == nullptr)
			return;

		{
			const render::GpuBufferMappedScope mapping = rayBuffer->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			std::memcpy(mapping.GetMappedMemory(), rays, sizeof(rays));
		}

		const TShared<render::GpuParameterSet> parameterSet = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
		B3D_TEST_ASSERT(parameterSet->SetAccelerationStructure("Scene", topLevel))
		parameterSet->SetStorageBuffer("Rays", rayBuffer);
		parameterSet->SetStorageBuffer("Results", resultBuffer);

		const GpuAccessState rayTracingWrite(GpuStageFlag::RayTracingShaderNonUniform, GpuAccessFlag::Write);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);
		GpuAccessState resultState;
		GpuAccessState readbackState;

		// Runs ray generation record @p rayGenerationRecordIndex once per ray, and returns the results
		const auto fnDispatch = [&](const char* name, u32 rayGenerationRecordIndex)
		{
			Vector<u32> results(kRayCount, 0);

			const TShared<render::GpuCommandBuffer> commands = pool->Create(render::GpuCommandBufferCreateInformation::Create(name));
			commands->IssueBarriers(GpuBufferBarrier(resultBuffer, resultState, rayTracingWrite));
			commands->SetGpuParameterSet(parameterSet);
			B3D_TEST_ASSERT(commands->DispatchRays(table, rayGenerationRecordIndex, kRayCount, 1))

			const GpuBufferBarrier copyBarriers[] = { GpuBufferBarrier(resultBuffer, rayTracingWrite, transferRead), GpuBufferBarrier(readback, readbackState, transferWrite) };
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(copyBarriers, 2)));
			commands->CopyBufferToBuffer(resultBuffer, readback, 0, 0, kRayCount * sizeof(u32));
			commands->IssueBarriers(GpuBufferBarrier(readback, transferWrite, hostRead));
			context->SubmitCommandBuffer(commands);
			device->WaitUntilIdle();

			resultState = transferRead;
			readbackState = hostRead;

			const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(mapping.IsValid())
				std::memcpy(results.data(), mapping.GetMappedMemory(), kRayCount * sizeof(u32));

			return results;
		};

		const Vector<u32> results = fnDispatch("Ray tracing pipeline records", 0);
		for(u32 rayIndex = 0; rayIndex < kRayCount; rayIndex++)
		{
			B3D_TEST_ASSERT_MSG(results[rayIndex] == expectedResults[rayIndex], StringUtility::Format("Ray {0} wrote {1}, expected {2}.", rayIndex, results[rayIndex],
				expectedResults[rayIndex]))
		}

		// Ray generation record 1 runs WriteIndices
		const Vector<u32> indices = fnDispatch("Second ray generation record", 1);
		for(u32 rayIndex = 0; rayIndex < kRayCount; rayIndex++)
			B3D_TEST_ASSERT_MSG(indices[rayIndex] == 1000 + rayIndex, StringUtility::Format("Thread {0} wrote {1}.", rayIndex, indices[rayIndex]))
	}, "GpuRayTracingTestSuite::TestRayTracingPipelineRecords", true);
}
