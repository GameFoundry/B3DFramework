//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12GpuPipelineParameterLayout.h"
#include "B3DD3D12Utility.h"
#include "B3DD3D12GpuDevice.h"
#include "GpuBackend/B3DGpuProgramParameterDescription.h"

using namespace b3d;
using namespace b3d::render;

D3D12GpuPipelineParameterLayout::D3D12GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation, D3D12GpuDevice& device)
	: GpuPipelineParameterLayout(device, createInformation)
	, mDevice(device)
{
	CreateRootSignature();
}

D3D12GpuPipelineParameterLayout::~D3D12GpuPipelineParameterLayout()
{
	mRootSignature.Reset();
}

void D3D12GpuPipelineParameterLayout::CreateRootSignature()
{
	// Count the number of root parameters we need
	const u32 setCount = (u32)mSets.Size();

	// Calculate total number of descriptor ranges
	u32 totalDescriptorRangeCount = 0;
	for (u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		// Each set may contain multiple types of descriptors
		// We need to count how many ranges we need
		totalDescriptorRangeCount += mSets[setIndex]->GetBindingCount();
	}

	if (totalDescriptorRangeCount == 0)
	{
		// Create empty root signature
		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.NumParameters = 0;
		rootSignatureDesc.pParameters = nullptr;
		rootSignatureDesc.NumStaticSamplers = 0;
		rootSignatureDesc.pStaticSamplers = nullptr;
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

		if (FAILED(hr))
		{
			if (error)
			{
				B3D_LOG(Error, LogRenderBackend, "Failed to serialize root signature: {0}",
					(const char*)error->GetBufferPointer());
			}
			return;
		}

		hr = mDevice.GetD3D12Device()->CreateRootSignature(
			0,
			signature->GetBufferPointer(),
			signature->GetBufferSize(),
			IID_PPV_ARGS(&mRootSignature)
		);

		B3D_ASSERT(SUCCEEDED(hr) && "Failed to create root signature");
		return;
	}

	// Allocate space for root parameters and descriptor ranges
	Vector<D3D12_ROOT_PARAMETER> rootParameters;
	Vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;

	rootParameters.reserve(totalDescriptorRangeCount);
	descriptorRanges.reserve(totalDescriptorRangeCount);

	// Helper lambda to convert shader stage bits to D3D12 visibility
	auto getShaderVisibility = [](const GpuProgramStageBits& bits) -> D3D12_SHADER_VISIBILITY
	{
		u32 stageCount = 0;
		D3D12_SHADER_VISIBILITY lastVisibility = D3D12_SHADER_VISIBILITY_ALL;

		if (bits.IsSet(GpuProgramStageBit::Vertex))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		}
		if (bits.IsSet(GpuProgramStageBit::Fragment))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		}
		if (bits.IsSet(GpuProgramStageBit::Geometry))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_GEOMETRY;
		}
		if (bits.IsSet(GpuProgramStageBit::Hull))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_HULL;
		}
		if (bits.IsSet(GpuProgramStageBit::Domain))
		{
			stageCount++;
			lastVisibility = D3D12_SHADER_VISIBILITY_DOMAIN;
		}

		// If used in multiple stages, make it visible to all
		if (stageCount > 1)
			return D3D12_SHADER_VISIBILITY_ALL;

		return lastVisibility;
	};

	// Helper lambda to get descriptor range type
	auto getDescriptorRangeType = [](GpuParameterType paramType, GpuParameterObjectType objectType) -> D3D12_DESCRIPTOR_RANGE_TYPE
	{
		switch (paramType)
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
			// Differentiate between read-only and read-write
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

	// Build root parameters from uniform information
	for (u32 setIndex = 0; setIndex < setCount; setIndex++)
	{
		const TShared<GpuPipelineParameterSetLayout>& setLayout = mSets[setIndex];
		const u32 bindingCount = setLayout->GetBindingCount();

		// Iterate through all parameter types to find all uniforms
		for (u32 typeIndex = 0; typeIndex < (u32)GpuParameterType::Count; typeIndex++)
		{
			const GpuParameterType type = (GpuParameterType)typeIndex;
			const u32 typeBindingCount = setLayout->GetBindingCount(type);

			for (u32 sequentialIndex = 0; sequentialIndex < typeBindingCount; sequentialIndex++)
			{
				const UniformInformation* uniformInfo = setLayout->TryGetUniformInformation(type, sequentialIndex);
				if (uniformInfo == nullptr)
					continue;

				// Create descriptor range
				D3D12_DESCRIPTOR_RANGE range = {};
				range.RangeType = getDescriptorRangeType(uniformInfo->Type, uniformInfo->ObjectType);
				range.NumDescriptors = uniformInfo->ArraySize;
				range.BaseShaderRegister = MapSlotToRegister(uniformInfo->Slot); // Slots encode the HLSL register, see MapRegisterToSlot()
				range.RegisterSpace = setIndex; // Use set as register space
				range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				u32 rangeIndex = (u32)descriptorRanges.size();
				descriptorRanges.push_back(range);

				// Create root parameter for this descriptor table
				D3D12_ROOT_PARAMETER rootParam = {};
				rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
				rootParam.ShaderVisibility = getShaderVisibility(uniformInfo->Usage);
				rootParam.DescriptorTable.NumDescriptorRanges = 1;
				rootParam.DescriptorTable.pDescriptorRanges = &descriptorRanges[rangeIndex];

				rootParameters.push_back(rootParam);
			}
		}
	}

	// Create root signature
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = (UINT)rootParameters.size();
	rootSignatureDesc.pParameters = rootParameters.data();
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

	if (FAILED(hr))
	{
		if (error)
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to serialize root signature: {0}",
				(const char*)error->GetBufferPointer());
		}
		return;
	}

	hr = mDevice.GetD3D12Device()->CreateRootSignature(
		0,
		signature->GetBufferPointer(),
		signature->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)
	);

	if (FAILED(hr))
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create root signature");
	}
	else
	{
		B3D_LOG(Info, LogRenderBackend, "Created root signature with {0} parameters", rootParameters.size());
	}
}
