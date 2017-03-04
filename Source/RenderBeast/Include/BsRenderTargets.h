//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsPixelUtil.h"
#include "BsRendererCamera.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/**
	 * Allocates and handles all the required render targets for rendering a scene from a specific view.
	 *
	 * @note	Core thread only.
	 */
	class RenderTargets
	{
	public:
		/**
		 * Creates a new set of render targets. This will not actually allocate the internal render targets - this happens
		 * the first time you call bind().
		 *
		 * @param[in]	view			Information about the view that the render targets will be used for. Determines size
		 *								of the render targets, and the output color render target.
		 * @param[in]	hdr				Should the render targets support high dynamic range rendering.
		 */
		static SPtr<RenderTargets> create(const RENDERER_VIEW_TARGET_DESC& view, bool hdr);

		/**
		 * Allocates the textures required for rendering. Allocations are pooled so this is generally a fast operation
		 * unless the size or other render target options changed. This must be called before binding render targets.
		 */
		void allocate();

		/**
		 * Deallocates textures by returning them to the pool. This should be done when the caller is done using the render
		 * targets, so that other systems might re-use them. This will not release any memory unless all render targets
		 * pointing to those textures go out of scope.
		 */
		void release();

		/**	Binds the GBuffer render target for rendering. */
		void bindGBuffer();

		/**	Binds the scene color render target for rendering. */
		void bindSceneColor(bool readOnlyDepthStencil);

		/** Returns the scene color render target. */
		SPtr<RenderTexture> getSceneColorRT() const { return mSceneColorRT; }

		/** Returns the gbuffer texture that scene color is stored in. */
		SPtr<Texture> getSceneColor() const;

		/**	Returns the first color texture of the gbuffer as a bindable texture. */
		SPtr<Texture> getTextureA() const;

		/**	Returns the second color texture of the gbuffer as a bindable texture. */
		SPtr<Texture> getTextureB() const;

		/**	Returns the third color texture of the gbuffer as a bindable texture. */
		SPtr<Texture> getTextureC() const;

		/**	Returns the depth texture of the gbuffer as a bindable texture. */
		SPtr<Texture> getTextureDepth() const;

		/** 
		 * Returns a scene color texture with a single-sample per pixel. If no multisampling is used, this is the same as
		 * getSceneColor().
		 */
		SPtr<Texture> getSceneColorNonMSAA() const;

		/** Returns a render target that can be used for rendering to the texture returned by getSceneColorNonMSAA(). */
		SPtr<RenderTexture> getSceneColorNonMSAART() const;

		/** 
		 * Returns a buffer that is meant to be used for rendering when MSAA is used, since writes to multisampled textures
		 * aren't supported on all render backends.
		 */
		SPtr<GpuBuffer> getFlattenedSceneColorBuffer() const;

		/**	Checks if the targets support HDR rendering. */
		bool getHDR() const { return mHDR; }

		/**	Returns the number of samples per pixel supported by the targets. */
		UINT32 getNumSamples() const { return mViewTarget.numSamples; }

		/** Gets the width of the targets, in pixels. */
		UINT32 getWidth() const { return mWidth; }

		/** Gets the height of the targets, in pixels. */
		UINT32 getHeight() const { return mHeight; }

	private:
		RenderTargets(const RENDERER_VIEW_TARGET_DESC& view, bool hdr);

		RENDERER_VIEW_TARGET_DESC mViewTarget;

		SPtr<PooledRenderTexture> mSceneColorTex;
		SPtr<PooledRenderTexture> mAlbedoTex;
		SPtr<PooledRenderTexture> mNormalTex;
		SPtr<PooledRenderTexture> mRoughMetalTex;
		SPtr<PooledRenderTexture> mDepthTex;

		SPtr<PooledRenderTexture> mSceneColorNonMSAATex;
		SPtr<PooledStorageBuffer> mFlattenedSceneColorBuffer;

		SPtr<RenderTexture> mGBufferRT;
		SPtr<RenderTexture> mSceneColorRT;

		PixelFormat mSceneColorFormat;
		PixelFormat mAlbedoFormat;
		PixelFormat mNormalFormat;
		bool mHDR;

		UINT32 mWidth;
		UINT32 mHeight;
	};

	/** @} */
}}