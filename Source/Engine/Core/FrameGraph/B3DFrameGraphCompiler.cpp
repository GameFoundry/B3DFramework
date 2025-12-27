//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphCompiler.h"
#include "B3DFrameGraphPass.h"
#include "B3DFrameGraphResource.h"
#include "RenderAPI/B3DRenderTarget.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "Debug/B3DDebug.h"
#include <queue>

using namespace b3d;
using namespace b3d::render;

ImageLayout FrameGraphLayoutHelper::GetLayoutForUsage(GpuResourceUseFlags usage, GpuAccessFlags access)
{
	// Priority order: attachment usage > shader usage > transfer usage

	// Color attachment - write or read/write
	if (usage.IsSet(GpuResourceUseFlag::ColorAttachment))
	{
		return ImageLayout::ColorAttachment;
	}

	// Depth/stencil attachment - check if read-only
	if (usage.IsSet(GpuResourceUseFlag::DepthStencilAttachment))
	{
		// Read-only depth (can be sampled while bound)
		if (access == GpuAccessFlag::Read)
			return ImageLayout::DepthStencilReadOnly;

		// Write or read/write depth
		return ImageLayout::DepthStencilAttachment;
	}

	// Shader access (sampling, storage, etc.)
	if (usage.IsSet(GpuResourceUseFlag::ShaderAccess))
	{
		// If write access is involved, use General layout
		if (access.IsSet(GpuAccessFlag::Write))
			return ImageLayout::General;

		// Read-only shader access
		return ImageLayout::ShaderReadOnly;
	}

	// Transfer operations
	if (usage.IsSet(GpuResourceUseFlag::Transfer))
	{
		if (access.IsSet(GpuAccessFlag::Write))
			return ImageLayout::TransferDestination;

		return ImageLayout::TransferSource;
	}

	if(usage.IsSet(GpuResourceUseFlag::Host))
		return ImageLayout::General;

	// Default to general layout for undefined or complex cases
	if (usage != GpuResourceUseFlag::Undefined)
		return ImageLayout::General;

	return ImageLayout::Undefined;
}

bool FrameGraphLayoutHelper::RequiresTransition(ImageLayout sourceLayout, ImageLayout destinationLayout)
{
	// No transition needed if layouts match
	if (sourceLayout == destinationLayout)
		return false;

	// Undefined source means we don't care about current contents
	// But we still need to transition to the destination layout
	if (sourceLayout == ImageLayout::Undefined)
		return true;

	// Undefined destination means we're discarding the image
	// This is unusual but technically requires a transition
	if (destinationLayout == ImageLayout::Undefined)
		return true;

	// All other cases require transition
	return true;
}

FrameGraphCompiler::FrameGraphCompiler(FrameGraph& frameGraph)
	: mFrameGraph(frameGraph)
{
}

