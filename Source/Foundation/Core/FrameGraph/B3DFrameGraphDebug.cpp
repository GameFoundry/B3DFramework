//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphDebug.h"
#include "B3DFrameGraphPass.h"
#include "String/B3DStringFormat.h"

using namespace b3d;
using namespace b3d::render;

String FrameGraphDebug::GenerateGraphText(const Vector<UPtr<FrameGraphPassNode>>& nodes)
{
	String result = "Frame Graph Dependencies:\n";
	result += "=========================\n\n";

	for (const auto& node : nodes)
	{
		FrameGraphPass* pass = node->GetPass();
		result += StringFormat("Pass: {0}\n", pass->GetName());

		if (node->IsCulled())
		{
			result += "  [CULLED]\n";
		}
		else
		{
			result += StringFormat("  Reference Count: {0}\n", node->GetReferenceCount());

			const auto& incoming = node->GetIncomingDependencies();
			if (!incoming.empty())
			{
				result += "  Incoming Dependencies:\n";
				for (const auto& dep : incoming)
				{
					const char* typeStr = "???";
					switch (dep.DependencyType)
					{
					case FrameGraphPassDependency::Type::ReadAfterWrite:
						typeStr = "RAW";
						break;
					case FrameGraphPassDependency::Type::WriteAfterRead:
						typeStr = "WAR";
						break;
					case FrameGraphPassDependency::Type::WriteAfterWrite:
						typeStr = "WAW";
						break;
					}

					result += StringFormat("    <- {0} (Resource {1}, {2})\n",
						dep.ProducerPass->GetName(),
						dep.Resource.Index,
						typeStr);
				}
			}

			const auto& outgoing = node->GetOutgoingDependencies();
			if (!outgoing.empty())
			{
				result += "  Outgoing Dependencies:\n";
				for (const auto& dep : outgoing)
				{
					result += StringFormat("    -> {0} (Resource {1})\n",
						dep.ConsumerPass->GetName(),
						dep.Resource.Index);
				}
			}
		}

		result += "\n";
	}

	return result;
}

String FrameGraphDebug::GenerateGraphDOT(const Vector<UPtr<FrameGraphPassNode>>& nodes)
{
	String result = "digraph FrameGraph {\n";
	result += "  rankdir=TB;\n";
	result += "  node [shape=box];\n\n";

	// Add nodes
	for (u32 i = 0; i < nodes.size(); i++)
	{
		const auto& node = nodes[i];
		FrameGraphPass* pass = node->GetPass();

		String nodeStyle = node->IsCulled() ?
			"style=filled,fillcolor=gray,fontcolor=white" :
			"style=filled,fillcolor=lightblue";

		result += StringFormat("  pass{0} [label=\"{1}\",{2}];\n",
			i, pass->GetName(), nodeStyle);
	}

	result += "\n";

	// Add edges
	for (u32 i = 0; i < nodes.size(); i++)
	{
		const auto& node = nodes[i];
		const auto& outgoing = node->GetOutgoingDependencies();

		for (const auto& dep : outgoing)
		{
			// Find consumer index
			u32 consumerIndex = 0;
			for (u32 j = 0; j < nodes.size(); j++)
			{
				if (nodes[j]->GetPass() == dep.ConsumerPass)
				{
					consumerIndex = j;
					break;
				}
			}

			const char* depType = "???";
			String edgeColor = "black";
			switch (dep.DependencyType)
			{
			case FrameGraphPassDependency::Type::ReadAfterWrite:
				depType = "RAW";
				edgeColor = "blue";
				break;
			case FrameGraphPassDependency::Type::WriteAfterRead:
				depType = "WAR";
				edgeColor = "orange";
				break;
			case FrameGraphPassDependency::Type::WriteAfterWrite:
				depType = "WAW";
				edgeColor = "red";
				break;
			}

			result += StringFormat("  pass{0} -> pass{1} [label=\"R{2}\\n{3}\",color={4}];\n",
				i, consumerIndex, dep.Resource.Index, depType, edgeColor);
		}
	}

	result += "}\n";
	return result;
}
