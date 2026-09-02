//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanPushConstantTestSuite.h"
#include "Managers/B3DVulkanDescriptorManager.h"

using namespace b3d;
using namespace b3d::render;

VulkanPushConstantTestSuite::VulkanPushConstantTestSuite()
	: TestSuite("VulkanPushConstantTestSuite")
{
	B3D_ADD_TEST(VulkanPushConstantTestSuite::TestPipelineLayoutKey)
}

void VulkanPushConstantTestSuite::TestPipelineLayoutKey()
{
	VkPushConstantRange vertexRange;
	vertexRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	vertexRange.offset = 0;
	vertexRange.size = 4;

	VkPushConstantRange largerVertexRange = vertexRange;
	largerVertexRange.size = 8;

	VkPushConstantRange offsetVertexRange = vertexRange;
	offsetVertexRange.offset = 4;

	VkPushConstantRange fragmentRange = vertexRange;
	fragmentRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VulkanPipelineLayoutKey noRange(nullptr, 0, {});
	VulkanPipelineLayoutKey vertex(nullptr, 0, vertexRange);
	VulkanPipelineLayoutKey sameVertex(nullptr, 0, vertexRange);
	VulkanPipelineLayoutKey largerVertex(nullptr, 0, largerVertexRange);
	VulkanPipelineLayoutKey offsetVertex(nullptr, 0, offsetVertexRange);
	VulkanPipelineLayoutKey fragment(nullptr, 0, fragmentRange);

	B3D_TEST_ASSERT(vertex == sameVertex)
	B3D_TEST_ASSERT(vertex.CalculateHash() == sameVertex.CalculateHash())
	B3D_TEST_ASSERT(!(noRange == vertex))
	B3D_TEST_ASSERT(!(vertex == largerVertex))
	B3D_TEST_ASSERT(!(vertex == offsetVertex))
	B3D_TEST_ASSERT(!(vertex == fragment))
}
