//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphCompiler.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

FrameGraphCompiler::FrameGraphCompiler(FrameGraph& frameGraph)
	: mFrameGraph(frameGraph)
{
}

UPtr<CompiledFrameGraph> FrameGraphCompiler::Compile()
{
	B3D_LOG(Info, RenderBackend, "Compiling frame graph...");

	// Phase 1: Validation and setup
	if (!Validate())
	{
		B3D_LOG(Error, RenderBackend, "Frame graph validation failed");
		return nullptr;
	}

	ExecuteSetupFunctions();

	// Phase 2: Dependency analysis
	mDependencyAnalyzer = B3DMakeUnique<FrameGraphDependencyAnalyzer>(mFrameGraph);
	if (!mDependencyAnalyzer->Analyze())
	{
		B3D_LOG(Error, RenderBackend, "Dependency analysis failed");
		return nullptr;
	}

	// Phase 2: Dependency validation
	if (!ValidateDependencies(mDependencyAnalyzer->GetPassNodes()))
	{
		B3D_LOG(Warning, RenderBackend, "Dependency validation found potential issues");
	}

	// Phase 2: Pass culling
	mCulling = B3DMakeUnique<FrameGraphCulling>(mFrameGraph);
	auto& nodes = const_cast<Vector<UPtr<FrameGraphPassNode>>&>(mDependencyAnalyzer->GetPassNodes());
	mCulling->Cull(nodes, mDependencyAnalyzer->GetResourceLifetimes());

	if (mCulling->GetCulledPassCount() > 0)
	{
		B3D_LOG(Info, RenderBackend, "Culled {0} unused passes", mCulling->GetCulledPassCount());
	}

	// Phase 2: Topological sort
	mTopologicalSort = B3DMakeUnique<FrameGraphTopologicalSort>();
	if (!mTopologicalSort->Sort(mDependencyAnalyzer->GetPassNodes(), mSortedPasses))
	{
		B3D_LOG(Error, RenderBackend, "Topological sort failed - cycle detected");
		return nullptr;
	}

	B3D_LOG(Info, RenderBackend, "Frame graph compiled successfully. Execution order:");
	for (u32 i = 0; i < mSortedPasses.size(); i++)
	{
		B3D_LOG(Info, RenderBackend, "  {0}. {1}", i + 1, mSortedPasses[i]->GetPass()->GetName());
	}

	// Create compiled graph
	auto compiled = B3DMakeUnique<CompiledFrameGraph>();
	compiled->SortedPasses = mSortedPasses;
	compiled->ResourceLifetimes = mDependencyAnalyzer->GetResourceLifetimes();

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

	// Phase 1: No dependency analysis or cycle detection
	// Phase 2 will add: dependency DAG, topological sort, cycle detection

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
	const bool isRead = access.Access.IsSet(GpuAccessFlag::Read);
	const bool isWrite = access.Access.IsSet(GpuAccessFlag::Write);
	const bool isAttachment = access.Usage.IsSetAny(GpuResourceUseFlag::ColorAttachment | GpuResourceUseFlag::DepthStencilAttachment);

	// Attachments must be write or read-write
	if (isAttachment && !isWrite) // TODO - Not really, we can have read-only attachments
	{
		B3D_LOG(Error, RenderBackend,
			"Pass '{0}': Resource '{1}' used as attachment but not marked for write access",
			pass->GetName(), resource->GetName());
		return false;
	}

	// Phase 1: No advanced validation (write-after-write conflicts, read-after-write hazards, etc.)
	// Phase 3 will add: barrier calculation, hazard detection, layout validation

	return true;
}

bool FrameGraphCompiler::ValidateDependencies(const Vector<UPtr<FrameGraphPassNode>>& nodes)
{
	bool valid = true;

	// Check for passes with no inputs or outputs (isolated passes)
	for (const auto& node : nodes)
	{
		if (node->IsCulled())
			continue;

		FrameGraphPass* pass = node->GetPass();
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

		if (!hasWrite && node->GetOutgoingDependencies().empty())
		{
			B3D_LOG(Warning, RenderBackend,
				"Pass '{0}' only reads resources and has no outgoing dependencies. "
				"It will be culled unless it writes to an output resource.",
				pass->GetName());
		}
	}

	return valid;
}
