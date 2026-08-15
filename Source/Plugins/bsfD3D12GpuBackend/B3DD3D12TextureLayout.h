//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DD3D12Prerequisites.h"
#include "GpuBackend/B3DGpuTextureSubresource.h"

namespace b3d::render
{
	/** Exact enhanced-barrier layouts committed for the planes of one texture subresource. */
	class D3D12TextureLayout
	{
	public:
		/** Creates an undefined layout for every plane. */
		D3D12TextureLayout() : D3D12TextureLayout(D3D12_BARRIER_LAYOUT_UNDEFINED)
		{ }

		/** Uses @p layout for every plane. */
		explicit D3D12TextureLayout(D3D12_BARRIER_LAYOUT layout) : mColorOrDepthLayout(layout), mStencilLayout(layout)
		{ }

		/** Uses separate layouts for the color/depth and stencil planes. */
		D3D12TextureLayout(D3D12_BARRIER_LAYOUT colorOrDepthLayout, D3D12_BARRIER_LAYOUT stencilLayout) : mColorOrDepthLayout(colorOrDepthLayout), mStencilLayout(stencilLayout)
		{ }

		/** Returns a layout with invalid contents and no previous access. */
		static D3D12TextureLayout Undefined() { return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_UNDEFINED); }

		/** Returns the queue-neutral COMMON layout used for queue handoffs. */
		static D3D12TextureLayout Common() { return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_COMMON); }

		/** Returns the presentation-engine layout. */
		static D3D12TextureLayout Present() { return D3D12TextureLayout(D3D12_BARRIER_LAYOUT_PRESENT); }

		/** Returns the native layout for the single aspect selected by @p aspects. */
		D3D12_BARRIER_LAYOUT GetLayout(GpuTextureAspectFlags aspects) const
		{
			const bool hasDepth = aspects.IsSet(GpuTextureAspectFlag::Depth);
			const bool hasStencil = aspects.IsSet(GpuTextureAspectFlag::Stencil);
			B3D_ASSERT(!hasDepth || !hasStencil || mColorOrDepthLayout == mStencilLayout);

			return hasStencil && !hasDepth ? mStencilLayout : mColorOrDepthLayout;
		}

		/** Returns whether every selected aspect has no defined native layout. */
		bool IsUndefined(GpuTextureAspectFlags aspects) const
		{
			if(aspects.IsSetAny(GpuTextureAspectFlag::Color | GpuTextureAspectFlag::Depth) && mColorOrDepthLayout != D3D12_BARRIER_LAYOUT_UNDEFINED)
				return false;

			return !aspects.IsSet(GpuTextureAspectFlag::Stencil) || mStencilLayout == D3D12_BARRIER_LAYOUT_UNDEFINED;
		}

		/** Returns whether depth and stencil require separate native barriers. */
		bool HasSeparateDepthStencilLayouts() const { return mColorOrDepthLayout != mStencilLayout; }

		/** Returns whether both plane layouts match @p other. */
		bool operator==(const D3D12TextureLayout& other) const { return mColorOrDepthLayout == other.mColorOrDepthLayout && mStencilLayout == other.mStencilLayout; }

		/** Returns whether either plane layout differs from @p other. */
		bool operator!=(const D3D12TextureLayout& other) const { return !(*this == other); }

	private:
		D3D12_BARRIER_LAYOUT mColorOrDepthLayout;
		D3D12_BARRIER_LAYOUT mStencilLayout;
	};
} // namespace b3d::render
