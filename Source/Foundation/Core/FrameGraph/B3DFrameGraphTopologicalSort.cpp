//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphTopologicalSort.h"
#include "B3DFrameGraphPass.h"
#include "Debug/B3DDebug.h"
#include <queue>

using namespace b3d;
using namespace b3d::render;

bool FrameGraphTopologicalSort::Sort(const Vector<UPtr<FrameGraphPassNode>>& nodes,
	Vector<FrameGraphPassNode*>& outSortedNodes)
{
	outSortedNodes.clear();
	outSortedNodes.reserve(nodes.size());

	// Kahn's algorithm using a queue
	std::queue<FrameGraphPassNode*> readyQueue;

	// Find all nodes with no incoming dependencies
	for (const auto& node : nodes)
	{
		if (node->IsReady() && !node->IsCulled())
			readyQueue.push(node.get());
	}

	// Process nodes
	while (!readyQueue.empty())
	{
		FrameGraphPassNode* currentNode = readyQueue.front();
		readyQueue.pop();

		// Add to sorted list
		outSortedNodes.push_back(currentNode);

		// Process all outgoing dependencies
		const auto& outgoing = currentNode->GetOutgoingDependencies();
		for (const auto& dependency : outgoing)
		{
			// Find the consumer node
			FrameGraphPassNode* consumerNode = nullptr;
			for (const auto& node : nodes)
			{
				if (node->GetPass() == dependency.ConsumerPass)
				{
					consumerNode = node.get();
					break;
				}
			}

			if (!consumerNode || consumerNode->IsCulled())
				continue;

			// Decrement reference count
			consumerNode->DecrementReferenceCount();

			// If all dependencies satisfied, add to queue
			if (consumerNode->IsReady())
				readyQueue.push(consumerNode);
		}
	}

	// Check for cycles
	bool hasCycle = false;
	mCyclePasses.clear();

	for (const auto& node : nodes)
	{
		if (!node->IsCulled() && node->GetReferenceCount() > 0)
		{
			hasCycle = true;
			mCyclePasses.push_back(node->GetPass());
		}
	}

	if (hasCycle)
	{
		B3D_LOG(Error, RenderBackend, "Cycle detected in frame graph dependencies.");
		B3D_LOG(Error, RenderBackend, "The following passes are involved in the cycle:");

		for (const auto& pass : mCyclePasses)
		{
			B3D_LOG(Error, RenderBackend, "  Pass: {0}", pass->GetName());

			// Find the node
			FrameGraphPassNode* cycleNode = nullptr;
			for (const auto& node : nodes)
			{
				if (node->GetPass() == pass)
				{
					cycleNode = node.get();
					break;
				}
			}

			if (cycleNode)
			{
				B3D_LOG(Error, RenderBackend, "    Unresolved dependencies: {0}",
					cycleNode->GetReferenceCount());

				// Log incoming dependencies
				const auto& incoming = cycleNode->GetIncomingDependencies();
				for (const auto& dep : incoming)
				{
					B3D_LOG(Error, RenderBackend, "      <- {0} (via resource {1})",
						dep.ProducerPass->GetName(),
						dep.Resource.Index);
				}
			}
		}

		return false;
	}

	return true;
}
