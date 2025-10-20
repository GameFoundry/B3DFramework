//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphCulling.h"
#include "B3DFrameGraph.h"
#include "B3DFrameGraphPass.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

FrameGraphCulling::FrameGraphCulling(FrameGraph& frameGraph)
	: mFrameGraph(frameGraph)
	, mCulledPassCount(0)
{
}

void FrameGraphCulling::Cull(Vector<UPtr<FrameGraphPassNode>>& nodes,
	const UnorderedMap<FrameGraphResourceId, FrameGraphResourceLifetime>& resourceLifetimes)
{
	mCulledPassCount = 0;

	// Step 1: Mark all passes as culled initially
	for (auto& node : nodes)
		node->SetCulled(true);

	// Step 2: Find passes that write to output resources
	// In Phase 2, we consider a resource an output if:
	// - It's explicitly marked as output, OR
	// - It's imported AND written to (will be used outside frame graph)
	Vector<FrameGraphPassNode*> outputPasses;

	for (const auto& pair : resourceLifetimes)
	{
		const auto& lifetime = pair.second;

		// Check if this is an output resource
		bool isOutput = lifetime.IsOutput || (lifetime.IsImported && lifetime.IsWritten);

		if (isOutput && lifetime.LastUse != nullptr)
		{
			FrameGraphPassNode* node = FindNode(lifetime.LastUse, nodes);
			if (node)
				outputPasses.push_back(node);
		}
	}

	// Step 3: Perform reverse DFS from output passes
	for (FrameGraphPassNode* outputPass : outputPasses)
		MarkPassAsUsed(outputPass, nodes);

	// Step 4: Count culled passes
	for (const auto& node : nodes)
	{
		if (node->IsCulled())
		{
			mCulledPassCount++;
			B3D_LOG(Info, RenderBackend, "Culled unused pass: {0}", node->GetPass()->GetName());
		}
	}
}

void FrameGraphCulling::MarkPassAsUsed(FrameGraphPassNode* node,
	const Vector<UPtr<FrameGraphPassNode>>& allNodes)
{
	// Already marked as used
	if (!node->IsCulled())
		return;

	// Mark this pass as used
	node->SetCulled(false);

	// Recursively mark all incoming dependencies as used
	const auto& incoming = node->GetIncomingDependencies();
	for (const auto& dependency : incoming)
	{
		FrameGraphPassNode* producerNode = FindNode(dependency.ProducerPass, allNodes);
		if (producerNode)
			MarkPassAsUsed(producerNode, allNodes);
	}
}

FrameGraphPassNode* FrameGraphCulling::FindNode(FrameGraphPass* pass,
	const Vector<UPtr<FrameGraphPassNode>>& nodes)
{
	for (const auto& node : nodes)
	{
		if (node->GetPass() == pass)
			return node.get();
	}

	return nullptr;
}
