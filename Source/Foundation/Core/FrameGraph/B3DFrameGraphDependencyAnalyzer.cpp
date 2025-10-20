//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphDependencyAnalyzer.h"
#include "B3DFrameGraph.h"
#include "B3DFrameGraphPass.h"
#include "B3DFrameGraphResource.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

FrameGraphDependencyAnalyzer::FrameGraphDependencyAnalyzer(FrameGraph& frameGraph)
	: mFrameGraph(frameGraph)
{
}

bool FrameGraphDependencyAnalyzer::Analyze()
{
	// Create pass nodes
	const auto& passes = mFrameGraph.GetPasses();
	mPassNodes.reserve(passes.size());

	for (const auto& pass : passes)
		mPassNodes.push_back(B3DMakeUnique<FrameGraphPassNode>(pass.get()));

	// Analyze resource access patterns
	AnalyzeResourceAccess();

	// Build dependency edges
	BuildDependencies();

	// Track resource lifetimes
	TrackResourceLifetimes();

	return true;
}

void FrameGraphDependencyAnalyzer::AnalyzeResourceAccess()
{
	const auto& passes = mFrameGraph.GetPasses();

	for (const auto& pass : passes)
	{
		const auto& accesses = pass->GetResourceAccesses();

		for (const auto& access : accesses)
		{
			const bool isWrite = access.Access.IsSet(GpuAccessFlag::Write);
			const bool isRead = access.Access.IsSet(GpuAccessFlag::Read);

			if (isWrite)
				mResourceWriters[access.Resource].push_back(pass.get());

			if (isRead)
				mResourceReaders[access.Resource].push_back(pass.get());
		}
	}
}

void FrameGraphDependencyAnalyzer::BuildDependencies()
{
	const auto& passes = mFrameGraph.GetPasses();

	// Build a map from pass to its declaration index for ordering checks
	UnorderedMap<FrameGraphPass*, u32> passToIndex;
	for (u32 i = 0; i < passes.size(); i++)
		passToIndex[passes[i].get()] = i;

	// For each pass, check its resource accesses
	for (u32 consumerIndex = 0; consumerIndex < passes.size(); consumerIndex++)
	{
		const auto& pass = passes[consumerIndex];
		FrameGraphPassNode* consumerNode = FindNode(pass.get());
		if (!consumerNode)
			continue;

		const auto& accesses = pass->GetResourceAccesses();

		for (const auto& access : accesses)
		{
			const bool consumerWrites = access.Access.IsSet(GpuAccessFlag::Write);
			const bool consumerReads = access.Access.IsSet(GpuAccessFlag::Read);

			// Read-after-Write (RAW) - True dependency
			// Consumer reads, find all passes that write to this resource BEFORE this pass
			if (consumerReads)
			{
				auto writersIt = mResourceWriters.find(access.Resource);
				if (writersIt != mResourceWriters.end())
				{
					for (FrameGraphPass* producerPass : writersIt->second)
					{
						// Don't create self-dependency
						if (producerPass == pass.get())
							continue;

						// Only create dependency if producer comes BEFORE consumer in declaration order
						u32 producerIndex = passToIndex[producerPass];
						if (producerIndex >= consumerIndex)
							continue; // Producer declared after consumer, skip

						FrameGraphPassNode* producerNode = FindNode(producerPass);
						if (!producerNode)
							continue;

						FrameGraphPassDependency dependency;
						dependency.ProducerPass = producerPass;
						dependency.ConsumerPass = pass.get();
						dependency.Resource = access.Resource;
						dependency.DependencyType = FrameGraphPassDependency::Type::ReadAfterWrite;

						consumerNode->AddIncomingDependency(dependency);
						producerNode->AddOutgoingDependency(dependency);
					}
				}
			}

			// Write-after-Read (WAR) - Anti-dependency
			// Consumer writes, find all passes that read from this resource BEFORE this pass
			if (consumerWrites)
			{
				auto readersIt = mResourceReaders.find(access.Resource);
				if (readersIt != mResourceReaders.end())
				{
					for (FrameGraphPass* producerPass : readersIt->second)
					{
						// Don't create self-dependency
						if (producerPass == pass.get())
							continue;

						// Only create dependency if producer comes BEFORE consumer in declaration order
						u32 producerIndex = passToIndex[producerPass];
						if (producerIndex >= consumerIndex)
							continue;

						FrameGraphPassNode* producerNode = FindNode(producerPass);
						if (!producerNode)
							continue;

						FrameGraphPassDependency dependency;
						dependency.ProducerPass = producerPass;
						dependency.ConsumerPass = pass.get();
						dependency.Resource = access.Resource;
						dependency.DependencyType = FrameGraphPassDependency::Type::WriteAfterRead;

						consumerNode->AddIncomingDependency(dependency);
						producerNode->AddOutgoingDependency(dependency);
					}
				}

				// Write-after-Write (WAW) - Output dependency
				// Consumer writes, find all passes that write to this resource BEFORE this pass
				auto writersIt = mResourceWriters.find(access.Resource);
				if (writersIt != mResourceWriters.end())
				{
					for (FrameGraphPass* producerPass : writersIt->second)
					{
						// Don't create self-dependency
						if (producerPass == pass.get())
							continue;

						// Only create dependency if producer comes BEFORE consumer in declaration order
						u32 producerIndex = passToIndex[producerPass];
						if (producerIndex >= consumerIndex)
							continue;

						FrameGraphPassNode* producerNode = FindNode(producerPass);
						if (!producerNode)
							continue;

						FrameGraphPassDependency dependency;
						dependency.ProducerPass = producerPass;
						dependency.ConsumerPass = pass.get();
						dependency.Resource = access.Resource;
						dependency.DependencyType = FrameGraphPassDependency::Type::WriteAfterWrite;

						consumerNode->AddIncomingDependency(dependency);
						producerNode->AddOutgoingDependency(dependency);
					}
				}
			}
		}
	}

	// Initialize reference counts for topological sort
	for (const auto& node : mPassNodes)
		node->SetReferenceCount(static_cast<u32>(node->GetIncomingDependencies().size()));
}

