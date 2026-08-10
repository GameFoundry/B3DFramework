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

	/**
	 * Converts engine filter options to a D3D12 filter. @p useComparison selects the comparison variant of the
	 * filter, and @p useAnisotropic overrides the per-stage filters entirely.
	 */
	D3D12_FILTER GetD3D12Filter(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter, bool useComparison, bool useAnisotropic)
	{
		if (useAnisotropic)
			return useComparison ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;

		// D3D12 encodes the three per-stage filters as one bit each, in min/mag/mip order, with linear set and point
		// clear. D3D12_FILTER_COMPARISON_* is the same encoding offset by the comparison reduction type.
		u32 filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

		if (minFilter != FO_POINT)
			filter |= D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;

		if (magFilter != FO_POINT)
			filter |= D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;

		if (mipFilter != FO_POINT && mipFilter != FO_NONE)
			filter |= D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;

		if (useComparison)
			filter |= D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;

		return (D3D12_FILTER)filter;
	}
}

D3D12SamplerState::D3D12SamplerState(const SamplerStateCreateInformation& createInformation, GpuDevice& device)
	: SamplerState(createInformation), mDevice(static_cast<D3D12GpuDevice&>(device))
{
}

D3D12SamplerState::~D3D12SamplerState()
{
	if (mDescriptorHandle.ptr != 0)
		mDevice.GetDescriptorManager().FreeCPUDescriptor(D3D12DescriptorHeapType::Sampler, mDescriptorHandle);
}

void D3D12SamplerState::Initialize()
{
	SamplerState::Initialize();

	const SamplerStateInformation& information = GetInformation();
	const bool useComparison = information.ComparisonFunc != CMPF_ALWAYS_PASS;
	const bool useAnisotropic = information.MaxAniso > 1;

	D3D12_SAMPLER_DESC samplerDescription{};
	samplerDescription.AddressU = GetD3D12AddressMode(information.AddressMode.U);
	samplerDescription.AddressV = GetD3D12AddressMode(information.AddressMode.V);
	samplerDescription.AddressW = GetD3D12AddressMode(information.AddressMode.W);
	samplerDescription.Filter = GetD3D12Filter(information.MinFilter, information.MagFilter, information.MipFilter, useComparison, useAnisotropic);
	samplerDescription.MipLODBias = information.MipmapBias;
	samplerDescription.MinLOD = information.MipMin;
	samplerDescription.MaxLOD = information.MipMax;
	samplerDescription.MaxAnisotropy = useAnisotropic ? Math::Min(information.MaxAniso, 16u) : 1;
	samplerDescription.ComparisonFunc = useComparison ? D3D12Utility::GetComparisonFunc(information.ComparisonFunc) : D3D12_COMPARISON_FUNC_NEVER;
	samplerDescription.BorderColor[0] = information.BorderColor.R;
	samplerDescription.BorderColor[1] = information.BorderColor.G;
	samplerDescription.BorderColor[2] = information.BorderColor.B;
	samplerDescription.BorderColor[3] = information.BorderColor.A;

	mDescriptorHandle = mDevice.GetDescriptorManager().AllocateCPUDescriptor(D3D12DescriptorHeapType::Sampler);

	if (mDescriptorHandle.ptr == 0)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to allocate descriptor for sampler state");
		return;
	}

	mDevice.GetD3D12Device()->CreateSampler(&samplerDescription, mDescriptorHandle);
}
