//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12SamplerState.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Utility.h"
#include "Managers/B3DD3D12DescriptorManager.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Converts engine texture addressing mode to D3D12 address mode. */
	D3D12_TEXTURE_ADDRESS_MODE GetD3D12AddressMode(TextureAddressingMode mode)
	{
		switch (mode)
		{
		case TAM_WRAP:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		case TAM_MIRROR:
			return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
		case TAM_CLAMP:
			return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		case TAM_BORDER:
			return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		default:
			return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		}
	}

	/** Converts engine filter options to D3D12 filter. */
	D3D12_FILTER GetD3D12Filter(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter, bool useComparison, bool useAnisotropic)
	{
		// Handle anisotropic filtering
		if (useAnisotropic)
		{
			return useComparison ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
		}

		// Build filter from individual components
		// D3D12 filter encoding: MIN_MAG_MIP_POINT, MIN_MAG_MIP_LINEAR, etc.

		bool minPoint = (minFilter == FO_POINT);
		bool magPoint = (magFilter == FO_POINT);
		bool mipPoint = (mipFilter == FO_POINT || mipFilter == FO_NONE);

		if (useComparison)
		{
			// Comparison filters
			if (minPoint && magPoint && mipPoint)
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			else if (minPoint && magPoint && !mipPoint)
				return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
			else if (minPoint && !magPoint && mipPoint)
				return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
			else if (minPoint && !magPoint && !mipPoint)
				return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
			else if (!minPoint && magPoint && mipPoint)
				return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
			else if (!minPoint && magPoint && !mipPoint)
				return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			else if (!minPoint && !magPoint && mipPoint)
				return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
			else
				return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		}
		else
		{
			// Normal filters
			if (minPoint && magPoint && mipPoint)
				return D3D12_FILTER_MIN_MAG_MIP_POINT;
			else if (minPoint && magPoint && !mipPoint)
				return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			else if (minPoint && !magPoint && mipPoint)
				return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
			else if (minPoint && !magPoint && !mipPoint)
				return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
			else if (!minPoint && magPoint && mipPoint)
				return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			else if (!minPoint && magPoint && !mipPoint)
				return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			else if (!minPoint && !magPoint && mipPoint)
				return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			else
				return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
	}
}

D3D12SamplerState::D3D12SamplerState(const SamplerStateCreateInformation& createInformation, GpuDevice& device)
	: SamplerState(createInformation)
	, mDevice(static_cast<D3D12GpuDevice&>(device))
{
}

D3D12SamplerState::~D3D12SamplerState()
{
	// Free the descriptor if it was allocated
	if (mDescriptorHandle.ptr != 0)
	{
		D3D12DescriptorManager& descriptorManager = mDevice.GetDescriptorManager();
		descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::Sampler, mDescriptorHandle);
		mDescriptorHandle.ptr = 0;
	}
}

void D3D12SamplerState::Initialize()
{
	SamplerState::Initialize();

	const SamplerStateInformation& info = GetInformation();

	// Initialize D3D12 sampler descriptor
	ZeroMemory(&mSamplerDesc, sizeof(D3D12_SAMPLER_DESC));

	// Convert addressing modes
	mSamplerDesc.AddressU = GetD3D12AddressMode(info.AddressMode.U);
	mSamplerDesc.AddressV = GetD3D12AddressMode(info.AddressMode.V);
	mSamplerDesc.AddressW = GetD3D12AddressMode(info.AddressMode.W);

	// Convert filter mode
	bool useComparison = (info.ComparisonFunc != CMPF_ALWAYS_PASS);
	bool useAnisotropic = (info.MaxAniso > 1);
	mSamplerDesc.Filter = GetD3D12Filter(info.MinFilter, info.MagFilter, info.MipFilter, useComparison, useAnisotropic);

	// Set mipmap parameters
	mSamplerDesc.MipLODBias = info.MipmapBias;
	mSamplerDesc.MinLOD = info.MipMin;
	mSamplerDesc.MaxLOD = info.MipMax;

	// Set anisotropy
	if (useAnisotropic)
	{
		mSamplerDesc.MaxAnisotropy = std::min(info.MaxAniso, 16u); // D3D12 max is 16
	}
	else
	{
		mSamplerDesc.MaxAnisotropy = 1;
	}

	// Set comparison function
	if (useComparison)
	{
		mSamplerDesc.ComparisonFunc = D3D12Utility::GetComparisonFunc(info.ComparisonFunc);
	}
	else
	{
		mSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	}

	// Set border color
	mSamplerDesc.BorderColor[0] = info.BorderColor.R;
	mSamplerDesc.BorderColor[1] = info.BorderColor.G;
	mSamplerDesc.BorderColor[2] = info.BorderColor.B;
	mSamplerDesc.BorderColor[3] = info.BorderColor.A;

	// Allocate a descriptor from the sampler heap
	D3D12DescriptorManager& descriptorManager = mDevice.GetDescriptorManager();
	mDescriptorHandle = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::Sampler);

	if (mDescriptorHandle.ptr == 0)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to allocate descriptor for sampler state");
		return;
	}

	// Create the sampler descriptor
	ID3D12Device* d3d12Device = mDevice.GetD3D12Device();
	d3d12Device->CreateSampler(&mSamplerDesc, mDescriptorHandle);

	B3D_LOG(Info, LogRenderBackend, "Created D3D12 sampler state: filter={0}, addressU={1}, addressV={2}, addressW={3}, maxAniso={4}",
		(u32)mSamplerDesc.Filter,
		(u32)mSamplerDesc.AddressU,
		(u32)mSamplerDesc.AddressV,
		(u32)mSamplerDesc.AddressW,
		mSamplerDesc.MaxAnisotropy);
}
