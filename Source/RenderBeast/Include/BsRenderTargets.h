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
		RTT_ResolvedSceneColorSecondary,
		/**
		 * Buffer containing a hierarchical Z buffer. If passed to RenderTargets::generate() it will build
		 * the hierarchical Z buffer from the current contents of the scene depth buffer. Generated buffer can be
		 * accessed from getHiZ(). If using MSAA render targets, make sure to first generate the RTT_ResolvedDepth
		 * target as HiZ can't be built from multisampled depth buffer.
		 */
		RTT_HiZ,
		/**
		 * Buffer containing the resolved (non-multisampled) depth buffer. If multisampling isn't used then this will be
		 * the same buffer as the scene depth buffer. If passed to RenderTargets::generate() it will resolve
		 * the scene depth buffer into the newly allocated buffer. Scene depth must be previously allocated and populated
		 * with scene data.
		 */
		RTT_ResolvedDepth,
		/** Buffer containing ambient occlusion. */
		RTT_AmbientOcclusion
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

		/**
		 * Allocates the textures required for rendering. Allocations are pooled so this is generally a fast operation
		 * unless the size or other render target options changed. This must be called before binding render targets.
		 * Some render target types are also automatically populated with data, in which case calling generate() instead
		 * of allocate is a better option - see individual target descriptions for more details.
		 */
		void allocate(RenderTargetType type);

		/**
		 * Deallocates textures by returning them to the pool. This should be done when the caller is done using the render
		 * targets, so that other systems might re-use them. This will not release any memory unless all render targets
		 * pointing to those textures go out of scope.
		 */
		void release(RenderTargetType type);

		/**
		 * Binds the specified render target for rendering, and performs any clear operations as necessary.
		 */
		void bind(RenderTargetType type, bool readOnlyDepthStencil = false);

		/**
		 * Generates contents for the specified render target type. Target must first be allocated by calling allocate().
		 * Note that not all target types can have their contents automatically generated, most are meant to be populated
		 * by external code. And those that can usually have prerequisites. See individual target descriptions for more
		 * details.
		 */
		void generate(RenderTargetType type);

		/** Returns a bindable texture for a render target previously allocated using allocate().*/
		SPtr<Texture> get(RenderTargetType type, RenderSurfaceMaskBits surface = RT_COLOR0) const;

		/** Returns a render target previously allocated using allocate(). */
		SPtr<RenderTexture> getRT(RenderTargetType type) const;

		/** 
		 * Flattened, buffer version of the texture returned by get(RTT_SceneColor). Only available when MSAA is used, since
		 * random writes to multisampled textures aren't supported on all render backends.
		 */
		SPtr<GpuBuffer> getSceneColorBuffer() const;

		/**
		 * Flattened, buffer version of the texture returned by get(RTT_LightAccumulation). Required when MSAA is used, 
		 * since random writes to multisampled textures aren't supported on all render backends.
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

		/** Returns a pooled texture previously allocated with a call to allocate(). */
		SPtr<PooledRenderTexture> getPooledTexture(RenderTargetType type, RenderSurfaceMaskBits surface = RT_COLOR0) const;

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

		SPtr<PooledRenderTexture> mHiZ;
		SPtr<PooledRenderTexture> mResolvedDepthTex;
		SPtr<PooledRenderTexture> mAmbientOcclusionTex;

		SPtr<RenderTexture> mGBufferRT;
		SPtr<RenderTexture> mSceneColorRT;
		SPtr<RenderTexture> mLightAccumulationRT;
		SPtr<RenderTexture> mLightOcclusionRT;

		PixelFormat mSceneColorFormat;
		PixelFormat mAlbedoFormat;
		PixelFormat mNormalFormat;
		bool mHDR;

		UINT32 mWidth;
		UINT32 mHeight;

		BuildHiZ mBuildHiZ;
	};

	/** @} */
}}