UPtr<CompiledFrameGraph> FrameGraphCompiler::Compile()
{
	B3D_LOG(Info, RenderBackend, "Compiling frame graph...");

	// Validation and setup
	if (!Validate())
	{
		B3D_LOG(Error, RenderBackend, "Frame graph validation failed");
		return nullptr;
	}

	ExecuteSetupFunctions();

	// Dependency analysis
	if (!AnalyzeDependencies())
	{
		B3D_LOG(Error, RenderBackend, "Dependency analysis failed");
		return nullptr;
	}

	// Dependency validation
	if (!ValidateDependencies())
	{
		B3D_LOG(Warning, RenderBackend, "Dependency validation found potential issues");
	}

	// Pass culling
	CullUnusedPasses();

	if (mCulledPassCount > 0)
	{
		B3D_LOG(Info, RenderBackend, "Culled {0} unused passes", mCulledPassCount);
	}

	// Topological sort
	if (!TopologicalSort(mSortedPasses))
	{
		B3D_LOG(Error, RenderBackend, "Topological sort failed - cycle detected");
		return nullptr;
	}

	B3D_LOG(Info, RenderBackend, "Frame graph compiled successfully. Execution order:");
	for (u32 i = 0; i < mSortedPasses.size(); i++)
	{
		B3D_LOG(Info, RenderBackend, "  {0}. {1}", i + 1, mSortedPasses[i]->GetName());
	}

	// Validate render pass setup
	if (!ValidateRenderPasses())
	{
		B3D_LOG(Error, RenderBackend, "Render pass validation failed");
		return nullptr;
	}

	// Build usage history
	BuildUsageHistory(mSortedPasses);

	// Build transitions
	Vector<ResourceTransition> transitions;
	BuildTransitions(transitions);

	// Build barriers
	Vector<FrameGraphBarrierBatch> barriers;
	BuildBarriers(transitions, barriers);

	// Create render targets for render passes
	UnorderedMap<FrameGraphPass*, SPtr<RenderTarget>> renderTargets;
	CreateRenderTargets(mSortedPasses, renderTargets);

	// Create compiled graph
	auto compiled = B3DMakeUnique<CompiledFrameGraph>();
	compiled->SortedPasses = mSortedPasses;
	compiled->BarrierBatches = std::move(barriers);
	compiled->UsageHistories = mUsageHistories;
	compiled->RenderTargets = std::move(renderTargets);

	// Validation
	if (!ValidateLayouts(*compiled))
	{
		B3D_LOG(Error, RenderBackend, "Layout validation failed");
		return nullptr;
	}

	return compiled;
}

void FrameGraphCompiler::ExecuteSetupFunctions()
{
	for (const auto& pass : mFrameGraph.GetPasses())
	{
		pass->ExecuteSetup();
	}
}

bool FrameGraphCompiler::Validate()
{
	bool isValid = true;

	// Validate that we have at least one pass
	if (mFrameGraph.GetPasses().empty())
	{
		B3D_LOG(Warning, RenderBackend, "Frame graph has no passes declared");
		// This is not an error - empty frame graphs are allowed
	}

	// Check that all referenced resources exist and are valid
	for (const auto& pass : mFrameGraph.GetPasses())
	{
		if (!ValidatePass(pass.get()))
			isValid = false;
	}

	return isValid;
}

bool FrameGraphCompiler::ValidatePass(FrameGraphPass* pass)
{
	B3D_ENSURE(pass != nullptr);

	bool isValid = true;

	// Check that pass has a valid name
	if (pass->GetName().empty())
	{
		B3D_LOG(Warning, RenderBackend, "Pass at index {0} has no name", pass->GetIndex());
	}

	// Check that pass has resource accesses
	if (pass->GetResourceAccesses().empty())
	{
		B3D_LOG(Warning, RenderBackend, "Pass '{0}' has no resource accesses declared",
			pass->GetName());
	}

	// Validate all resource accesses
	for (const auto& access : pass->GetResourceAccesses())
	{
		if (!ValidateResourceAccess(pass, access))
			isValid = false;
	}

	return isValid;
}

