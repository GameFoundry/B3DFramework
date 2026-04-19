//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanBuiltinResources.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuBuffer.h"

using namespace b3d;
using namespace b3d::render;

VulkanBuiltinResources::VulkanBuiltinResources(VulkanGpuDevice& gpuDevice)
	:mGpuDevice(gpuDevice)
{ }

void VulkanBuiltinResources::Initialize()
{
	DummyReadBuffer = std::static_pointer_cast<VulkanGpuBuffer>(mGpuDevice.CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, 1, GpuBufferFlag::StoreOnGPU)));
	DummyStorageBuffer = std::static_pointer_cast<VulkanGpuBuffer>(mGpuDevice.CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, 1, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU)));
	DummyUniformBuffer = std::static_pointer_cast<VulkanGpuBuffer>(mGpuDevice.CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(16, GpuBufferFlag::StoreOnGPU)));
	DummyStructuredBuffer = std::static_pointer_cast<VulkanGpuBuffer>(mGpuDevice.CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(16, 1, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU)));
	DummyVertexBuffer = std::static_pointer_cast<VulkanGpuBuffer>(mGpuDevice.CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(16, 1, GpuBufferFlag::StoreOnGPU)));
}

void VulkanBuiltinResources::Cleanup()
{
	DummyReadBuffer = nullptr;
	DummyStorageBuffer = nullptr;
	DummyUniformBuffer = nullptr;
	DummyStructuredBuffer = nullptr;
	DummyVertexBuffer = nullptr;
}
