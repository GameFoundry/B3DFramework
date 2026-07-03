//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DPixelUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup D3D12GpuBackend
		 *  @{
		 */

		/** Utility class containing helper methods for D3D12 conversions and operations. */
		class D3D12Utility
		{
		public:
			/** Converts an engine pixel format to DXGI format. */
			static DXGI_FORMAT GetDXGIFormat(PixelFormat format);

			/** Converts a DXGI format to engine pixel format. */
			static PixelFormat GetPixelFormat(DXGI_FORMAT format);

			/** Converts engine buffer flags to D3D12 resource flags. */
			static D3D12_RESOURCE_FLAGS GetBufferResourceFlags(GpuBufferFlags flags);

			/** Converts engine texture usage flags to D3D12 resource flags. */
			static D3D12_RESOURCE_FLAGS GetTextureResourceFlags(TextureUsageFlags usage);

			/** Determines the D3D12 heap type to place a buffer of the provided type and flags in. */
			static D3D12_HEAP_TYPE GetHeapType(GpuBufferType type, GpuBufferFlags flags);

			/** Converts engine comparison function to D3D12 comparison function. */
			static D3D12_COMPARISON_FUNC GetComparisonFunc(CompareFunction func);

			/** Converts engine texture addressing mode to D3D12 texture address mode. */
			static D3D12_TEXTURE_ADDRESS_MODE GetTextureAddressMode(TextureAddressingMode mode);

			/** Converts engine filter options to D3D12 filter. */
			static D3D12_FILTER GetFilter(FilterOptions minFilter, FilterOptions magFilter, FilterOptions mipFilter);

			/** Converts engine blend factor to D3D12 blend. */
			static D3D12_BLEND GetBlend(BlendFactor factor);

			/** Converts engine blend operation to D3D12 blend operation. */
			static D3D12_BLEND_OP GetBlendOp(BlendOperation op);

			/** Converts engine cull mode to D3D12 cull mode. */
			static D3D12_CULL_MODE GetCullMode(CullingMode mode);

			/** Converts engine fill mode to D3D12 fill mode. */
			static D3D12_FILL_MODE GetFillMode(PolygonMode mode);

			/** Converts engine stencil operation to D3D12 stencil operation. */
			static D3D12_STENCIL_OP GetStencilOp(StencilOperation op);

			/** Converts engine draw operation to D3D12 primitive topology. */
			static D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology(DrawOperationType drawOp);

			/** Converts engine draw operation to D3D12 primitive topology type. */
			static D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(DrawOperationType drawOp);

			/** Returns the number of bytes per pixel for the specified format. */
			static u32 GetBytesPerPixel(DXGI_FORMAT format);

			/** Checks if the specified format is a depth-stencil format. */
			static bool IsDepthStencilFormat(DXGI_FORMAT format);

			/**
			 * Calculates the size and offset for a constant buffer element according to HLSL packing rules.
			 * Updates the offset parameter to the next available offset after the element.
			 *
			 * @param type			Data type of the parameter.
			 * @param arraySize		Number of elements in the array (1 if not an array).
			 * @param offset		Current offset in the constant buffer (in float4 units). Will be updated.
			 * @return				Size of the element in float4 units.
			 */
			static u32 CalcConstantBufferElementSizeAndOffset(GpuDataParameterType type, u32 arraySize, u32& offset);
		};

		/** HLSL shader register classes, used for encoding engine parameter slots. See MapRegisterToSlot(). */
		enum class D3D12RegisterClass
		{
			ConstantBuffer = 0,	 // b registers
			ShaderResource = 1,	 // t registers
			UnorderedAccess = 2, // u registers
			Sampler = 3,		 // s registers

			Count = 4
		};

		/**
		 * Maps an HLSL shader register to an engine parameter slot. HLSL registers are only unique within their register
		 * class (b/t/u/s), while engine slots must be unique across all parameters of a set, so the class is encoded
		 * into the slot. Inverted by MapSlotToRegister().
		 */
		constexpr u32 MapRegisterToSlot(u32 registerIndex, D3D12RegisterClass registerClass)
		{
			return registerIndex * (u32)D3D12RegisterClass::Count + (u32)registerClass;
		}

		/** Recovers the HLSL shader register from an engine parameter slot encoded by MapRegisterToSlot(). */
		constexpr u32 MapSlotToRegister(u32 slot)
		{
			return slot / (u32)D3D12RegisterClass::Count;
		}

		/** @} */
	} // namespace render
} // namespace b3d
