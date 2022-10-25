//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Image/BsTexture.h"
#include "Image/BsPixelData.h"
#include "RenderAPI/BsIndexBuffer.h"
#include "RenderAPI/BsVertexData.h"
#include "RenderAPI/BsSamplerState.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Helper class that maps engine types to DirectX 11 types. */
		class D3D11Mappings
		{
		public:
			/**	Converts engine to DX11 specific texture addressing mode. */
			static D3D11_TEXTURE_ADDRESS_MODE Get(TextureAddressingMode tam);

			/**	Converts engine to DX11 specific blend factor. */
			static D3D11_BLEND Get(BlendFactor bf);

			/**	Converts engine to DX11 specific blend operation. */
			static D3D11_BLEND_OP Get(BlendOperation bo);

			/**	Converts engine to DX11 specific comparison function. */
			static D3D11_COMPARISON_FUNC Get(CompareFunction cf);

			/**	Converts engine to DX11 specific culling mode. */
			static D3D11_CULL_MODE Get(CullingMode cm);

			/**	Converts engine to DX11 specific polygon fill mode. */
			static D3D11_FILL_MODE Get(PolygonMode mode);

			/** Return DirectX 11 stencil operation and optionally invert it (greater than becomes less than, etc.). */
			static D3D11_STENCIL_OP Get(StencilOperation op, bool invert = false);

			/**
			 * Converts engine texture filter type to DirectX 11 filter shift (used for combining to get actual min/mag/mip
			 * filter bit location).
			 */
			static DWORD Get(FilterType ft);

			/**
			 * Returns DirectX 11 texture filter from the provided min, mag and mip filter options, and optionally a filter
			 * with comparison support.
			 */
			static D3D11_FILTER Get(const FilterOptions min, const FilterOptions mag, const FilterOptions mip, const bool comparison = false);

			/**	Converts engine to DX11 buffer usage. */
			static DWORD Get(GpuBufferUsage usage);

			/** Converts engine to DX11 lock options, while also constraining the options depending on provided usage type. */
			static D3D11_MAP Get(GpuLockOptions options, GpuBufferUsage usage);

			/**	Converts engine to DX11 vertex element type. */
			static DXGI_FORMAT Get(VertexElementType type);

			/**	Returns a string describing the provided vertex element semantic. */
			static LPCSTR Get(VertexElementSemantic sem);

			/**
			 * Returns engine semantic from the provided semantic string. Throws an exception for semantics that do not exist.
			 */
			static VertexElementSemantic Get(LPCSTR sem);

			/**	Converts DirectX 11 GPU parameter component type to engine vertex element type. */
			static VertexElementType GetInputType(D3D_REGISTER_COMPONENT_TYPE type);

			/**	Returns DX11 primitive topology based on the provided draw operation type. */
			static D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveType(DrawOperationType type);

			/**	Converts engine color to DX11 color. */
			static void Get(const Color& inColor, float* outColor);

			/**	Checks does the provided map value include writing. */
			static bool IsMappingWrite(D3D11_MAP map);

			/**	Checks does the provided map value include reading. */
			static bool IsMappingRead(D3D11_MAP map);

			/**	Converts DX11 pixel format to engine pixel format. */
			static PixelFormat GetPf(DXGI_FORMAT d3dPF);

			/**
			 * Converts engine pixel format to DX11 pixel format. Some formats depend on whether hardware gamma is used or not,
			 * in which case set the @p hwGamma parameter as needed.
			 */
			static DXGI_FORMAT GetPf(PixelFormat format, bool hwGamma);

			/** Converts engine GPU buffer format to DX11 GPU buffer format. */
			static DXGI_FORMAT GetBf(GpuBufferFormat format);

			/**
			 * Returns a typeless version of a depth stencil format. Required for creating a depth stencil texture it can be
			 * bound both for shader reading and depth/stencil writing.
			 */
			static DXGI_FORMAT GetTypelessDepthStencilPf(PixelFormat format);

			/** Returns a format of a depth stencil texture that can be used for reading the texture in the shader. */
			static DXGI_FORMAT GetShaderResourceDepthStencilPf(PixelFormat format);

			/**	Converts engine to DX11 buffer usage. */
			static D3D11_USAGE GetUsage(GpuBufferUsage mUsage);

			/**	Converts engine to DX11 buffer access flags. */
			static UINT GetAccessFlags(GpuBufferUsage mUsage);

			/**	Converts engine to DX11 lock options. */
			static D3D11_MAP GetLockOptions(GpuLockOptions lockOptions);

			/**	Checks is the provided buffer usage dynamic. */
			static bool IsDynamic(GpuBufferUsage mUsage);

			/**	Finds the closest pixel format that DX11 supports. */
			static PixelFormat GetClosestSupportedPf(PixelFormat format, TextureType texType, int usage);

			/**
			 * Returns size in bytes of a pixel surface of the specified size and format, while using DX11 allocation rules for
			 * padding.
			 */
			static u32 GetSizeInBytes(PixelFormat pf, u32 width = 1, u32 height = 1);
		};

		/** @} */
	} // namespace ct
} // namespace bs
