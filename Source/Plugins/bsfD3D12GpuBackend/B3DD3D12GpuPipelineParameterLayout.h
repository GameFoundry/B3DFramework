//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DD3D12Resource.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Root parameter occupied by the push-constant block. */
		constexpr u32 kD3D12PushConstantRootParameterIndex = 0;

		/** Number of root CBVs reserved for dynamically offset uniform buffers. */
		constexpr u32 kD3D12DynamicConstantBufferCount = 8;

		/** Describes one binding packed into a D3D12 descriptor table. */
		struct D3D12DescriptorBindingLayout
		{
			D3D12DescriptorBindingLayout() = default;

			GpuParameterType Type = GpuParameterType::Unknown; /**< Generic parameter type. */
			GpuParameterObjectType ObjectType = GPOT_UNKNOWN; /**< Shader object type used to select typed null descriptors. */
			D3D12_DESCRIPTOR_RANGE_TYPE RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; /**< Native descriptor type. */
			u32 Slot = ~0u; /**< Engine parameter slot. */
			u32 ShaderRegister = 0; /**< First HLSL register occupied by the binding. */
			u32 DescriptorCount = 0; /**< Number of descriptors occupied by the binding. */
			u32 TableOffset = 0; /**< First descriptor within the packed table. */
		};

		/** Describes a consecutive native range within a D3D12 descriptor table. */
		struct D3D12DescriptorRangeLayout
		{
			D3D12DescriptorRangeLayout() = default;

			D3D12_DESCRIPTOR_RANGE_TYPE Type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; /**< Native descriptor type. */
			u32 BaseShaderRegister = 0; /**< First HLSL register in the range. */
			u32 DescriptorCount = 0; /**< Number of consecutive registers in the range. */
			u32 TableOffset = 0; /**< First descriptor within the packed table. */
		};

		/** Describes one packed resource or sampler table belonging to a parameter set. */
		struct D3D12DescriptorTableLayout
		{
			D3D12DescriptorTableLayout() = default;

			TInlineArray<D3D12DescriptorBindingLayout, 16> Bindings; /**< Bindings in descriptor-table order. */
			TInlineArray<D3D12DescriptorRangeLayout, 8> Ranges; /**< Consecutive native ranges covering the bindings. */
			u32 DescriptorCount = 0; /**< Total number of descriptors in the table. */
			u32 RootParameterIndex = ~0u; /**< Root parameter used to bind the table. */
		};

		/** Describes one uniform buffer assigned to a dynamic root-CBV position. */
		struct D3D12RootConstantBufferLayout
		{
			D3D12RootConstantBufferLayout() = default;

			u32 Slot = ~0u; /**< Engine parameter slot. */
			u32 RootParameterIndex = ~0u; /**< Root parameter used to bind the CBV. */
		};

		/** D3D12 binding layout for one engine parameter set. */
		struct D3D12DescriptorSetLayout
		{
			D3D12DescriptorSetLayout() = default;

			D3D12DescriptorTableLayout ResourceTable; /**< Combined CBV/SRV/UAV table. */
			D3D12DescriptorTableLayout SamplerTable; /**< Sampler table stored in the sampler heap. */
			TInlineArray<D3D12RootConstantBufferLayout, 4> RootConstantBuffers; /**< Uniform buffers assigned to root CBVs. */
		};

		/** D3D12 parameter-set layout. */
		class D3D12GpuPipelineParameterSetLayout : public GpuPipelineParameterSetLayout
		{
		public:
			/** Creates a parameter-set layout from @p parameterDescription. */
			explicit D3D12GpuPipelineParameterSetLayout(const GpuProgramParameterDescription& parameterDescription);
		};

		/** DirectX 12 implementation of GPU pipeline parameter layout. */
		class D3D12GpuPipelineParameterLayout : public GpuPipelineParameterLayout
		{
		public:
			/** Creates the root signature described by @p createInformation. */
			D3D12GpuPipelineParameterLayout(const GpuPipelineParameterLayoutCreateInformation& createInformation, D3D12GpuDevice& device);
			~D3D12GpuPipelineParameterLayout() override;

			/** Returns the tracked D3D12 root signature. */
			D3D12RootSignature* GetRootSignature() const { return mRootSignature; }

			/** Returns the D3D12 binding layout for the given parameter set. */
			const D3D12DescriptorSetLayout& GetDescriptorSetLayout(u32 setIndex) const { return mDescriptorSetLayouts[setIndex]; }

		private:
			/** Creates the D3D12 root signature. */
			void CreateRootSignature();

			D3D12GpuDevice& mDevice;
			D3D12RootSignature* mRootSignature = nullptr;
			Vector<D3D12DescriptorSetLayout> mDescriptorSetLayouts; /**< Packed descriptor and root-CBV layouts per set. */
		};

		/** @} */
	} // namespace render
} // namespace b3d
