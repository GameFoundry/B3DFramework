//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "Image/B3DPixelData.h"
#include "Utility/B3DCommonTypes.h"
#include "GpuBackend/B3DVertexDescription.h"

namespace b3d
{
	struct GpuBufferInformation;

	namespace render
	{
		/**
		 * Returns true when the provided engine pixel format has a Metal pixel-format mapping (in
		 * the requested gamma variant). Unlike MetalUtility::GetPixelFormat this is callable from
		 * plain C++ translation units (e.g. the sim-side texture manager), where Metal types are
		 * unavailable.
		 */
		bool IsMetalPixelFormatSupported(PixelFormat format, bool gamma);
	} // namespace render
} // namespace b3d

#ifdef __OBJC__

namespace b3d
{
	namespace render
	{
		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Utility helpers for converting engine types to their Metal equivalents.
		 *
		 * Only visible from Objective-C++ translation units.
		 */
		class MetalUtility
		{
		public:
			/**
			 * Converts a B3D pixel format to the closest matching MTLPixelFormat.
			 *
			 * @param	format	Engine pixel format.
			 * @param	gamma	If true, the hardware sRGB variant of the format is returned when available.
			 *
			 * @return	Matching MTLPixelFormat, or MTLPixelFormatInvalid if no suitable mapping exists.
			 */
			static MTLPixelFormat GetPixelFormat(PixelFormat format, bool gamma);

			/** Converts an engine texture type to the Metal texture-target enum. */
			static MTLTextureType GetTextureType(TextureType type, u32 sampleCount, u32 arraySliceCount);

			/** Converts an engine filter + address mode bundle to a freshly configured MTLSamplerDescriptor. */
			static MTLSamplerMinMagFilter GetMinMagFilter(FilterOptions filter);

			/** Returns the MSAA mip filter for the given engine filter. */
			static MTLSamplerMipFilter GetMipFilter(FilterOptions filter);

			/** Converts an engine texture addressing mode to its Metal equivalent. */
			static MTLSamplerAddressMode GetAddressMode(TextureAddressingMode mode);

			/** Converts an engine compare function to its Metal equivalent. */
			static MTLCompareFunction GetCompareFunction(CompareFunction func);

			/** Converts an engine blend factor to its Metal equivalent. */
			static MTLBlendFactor GetBlendFactor(BlendFactor factor);

			/** Converts an engine blend operation to its Metal equivalent. */
			static MTLBlendOperation GetBlendOperation(BlendOperation op);

			/** Converts an engine culling mode to its Metal equivalent. */
			static MTLCullMode GetCullMode(CullingMode mode);

			/** Returns the front-face winding order used by Metal given an engine culling mode. */
			static MTLWinding GetFrontFaceWinding(CullingMode mode);

			/** Converts an engine polygon mode to the Metal triangle fill mode. */
			static MTLTriangleFillMode GetFillMode(PolygonMode mode);

			/** Converts an engine stencil operation to its Metal equivalent. */
			static MTLStencilOperation GetStencilOperation(StencilOperation op);

			/** Converts a DrawOperationType to a Metal primitive type used in draw calls. */
			static MTLPrimitiveType GetPrimitiveType(DrawOperationType op);

			/** Converts a DrawOperationType to the Metal primitive topology class used in a pipeline descriptor. */
			static MTLPrimitiveTopologyClass GetPrimitiveTopologyClass(DrawOperationType op);

			/** Converts an engine vertex element type to a Metal vertex attribute format. */
			static MTLVertexFormat GetVertexFormat(VertexElementType type);

			/**
			 * Returns the number of bytes per row for a single slice of a texture subresource with the
			 * given format and width. Accounts for block-compressed formats.
			 */
			static u32 GetTextureRowPitch(PixelFormat format, u32 width);

			/**
			 * Returns the number of bytes occupied by a full 2D slice of a texture with the given
			 * format and dimensions. For block-compressed formats the height is rounded up to a block.
			 */
			static u32 GetTextureSlicePitch(PixelFormat format, u32 width, u32 height);

			/**
			 * Maps a buffer's engine usage (type + flags) to the MTLStorageMode of its backing
			 * memory. Buffers created with @c GpuBufferFlag::StoreOnCPUWithGPUAccess, or typed as
			 * @c StagingRead / @c StagingWrite, are CPU-visible and use shared storage; everything
			 * else is GPU-private.
			 */
			static MTLStorageMode GetBufferStorageMode(const GpuBufferInformation& information);

			/**
			 * Composes the flat @c MTLResourceOptions mask for a resource with the provided storage
			 * mode. CPU cache mode stays at the default. Hazard tracking follows
			 * @c B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION.
			 */
			static MTLResourceOptions GetResourceOptions(MTLStorageMode storageMode);
		};

		/** @} */
	} // namespace render
} // namespace b3d

#endif // __OBJC__
