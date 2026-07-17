//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBackendTestSuite.h"
#include "GpuBackend/B3DGpuHazards.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

#include <algorithm>

using namespace b3d;
using namespace b3d::render;

GpuBackendTestSuite::GpuBackendTestSuite()
	: TestSuite("GpuBackendTestSuite")
{
	B3D_ADD_TEST(GpuBackendTestSuite::TestHazardHistoryEpochs)
	B3D_ADD_TEST(GpuBackendTestSuite::TestTransitionRecipe)
	B3D_ADD_TEST(GpuBackendTestSuite::TestResourceTransitionRecipe)
}

void GpuBackendTestSuite::TestHazardHistoryEpochs()
{
	GpuHazardStateWithHistory hazardHistory;
	for(u32 accessIndex = 0; accessIndex < 1000; ++accessIndex)
	{
		if((accessIndex & 1) == 0)
			hazardHistory.RegisterStageAccess(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		else
			hazardHistory.RegisterStageAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
	}

	B3D_TEST_ASSERT(hazardHistory.GetCompletedEpochs().Size() == 0)

	const GpuAccessScope& currentAccessScope = hazardHistory.GetCurrentAccessScope();
	B3D_TEST_ASSERT(currentAccessScope.GetAccess(GpuStageFlag::Transfer) == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(currentAccessScope.GetAccess(GpuStageFlag::ComputeShaderNonUniform) == GpuAccessFlag::Write)

	hazardHistory.RegisterBarrier(GpuHazardStageAndAccess(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write));
	B3D_TEST_ASSERT(hazardHistory.GetCompletedEpochs().Size() == 1)
	B3D_TEST_ASSERT(!hazardHistory.GetCurrentAccessScope().IsValid())

	const GpuHazardStateWithHistory::Epoch& completedEpoch = hazardHistory.GetCompletedEpochs()[0];
	B3D_TEST_ASSERT(completedEpoch.AccessScope.GetAccess(GpuStageFlag::Transfer) == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(completedEpoch.AccessScope.GetAccess(GpuStageFlag::ComputeShaderNonUniform) == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(completedEpoch.IssuedBarrier.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(completedEpoch.IssuedBarrier.SourceAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(completedEpoch.IssuedBarrier.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)
	B3D_TEST_ASSERT(completedEpoch.IssuedBarrier.DestinationAccess == GpuAccessFlag::Write)

	for(u32 accessIndex = 0; accessIndex < 1000; ++accessIndex)
		hazardHistory.RegisterStageAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);

	B3D_TEST_ASSERT(hazardHistory.GetCompletedEpochs().Size() == 1)
	B3D_TEST_ASSERT(hazardHistory.GetCurrentAccessScope().GetAccess(GpuStageFlag::ColorAttachment) == GpuAccessFlag::Write)

	const GpuAccessScope accumulatedAccessScope = hazardHistory.GetAccumulatedAccessScope();
	B3D_TEST_ASSERT(accumulatedAccessScope.GetStages().IsSet(GpuStageFlag::Transfer))
	B3D_TEST_ASSERT(accumulatedAccessScope.GetStages().IsSet(GpuStageFlag::ComputeShaderNonUniform))
	B3D_TEST_ASSERT(accumulatedAccessScope.GetStages().IsSet(GpuStageFlag::ColorAttachment))

	GpuHazardStateWithHistory leadingBarrierHistory;
	leadingBarrierHistory.RegisterBarrier(GpuHazardStageAndAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	leadingBarrierHistory.RegisterStageAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

	B3D_TEST_ASSERT(leadingBarrierHistory.GetCompletedEpochs().Size() == 1)
	B3D_TEST_ASSERT(!leadingBarrierHistory.GetCompletedEpochs()[0].AccessScope.IsValid())
	B3D_TEST_ASSERT(leadingBarrierHistory.GetCurrentAccessScope().GetAccess(GpuStageFlag::FragmentShaderNonUniform) == GpuAccessFlag::Read)
}

void GpuBackendTestSuite::TestTransitionRecipe()
{
	const GpuQueueId sourceQueueId(GQT_GRAPHICS, 0);

	GpuHazardState sourceHazardState;
	sourceHazardState.ClearSafeAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write);

	GpuHazardStateWithHistory fragmentReadHistory;
	fragmentReadHistory.RegisterStageAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

	const GpuHazardState::TransitionRecipe fragmentReadRecipe = sourceHazardState.BuildTransitionRecipe(sourceQueueId, fragmentReadHistory);
	B3D_TEST_ASSERT(fragmentReadRecipe.SourceQueueId.Id == sourceQueueId.Id)
	B3D_TEST_ASSERT(fragmentReadRecipe.MemoryDependency.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(fragmentReadRecipe.MemoryDependency.SourceAccess == GpuAccessFlag::Write)
	B3D_TEST_ASSERT(fragmentReadRecipe.MemoryDependency.DestinationStages == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(fragmentReadRecipe.MemoryDependency.DestinationAccess == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(!fragmentReadRecipe.ExecutionDependency.IsValid())
	B3D_TEST_ASSERT(fragmentReadRecipe.RemainingHazardState.MemoryBarrierTracking.GetUnsafeAccessStages(GpuStageFlag::FragmentShaderNonUniform) == GpuStageFlag::None)
	B3D_TEST_ASSERT(fragmentReadRecipe.RemainingHazardState.MemoryBarrierTracking.GetUnsafeAccessStages(GpuStageFlag::ComputeShaderNonUniform).IsSet(GpuStageFlag::Transfer))

	GpuHazardStateWithHistory computeReadHistory;
	computeReadHistory.RegisterStageAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);

	const GpuHazardState::TransitionRecipe computeReadRecipe = fragmentReadRecipe.RemainingHazardState.BuildTransitionRecipe(sourceQueueId, computeReadHistory);
	B3D_TEST_ASSERT(computeReadRecipe.MemoryDependency.SourceStages == GpuStageFlag::Transfer)
	B3D_TEST_ASSERT(computeReadRecipe.MemoryDependency.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)

	GpuHazardStateWithHistory multiEpochHistory;
	multiEpochHistory.RegisterStageAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	multiEpochHistory.RegisterBarrier(GpuHazardStageAndAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read));
	multiEpochHistory.RegisterStageAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);

	const GpuHazardState::TransitionRecipe multiEpochRecipe = sourceHazardState.BuildTransitionRecipe(sourceQueueId, multiEpochHistory);
	B3D_TEST_ASSERT(multiEpochHistory.GetFirstAccessScope().GetStages() == GpuStageFlag::FragmentShaderNonUniform)
	B3D_TEST_ASSERT(multiEpochRecipe.MemoryDependency.DestinationStages == (GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::ComputeShaderNonUniform))

	const GpuAccessScope destinationAccessScope = multiEpochRecipe.GetDestinationAccessScope();
	B3D_TEST_ASSERT(destinationAccessScope.GetAccess(GpuStageFlag::FragmentShaderNonUniform) == GpuAccessFlag::Read)
	B3D_TEST_ASSERT(destinationAccessScope.GetAccess(GpuStageFlag::ComputeShaderNonUniform) == GpuAccessFlag::Read)

	GpuHazardStateWithHistory leadingBarrierHistory;
	leadingBarrierHistory.RegisterBarrier(GpuHazardStageAndAccess(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read));
	leadingBarrierHistory.RegisterStageAccess(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);

	const GpuHazardState::TransitionRecipe leadingBarrierRecipe = sourceHazardState.BuildTransitionRecipe(sourceQueueId, leadingBarrierHistory);
	B3D_TEST_ASSERT(!leadingBarrierRecipe.MemoryDependency.IsValid())
	B3D_TEST_ASSERT(leadingBarrierRecipe.RequiresCrossQueueDependency)
	B3D_TEST_ASSERT(leadingBarrierRecipe.RemainingHazardState.MemoryBarrierTracking.GetUnsafeAccessStages(GpuStageFlag::FragmentShaderNonUniform) == GpuStageFlag::None)
}

void GpuBackendTestSuite::TestResourceTransitionRecipe()
{
	B3D_TEST_ASSERT(GpuBackendUtility::GetStageFlags(GpuResourceUseFlag::Host) == GpuStageFlag::Host)
	B3D_TEST_ASSERT(GpuBackendUtility::GetStageFlags(GpuResourceUseFlag::ShaderAccess |
		GpuResourceUseFlag::StageVertexShader) == GpuStageFlag::VertexShaderNonUniform)

	const GpuQueueId sourceQueueId(GQT_GRAPHICS, 0);
	const GpuQueueId sampleQueueId(GQT_COMPUTE, 0);
	const GpuQueueId secondSampleQueueId(GQT_TRANSFER, 0);

	GpuHazardState sourceHazards;
	sourceHazards.ClearSafeAccess(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);

	GpuResourceRemainingHazards remainingHazards;
	remainingHazards.Add(sourceQueueId, sourceHazards);

	GpuHazardStateWithHistory sampleReadHistory;
	sampleReadHistory.RegisterStageAccess(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);

	GpuResourceRemainingHazards::TransitionRecipe sampleReadRecipe = remainingHazards.BuildTransitionRecipe(sampleReadHistory, sampleQueueId);
	B3D_TEST_ASSERT(sampleReadRecipe.PerQueueTransitionRecipes.Size() == 1)
	B3D_TEST_ASSERT(sampleReadRecipe.PerQueueTransitionRecipes[0].SourceQueueId.Id == sourceQueueId.Id)
	B3D_TEST_ASSERT(sampleReadRecipe.PerQueueTransitionRecipes[0].MemoryDependency.SourceStages == GpuStageFlag::ColorAttachment)
	B3D_TEST_ASSERT(sampleReadRecipe.PerQueueTransitionRecipes[0].MemoryDependency.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)

	// Model submission B: its sampled read is added, while A's write remains unsafe for destinations that B's
	// boundary did not make safe. A later Host read must therefore still discover A's write and request a barrier.
	GpuResourceRemainingHazards hazardsAfterSample = std::move(sampleReadRecipe.RemainingHazards);
	hazardsAfterSample.Add(sampleQueueId, sampleReadHistory.State);

	// Acquiring A's write on B must not clear it from C's perspective, even when C performs the same-stage read as B.
	const GpuResourceRemainingHazards::TransitionRecipe secondSampleReadRecipe = hazardsAfterSample.BuildTransitionRecipe(sampleReadHistory,
		secondSampleQueueId);
	const auto secondSampleSourceQueueRecipe = std::find_if(secondSampleReadRecipe.PerQueueTransitionRecipes.begin(),
		secondSampleReadRecipe.PerQueueTransitionRecipes.end(), [sourceQueueId](const GpuHazardState::TransitionRecipe& recipe)
		{
			return recipe.SourceQueueId.Id == sourceQueueId.Id;
		});
	B3D_TEST_ASSERT(secondSampleSourceQueueRecipe != secondSampleReadRecipe.PerQueueTransitionRecipes.end())
	B3D_TEST_ASSERT(secondSampleSourceQueueRecipe->MemoryDependency.SourceStages == GpuStageFlag::ColorAttachment)
	B3D_TEST_ASSERT(secondSampleSourceQueueRecipe->MemoryDependency.DestinationStages == GpuStageFlag::ComputeShaderNonUniform)

	GpuHazardStateWithHistory hostReadHistory;
	hostReadHistory.RegisterStageAccess(GpuStageFlag::Host, GpuAccessFlag::Read);

	const GpuResourceRemainingHazards::TransitionRecipe hostReadRecipe = hazardsAfterSample.BuildTransitionRecipe(hostReadHistory, secondSampleQueueId);
	const auto sourceQueueRecipe = std::find_if(hostReadRecipe.PerQueueTransitionRecipes.begin(), hostReadRecipe.PerQueueTransitionRecipes.end(),
		[sourceQueueId](const GpuHazardState::TransitionRecipe& recipe) { return recipe.SourceQueueId.Id == sourceQueueId.Id; });
	B3D_TEST_ASSERT(sourceQueueRecipe != hostReadRecipe.PerQueueTransitionRecipes.end())
	B3D_TEST_ASSERT(sourceQueueRecipe->MemoryDependency.SourceStages == GpuStageFlag::ColorAttachment)
	B3D_TEST_ASSERT(sourceQueueRecipe->MemoryDependency.DestinationStages == GpuStageFlag::Host)
}