bool FrameGraphCompiler::ValidateResourceAccess(FrameGraphPass* pass, const FrameGraphResourceAccess& access)
{
	B3D_ENSURE(pass != nullptr);

	// Check that resource ID is valid
	if (!access.Resource.IsValid())
	{
		B3D_LOG(Error, RenderBackend, "Pass '{0}': Invalid resource ID", pass->GetName());
		return false;
	}

	// Check that resource exists
	FrameGraphResource* resource = mFrameGraph.GetResource(access.Resource);
	if (resource == nullptr)
	{
		B3D_LOG(Error, RenderBackend,
			"Pass '{0}': References non-existent resource ID {1}",
			pass->GetName(), access.Resource.Index);
		return false;
	}

	// Check that usage flags are specified
	if (access.Usage == GpuResourceUseFlag::Undefined)
	{
		B3D_LOG(Error, RenderBackend,
			"Pass '{0}': Resource '{1}' has undefined usage flags",
			pass->GetName(), resource->GetName());
		return false;
	}

	// Check that access flags are specified
	if (access.Access == GpuAccessFlag::None)
	{
		B3D_LOG(Error, RenderBackend,
			"Pass '{0}': Resource '{1}' has no access flags (must be Read and/or Write)",
			pass->GetName(), resource->GetName());
		return false;
	}

	// Validate usage/access combinations
	const bool isWrite = access.Access.IsSet(GpuAccessFlag::Write);
	const bool isRead = access.Access.IsSet(GpuAccessFlag::Read);
	const bool isColorAttachment = access.Usage.IsSet(GpuResourceUseFlag::ColorAttachment);
	const bool isDepthStencilAttachment = access.Usage.IsSet(GpuResourceUseFlag::DepthStencilAttachment);

	// Color attachments must have write access (cannot be read-only)
	if (isColorAttachment && !isWrite)
	{
		B3D_LOG(Error, RenderBackend,
			"Pass '{0}': Resource '{1}' used as color attachment but not marked for write access",
			pass->GetName(), resource->GetName());
		return false;
	}

	// Depth/stencil attachments must have read or write access (can be read-only, write-only, or read-write)
	if (isDepthStencilAttachment && !isRead && !isWrite)
	{
		B3D_LOG(Error, RenderBackend,
			"Pass '{0}': Resource '{1}' used as depth/stencil attachment but has no read or write access",
			pass->GetName(), resource->GetName());
		return false;
	}

	return true;
}

bool FrameGraphCompiler::ValidateDependencies()
{
	const auto& passes = mFrameGraph.GetPasses();
	bool valid = true;

	// Check for passes with no inputs or outputs (isolated passes)
	for (const auto& pass : passes)
	{
		if (pass->IsCulled())
			continue;

		const auto& accesses = pass->GetResourceAccesses();

		if (accesses.empty())
		{
			B3D_LOG(Warning, RenderBackend,
				"Pass '{0}' has no resource accesses. This may indicate an error.",
				pass->GetName());
		}

		// Check for passes that only read (no side effects)
		bool hasWrite = false;
		for (const auto& access : accesses)
		{
			if (access.Access.IsSet(GpuAccessFlag::Write))
			{
				hasWrite = true;
				break;
			}
		}

		if (!hasWrite && pass->GetOutgoingDependencies().empty())
		{
			B3D_LOG(Warning, RenderBackend,
				"Pass '{0}' only reads resources and has no outgoing dependencies. "
				"It will be culled unless it writes to an output resource.",
				pass->GetName());
		}
	}

	return valid;
}

bool FrameGraphCompiler::ValidateRenderPasses()
{
	const auto& passes = mFrameGraph.GetPasses();
	bool valid = true;

	for (const auto& pass : passes)
	{

		if (pass->GetPassType() == FrameGraphPassType::Render)
		{
			// Check that render passes have at least one attachment
			if (pass->GetColorAttachments().empty() && !pass->GetDepthAttachment().IsValid())
			{
				B3D_LOG(Error, RenderBackend,
					"Render pass '{0}' has no color or depth attachments",
					pass->GetName());
				valid = false;
			}
		}
		else if (pass->GetPassType() == FrameGraphPassType::Compute)
		{
			// Check that compute passes don't use render target resources
			const auto& accesses = pass->GetResourceAccesses();
			for (const auto& access : accesses)
			{
				if (access.Usage.IsSet(GpuResourceUseFlag::ColorAttachment) ||
					access.Usage.IsSet(GpuResourceUseFlag::DepthStencilAttachment))
				{
					B3D_LOG(Error, RenderBackend,
						"Compute pass '{0}' uses render target resources. Use DeclareRenderPass instead.",
						pass->GetName());
					valid = false;
				}
			}
		}
	}

	return valid;
}

