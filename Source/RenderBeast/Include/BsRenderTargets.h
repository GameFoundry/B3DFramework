//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsPixelUtil.h"
#include "BsRendererView.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Types of render target textures that can be allocated by RenderTargets manager. */
	enum RenderTargetType
	{
		/** 
		 * Buffer containing albedo, normals, metalness/roughness and other material data, populated during base pass and
		 * used during lighting and other operations.
		 */
		RTT_GBuffer,
		/** Buffer containing intermediate lighting information used during deferred lighting pass. */
		RTT_LightAccumulation,
		/** Buffer containing temporary combined occlusion data for a specific light (from shadow maps or attenuation. */
		RTT_LightOcclusion,
		/** Buffer containing (potentially multisampled) scene color information. */
		RTT_SceneColor,
		/** 
		 * Buffer containing non-MSAA final scene color information. If MSAA is not used then this is equivalent to
		 * RTT_SceneColor;
		 */
		RTT_ResolvedSceneColor,
		/**
		 * Secondary resolved scene color texture that can be used for ping-ponging between primary and secondary scene
		 * color textures. Primarily useful for post-processing effects.
		 */
		RTT_ResolvedSceneColorSecondary
	};

	/**
	 * Allocates and handles all the required render targets for rendering a scene from a specific view.
	 *
	 * @note	Core thread only.
	 */
	class RenderTargets
	{
	public:
		/** 
		 * Prepares any internal data for rendering. Should be called at the beginning of each frame, before allocating,
		 * retrieving or binding any textures. Must eventually be followed by cleanup().
		 */
		void prepare();

		/**
		 * Cleans up any internal data after rendering. Should be called after done rendering for a frame. All allocations
		 * must be released at this point and no further allocations or texture binds should be done until the next call
		 * to prepare().
		 */
		void cleanup();

		/**	Returns the depth buffer as a bindable texture. */
		SPtr<Texture> getSceneDepth() const;

		/**
		 * Allocates the textures required for rendering. Allocations are pooled so this is generally a fast operation
		 * unless the size or other render target options changed. This must be called before binding render targets.
		 */
		void allocate(RenderTargetType type);

		/**
		 * Deallocates textures by returning them to the pool. This should be done when the caller is done using the render
		 * targets, so that other systems might re-use them. This will not release any memory unless all render targets
		 * pointing to those textures go out of scope.
		 */
		void release(RenderTargetType type);

		/**	Binds the GBuffer render target for rendering. */
		void bindGBuffer();

		/**	Returns the first color texture of the gbuffer as a bindable texture. */
		SPtr<Texture> getGBufferA() const;

		/**	Returns the second color texture of the gbuffer as a bindable texture. */
		SPtr<Texture> getGBufferB() const;

		/**	Returns the third color texture of the gbuffer as a bindable texture. */
		SPtr<Texture> getGBufferC() const;

		/**	
		 * Binds the scene color render target for rendering. If using MSAA this texture will be allocated as a texture 
		 * with multiple samples.
		 */
		void bindSceneColor(bool readOnlyDepthStencil);

		/**
		 * Binds the non-MSAA version of the scene color texture for rendering. If not using MSAA this is equivalent to
		 * calling bindSceneColor() (as long as @p secondary is false).
		 * 
		 * @param[in]	secondary	If true, a seconday scene color texture will be bound. This texture can be used
		 *							for ping-pong operations between it and the primary scene color.
		 */
		void bindResolvedSceneColor(bool secondary = false);

		/** Binds the light accumulation render target for rendering. */
		void bindLightAccumulation();

		/** 
		 * Binds the light occlusion render target for rendering. Light occlusion texture and GBuffer must be allocated,
		 * and depth must have been previously rendered to the depth buffer. Target is cleared before the method returns.
		 */
		void bindLightOcclusion();

		/** Returns the texture containing (potentially multisampled) scene color. */
		SPtr<Texture> getSceneColor() const;

		/** 
		 * Flattened, buffer version of the texture returned by getSceneColor(). Only available when MSAA is used, since
		 * random writes to multisampled textures aren't supported on all render backends.
		 */
		SPtr<GpuBuffer> getSceneColorBuffer() const;

		/**
		 * Returns a non-MSAA version of the scene color texture. If MSAA is not used this is equivalent to calling
		 * getSceneColor() (as long as @p secondary is set to false).
		 * 
		 * @param[in]	secondary	If true, a seconday scene color texture will be returned. This texture can be used
		 *							for ping-pong operations between it and the primary scene color.
		 */
		SPtr<Texture> getResolvedSceneColor(bool secondary = false) const;

		/**
		 * Returns a non-MSAA version of the scene color render target. If MSAA is not used this will return the default
		 * scene color render target (as long as @p secondary is set to false).
		 * 
		 * @param[in]	secondary	If true, a seconday scene color target will be returned. This target can be used
		 *							for ping-pong operations between it and the primary scene color.
		 */
		SPtr<RenderTarget> getResolvedSceneColorRT(bool secondary = false) const;

		/** Returns the texture for storing of the intermediate lighting information. */
		SPtr<Texture> getLightAccumulation() const;

		/** Returns the texture for storing shadow/attenuation data for single light, from viewers perspective. */
		SPtr<Texture> getLightOcclusion() const;

		/**
		 * Flattened, buffer version of the texture returned by getLightAccumulation(). Required when MSAA is used, since
		 * random writes to multisampled textures aren't supported on all render backends.
		 */
		SPtr<GpuBuffer> getLightAccumulationBuffer() const;

		/**	Checks if the targets support HDR rendering. */
		bool getHDR() const { return mHDR; }

		/**	Returns the number of samples per pixel supported by the targets. */
		UINT32 getNumSamples() const { return mViewTarget.numSamples; }

		/** Gets the width of the targets, in pixels. */
		UINT32 getWidth() const { return mWidth; }

		/** Gets the height of the targets, in pixels. */
		UINT32 getHeight() const { return mHeight; }

		/**
		 * Creates a new set of render targets. Note in order to actually use the render targets you need to call the
		 * relevant allocate* method before use.
		 *
		 * @param[in]	view			Information about the view that the render targets will be used for. Determines size
		 *								of the render targets, and the output color render target.
		 * @param[in]	hdr				Should the render targets support high dynamic range rendering.
		 */
		static SPtr<RenderTargets> create(const RENDERER_VIEW_TARGET_DESC& view, bool hdr);

	private:
		RenderTargets(const RENDERER_VIEW_TARGET_DESC& view, bool hdr);

		RENDERER_VIEW_TARGET_DESC mViewTarget;

		SPtr<PooledRenderTexture> mAlbedoTex;
		SPtr<PooledRenderTexture> mNormalTex;
		SPtr<PooledRenderTexture> mRoughMetalTex;
		SPtr<PooledRenderTexture> mDepthTex;

		SPtr<PooledRenderTexture> mLightAccumulationTex;
		SPtr<PooledRenderTexture> mLightOcclusionTex;
		SPtr<PooledStorageBuffer> mFlattenedLightAccumulationBuffer;

		SPtr<PooledRenderTexture> mSceneColorTex;
		SPtr<PooledStorageBuffer> mFlattenedSceneColorBuffer;
		SPtr<PooledRenderTexture> mResolvedSceneColorTex1;
		SPtr<PooledRenderTexture> mResolvedSceneColorTex2;

		SPtr<RenderTexture> mGBufferRT;
		SPtr<RenderTexture> mSceneColorRT;
		SPtr<RenderTexture> mResolvedSceneColorRT1;
		SPtr<RenderTexture> mResolvedSceneColorRT2;
		SPtr<RenderTexture> mLightAccumulationRT;
		SPtr<RenderTexture> mLightOcclusionRT;

		PixelFormat mSceneColorFormat;
		PixelFormat mAlbedoFormat;
		PixelFormat mNormalFormat;
		bool mHDR;

		UINT32 mWidth;
		UINT32 mHeight;
	};

	/** @} */
}}