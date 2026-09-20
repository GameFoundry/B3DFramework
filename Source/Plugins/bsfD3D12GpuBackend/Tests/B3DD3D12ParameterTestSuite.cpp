//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12ParameterTestSuite.h"
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuDevice.h"

using namespace b3d;

D3D12ParameterTestSuite::D3D12ParameterTestSuite() : TestSuite("D3D12ParameterTestSuite")
{
	B3D_ADD_TEST(D3D12ParameterTestSuite::TestUniformBufferLayout)
}

void D3D12ParameterTestSuite::TestUniformBufferLayout()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(String(backend.GetBackendName()) != "bsfD3D12GpuBackend" || backend.GetDeviceCount() == 0)
		return;

	GpuDevice* const device = backend.GetDevice(0).get();

	// More than eight static bindings remain in tables; eight dynamic bindings can still use the full root block.
	for(u32 dynamicCount : { 0u, 2u, render::kD3D12DynamicConstantBufferCount })
	{
		const u32 staticCount = dynamicCount == render::kD3D12DynamicConstantBufferCount ? 6 : 10;
		GpuPipelineParameterLayoutCreateInformation createInformation;
		createInformation.Compute = B3DMakeShared<GpuProgramParameterDescription>();
		for(u32 bindingIndex = 0; bindingIndex < staticCount + dynamicCount; bindingIndex++)
		{
			GpuUniformBufferInformation buffer;
			buffer.Name = "Buffer" + ToString(bindingIndex);
			buffer.Set = bindingIndex % 2;
			buffer.Slot = device->GetUniformBufferParameterSlot(bindingIndex / 2);
			buffer.Size = 4;
			buffer.UsesDynamicOffset = bindingIndex >= staticCount;
			createInformation.Compute->UniformBuffers[buffer.Name] = buffer;
		}

		const TShared<GpuPipelineParameterLayout> layout = device->CreateGpuPipelineParameterLayout(createInformation);
		B3D_TEST_ASSERT(layout != nullptr)
		if(layout == nullptr)
			return;

		const auto& nativeLayout = static_cast<const render::D3D12GpuPipelineParameterLayout&>(*layout);
		B3D_TEST_ASSERT(nativeLayout.GetRootSignature() != nullptr)
		B3D_TEST_ASSERT(layout->GetSetCount() == 2)
		for(u32 setIndex = 0; setIndex < layout->GetSetCount(); setIndex++)
		{
			const render::D3D12DescriptorSetLayout& setLayout = nativeLayout.GetDescriptorSetLayout(setIndex);
			B3D_TEST_ASSERT(setLayout.RootConstantBuffers.size() == dynamicCount / 2)
			B3D_TEST_ASSERT(setLayout.ResourceTable.DescriptorCount == staticCount / 2)
			B3D_TEST_ASSERT(setLayout.ResourceTable.RootParameterIndex == 1 + render::kD3D12DynamicConstantBufferCount + setIndex * 2)
			for(const auto& binding : setLayout.ResourceTable.Bindings)
			{
				B3D_TEST_ASSERT(binding.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
				B3D_TEST_ASSERT(layout->GetSet(setIndex)->GetDynamicOffsetIndex(binding.Slot) == ~0u)
			}
			for(const auto& binding : setLayout.RootConstantBuffers)
				B3D_TEST_ASSERT(layout->GetSet(setIndex)->GetDynamicOffsetIndex(binding.Slot) != ~0u)
		}
	}
}
