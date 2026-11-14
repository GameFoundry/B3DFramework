//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "B3DHLSLShaderABI.h"
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
			/**
			 * Converts an engine pixel format to DXGI format. When @p sRGB is set, formats with an sRGB DXGI
			 * variant map to it (so sampling decodes to linear); formats without one keep their linear variant.
			 */
			static DXGI_FORMAT GetDXGIFormat(PixelFormat format, bool sRGB = false);

			/** Returns the resource format compatible with views using @p viewFormat. */
			static DXGI_FORMAT GetTextureResourceFormat(DXGI_FORMAT viewFormat);

			/** Returns the shader-resource view format compatible with @p viewFormat. */
			static DXGI_FORMAT GetShaderResourceViewFormat(DXGI_FORMAT viewFormat);

			/** Returns the shader resource view dimension of a texture view of @p viewType, of a texture with multiple samples if @p isMultisampled is true. */
			static D3D12_SRV_DIMENSION GetShaderResourceViewDimension(TextureViewType viewType, bool isMultisampled);

			/** Returns the unordered access view dimension of a texture view of @p viewType. Cube views have no unordered access dimension. */
			static D3D12_UAV_DIMENSION GetUnorderedAccessViewDimension(TextureViewType viewType);

			/** Converts engine buffer flags to D3D12 resource flags. */
			static D3D12_RESOURCE_FLAGS GetBufferResourceFlags(GpuBufferFlags flags);

			/** Converts engine comparison function to D3D12 comparison function. */
			static D3D12_COMPARISON_FUNC GetComparisonFunc(CompareFunction func);

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
		};

		/** @} */
	} // namespace render
} // namespace b3d
