//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Represents a part of a texture. */
	enum class GpuTextureAspectFlag
	{
		Color = 1 << 0, /**< Color plane. */
		Depth = 1 << 1, /**< Depth plane. */
		Stencil = 1 << 2, /**< Stencil plane. */
	};

	using GpuTextureAspectFlags = Flags<GpuTextureAspectFlag>;
	B3D_FLAGS_OPERATORS(GpuTextureAspectFlag)

	/** Stable order used when expanding a texture aspect mask. */
	constexpr std::array<GpuTextureAspectFlag, 3> kGpuTextureAspects = {
		GpuTextureAspectFlag::Color,
		GpuTextureAspectFlag::Depth,
		GpuTextureAspectFlag::Stencil
	};

	/** Represents a range of subresources in a texture. */
	struct GpuTextureSubresourceRange
	{
		GpuTextureSubresourceRange(u32 baseMipLevel = 0, u32 mipLevelCount = 1, u32 baseArrayLayer = 0, u32 arrayLayerCount = 1, GpuTextureAspectFlags aspectMask = GpuTextureAspectFlag::Color)
			:BaseMipLevel(baseMipLevel), MipLevelCount(mipLevelCount), BaseArrayLayer(baseArrayLayer), ArrayLayerCount(arrayLayerCount), AspectMask(aspectMask)
		{ }

		/** Creates a subresource range covering only the highest (first) mip level. */
		static GpuTextureSubresourceRange TopMip(u32 baseArrayLayer = 0, u32 arrayLayerCount = 1, GpuTextureAspectFlags aspectMask = GpuTextureAspectFlag::Color | GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil)
		{
			return GpuTextureSubresourceRange(0, 1, baseArrayLayer, arrayLayerCount, aspectMask);
		}

		/** Creates a subresource range covering all subresources. */
		static GpuTextureSubresourceRange AllSubresources(GpuTextureAspectFlags aspectMask = GpuTextureAspectFlag::Color | GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil)
		{
			return GpuTextureSubresourceRange(0, ~0u, 0, ~0u, aspectMask);
		}

		/** Returns whether the range identifies exactly one texture aspect. */
		bool HasSingleAspect() const
		{
			const u32 aspectBits = (u32)AspectMask;
			return aspectBits != 0 && (aspectBits & (aspectBits - 1)) == 0;
		}

		/** Returns the number of texture aspects covered by the range. */
		u32 GetAspectCount() const
		{
			u32 count = 0;
			for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
			{
				if(AspectMask.IsSet(aspect))
					count++;
			}

			return count;
		}

		u32 BaseMipLevel = 0;
		u32 MipLevelCount = 1;
		u32 BaseArrayLayer = 0;
		u32 ArrayLayerCount = 1;
		GpuTextureAspectFlags AspectMask = GpuTextureAspectFlag::Color;
	};

	/** Identifies a single texture subresource. */
	struct GpuTextureSubresource
	{
		u32 MipLevel = 0; /**< Mip level index. */
		u32 ArrayLayer = 0; /**< Array layer or cube face index. */
		GpuTextureAspectFlag Aspect = GpuTextureAspectFlag::Color; /**< Image aspect. */

		GpuTextureSubresource(u32 mipLevel = 0, u32 arrayLayer = 0, GpuTextureAspectFlag aspect = GpuTextureAspectFlag::Color)
			: MipLevel(mipLevel), ArrayLayer(arrayLayer), Aspect(aspect)
		{ }

		bool operator==(const GpuTextureSubresource& other) const
		{
			return ArrayLayer == other.ArrayLayer && MipLevel == other.MipLevel && Aspect == other.Aspect;
		}
	};

	/** @} */
} // namespace b3d