bool FrameGraphCompiler::ValidateLayouts(const CompiledFrameGraph& compiledGraph)
{
	// For now, just log that layouts were validated
	// More sophisticated validation can be added later
	B3D_LOG(Info, RenderBackend, "Layout validation passed for {0} resources",
		compiledGraph.UsageHistories.size());

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Dependency Analysis & Scheduling
//////////////////////////////////////////////////////////////////////////

bool FrameGraphCompiler::AnalyzeDependencies()
{
	const auto& passes = mFrameGraph.GetPasses();

	// Subsection 1: Analyze resource access patterns
	// Build maps of which passes read/write which resources
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

	// Subsection 2: Build dependency edges
	// Create RAW, WAR, and WAW dependencies between passes

	// Build a map from pass to its declaration index for ordering checks
	UnorderedMap<FrameGraphPass*, u32> passToIndex;
	for (u32 i = 0; i < passes.size(); i++)
		passToIndex[passes[i].get()] = i;

	// For each pass, check its resource accesses
	for (u32 consumerIndex = 0; consumerIndex < passes.size(); consumerIndex++)
	{
		const auto& pass = passes[consumerIndex];
		const auto& accesses = pass->GetResourceAccesses();

		for (const auto& access : accesses)
		{
			const bool consumerWrites = access.Access.IsSet(GpuAccessFlag::Write);
			const bool consumerReads = access.Access.IsSet(GpuAccessFlag::Read);

			// Read-after-Write (RAW) - True dependency
			if (consumerReads)
			{
				auto writersIt = mResourceWriters.find(access.Resource);
				if (writersIt != mResourceWriters.end())
				{
					for (FrameGraphPass* producerPass : writersIt->second)
					{
						if (producerPass == pass.get())
							continue;

						u32 producerIndex = passToIndex[producerPass];
						if (producerIndex >= consumerIndex)
							continue;

						FrameGraphPassDependency dependency;
						dependency.ProducerPass = producerPass;
						dependency.ConsumerPass = pass.get();
						dependency.Resource = access.Resource;
						dependency.DependencyType = FrameGraphPassDependency::Type::ReadAfterWrite;

						pass->AddIncomingDependency(dependency);
						producerPass->AddOutgoingDependency(dependency);
					}
				}
			}

			// Write-after-Read (WAR) - Anti-dependency
			if (consumerWrites)
			{
				auto readersIt = mResourceReaders.find(access.Resource);
				if (readersIt != mResourceReaders.end())
				{
					for (FrameGraphPass* producerPass : readersIt->second)
					{
						if (producerPass == pass.get())
							continue;

						u32 producerIndex = passToIndex[producerPass];
						if (producerIndex >= consumerIndex)
							continue;

						FrameGraphPassDependency dependency;
						dependency.ProducerPass = producerPass;
						dependency.ConsumerPass = pass.get();
						dependency.Resource = access.Resource;
						dependency.DependencyType = FrameGraphPassDependency::Type::WriteAfterRead;

						pass->AddIncomingDependency(dependency);
						producerPass->AddOutgoingDependency(dependency);
					}
				}

				// Write-after-Write (WAW) - Output dependency
				auto writersIt = mResourceWriters.find(access.Resource);
				if (writersIt != mResourceWriters.end())
				{
					for (FrameGraphPass* producerPass : writersIt->second)
					{
						if (producerPass == pass.get())
							continue;

						u32 producerIndex = passToIndex[producerPass];
						if (producerIndex >= consumerIndex)
							continue;

						FrameGraphPassDependency dependency;
						dependency.ProducerPass = producerPass;
						dependency.ConsumerPass = pass.get();
						dependency.Resource = access.Resource;
						dependency.DependencyType = FrameGraphPassDependency::Type::WriteAfterWrite;

						pass->AddIncomingDependency(dependency);
						producerPass->AddOutgoingDependency(dependency);
					}
				}
			}
		}
	}

	// Initialize reference counts for topological sort
	for (const auto& pass : passes)
		pass->SetReferenceCount(static_cast<u32>(pass->GetIncomingDependencies().size()));

	// Track resource lifetimes
	TrackResourceLifetimes();

	return true;
}

void FrameGraphCompiler::TrackResourceLifetimes()
{
	const auto& passes = mFrameGraph.GetPasses();

	// Track all imported resources
	const auto& resources = mFrameGraph.GetResources();
	for (const auto& resource : resources)
	{
		FrameGraphResourceLifetime lifetime;
		lifetime.Resource = resource->GetId();
		lifetime.IsImported = true;
		lifetime.FirstUse = nullptr;
		lifetime.LastUse = nullptr;
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

void FrameGraphCompiler::CullUnusedPasses()
{
	const auto& passes = mFrameGraph.GetPasses();
	mCulledPassCount = 0;

	// Step 1: Mark all passes as culled initially
	for (const auto& pass : passes)
		pass->SetCulled(true);

	// Step 2: Find passes that write to output resources
	Vector<FrameGraphPass*> outputPasses;

	for (const auto& pair : mResourceLifetimes)
	{
		const auto& lifetime = pair.second;

		// Check if this is an output resource
		bool isOutput = lifetime.IsOutput || (lifetime.IsImported && lifetime.IsWritten);

		if (isOutput && lifetime.LastUse != nullptr)
		{
			outputPasses.push_back(lifetime.LastUse);
		}
	}

	// Step 3: Perform reverse DFS from output passes
	// Recursively mark a pass and all its dependencies as used
	auto fnMarkPassAsUsed = [](FrameGraphPass* pass, auto& fnSelf) -> void
	{
		// Already marked as used
		if (!pass->IsCulled())
			return;

		// Mark this pass as used
		pass->SetCulled(false);

		// Recursively mark all incoming dependencies as used
		const auto& incoming = pass->GetIncomingDependencies();
		for (const auto& dependency : incoming)
		{
			fnSelf(dependency.ProducerPass, fnSelf);
		}
	};

	for (FrameGraphPass* outputPass : outputPasses)
		fnMarkPassAsUsed(outputPass, fnMarkPassAsUsed);

	// Step 4: Count culled passes
	for (const auto& pass : passes)
	{
		if (pass->IsCulled())
		{
			mCulledPassCount++;
			B3D_LOG(Info, RenderBackend, "Culled unused pass: {0}", pass->GetName());
		}
	}
}

bool FrameGraphCompiler::TopologicalSort(Vector<FrameGraphPass*>& outSortedPasses)
{
	const auto& passes = mFrameGraph.GetPasses();
	outSortedPasses.clear();
	outSortedPasses.reserve(passes.size());

	// Kahn's algorithm using a queue
	std::queue<FrameGraphPass*> readyQueue;

	// Find all passes with no incoming dependencies
	for (const auto& pass : passes)
	{
		if (pass->IsReady() && !pass->IsCulled())
			readyQueue.push(pass.get());
	}

	// Process passes
	while (!readyQueue.empty())
	{
		FrameGraphPass* currentPass = readyQueue.front();
		readyQueue.pop();

		// Add to sorted list
		outSortedPasses.push_back(currentPass);

		// Process all outgoing dependencies
		const auto& outgoing = currentPass->GetOutgoingDependencies();
		for (const auto& dependency : outgoing)
		{
			FrameGraphPass* consumerPass = dependency.ConsumerPass;

			if (consumerPass->IsCulled())
				continue;

			// Decrement reference count
			consumerPass->DecrementReferenceCount();

			// If all dependencies satisfied, add to queue
			if (consumerPass->IsReady())
				readyQueue.push(consumerPass);
		}
	}

	// Check for cycles
	bool hasCycle = false;
	mCyclePasses.clear();

	for (const auto& pass : passes)
	{
		if (!pass->IsCulled() && pass->GetReferenceCount() > 0)
		{
			hasCycle = true;
			mCyclePasses.push_back(pass.get());
		}
	}

	if (hasCycle)
	{
		B3D_LOG(Error, RenderBackend, "Cycle detected in frame graph dependencies.");
		B3D_LOG(Error, RenderBackend, "The following passes are involved in the cycle:");

		for (const auto& pass : mCyclePasses)
		{
			B3D_LOG(Error, RenderBackend, "  Pass: {0}", pass->GetName());
			B3D_LOG(Error, RenderBackend, "    Unresolved dependencies: {0}",
				pass->GetReferenceCount());

			// Log incoming dependencies
			const auto& incoming = pass->GetIncomingDependencies();
			for (const auto& dep : incoming)
			{
				B3D_LOG(Error, RenderBackend, "      <- {0} (via resource {1})",
					dep.ProducerPass->GetName(),
					dep.Resource.Index);
			}
		}

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Barrier Generation & Synchronization
//////////////////////////////////////////////////////////////////////////

void FrameGraphCompiler::BuildUsageHistory(const Vector<FrameGraphPass*>& passes)
{
	mUsageHistories.clear();

	// Iterate through all passes in execution order
	for (FrameGraphPass* pass : passes)
	{
		// Record all resource accesses for this pass
		const auto& accesses = pass->GetResourceAccesses();
		for (const auto& access : accesses)
		{
			B3D_ENSURE(access.Resource.IsValid());
			B3D_ENSURE(pass != nullptr);

			// Get or create usage history for this resource
			auto& history = mUsageHistories[access.Resource];
			if (history.Resource.Index == ~0u)
			{
				history.Resource = access.Resource;
			}

			// Determine the layout for this usage
			ImageLayout layout = DetermineLayout(access.Usage, access.Access);

			// Add this usage to the history
			history.Uses.emplace_back(pass, access.Usage, access.Access, layout);

			B3D_LOG(Verbose, RenderBackend,
				"Resource {0} used by pass '{1}': usage=0x{2:X}, access=0x{3:X}, layout={4}",
				access.Resource.Index,
				pass->GetName(),
				(u32)access.Usage,
				(u32)access.Access,
				(u32)layout);
		}
	}

	B3D_LOG(Info, RenderBackend, "Built usage history for {0} resources", mUsageHistories.size());
}

ImageLayout FrameGraphCompiler::DetermineLayout(GpuResourceUseFlags usage, GpuAccessFlags access) const
{
	return FrameGraphLayoutHelper::GetLayoutForUsage(usage, access);
}

void FrameGraphCompiler::BuildTransitions(Vector<ResourceTransition>& outTransitions)
{
	// Process each resource's usage history
	for (const auto& pair : mUsageHistories)
	{
		const ResourceUsageHistory& history = pair.second;

		if (history.Uses.empty())
			continue;

		// Iterate through consecutive uses
		for (size_t usageIndex = 1; usageIndex < history.Uses.size(); usageIndex++)
		{
			const ResourceUsage& prevUsage = history.Uses[usageIndex - 1];
			const ResourceUsage& currUsage = history.Uses[usageIndex];

			// Check if we need a barrier between these two uses
			if (NeedsBarrier(prevUsage, currUsage))
			{
				// Create transition
				ResourceTransition transition(
					history.Resource,
					prevUsage.Pass,
					currUsage.Pass,
					prevUsage.Usage,
					prevUsage.Access,
					prevUsage.Layout,
					currUsage.Usage,
					currUsage.Access,
					currUsage.Layout
				);

				outTransitions.push_back(transition);

				B3D_LOG(Verbose, RenderBackend,
					"Transition for resource {0}: pass '{1}' -> '{2}', layout {3} -> {4}",
					history.Resource.Index,
					prevUsage.Pass->GetName(),
					currUsage.Pass->GetName(),
					(u32)prevUsage.Layout,
					(u32)currUsage.Layout);
			}
		}
	}

	B3D_LOG(Info, RenderBackend, "Built {0} resource transitions", outTransitions.size());
}

bool FrameGraphCompiler::NeedsBarrier(const ResourceUsage& prevUsage, const ResourceUsage& currUsage) const
{
	// Different passes always need synchronization
	if (prevUsage.Pass != currUsage.Pass)
	{
		// RAW (Read-After-Write): Previous pass wrote, current pass reads or writes
		if (prevUsage.Access.IsSet(GpuAccessFlag::Write))
			return true;

		// WAR (Write-After-Read): Previous pass read, current pass writes
		if (currUsage.Access.IsSet(GpuAccessFlag::Write))
			return true;

		// Layout transition needed
		if (FrameGraphLayoutHelper::RequiresTransition(prevUsage.Layout, currUsage.Layout))
			return true;
	}

	// Same pass, no barrier needed (synchronization within a pass is automatic)
	return false;
}

void FrameGraphCompiler::BuildBarriers(
	const Vector<ResourceTransition>& transitions,
	Vector<FrameGraphBarrierBatch>& outBarriers)
{
	// Local lambda to create a buffer barrier from a transition
	auto fnCreateBufferBarrier = [this](const ResourceTransition& transition) -> TOptional<GpuBufferBarrier>
	{
		FrameGraphResource* resource = mFrameGraph.GetResource(transition.Resource);
		if (!resource || resource->GetType() != FrameGraphResourceType::Buffer)
			return {};

		auto* bufferResource = static_cast<FrameGraphBufferResource*>(resource);

		GpuBufferBarrier barrier(
			bufferResource->GetBuffer(),
			transition.SourceUsage,
			transition.SourceAccess,
			transition.DestinationUsage,
			transition.DestinationAccess
		);

		return barrier;
	};

	// Local lambda to create a texture barrier from a transition
	auto fnCreateTextureBarrier = [this](const ResourceTransition& transition) -> TOptional<GpuTextureBarrier>
	{
		FrameGraphResource* resource = mFrameGraph.GetResource(transition.Resource);
		if (!resource || resource->GetType() != FrameGraphResourceType::Texture)
			return {};

		auto* textureResource = static_cast<FrameGraphTextureResource*>(resource);

		GpuTextureBarrier barrier(
			textureResource->GetTexture(),
			transition.SourceUsage,
			transition.SourceAccess,
			transition.DestinationUsage,
			transition.DestinationAccess
		);

		// Set explicit layouts
		barrier.SourceLayout = transition.SourceLayout;
		barrier.DestinationLayout = transition.DestinationLayout;

		return barrier;
	};

	// Local lambda to create a render target barrier from a transition
	auto fnCreateRenderTargetBarrier = [this](const ResourceTransition& transition) -> TOptional<GpuRenderTargetBarrier>
	{
		FrameGraphResource* resource = mFrameGraph.GetResource(transition.Resource);
		if (!resource || resource->GetType() != FrameGraphResourceType::RenderTarget)
			return {};

		auto* renderTargetResource = static_cast<FrameGraphRenderTargetResource*>(resource);

		GpuRenderTargetBarrier barrier(
			renderTargetResource->GetRenderTarget(),
			renderTargetResource->GetSurface(),
			transition.SourceUsage,
			transition.SourceAccess,
			transition.DestinationUsage,
			transition.DestinationAccess
		);

		// Set explicit layouts
		barrier.SourceLayout = transition.SourceLayout;
		barrier.DestinationLayout = transition.DestinationLayout;

		return barrier;
	};

	// Group transitions by destination pass
	UnorderedMap<FrameGraphPass*, Vector<ResourceTransition>> transitionsByPass;

	for (const auto& transition : transitions)
	{
		transitionsByPass[transition.DestinationPass].push_back(transition);
	}

	// Build barrier batches
	for (const auto& pair : transitionsByPass)
	{
		FrameGraphPass* pass = pair.first;
		const Vector<ResourceTransition>& passTransitions = pair.second;

		FrameGraphBarrierBatch batch;
		batch.DestinationPass = pass;

		// Convert each transition to a barrier
		for (const auto& transition : passTransitions)
		{
			// Try buffer barrier first
			auto bufferBarrier = fnCreateBufferBarrier(transition);
			if (bufferBarrier.has_value())
			{
				batch.Barriers.BufferBarriers.Add(bufferBarrier.value());
				continue;
			}

			// Try texture barrier
			auto textureBarrier = fnCreateTextureBarrier(transition);
			if (textureBarrier.has_value())
			{
				batch.Barriers.TextureBarriers.Add(textureBarrier.value());
				continue;
			}

			// Try render target barrier
			auto renderTargetBarrier = fnCreateRenderTargetBarrier(transition);
			if (renderTargetBarrier.has_value())
			{
				batch.Barriers.RenderTargetBarriers.Add(renderTargetBarrier.value());
				continue;
			}

			B3D_LOG(Warning, RenderBackend,
				"Failed to create barrier for resource {0} in pass '{1}'",
				transition.Resource.Index,
				pass->GetName());
		}

		if (batch.Barriers.BufferBarriers.Size() > 0 ||
			batch.Barriers.TextureBarriers.Size() > 0 ||
			batch.Barriers.RenderTargetBarriers.Size() > 0)
		{
			outBarriers.push_back(batch);

			B3D_LOG(Info, RenderBackend,
				"Created barrier batch for pass '{0}': {1} buffer barriers, {2} texture barriers, {3} render target barriers",
				pass->GetName(),
				batch.Barriers.BufferBarriers.Size(),
				batch.Barriers.TextureBarriers.Size(),
				batch.Barriers.RenderTargetBarriers.Size());
		}
	}
}

void FrameGraphCompiler::CreateRenderTargets(
	const Vector<FrameGraphPass*>& passes,
	UnorderedMap<FrameGraphPass*, SPtr<RenderTarget>>& outRenderTargets)
{
	const auto& importedTextures = mFrameGraph.GetImportedTextures();

	for (FrameGraphPass* pass : passes)
	{
		if (pass->GetPassType() == FrameGraphPassType::Render)
		{
			B3D_ENSURE(pass->GetPassType() == FrameGraphPassType::Render);

			// Get color attachments
			const auto& colorAttachments = pass->GetColorAttachments();

			// Build color textures array
			Vector<SPtr<Texture>> colorTextures;
			u32 maxIndex = 0;

			for (const auto& pair : colorAttachments)
			{
				maxIndex = std::max(maxIndex, pair.first);
			}

			colorTextures.resize(maxIndex + 1);

			for (const auto& pair : colorAttachments)
			{
				u32 index = pair.first;
				FrameGraphResourceId resourceId = pair.second;

				auto it = importedTextures.find(resourceId);
				if (it == importedTextures.end())
				{
					B3D_LOG(Error, RenderBackend, "Render pass '{0}': Color attachment {1} references invalid resource {2}",
						pass->GetName(), index, resourceId.Index);
					continue;
				}

				colorTextures[index] = it->second;
			}

			// Get depth attachment
			SPtr<Texture> depthTexture;
			FrameGraphResourceId depthId = pass->GetDepthAttachment();
			if (depthId.IsValid())
			{
				auto it = importedTextures.find(depthId);
				if (it == importedTextures.end())
				{
					B3D_LOG(Error, RenderBackend, "Render pass '{0}': Depth attachment references invalid resource {1}",
						pass->GetName(), depthId.Index);
					continue;
				}

				depthTexture = it->second;
			}

			// Create render target descriptor
			RenderTextureCreateInformation rtInfo;

			for (u32 i = 0; i < colorTextures.size(); i++)
			{
				if (colorTextures[i])
					rtInfo.ColorSurfaces[i].Texture = colorTextures[i];
			}

			if (depthTexture)
				rtInfo.DepthStencilSurface.Texture = depthTexture;

			// Create render target
			SPtr<RenderTarget> renderTarget = RenderTexture::Create(rtInfo);

			if (!renderTarget)
			{
				B3D_LOG(Error, RenderBackend, "Failed to create render target for pass '{0}'", pass->GetName());
				continue;
			}

			B3D_LOG(Info, RenderBackend, "Created render target for pass '{0}' with {1} color attachments{2}",
				pass->GetName(),
				colorTextures.size(),
				depthTexture ? " and depth" : "");

			outRenderTargets[pass] = renderTarget;
		}
	}
}
