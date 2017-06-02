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
#include "BsLight.h"
#include "BsLightRendering.h"

namespace bs { namespace ct
{
	struct FrameInfo;
	class RendererLight;
	class RendererScene;
	struct ShadowInfo;

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

	BS_PARAM_BLOCK_BEGIN(ShadowProjectVertParamsDef)
		BS_PARAM_BLOCK_ENTRY(Vector4, gPositionAndScale)
	BS_PARAM_BLOCK_END

	extern ShadowProjectVertParamsDef gShadowProjectVertParamsDef;

	/** Material used for populating the stencil buffer when projecting non-omnidirectional shadows. */
	template<bool Directional, bool ZFailStencil>
	class ShadowProjectStencilMat : public RendererMaterial<ShadowProjectStencilMat<Directional, ZFailStencil>>
	{
		RMAT_DEF("ShadowProjectStencil.bsl");

	public:
		ShadowProjectStencilMat();

		/** Binds the material and its parameters to the pipeline. */
		void bind(const SPtr<GpuParamBlockBuffer>& perCamera);

	private:
		SPtr<GpuParamBlockBuffer> mVertParams;
	};

	/** Contains all variations of the ShadowProjectStencilMat material. */
	class ShadowProjectStencilMaterials
	{
	public:
		/** 
		 * Binds a shader that can be used for populating the stencil buffer during non-omnidirectional shadow rendering. 
		 *
		 * @param[in]	directional		Set to true if shadows from a directional light are being rendered.
		 * @param[in]	useZFailStencil	If true the material will use z-fail operation to modify the stencil buffer. If 
		 *								false z-pass will be used instead. Z-pass is a more performant alternative as it
		 *								doesn't disable hi-z optimization, but it cannot handle the case when the viewer is
		 *								inside the drawn geometry.
		 * @param[in]	perCamera		Buffer containing information about the current view.
		 */
		void bind(bool directional, bool useZFailStencil, const SPtr<GpuParamBlockBuffer>& perCamera);
	private:
		ShadowProjectStencilMat<true, true> mTT;
		ShadowProjectStencilMat<false, true> mFT;
		ShadowProjectStencilMat<false, false> mFF;
	};

	BS_PARAM_BLOCK_BEGIN(ShadowProjectParamsDef)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMixedToShadowSpace)
		BS_PARAM_BLOCK_ENTRY(Vector2, gShadowMapSize)
		BS_PARAM_BLOCK_ENTRY(Vector2, gShadowMapSizeInv)
		BS_PARAM_BLOCK_ENTRY(float, gSoftTransitionScale)
		BS_PARAM_BLOCK_ENTRY(float, gFadePercent)
		BS_PARAM_BLOCK_ENTRY(float, gFadePlaneDepth)
		BS_PARAM_BLOCK_ENTRY(float, gInvFadePlaneRange)
	BS_PARAM_BLOCK_END

	extern ShadowProjectParamsDef gShadowProjectParamsDef;

	/** Material used for projecting depth into a shadow accumulation buffer for non-omnidirectional shadow maps. */
	template<int ShadowQuality, bool Directional, bool MSAA>
	class ShadowProjectMat : public RendererMaterial<ShadowProjectMat<ShadowQuality, Directional, MSAA>>
	{
		RMAT_DEF("ShadowProject.bsl");

	public:
		ShadowProjectMat();

		/** Binds the material and its parameters to the pipeline. */
		void bind(const Light& light, const SPtr<Texture>& shadowMap, const SPtr<GpuParamBlockBuffer> shadowParams, 
			const SPtr<GpuParamBlockBuffer>& perCamera);

	private:
		SPtr<SamplerState> mSamplerState;
		SPtr<GpuParamBlockBuffer> mVertParams;

		GBufferParams mGBufferParams;

		GpuParamTexture mShadowMapParam;
		GpuParamSampState mShadowSamplerParam;
	};

	BS_PARAM_BLOCK_BEGIN(ShadowProjectOmniParamsDef)
		BS_PARAM_BLOCK_ENTRY_ARRAY(Matrix4, gFaceVPMatrices, 6)
		BS_PARAM_BLOCK_ENTRY(Vector4, gLightPosAndRadius)
		BS_PARAM_BLOCK_ENTRY(float, gInvResolution)
		BS_PARAM_BLOCK_ENTRY(float, gFadePercent)
		BS_PARAM_BLOCK_ENTRY(float, gDepthBias)
	BS_PARAM_BLOCK_END

	extern ShadowProjectOmniParamsDef gShadowProjectOmniParamsDef;

