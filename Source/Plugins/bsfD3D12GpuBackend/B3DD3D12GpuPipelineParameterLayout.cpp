//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

#include <algorithm>

using namespace b3d;
using namespace b3d::render;

D3D12GpuPipelineParameterSetLayout::D3D12GpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription)
	: GpuPipelineParameterSetLayout(parameterDescription)
{ }

D3D12GpuPipelineParameterLayout::D3D12GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation, D3D12GpuDevice& device) : GpuPipelineParameterLayout(device, createInformation), mDevice(device)
{
	CreateRootSignature();
}

D3D12GpuPipelineParameterLayout::~D3D12GpuPipelineParameterLayout()
{
	if (mRootSignature != nullptr)
		mRootSignature->Destroy();
}

void D3D12GpuPipelineParameterLayout::CreateRootSignature()
{
	constexpr u32 kReservedRootParameterRegisterSpace = 0xFFFFu;
	const u32 setCount = (u32)mSets.Size();

	u32 totalBindingCount = 0;
	for (u32 setIndex = 0; setIndex < setCount; setIndex++)
		totalBindingCount += mSets[setIndex]->GetBindingCount();

	mDescriptorSetLayouts.clear();
	mDescriptorSetLayouts.resize(setCount);

	Vector<D3D12_ROOT_PARAMETER> rootParameters;
	Vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;

	rootParameters.reserve(1 + kD3D12DynamicConstantBufferCount + setCount * 2);
	descriptorRanges.reserve(totalBindingCount + setCount * 2);

	auto fnGetShaderVisibility = [](const GpuProgramStageBits& stageBits) -> D3D12_SHADER_VISIBILITY
	{
		if(stageBits.IsSet(GpuProgramStageBit::Compute))
			return D3D12_SHADER_VISIBILITY_ALL;

		u32 stageCount = 0;
		D3D12_SHADER_VISIBILITY lastVisibility = D3D12_SHADER_VISIBILITY_ALL;

		if (stageBits.IsSet(GpuProgramStageBit::Vertex))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		}
		if (stageBits.IsSet(GpuProgramStageBit::Fragment))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}
		if (stageBits.IsSet(GpuProgramStageBit::Geometry))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
		}
		if (stageBits.IsSet(GpuProgramStageBit::Hull))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_HULL;
		}
		if (stageBits.IsSet(GpuProgramStageBit::Domain))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
		}

		// If used in multiple stages, make it visible to all
		if (stageCount > 1)
			return D3D12_SHADER_VISIBILITY_ALL;

		return lastVisibility;
	};

	auto fnGetDescriptorRangeType = [](GpuParameterType parameterType, GpuParameterObjectType objectType) -> D3D12_DESCRIPTOR_RANGE_TYPE
	{
		switch (parameterType)
		{
		case GpuParameterType::UniformBuffer:
			return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

		case GpuParameterType::SampledTexture:
			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

		case GpuParameterType::StorageTexture:
			return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

		case GpuParameterType::Sampler:
			return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;

		case GpuParameterType::StorageBuffer:
			switch (objectType)
			{
			case GPOT_BYTE_BUFFER:
			case GPOT_STRUCTURED_BUFFER:
				return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

			case GPOT_RWBYTE_BUFFER:
			case GPOT_RWSTRUCTURED_BUFFER:
			case GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER:
			case GPOT_RWTYPED_BUFFER:
			case GPOT_RWAPPEND_BUFFER:
			case GPOT_RWCONSUME_BUFFER:
				return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;

			default:
				return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			}

		default:
			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		}
	};

	// Root parameter 0 reserves four DWORDs for push-constants. 
	// TODO - Until that API exists no shader references this reserved register and command buffers leave the values unset.
	D3D12_ROOT_PARAMETER rootConstantParameter = {};
	rootConstantParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootConstantParameter.Constants.ShaderRegister = 0;
	rootConstantParameter.Constants.RegisterSpace = kReservedRootParameterRegisterSpace;
	rootConstantParameter.Constants.Num32BitValues = kD3D12RootConstantValueCount;
	rootConstantParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters.push_back(rootConstantParameter);

	// Reserve a stable block of root parameters for dynamic uniform buffers. Unused entries occupy registers in a
	// private space so every following descriptor table retains the same root index.
	for(u32 rootBufferIndex = 0; rootBufferIndex < kD3D12DynamicConstantBufferCount; rootBufferIndex++)
	{
		D3D12_ROOT_PARAMETER rootBufferParameter = {};
		rootBufferParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootBufferParameter.Descriptor.ShaderRegister = rootBufferIndex + 1;
		rootBufferParameter.Descriptor.RegisterSpace = kReservedRootParameterRegisterSpace;
		rootBufferParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParameters.push_back(rootBufferParameter);
	}

	// TODO(d3d12): Add a dynamic-offset flag to buffer bindings and a matching buffer capability flag. Only flagged
	// bindings should consume these root CBVs; Vulkan should select its dynamic descriptor types from the same flag,
	// and core pipeline-layout validation should enforce the shared limit of kD3D12DynamicConstantBufferCount.

	// Determine root constant buffers
	u32 rootBufferCount = 0;
	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		const TShared<GpuPipelineParameterSetLayout>& setLayout = mSets[setIndex];
		const u32 uniformBufferCount = setLayout->GetBindingCount(GpuParameterType::UniformBuffer);
		TInlineArray<const UniformInformation*, 8> uniformBuffers;
		uniformBuffers.reserve(uniformBufferCount);

		for(u32 sequentialIndex = 0; sequentialIndex < uniformBufferCount; sequentialIndex++)
		{
			const UniformInformation* uniformInformation = setLayout->TryGetUniformInformation(GpuParameterType::UniformBuffer, sequentialIndex);
			if(uniformInformation != nullptr)
				uniformBuffers.Add(uniformInformation);
		}

		std::sort(uniformBuffers.begin(), uniformBuffers.end(), [](const UniformInformation* lhs, const UniformInformation* rhs)
		{
			return MapSlotToRegister(lhs->Slot) < MapSlotToRegister(rhs->Slot);
		});

		for(const UniformInformation* uniformInformation : uniformBuffers)
		{
			if(uniformInformation->ArraySize != 1)
				continue;

			if(rootBufferCount >= kD3D12DynamicConstantBufferCount)
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12 root signature requires more than the supported {0} dynamic uniform buffers.", kD3D12DynamicConstantBufferCount);
				return;
			}

			const u32 rootParameterIndex = 1 + rootBufferCount++;
			D3D12_ROOT_PARAMETER& rootBufferParameter = rootParameters[rootParameterIndex];
			rootBufferParameter.Descriptor.ShaderRegister = MapSlotToRegister(uniformInformation->Slot);
			rootBufferParameter.Descriptor.RegisterSpace = setIndex;
			rootBufferParameter.ShaderVisibility = fnGetShaderVisibility(uniformInformation->Usage);

			D3D12RootConstantBufferLayout rootBufferLayout;
			rootBufferLayout.Slot = uniformInformation->Slot;
			rootBufferLayout.DynamicOffsetIndex = uniformInformation->DynamicOffsetIndex;
			rootBufferLayout.RootParameterIndex = rootParameterIndex;
			mDescriptorSetLayouts[setIndex].RootConstantBuffers.Add(rootBufferLayout);
		}
	}

	Vector<GpuProgramStageBits> resourceTableStages(setCount, GpuProgramStageBit::None);
	Vector<GpuProgramStageBits> samplerTableStages(setCount, GpuProgramStageBit::None);

	// Determine bindings for all resource tables (a pair per set)
	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		const TShared<GpuPipelineParameterSetLayout>& setLayout = mSets[setIndex];
		D3D12DescriptorSetLayout& descriptorSetLayout = mDescriptorSetLayouts[setIndex];

		for(u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
		{
			const GpuParameterType type = (GpuParameterType)typeIndex;
			const u32 typeBindingCount = setLayout->GetBindingCount(type);

			for(u32 sequentialIndex = 0; sequentialIndex < typeBindingCount; sequentialIndex++)
			{
				const UniformInformation* uniformInformation = setLayout->TryGetUniformInformation(type, sequentialIndex);
				if(uniformInformation == nullptr)
					continue;

				if(type == GpuParameterType::UniformBuffer && uniformInformation->ArraySize == 1)
					continue;

				D3D12DescriptorBindingLayout bindingLayout;
				bindingLayout.Type = type;
				bindingLayout.ObjectType = uniformInformation->ObjectType;
				bindingLayout.RangeType = fnGetDescriptorRangeType(type, uniformInformation->ObjectType);
				bindingLayout.Slot = uniformInformation->Slot;
				bindingLayout.ShaderRegister = MapSlotToRegister(uniformInformation->Slot);
				bindingLayout.DescriptorCount = uniformInformation->ArraySize;

				if(type == GpuParameterType::Sampler)
				{
					descriptorSetLayout.SamplerTable.Bindings.Add(bindingLayout);
					samplerTableStages[setIndex] |= uniformInformation->Usage;
				}
				else
				{
					descriptorSetLayout.ResourceTable.Bindings.Add(bindingLayout);
					resourceTableStages[setIndex] |= uniformInformation->Usage;
				}
			}
		}
	}

	auto fnPackTable = [](D3D12DescriptorTableLayout& tableLayout)
	{
		std::sort(tableLayout.Bindings.begin(), tableLayout.Bindings.end(), [](const D3D12DescriptorBindingLayout& lhs, const D3D12DescriptorBindingLayout& rhs)
		{
			if(lhs.RangeType != rhs.RangeType)
				return lhs.RangeType < rhs.RangeType;

			return lhs.ShaderRegister < rhs.ShaderRegister;
		});

		for(D3D12DescriptorBindingLayout& bindingLayout : tableLayout.Bindings)
		{
			bindingLayout.TableOffset = tableLayout.DescriptorCount;

			D3D12DescriptorRangeLayout* previousRange = tableLayout.Ranges.size() == 0 ? nullptr : &tableLayout.Ranges.back();
			const bool extendsPreviousRange = previousRange != nullptr && previousRange->Type == bindingLayout.RangeType && previousRange->BaseShaderRegister + previousRange->DescriptorCount == bindingLayout.ShaderRegister;
			if(extendsPreviousRange)
			{
				previousRange->DescriptorCount += bindingLayout.DescriptorCount;
			}
			else
			{
				D3D12DescriptorRangeLayout rangeLayout;
				rangeLayout.Type = bindingLayout.RangeType;
				rangeLayout.BaseShaderRegister = bindingLayout.ShaderRegister;
				rangeLayout.DescriptorCount = bindingLayout.DescriptorCount;
				rangeLayout.TableOffset = bindingLayout.TableOffset;

				tableLayout.Ranges.Add(rangeLayout);
			}

			tableLayout.DescriptorCount += bindingLayout.DescriptorCount;
		}
	};

	// Group sequential bindings into contiguous ranges
	for(D3D12DescriptorSetLayout& descriptorSetLayout : mDescriptorSetLayouts)
	{
		fnPackTable(descriptorSetLayout.ResourceTable);
		fnPackTable(descriptorSetLayout.SamplerTable);
	}

	auto fnAddTableRootParameter = [&rootParameters, &descriptorRanges, fnGetShaderVisibility, kReservedRootParameterRegisterSpace](D3D12DescriptorTableLayout& tableLayout, const GpuProgramStageBits& stages,
		u32 setIndex, D3D12_DESCRIPTOR_RANGE_TYPE dummyRangeType)
	{
		const u32 firstRangeIndex = (u32)descriptorRanges.size();
		if(tableLayout.Ranges.size() == 0)
		{
			D3D12_DESCRIPTOR_RANGE range = {};
			range.RangeType = dummyRangeType;
			range.NumDescriptors = 1;
			range.BaseShaderRegister = setIndex;
			range.RegisterSpace = kReservedRootParameterRegisterSpace;
			range.OffsetInDescriptorsFromTableStart = 0;

			descriptorRanges.push_back(range);
		}
		else
		{
			for(const D3D12DescriptorRangeLayout& rangeLayout : tableLayout.Ranges)
			{
				D3D12_DESCRIPTOR_RANGE range = {};
				range.RangeType = rangeLayout.Type;
				range.NumDescriptors = rangeLayout.DescriptorCount;
				range.BaseShaderRegister = rangeLayout.BaseShaderRegister;
				range.RegisterSpace = setIndex;
				range.OffsetInDescriptorsFromTableStart = rangeLayout.TableOffset;

				descriptorRanges.push_back(range);
			}
		}

		tableLayout.RootParameterIndex = (u32)rootParameters.size();

		D3D12_ROOT_PARAMETER rootParameter = {};
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable.NumDescriptorRanges = (UINT)descriptorRanges.size() - firstRangeIndex;
		rootParameter.DescriptorTable.pDescriptorRanges = descriptorRanges.data() + firstRangeIndex;
		rootParameter.ShaderVisibility = tableLayout.DescriptorCount > 0 ? fnGetShaderVisibility(stages) : D3D12_SHADER_VISIBILITY_ALL;
	
		rootParameters.push_back(rootParameter);
	};

	// Build root parameters for each table
	for(u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		D3D12DescriptorSetLayout& descriptorSetLayout = mDescriptorSetLayouts[setIndex];
		fnAddTableRootParameter(descriptorSetLayout.ResourceTable, resourceTableStages[setIndex], setIndex, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
		fnAddTableRootParameter(descriptorSetLayout.SamplerTable, samplerTableStages[setIndex], setIndex, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER);
	}

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = (UINT)rootParameters.size();
	rootSignatureDesc.pParameters = rootParameters.empty() ? nullptr : rootParameters.data();
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	if (FAILED(hr))
	{
		if (error)
			B3D_LOG(Error, LogRenderBackend, "Failed to serialize root signature: {0}", (const char*)error->GetBufferPointer());

		return;
	}

	ComPtr<ID3D12RootSignature> rootSignature;
	hr = mDevice.GetD3D12Device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));

	if (FAILED(hr))
		B3D_LOG(Error, LogRenderBackend, "Failed to create root signature");
	else
	{
		mRootSignature = mDevice.GetResourceManager().Create<D3D12RootSignature>(std::move(rootSignature), "D3D12 root signature");
		B3D_LOG(Verbose, LogRenderBackend, "Created root signature with {0} parameters", rootParameters.size());
	}
}
