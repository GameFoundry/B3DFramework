//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsModule.h"
#include "BsMatrix4.h"
#include "BsConvexVolume.h"
#include "BsParamBlocks.h"
#include "BsRendererMaterial.h"
#include "BsTextureAtlasLayout.h"

namespace bs { namespace ct
{
	struct FrameInfo;
	class RendererLight;
	class RendererScene;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Number of frustum splits when rendering cascaded shadow maps. */
	const UINT32 NUM_CASCADE_SPLITS = 4;

	BS_PARAM_BLOCK_BEGIN(ShadowParamsDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
		BS_PARAM_BLOCK_ENTRY(float, gDepthBias)
		BS_PARAM_BLOCK_ENTRY(float, gDepthRange)
	BS_PARAM_BLOCK_END

	extern ShadowParamsDef gShadowParamsDef;
	
	/** Material used for rendering a single face of a shadow map. */
	class ShadowDepthNormalMat : public RendererMaterial<ShadowDepthNormalMat>
	{
		RMAT_DEF("ShadowDepthNormal.bsl");

	public:
		ShadowDepthNormalMat();

		/** Binds the material to the pipeline, ready to be used on subsequent draw calls. */
		void bind(const SPtr<GpuParamBlockBuffer>& shadowParams);

		/** Sets a new buffer that determines per-object properties. */
		void setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams);
	};

	/** Material used for rendering a single face of a shadow map, for a directional light. */
	class ShadowDepthDirectionalMat : public RendererMaterial<ShadowDepthDirectionalMat>
	{
		RMAT_DEF("ShadowDepthDirectional.bsl");

	public:
		ShadowDepthDirectionalMat();

		/** Binds the material to the pipeline, ready to be used on subsequent draw calls. */
		void bind(const SPtr<GpuParamBlockBuffer>& shadowParams);

		/** Sets a new buffer that determines per-object properties. */
		void setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams);
	};

	BS_PARAM_BLOCK_BEGIN(ShadowCubeMatricesDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Matrix4, gFaceVPMatrices, 6)
	BS_PARAM_BLOCK_END

	extern ShadowCubeMatricesDef gShadowCubeMatricesDef;