	/** Material used for projecting depth into a shadow accumulation buffer for omnidirectional shadow maps. */
	template<int ShadowQuality, bool MSAA>
	class ShadowProjectOmniMat : public RendererMaterial<ShadowProjectOmniMat<ShadowQuality, MSAA>>
	{
		RMAT_DEF("ShadowProjectOmni.bsl");

	public:
		ShadowProjectOmniMat();

		/** Binds the material and its parameters to the pipeline. */
		void bind(const Light& light, const SPtr<Texture>& shadowMap, const SPtr<GpuParamBlockBuffer> shadowParams, 
			const SPtr<GpuParamBlockBuffer>& perCamera);

	private:
		SPtr<SamplerState> mSamplerState;
		SPtr<GpuParamBlockBuffer> mVertParams;

		GBufferParams mGBufferParams;

		GpuParamTexture mShadowMapParam;
		GpuParamSampState mShadowSamplerParam;
	};

	/** Information about a shadow cast from a single light. */
	struct ShadowInfo
	{
		/** Updates normalized area coordinates based on the non-normalized ones and the provided atlas size. */
		void updateNormArea(UINT32 atlasSize);

		UINT32 lightIdx; /**< Index of the light casting this shadow. */
		Rect2I area; /**< Area of the shadow map in pixels, relative to its source texture. */
		Rect2 normArea; /**< Normalized shadow map area in [0, 1] range. */
		UINT32 textureIdx; /**< Index of the texture the shadow map is stored in. */

		float depthNear; /**< Distance to the near plane. */
		float depthFar; /**< Distance to the far plane. */
		float depthFade; /**< Distance to the plane at which to start fading out the shadows (only for CSM). */
		float fadeRange; /**< Distance from the fade plane to the far plane (only for CSM). */

		float depthBias; /**< Bias used to reduce shadow acne. */
		float depthRange; /**< Length of the range covered by the shadow caster volume. */

		UINT32 cascadeIdx; /**< Index of a cascade. Only relevant for CSM. */

		/** View-projection matrix from the shadow casters point of view. */
		Matrix4 shadowVPTransform; 

		/** View-projection matrix for each cubemap face, used for omni-directional shadows. */
		Matrix4 shadowVPTransforms[6]; 

		/** Determines the fade amount of the shadow, for each view in the scene. */
		SmallVector<float, 4> fadePerView;
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

		/** Provides information about a shadow for the specified cascade. */
		void setShadowInfo(UINT32 cascadeIdx, const ShadowInfo& info) { mShadowInfos[cascadeIdx] = info; }

		/** @copydoc setShadowInfo */
		const ShadowInfo& getShadowInfo(UINT32 cascadeIdx) const { return mShadowInfos[cascadeIdx]; }
	private:
		SPtr<RenderTexture> mTargets[NUM_CASCADE_SPLITS];
		ShadowInfo mShadowInfos[NUM_CASCADE_SPLITS];
	};

	/** Provides functionality for rendering shadow maps. */
	class ShadowRendering : public Module<ShadowRendering>
	{
		/** Contains information required for generating a shadow map for a specific light. */
		struct ShadowMapOptions
		{
			UINT32 lightIdx;
			UINT32 mapSize;
			SmallVector<float, 4> fadePercents;
		};

		/** Contains references to all shadows cast by a specific light. */
		struct LightShadows
		{
			UINT32 startIdx;
			UINT32 numShadows;
		};
	public:
		ShadowRendering(UINT32 shadowMapSize);

		/** For each visibile shadow casting light, renders a shadow map from its point of view. */
		void renderShadowMaps(RendererScene& scene, const FrameInfo& frameInfo);

		/** 
		 * Renders shadow occlusion values for the specified light, into the currently bound render target. 
		 * The system uses shadow maps rendered by renderShadowMaps().
		 */
		void renderShadowOcclusion(const RendererScene& scene, const RendererLight& light, UINT32 viewIdx);

		/** Changes the default shadow map size. Will cause all shadow maps to be rebuilt. */
		void setShadowMapSize(UINT32 size);
	private:
		/** Renders cascaded shadow maps for the provided directional light viewed from the provided view. */
		void renderCascadedShadowMaps(UINT32 viewIdx, UINT32 lightIdx, RendererScene& scene, const FrameInfo& frameInfo);

		/** Renders shadow maps for the provided spot light. */
		void renderSpotShadowMap(const RendererLight& light, const ShadowMapOptions& options, RendererScene& scene,
			const FrameInfo& frameInfo);

		/** Renders shadow maps for the provided radial light. */
		void renderRadialShadowMap(const RendererLight& light, const ShadowMapOptions& options, RendererScene& scene, 
			const FrameInfo& frameInfo);