void FrameGraphDependencyAnalyzer::TrackResourceLifetimes()
{
	const auto& passes = mFrameGraph.GetPasses();

	// Track all imported resources
	const auto& resources = mFrameGraph.GetResources();
	for (const auto& resource : resources)
	{
		FrameGraphResourceLifetime lifetime;
		lifetime.Resource = resource->GetId();
		lifetime.IsImported = true;
		lifetime.FirstUse = nullptr; // Set when first accessed
		lifetime.LastUse = nullptr;  // Set when last accessed
		lifetime.IsWritten = false;

		mResourceLifetimes[resource->GetId()] = lifetime;
	}

	// Iterate through passes in declaration order to track first/last use
	for (const auto& pass : passes)
	{
		const auto& accesses = pass->GetResourceAccesses();

		for (const auto& access : accesses)
		{
			auto it = mResourceLifetimes.find(access.Resource);
			if (it == mResourceLifetimes.end())
			{
				// Resource not imported, create lifetime entry
				FrameGraphResourceLifetime lifetime;
				lifetime.Resource = access.Resource;
				lifetime.IsImported = false;
				lifetime.FirstUse = pass.get();
				lifetime.LastUse = pass.get();
				lifetime.IsWritten = access.Access.IsSet(GpuAccessFlag::Write);

				mResourceLifetimes[access.Resource] = lifetime;
			}
			else
			{
				// Update existing lifetime
				if (it->second.FirstUse == nullptr)
					it->second.FirstUse = pass.get();

				it->second.LastUse = pass.get();

				if (access.Access.IsSet(GpuAccessFlag::Write))
					it->second.IsWritten = true;
			}
		}
	}

	// Mark explicit outputs
	const auto& outputResources = mFrameGraph.GetOutputResources();
	for (FrameGraphResourceId outputId : outputResources)
	{
		auto it = mResourceLifetimes.find(outputId);
		if (it != mResourceLifetimes.end())
			it->second.IsOutput = true;
	}
}

FrameGraphPassNode* FrameGraphDependencyAnalyzer::FindNode(FrameGraphPass* pass)
{
	for (const auto& node : mPassNodes)
	{
		if (node->GetPass() == pass)
			return node.get();
	}

	return nullptr;
}

bool FrameGraphDependencyAnalyzer::IsResourceWritten(FrameGraphPass* pass, FrameGraphResourceId resource)
{
	const auto& accesses = pass->GetResourceAccesses();
	for (const auto& access : accesses)
	{
		if (access.Resource == resource && access.Access.IsSet(GpuAccessFlag::Write))
			return true;
	}

	return false;
}

bool FrameGraphDependencyAnalyzer::IsResourceRead(FrameGraphPass* pass, FrameGraphResourceId resource)
{
	const auto& accesses = pass->GetResourceAccesses();
	for (const auto& access : accesses)
	{
		if (access.Resource == resource && access.Access.IsSet(GpuAccessFlag::Read))
			return true;
	}

	return false;
}

void FrameGraphDependencyAnalyzer::Reset()
{
	for (auto& node : mPassNodes)
		node->Reset();

	mPassNodes.clear();
	mResourceLifetimes.clear();
	mResourceWriters.clear();
	mResourceReaders.clear();
}
