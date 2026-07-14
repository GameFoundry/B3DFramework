//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBackendTestSuite.h"
#include "GpuBackend/B3DGpuHazardTracking.h"

using namespace b3d;
using namespace b3d::render;

GpuBackendTestSuite::GpuBackendTestSuite()
	: TestSuite("GpuBackendTestSuite")
{
	B3D_ADD_TEST(GpuBackendTestSuite::TestHazardTrackingAccessEpochs)
	B3D_ADD_TEST(GpuBackendTestSuite::TestCrossCommandBufferRecipe)
}

void GpuBackendTestSuite::TestHazardTrackingAccessEpochs()
{
	GpuHazardTracking hazardTracking;
	for(u32 accessIndex = 0; accessIndex < 1000; ++accessIndex)
	{
		if((accessIndex & 1) == 0)
			hazardTracking.RegisterStageAccess(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		else
			hazardTracking.RegisterStageAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
	}

	B3D_TEST_ASSERT(hazardTracking.GetCompletedEpochs().Size() == 0)

	const GpuAccessScope& currentAccessScope = hazardTracking.GetCurrentAccessScope();
	B3D_TEST_ASSERT(currentAccessScope.GetAccess(GpuStageFlag::Transfer) == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(currentAccessScope.GetAccess(GpuStageFlag::ComputeShaderNonUniform) == GpuAccessFlag::Write)

	hazardTracking.RegisterBarrier(GpuHazardStageAndAccess(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write));
	B3D_TEST_ASSERT(hazardTracking.GetCompletedEpochs().Size() == 1)
	B3D_TEST_ASSERT(!hazardTracking.GetCurrentAccessScope().IsValid())

	const GpuHazardTracking::Epoch& completedEpoch = hazardTracking.GetCompletedEpochs()[0];
	B3D_TEST_ASSERT(completedEpoch.AccessScope.GetAccess(GpuStageFlag::Transfer) == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(completedEpoch.AccessScope.GetAccess(GpuStageFlag::ComputeShaderNonUniform) == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(completedEpoch.IssuedBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(completedEpoch.IssuedBarrier.SourceAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(completedEpoch.IssuedBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(completedEpoch.IssuedBarrier.DestinationAccess == GpuAccessFlag::Write)

	for(u32 accessIndex = 0; accessIndex < 1000; ++accessIndex)
		hazardTracking.RegisterStageAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);

	B3D_TEST_ASSERT(hazardTracking.GetCompletedEpochs().Size() == 1)
	B3D_TEST_ASSERT(hazardTracking.GetCurrentAccessScope().GetAccess(GpuStageFlag::ColorAttachment) == GpuAccessFlag::Write)

	const GpuAccessScope accumulatedAccessScope = hazardTracking.GetAccumulatedAccessScope();
	B3D_TEST_ASSERT(accumulatedAccessScope.GetStages().IsSet(GpuStageFlag::Transfer))
	B3D_TEST_ASSERT(accumulatedAccessScope.GetStages().IsSet(GpuStageFlag::ComputeShaderNonUniform))
	B3D_TEST_ASSERT(accumulatedAccessScope.GetStages().IsSet(GpuStageFlag::ColorAttachment))

	GpuHazardTracking leadingBarrierTracking;
	leadingBarrierTracking.RegisterBarrier(GpuHazardStageAndAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	leadingBarrierTracking.RegisterStageAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

	B3D_TEST_ASSERT(leadingBarrierTracking.GetCompletedEpochs().Size() == 1)
	B3D_TEST_ASSERT(!leadingBarrierTracking.GetCompletedEpochs()[0].AccessScope.IsValid())
	B3D_TEST_ASSERT(leadingBarrierTracking.GetCurrentAccessScope().GetAccess(GpuStageFlag::FragmentShaderNonUniform) == GpuAccessFlag::Read)
}

void GpuBackendTestSuite::TestCrossCommandBufferRecipe()
{
	GpuHazardState sourceHazardState;
	sourceHazardState.ClearSafeAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write);

	GpuHazardTracking fragmentReadTracking;
	fragmentReadTracking.RegisterStageAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

	const GpuHazardTracking::CrossCommandBufferRecipe fragmentReadRecipe = fragmentReadTracking.DetermineCrossCommandBufferRecipe(sourceHazardState);
	B3D_TEST_ASSERT(fragmentReadRecipe.MemoryDependency.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(fragmentReadRecipe.MemoryDependency.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(fragmentReadRecipe.MemoryDependency.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(fragmentReadRecipe.MemoryDependency.DestinationAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(!fragmentReadRecipe.ExecutionDependency.IsValid())
	B3D_TEST_ASSERT(fragmentReadRecipe.RemainingHazards.MemoryBarrierTracking.GetUnsafeAccessStages(GpuStageFlag::FragmentShaderNonUniform) == GpuStageFlag::None)
	B3D_TEST_ASSERT(fragmentReadRecipe.RemainingHazards.MemoryBarrierTracking.GetUnsafeAccessStages(GpuStageFlag::ComputeShaderNonUniform).IsSet(GpuStageFlag::Transfer))

	GpuHazardTracking computeReadTracking;
	computeReadTracking.RegisterStageAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);

	const GpuHazardTracking::CrossCommandBufferRecipe computeReadRecipe = computeReadTracking.DetermineCrossCommandBufferRecipe(fragmentReadRecipe.RemainingHazards);
	B3D_TEST_ASSERT(computeReadRecipe.MemoryDependency.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(computeReadRecipe.MemoryDependency.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)

	GpuHazardTracking multiEpochTracking;
	multiEpochTracking.RegisterStageAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	multiEpochTracking.RegisterBarrier(GpuHazardStageAndAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read));
	multiEpochTracking.RegisterStageAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);

	const GpuHazardTracking::CrossCommandBufferRecipe multiEpochRecipe = multiEpochTracking.DetermineCrossCommandBufferRecipe(sourceHazardState);
	B3D_TEST_ASSERT(multiEpochTracking.GetFirstAccessScope().GetStages() == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(multiEpochRecipe.MemoryDependency.DestinationStages == (GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform))

	const GpuAccessScope destinationAccessScope = multiEpochRecipe.GetDestinationAccessScope();
	B3D_TEST_ASSERT(destinationAccessScope.GetAccess(GpuStageFlag::FragmentShaderNonUniform) == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(destinationAccessScope.GetAccess(GpuStageFlag::ComputeShaderNonUniform) == GpuAccessFlag::Read)

	GpuHazardTracking leadingBarrierTracking;
	leadingBarrierTracking.RegisterBarrier(GpuHazardStageAndAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	leadingBarrierTracking.RegisterStageAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

	const GpuHazardTracking::CrossCommandBufferRecipe leadingBarrierRecipe = leadingBarrierTracking.DetermineCrossCommandBufferRecipe(sourceHazardState);
	B3D_TEST_ASSERT(!leadingBarrierRecipe.MemoryDependency.IsValid())
	B3D_TEST_ASSERT(leadingBarrierRecipe.RequiresCrossQueueDependency)
	B3D_TEST_ASSERT(leadingBarrierRecipe.RemainingHazards.MemoryBarrierTracking.GetUnsafeAccessStages(GpuStageFlag::FragmentShaderNonUniform) == GpuStageFlag::None)
}