	BS_PARAM_BLOCK_BEGIN(ShadowCubeMasksDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(int, gFaceMasks, 6)
	BS_PARAM_BLOCK_END

	extern ShadowCubeMasksDef gShadowCubeMasksDef;

	/** Material used for rendering an omni directional cube shadow map. */
	class ShadowDepthCubeMat : public RendererMaterial<ShadowDepthCubeMat>
	{
		RMAT_DEF("ShadowDepthCube.bsl");

	public:
		ShadowDepthCubeMat();

		/** Binds the material to the pipeline, ready to be used on subsequent draw calls. */
		void bind(const SPtr<GpuParamBlockBuffer>& shadowParams, const SPtr<GpuParamBlockBuffer>& shadowCubeParams);

		/** Sets a new buffer that determines per-object properties. */
		void setPerObjectBuffer(const SPtr<GpuParamBlockBuffer>& perObjectParams, 
			const SPtr<GpuParamBlockBuffer>& shadowCubeMasks);
	};

	/** Information about a single shadow map in a shadow map atlas. */
	struct ShadowMapInfo
	{
		/** Updates normalized area coordinates based on the non-normalized ones and the provided atlas size. */
		void updateNormArea(UINT32 atlasSize);

		Rect2I area; // Area in pixels
		Rect2 normArea; // Normalized area in [0, 1] range
		UINT32 atlasIdx;
	};

	/** 
	 * Contains a texture that serves as an atlas for one or multiple shadow maps. Provides methods for inserting new maps
	 * in the atlas. 
	 */
	class ShadowMapAtlas
	{
	public:
		ShadowMapAtlas(UINT32 size);
		~ShadowMapAtlas();

		/** 
		 * Registers a new map in the shadow map atlas. Returns true if the map fits in the atlas, or false otherwise.
		 * Resets the last used counter to zero.
		 */
		bool addMap(UINT32 size, Rect2I& area, UINT32 border = 4);

		/** Clears all shadow maps from the atlas. Increments the last used counter.*/
		void clear();

		/** Checks have any maps been added to the atlas. */
		bool isEmpty() const;

		/** 
		 * Returns the value of the last used counter. See addMap() and clear() for information on how the counter is
		 * incremented/decremented.
		 */
		UINT32 getLastUsedCounter() const { return mLastUsedCounter; }

		/** Returns the bindable atlas texture. */
		SPtr<Texture> getTexture() const;

		/** Returns the render target that allows you to render into the atlas. */
		SPtr<RenderTexture> getTarget() const;

	private:
		SPtr<PooledRenderTexture> mAtlas;

		TextureAtlasLayout mLayout;
		UINT32 mLastUsedCounter;
	};

	/** Contains common code for different shadow map types. */
	class ShadowMapBase
	{
	public:
		ShadowMapBase(UINT32 size);
		virtual ~ShadowMapBase() {}

		/** Returns the bindable shadow map texture. */
		SPtr<Texture> getTexture() const;

		/** Returns the size of a single face of the shadow map texture, in pixels. */
		UINT32 getSize() const { return mSize; }

		/** Makes the shadow map available for re-use and increments the counter returned by getLastUsedCounter(). */
		void clear() { mIsUsed = false; mLastUsedCounter++; }

		/** Marks the shadow map as used and resets the last used counter to zero. */
		void markAsUsed() { mIsUsed = true; mLastUsedCounter = 0; }

		/** Returns true if the object is storing a valid shadow map. */
		bool isUsed() const { return mIsUsed; }

		/** 
		 * Returns the value of the last used counter. See incrementUseCounter() and markAsUsed() for information on how is
		 * the counter incremented/decremented.
		 */
		UINT32 getLastUsedCounter() const { return mLastUsedCounter; }

	protected:
		SPtr<PooledRenderTexture> mShadowMap;
		UINT32 mSize;

		bool mIsUsed;
		UINT32 mLastUsedCounter;
	};

	/** Contains a cubemap for storing an omnidirectional cubemap. */
	class ShadowCubemap : public ShadowMapBase
	{
	public:
		ShadowCubemap(UINT32 size);
		~ShadowCubemap();

		/** Returns a render target encompassing all six faces of the shadow cubemap. */
		SPtr<RenderTexture> getTarget() const;
	};

	/** Contains a texture required for rendering cascaded shadow maps. */
	class ShadowCascadedMap : public ShadowMapBase
	{
	public:
		ShadowCascadedMap(UINT32 size);
		~ShadowCascadedMap();

		/** Returns a render target that allows rendering into a specific cascade of the cascaded shadow map. */
		SPtr<RenderTexture> getTarget(UINT32 cascadeIdx) const;
	private:

		SPtr<RenderTexture> mTargets[NUM_CASCADE_SPLITS];
	};

	/** Provides functionality for rendering shadow maps. */
	class ShadowRendering : public Module<ShadowRendering>
	{
	public:
		ShadowRendering(UINT32 shadowMapSize);

		/** For each visibile shadow casting light, renders a shadow map from its point of view. */
		void renderShadowMaps(RendererScene& scene, const FrameInfo& frameInfo);

		/** Changes the default shadow map size. Will cause all shadow maps to be rebuilt. */
		void setShadowMapSize(UINT32 size);
	private:
		/** Renders cascaded shadow maps for the provided directional light viewed from the provided view. */
		void renderCascadedShadowMaps(const RendererView& view, const RendererLight& light, RendererScene& scene,
			const FrameInfo& frameInfo);

		/** Renders shadow maps for the provided spot light. */
		void renderSpotShadowMaps(const RendererLight& light, RendererScene& scene, const FrameInfo& frameInfo);

		/** Renders shadow maps for the provided radial light. */
		void renderRadialShadowMaps(const RendererLight& light, RendererScene& scene, const FrameInfo& frameInfo);

		/**
		 * Generates a frustum for a single cascade of a cascaded shadow map. Also outputs spherical bounds of the
		 * split view frustum.
		 * 
		 * @param[in]	view		View whose frustum to split.
		 * @param[in]	lightDir	Direction of the light for which we're generating the shadow map.
		 * @param[in]	cascade		Index of the cascade to generate the frustum for.
		 * @param[in]	numCascades	Maximum number of cascades in the cascaded shadow map. Must be greater than zero.
		 * @param[out]	outBounds	Spherical bounds of the split view frustum.
		 * @return					Convex volume covering the area of the split view frustum visible from the light.
		 */
		static ConvexVolume getCSMSplitFrustum(const RendererView& view, const Vector3& lightDir, UINT32 cascade, 
			UINT32 numCascades, Sphere& outBounds);

		/**
		 * Finds the distance (along the view direction) of the frustum split for the specified index. Used for cascaded
		 * shadow maps.
		 * 
		 * @param[in]	view			View whose frustum to split.
		 * @param[in]	index			Index of the split. 0 = near plane.
		 * @param[in]	numCascades		Maximum number of cascades in the cascaded shadow map. Must be greater than zero
		 *								and greater or equal to @p index.
		 * @return						Distance to the split position along the view direction.
		 */
		static float getCSMSplitDistance(const RendererView& view, UINT32 index, UINT32 numCascades);

		/** Size of a single shadow map atlas, in pixels. */
		static const UINT32 MAX_ATLAS_SIZE;

		/** Determines how long will an unused shadow map atlas stay allocated, in frames. */
		static const UINT32 MAX_UNUSED_FRAMES;

		ShadowDepthNormalMat mDepthNormalMat;
		ShadowDepthCubeMat mDepthCubeMat;
		ShadowDepthDirectionalMat mDepthDirectionalMat;

		UINT32 mShadowMapSize;

		Vector<ShadowMapAtlas> mDynamicShadowMaps;
		Vector<ShadowCascadedMap> mCascadedShadowMaps;
		Vector<ShadowCubemap> mShadowCubemaps;

		Vector<bool> mRenderableVisibility; // Transient
	};

	/* @} */
}}