		/** 
		 * Calculates optimal shadow map size, taking into account all views in the scene. Also calculates a fade value
		 * that can be used for fading out small shadow maps.
		 * 
		 * @param[in]	light			Light for which to calculate the shadow map properties. Cannot be a directional light.
		 * @param[in]	scene			Scene information containing all the views the light can be seen through.
		 * @param[out]	size			Optimal size of the shadow map, in pixels.
		 * @param[out]	fadePercents	Value in range [0, 1] determining how much should the shadow map be faded out. Each
		 *								entry corresponds to a single view.
		 * @param[out]	maxFadePercent	Maximum value in the @p fadePercents array.
		 */
		void calcShadowMapProperties(const RendererLight& light, RendererScene& scene, UINT32& size, 
			SmallVector<float, 4>& fadePercents, float& maxFadePercent) const;

		/**
		 * Draws a mesh representing near and far planes at the provided coordinates. The mesh is constructed using
		 * normalized device coordinates and requires no perspective transform. Near plane will be drawn using front facing
		 * triangles, and the far plane will be drawn using back facing triangles.
		 * 
		 * @param[in]	near		Location of the near plane, in NDC.
		 * @param[in]	far			Location of the far plane, in NDC.
		 * @param[in]	drawNear	If disabled, only the far plane will be drawn.
		 */
		void drawNearFarPlanes(float near, float far, bool drawNear = true);

		/** 
		 * Draws a frustum mesh using the provided vertices as its corners. Corners should be in the order specified
		 * by AABox::Corner enum.
		 */
		void drawFrustum(const std::array<Vector3, 8>& corners);

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

		/**
		 * Calculates a bias that can be applied when rendering shadow maps, in order to reduce shadow artifacts.
		 * 
		 * @param[in]	light		Light to calculate the depth bias for.
		 * @param[in]	depthRange	Range of depths (distance between near and far planes) covered by the shadow.
		 * @param[in]	mapSize		Size of the shadow map, in pixels.
		 * @return					Depth bias that can be passed to shadow depth rendering shader. 
		 */
		static float getDepthBias(const Light& light, float depthRange, UINT32 mapSize);

		/**
		 * Calculates a fade transition value that can be used for slowly fading-in the shadow, in order to avoid or reduce
		 * shadow acne.
		 *
		 * @param[in]	light		Light to calculate the fade transition size for.
		 * @param[in]	depthRange	Range of depths (distance between near and far planes) covered by the shadow.
		 * @param[in]	mapSize		Size of the shadow map, in pixels.
		 * @return					Value that determines the size of the fade transition region.
		 */
		static float getFadeTransition(const Light& light, float depthRange, UINT32 mapSize);

		/** Size of a single shadow map atlas, in pixels. */
		static const UINT32 MAX_ATLAS_SIZE;

		/** Determines how long will an unused shadow map atlas stay allocated, in frames. */
		static const UINT32 MAX_UNUSED_FRAMES;

		/** Determines the minimal resolution of a shadow map. */
		static const UINT32 MIN_SHADOW_MAP_SIZE;

		/** Determines the resolution at which shadow maps begin fading out. */
		static const UINT32 SHADOW_MAP_FADE_SIZE;

		/** Size of the border of a shadow map in a shadow map atlas, in pixels. */
		static const UINT32 SHADOW_MAP_BORDER;

		/** Percent of the length of a single cascade in a CSM, in which to fade out the cascade. */
		static const float CASCADE_FRACTION_FADE;

		ShadowDepthNormalMat mDepthNormalMat;
		ShadowDepthCubeMat mDepthCubeMat;
		ShadowDepthDirectionalMat mDepthDirectionalMat;

		ShadowProjectStencilMaterials mProjectStencilMaterials;

		UINT32 mShadowMapSize;

		Vector<ShadowMapAtlas> mDynamicShadowMaps;
		Vector<ShadowCascadedMap> mCascadedShadowMaps;
		Vector<ShadowCubemap> mShadowCubemaps;

		Vector<ShadowInfo> mShadowInfos;

		Vector<LightShadows> mSpotLightShadows;
		Vector<LightShadows> mRadialLightShadows;
		Vector<UINT32> mDirectionalLightShadows;

		SPtr<VertexDeclaration> mPositionOnlyVD;

		// Mesh information used for drawing near & far planes
		SPtr<IndexBuffer> mPlaneIB;
		SPtr<VertexBuffer> mPlaneVB;

		// Mesh information used for drawing a shadow frustum
		SPtr<IndexBuffer> mFrustumIB;
		SPtr<VertexBuffer> mFrustumVB;

		Vector<bool> mRenderableVisibility; // Transient
		Vector<ShadowMapOptions> mSpotLightShadowOptions; // Transient
		Vector<ShadowMapOptions> mRadialLightShadowOptions; // Transient
	};

	/* @} */
}}
