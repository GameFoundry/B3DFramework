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
	// Execute setup functions to populate resource accesses
	ExecuteSetupFunctions();

	// Validate
	if (!Validate())
	{
		B3D_LOG(Error, RenderBackend, "Frame graph validation failed");
		return nullptr;
	}

	// Create compiled graph
	auto compiled = B3DMakeUnique<CompiledFrameGraph>();

	// Phase 1: Just store passes in declaration order
	for (const auto& pass : mFrameGraph.GetPasses())
	{
		compiled->Passes.push_back(pass.get());
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
