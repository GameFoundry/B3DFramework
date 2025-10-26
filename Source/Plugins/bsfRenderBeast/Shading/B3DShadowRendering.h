//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DConvexVolume.h"
#include "Renderer/B3DGpuDataParameterBlock.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Image/B3DTextureAtlasLayout.h"
#include "B3DRendererLight.h"

namespace b3d
{
	namespace render
	{
		struct FrameInfo;
		class RendererLight;
		class RenderBeastScene;
		struct ShadowInfo;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_PARAM_BLOCK_BEGIN(ShadowParamsDef)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gNDCZToDeviceZ)
			B3D_PARAM_BLOCK_ENTRY(float, gDepthBias)
			B3D_PARAM_BLOCK_ENTRY(float, gInvDepthRange)
		B3D_PARAM_BLOCK_END

		extern ShadowParamsDef gShadowParamsDef;

		/** Material used for rendering a single face of a shadow map, while applying bias in the pixel shader. */
		class ShadowDepthNormalMat : public RendererMaterial<ShadowDepthNormalMat>
		{
			RMAT_DEF("ShadowDepthNormal.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool skinned, bool morph>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SKINNED", skinned),
					  ShaderVariationParameter("MORPH", morph) });

				return variation;
			}

		public:
			ShadowDepthNormalMat() = default;

			/** Binds the material to the pipeline, ready to be used on subsequent draw calls. */
			void Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& shadowParams);

			/** Sets a new buffer that determines per-object properties. */
			void SetPerObjectBuffer(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perObjectParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	skinned		True if the shadow caster supports bone animation.
			 * @param[in]	morph		True if the shadow caster supports morph shape animation.
			 */
			static ShadowDepthNormalMat* GetVariation(bool skinned, bool morph);
		};

		/** Material used for rendering a single face of a shadow map, without running the pixel shader. */
		class ShadowDepthNormalNoPSMat : public RendererMaterial<ShadowDepthNormalNoPSMat>
		{
			RMAT_DEF("ShadowDepthNormalNoPS.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool skinned, bool morph>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SKINNED", skinned),
					  ShaderVariationParameter("MORPH", morph) });

				return variation;
			}

		public:
			ShadowDepthNormalNoPSMat() = default;

			/** Binds the material to the pipeline, ready to be used on subsequent draw calls. */
			void Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& shadowParams);

			/** Sets a new buffer that determines per-object properties. */
			void SetPerObjectBuffer(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perObjectParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	skinned		True if the shadow caster supports bone animation.
			 * @param[in]	morph		True if the shadow caster supports morph shape animation.
			 */
			static ShadowDepthNormalNoPSMat* GetVariation(bool skinned, bool morph);
		};

		/** Material used for rendering a single face of a shadow map, for a directional light. */
		class ShadowDepthDirectionalMat : public RendererMaterial<ShadowDepthDirectionalMat>
		{
			RMAT_DEF("ShadowDepthDirectional.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool skinned, bool morph>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SKINNED", skinned),
					  ShaderVariationParameter("MORPH", morph) });

				return variation;
			}

		public:
			ShadowDepthDirectionalMat() = default;

			/** Binds the material to the pipeline, ready to be used on subsequent draw calls. */
			void Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& shadowParams);

			/** Sets a new buffer that determines per-object properties. */
			void SetPerObjectBuffer(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perObjectParams);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	skinned		True if the shadow caster supports bone animation.
			 * @param[in]	morph		True if the shadow caster supports morph shape animation.
			 */
			static ShadowDepthDirectionalMat* GetVariation(bool skinned, bool morph);
		};

		B3D_PARAM_BLOCK_BEGIN(ShadowCubeMatricesDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(Matrix4, gFaceVPMatrices, 6)
		B3D_PARAM_BLOCK_END

		extern ShadowCubeMatricesDef gShadowCubeMatricesDef;

		B3D_PARAM_BLOCK_BEGIN(ShadowCubeMasksDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(int, gFaceMasks, 6)
		B3D_PARAM_BLOCK_END

		extern ShadowCubeMasksDef gShadowCubeMasksDef;

		/** Material used for rendering an omni directional cube shadow map. */
		class ShadowDepthCubeMat : public RendererMaterial<ShadowDepthCubeMat>
		{
			RMAT_DEF("ShadowDepthCube.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool skinned, bool morph>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SKINNED", skinned),
					  ShaderVariationParameter("MORPH", morph) });

				return variation;
			}

		public:
			ShadowDepthCubeMat() = default;

			/** Binds the material to the pipeline, ready to be used on subsequent draw calls. */
			void Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& shadowParams, const SPtr<GpuBuffer>& shadowCubeParams);

			/** Sets a new buffer that determines per-object properties. */
			void SetPerObjectBuffer(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perObjectParams, const SPtr<GpuBuffer>& shadowCubeMasks);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	skinned		True if the shadow caster supports bone animation.
			 * @param[in]	morph		True if the shadow caster supports morph shape animation.
			 */
			static ShadowDepthCubeMat* GetVariation(bool skinned, bool morph);
		};

		B3D_PARAM_BLOCK_BEGIN(ShadowProjectVertParamsDef)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gPositionAndScale)
		B3D_PARAM_BLOCK_END

		extern ShadowProjectVertParamsDef gShadowProjectVertParamsDef;

		/** Material used for populating the stencil buffer when projecting non-omnidirectional shadows. */
		class ShadowProjectStencilMat : public RendererMaterial<ShadowProjectStencilMat>
		{
			RMAT_DEF("ShadowProjectStencil.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool directional, bool useZFailStencil>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("NEEDS_TRANSFORM", !directional),
					  ShaderVariationParameter("USE_ZFAIL_STENCIL", useZFailStencil) });

				return variation;
			};

		public:
			ShadowProjectStencilMat() = default;
			void Initialize() override;

			/** Binds the material and its parameters to the pipeline. */
			void Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perCamera);

			/** Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	directional		Set to true if shadows from a directional light are being rendered.
			 * @param[in]	useZFailStencil	If true the material will use z-fail operation to modify the stencil buffer. If
			 *								false z-pass will be used instead. Z-pass is a more performant alternative as it
			 *								doesn't disable hi-z optimization, but it cannot handle the case when the viewer is
			 *								inside the drawn geometry.
			 */
			static ShadowProjectStencilMat* GetVariation(bool directional, bool useZFailStencil);

		private:
			SPtr<GpuBuffer> mVertParams;
		};

		/** Common parameters used by the shadow projection materials. */
		struct ShadowProjectParams
		{
			ShadowProjectParams(const Light& light, const SPtr<Texture>& shadowMap, const SPtr<GpuBuffer>& shadowParams, const SPtr<GpuBuffer>& perCameraParams, GBufferTextures gbuffer)
				: Light(light), ShadowMap(shadowMap), ShadowParams(shadowParams), PerCamera(perCameraParams), Gbuffer(gbuffer)
			{}

			/** Light which is casting the shadow. */
			const Light& Light;

			/** Texture containing the shadow map. */
			const SPtr<Texture>& ShadowMap;

			/** Parameter block containing parameters specific for shadow projection. */
			const SPtr<GpuBuffer> ShadowParams;

			/** Parameter block containing parameters specific to this view. */
			const SPtr<GpuBuffer>& PerCamera;

			/** Contains the GBuffer textures. */
			GBufferTextures Gbuffer;
		};

		B3D_PARAM_BLOCK_BEGIN(ShadowProjectParamsDef)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gMixedToShadowSpace)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gShadowMapSize)
			B3D_PARAM_BLOCK_ENTRY(Vector2, gShadowMapSizeInv)
			B3D_PARAM_BLOCK_ENTRY(float, gSoftTransitionScale)
			B3D_PARAM_BLOCK_ENTRY(float, gFadePercent)
			B3D_PARAM_BLOCK_ENTRY(float, gFadePlaneDepth)
			B3D_PARAM_BLOCK_ENTRY(float, gInvFadePlaneRange)
			B3D_PARAM_BLOCK_ENTRY(float, gFace)
		B3D_PARAM_BLOCK_END

		extern ShadowProjectParamsDef gShadowProjectParamsDef;

		/** Material used for projecting depth into a shadow accumulation buffer for non-omnidirectional shadow maps. */
		class ShadowProjectMat : public RendererMaterial<ShadowProjectMat>
		{
			RMAT_DEF("ShadowProject.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 quality, bool directional, bool MSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SHADOW_QUALITY", quality),
					  ShaderVariationParameter("CASCADING", directional),
					  ShaderVariationParameter("NEEDS_TRANSFORM", !directional),
					  ShaderVariationParameter("MSAA_COUNT", MSAA ? 2 : 1) });

				return variation;
			};

		public:
			ShadowProjectMat() = default;
			void Initialize() override;

			/** Binds the material and its parameters to the pipeline. */
			void Bind(GpuCommandBuffer& commandBuffer, const ShadowProjectParams& params);

			/** Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	quality			Quality of the shadow filtering to use. In range [1, 4].
			 * @param[in]	directional		True if rendering a shadow from a directional light.
			 * @param[in]	MSAA			True if the GBuffer contains per-sample data.
			 */
			static ShadowProjectMat* GetVariation(u32 quality, bool directional, bool MSAA);

		private:
			SPtr<SamplerState> mSamplerState;
			SPtr<GpuBuffer> mVertParams;

			GBufferParameterBinding mGBufferParams;

			GpuParameterSampledTexture mShadowMapParam;
			GpuParameterSampler mShadowSamplerParam;
		};

		B3D_PARAM_BLOCK_BEGIN(ShadowProjectOmniParamsDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(Matrix4, gFaceVPMatrices, 6)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gLightPosAndRadius)
			B3D_PARAM_BLOCK_ENTRY(float, gInvResolution)
			B3D_PARAM_BLOCK_ENTRY(float, gFadePercent)
			B3D_PARAM_BLOCK_ENTRY(float, gDepthBias)
		B3D_PARAM_BLOCK_END

		extern ShadowProjectOmniParamsDef gShadowProjectOmniParamsDef;

		/** Material used for projecting depth into a shadow accumulation buffer for omnidirectional shadow maps. */
		class ShadowProjectOmniMat : public RendererMaterial<ShadowProjectOmniMat>
		{
			RMAT_DEF("ShadowProjectOmni.bsl");

			/** Helper method used for initializing variations of this material. */
			template <u32 quality, bool inside, bool MSAA>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SHADOW_QUALITY", quality),
					  ShaderVariationParameter("VIEWER_INSIDE_VOLUME", inside),
					  ShaderVariationParameter("NEEDS_TRANSFORM", true),
					  ShaderVariationParameter("MSAA_COUNT", MSAA ? 2 : 1) });

				return variation;
			};

		public:
			ShadowProjectOmniMat() = default;
			void Initialize() override;

			/** Binds the material and its parameters to the pipeline. */
			void Bind(GpuCommandBuffer& commandBuffer, const ShadowProjectParams& params);

			/** Returns the material variation matching the provided parameters.
			 *
			 * @param[in]	quality			Quality of the shadow filtering to use. In range [1, 4].
			 * @param[in]	inside			True if the viewer is inside the light volume.
			 * @param[in]	MSAA			True if the GBuffer contains per-sample data.
			 */
			static ShadowProjectOmniMat* GetVariation(u32 quality, bool inside, bool MSAA);

		private:
			SPtr<SamplerState> mSamplerState;
			SPtr<GpuBuffer> mVertParams;

			GBufferParameterBinding mGBufferParams;

			GpuParameterSampledTexture mShadowMapParam;
			GpuParameterSampler mShadowSamplerParam;
		};

		/** Pixel format used for rendering and storing shadow maps. */
		const PixelFormat kShadowMapFormat = PF_D16;

		/** Information about a shadow cast from a single light. */
		struct ShadowInfo
		{
			/** Updates normalized area coordinates based on the non-normalized ones and the provided atlas size. */
			void UpdateNormArea(u32 atlasSize);

			u32 LightIdx; /**< Index of the light casting this shadow. */
			Area2I Area; /**< Area of the shadow map in pixels, relative to its source texture. */
			Area2 NormArea; /**< Normalized shadow map area in [0, 1] range. */
			u32 TextureIdx; /**< Index of the texture the shadow map is stored in. */

			float DepthNear; /**< Distance to the near plane. */
			float DepthFar; /**< Distance to the far plane. */
			float DepthFade; /**< Distance to the plane at which to start fading out the shadows (only for CSM). */
			float FadeRange; /**< Distance from the fade plane to the far plane (only for CSM). */

			float DepthBias; /**< Bias used to reduce shadow acne. */
			float DepthRange; /**< Length of the range covered by the shadow caster volume. */

			u32 CascadeIdx; /**< Index of a cascade. Only relevant for CSM. */

			/** View-projection matrix from the shadow casters point of view. */
			Matrix4 ShadowVpTransform;

			/** View-projection matrix for each cubemap face, used for omni-directional shadows. */
			Matrix4 ShadowVpTransforms[6];

			/** Bounds of the geometry the shadow is being applied on. */
			Sphere SubjectBounds;

			/** Determines the fade amount of the shadow, for each view in the scene. */
			TInlineArray<float, 6> FadePerView;
		};

		/**
		 * Contains a texture that serves as an atlas for one or multiple shadow maps. Provides methods for inserting new maps
		 * in the atlas.
		 */
		class ShadowMapAtlas
		{
		public:
			ShadowMapAtlas(u32 size);

			/**
			 * Registers a new map in the shadow map atlas. Returns true if the map fits in the atlas, or false otherwise.
			 * Resets the last used counter to zero.
			 */
			bool AddMap(u32 size, Area2I& area, u32 border = 4);

			/** Clears all shadow maps from the atlas. Increments the last used counter.*/
			void Clear();

			/** Checks have any maps been added to the atlas. */
			bool IsEmpty() const;

			/**
			 * Returns the value of the last used counter. See addMap() and clear() for information on how the counter is
			 * incremented/decremented.
			 */
			u32 GetLastUsedCounter() const { return mLastUsedCounter; }

			/** Returns the bindable atlas texture. */
			SPtr<Texture> GetTexture() const;

			/** Returns the render target that allows you to render into the atlas. */
			SPtr<RenderTexture> GetTarget() const;

		private:
			SPtr<PooledRenderTexture> mAtlas;

			StaticTextureAtlasLayout mLayout;
			u32 mLastUsedCounter;
		};

		/** Contains common code for different shadow map types. */
		class ShadowMapBase
		{
		public:
			ShadowMapBase(u32 size);

			virtual ~ShadowMapBase() {}

			/** Returns the bindable shadow map texture. */
			SPtr<Texture> GetTexture() const;

			/** Returns the size of a single face of the shadow map texture, in pixels. */
			u32 GetSize() const { return mSize; }

			/** Makes the shadow map available for re-use and increments the counter returned by getLastUsedCounter(). */
			void Clear()
			{
				mIsUsed = false;
				mLastUsedCounter++;
			}

			/** Marks the shadow map as used and resets the last used counter to zero. */
			void MarkAsUsed()
			{
				mIsUsed = true;
				mLastUsedCounter = 0;
			}

			/** Returns true if the object is storing a valid shadow map. */
			bool IsUsed() const { return mIsUsed; }

			/**
			 * Returns the value of the last used counter. See incrementUseCounter() and markAsUsed() for information on how is
			 * the counter incremented/decremented.
			 */
			u32 GetLastUsedCounter() const { return mLastUsedCounter; }

		protected:
			SPtr<PooledRenderTexture> mShadowMap;
			u32 mSize;

			bool mIsUsed;
			u32 mLastUsedCounter;
		};

		/** Contains a cubemap for storing an omnidirectional cubemap. */
		class ShadowCubemap : public ShadowMapBase
		{
		public:
			ShadowCubemap(u32 size);

			/** Returns a render target encompassing all six faces of the shadow cubemap. */
			SPtr<RenderTexture> GetTarget() const;
		};

		/** Contains a texture required for rendering cascaded shadow maps. */
		class ShadowCascadedMap : public ShadowMapBase
		{
		public:
			ShadowCascadedMap(u32 size, u32 numCascades);

			/** Returns the total number of cascades in the cascade shadow map. */
			u32 GetNumCascades() const { return mNumCascades; }

			/** Returns a render target that allows rendering into a specific cascade of the cascaded shadow map. */
			SPtr<RenderTexture> GetTarget(u32 cascadeIdx) const;

			/** Provides information about a shadow for the specified cascade. */
			void SetShadowInfo(u32 cascadeIdx, const ShadowInfo& info) { mShadowInfos[cascadeIdx] = info; }

			/** @copydoc SetShadowInfo */
			const ShadowInfo& GetShadowInfo(u32 cascadeIdx) const { return mShadowInfos[cascadeIdx]; }

		private:
			u32 mNumCascades;
			Vector<SPtr<RenderTexture>> mTargets;
			Vector<ShadowInfo> mShadowInfos;
		};

		/** Provides functionality for rendering shadow maps. */
		class ShadowRendering
		{
			/** Contains information required for generating a shadow map for a specific light. */
			struct ShadowMapOptions
			{
				u32 LightIdx;
				u32 MapSize;
				TInlineArray<float, 6> FadePercents;
			};

			/** Contains references to all shadows cast by a specific light. */
			struct LightShadows
			{
				u32 StartIdx = 0;
				u32 NumShadows = 0;
			};

			/** Contains references to all shadows cast by a specific light, per view. */
			struct PerViewLightShadows
			{
				TInlineArray<LightShadows, 6> ViewShadows;
			};

		public:
			ShadowRendering(u32 shadowMapSize);

			/** For each visible shadow casting light, renders a shadow map from its point of view. */
			void RenderShadowMaps(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, const RendererViewGroup& viewGroup, const FrameInfo& frameInfo);

			/**
			 * Renders shadow occlusion values for the specified light, through the provided view, into the currently bound
			 * render target. The system uses shadow maps rendered by renderShadowMaps().
			 */
			void RenderShadowOcclusion(GpuCommandBuffer& commandBuffer, const RendererView& view, const RendererLight& light, GBufferTextures gbuffer) const;

			/** Changes the default shadow map size. Will cause all shadow maps to be rebuilt. */
			void SetShadowMapSize(u32 size);

		private:
			/** Renders cascaded shadow maps for the provided directional light viewed from the provided view. */
			void RenderCascadedShadowMaps(GpuCommandBuffer& commandBuffer, const RendererView& view, u32 lightIdx, RenderBeastScene& scene, const FrameInfo& frameInfo);

			/** Renders shadow maps for the provided spot light. */
			void RenderSpotShadowMap(GpuCommandBuffer& commandBuffer, const RendererLight& light, const ShadowMapOptions& options, RenderBeastScene& scene, const FrameInfo& frameInfo);

			/** Renders shadow maps for the provided radial light. */
			void RenderRadialShadowMap(GpuCommandBuffer& commandBuffer, const RendererLight& light, const ShadowMapOptions& options, RenderBeastScene& scene, const FrameInfo& frameInfo);

			/**
			 * Calculates optimal shadow map size, taking into account all views in the scene. Also calculates a fade value
			 * that can be used for fading out small shadow maps.
			 *
			 * @param[in]	light			Light for which to calculate the shadow map properties. Cannot be a directional light.
			 * @param[in]	viewGroup		All the views the shadow will (potentially) be seen through.
			 * @param[in]	border			Border to reduce the shadow map size by, in pixels.
			 * @param[out]	size			Optimal size of the shadow map, in pixels.
			 * @param[out]	fadePercents	Value in range [0, 1] determining how much should the shadow map be faded out. Each
			 *								entry corresponds to a single view.
			 * @param[out]	maxFadePercent	Maximum value in the @p fadePercents array.
			 */
			void CalcShadowMapProperties(const RendererLight& light, const RendererViewGroup& viewGroup, u32 border, u32& size, TInlineArray<float, 6>& fadePercents, float& maxFadePercent) const;

			/**
			 * Draws a mesh representing near and far planes at the provided coordinates. The mesh is constructed using
			 * normalized device coordinates and requires no perspective transform. Near plane will be drawn using front facing
			 * triangles, and the far plane will be drawn using back facing triangles.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	near			Location of the near plane, in NDC.
			 * @param	far				Location of the far plane, in NDC.
			 * @param	drawNear		If disabled, only the far plane will be drawn.
			 */
			void DrawNearFarPlanes(GpuCommandBuffer& commandBuffer, float near, float far, bool drawNear = true) const;

			/**
			 * Draws a frustum mesh using the provided vertices as its corners. Corners should be in the order specified
			 * by AABox::Corner enum.
			 */
			void DrawFrustum(GpuCommandBuffer& commandBuffer, const std::array<Vector3, 8>& corners) const;

			/**
			 * Calculates optimal shadow quality based on the quality set in the options and the actual shadow map resolution.
			 */
			static u32 GetShadowQuality(u32 requestedQuality, u32 shadowMapResolution, u32 minAllowedQuality);

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
			static ConvexVolume GetCsmSplitFrustum(const RendererView& view, const Vector3& lightDir, u32 cascade, u32 numCascades, Sphere& outBounds);

			/**
			 * Finds the distance (along the view direction) of the frustum split for the specified index. Used for cascaded
			 * shadow maps.
			 *
			 * @param[in]	view					View whose frustum to split.
			 * @param[in]	index					Index of the split. 0 = near plane.
			 * @param[in]	numCascades				Maximum number of cascades in the cascaded shadow map. Must be greater than
			 *										zero and greater or equal to @p index.
			 * @return								Distance to the split position along the view direction.
			 */
			static float GetCsmSplitDistance(const RendererView& view, u32 index, u32 numCascades);

			/**
			 * Calculates a bias that can be applied when rendering shadow maps, in order to reduce shadow artifacts.
			 *
			 * @param[in]	light		Light to calculate the depth bias for.
			 * @param[in]	radius		Radius of the light bounds.
			 * @param[in]	depthRange	Range of depths (distance between near and far planes) covered by the shadow.
			 * @param[in]	mapSize		Size of the shadow map, in pixels.
			 * @return					Depth bias that can be passed to shadow depth rendering shader.
			 */
			static float GetDepthBias(const Light& light, float radius, float depthRange, u32 mapSize);

			/**
			 * Calculates a fade transition value that can be used for slowly fading-in the shadow, in order to avoid or reduce
			 * shadow acne.
			 *
			 * @param[in]	light		Light to calculate the fade transition size for.
			 * @param[in]	radius		Radius of the light bounds.
			 * @param[in]	depthRange	Range of depths (distance between near and far planes) covered by the shadow.
			 * @param[in]	mapSize		Size of the shadow map, in pixels.
			 * @return					Value that determines the size of the fade transition region.
			 */
			static float GetFadeTransition(const Light& light, float radius, float depthRange, u32 mapSize);

			/** Size of a single shadow map atlas, in pixels. */
			static const u32 kMaxAtlasSize;

			/** Determines how long will an unused shadow map atlas stay allocated, in frames. */
			static const u32 kMaxUnusedFrames;

			/** Determines the minimal resolution of a shadow map. */
			static const u32 kMinShadowMapSize;

			/** Determines the resolution at which shadow maps begin fading out. */
			static const u32 kShadowMapFadeSize;

			/** Size of the border of a shadow map in a shadow map atlas, in pixels. */
			static const u32 kShadowMapBorder;

			/** Percent of the length of a single cascade in a CSM, in which to fade out the cascade. */
			static const float kCascadeFractionFade;

			u32 mShadowMapSize;

			Vector<ShadowMapAtlas> mDynamicShadowMaps;
			Vector<ShadowCascadedMap> mCascadedShadowMaps;
			Vector<ShadowCubemap> mShadowCubemaps;

			Vector<ShadowInfo> mShadowInfos;

			Vector<LightShadows> mSpotLightShadows;
			Vector<LightShadows> mRadialLightShadows;
			Vector<PerViewLightShadows> mDirectionalLightShadows;

			SPtr<VertexDescription> mPositionOnlyVertexDescription;

			// Mesh information used for drawing near & far planes
			mutable SPtr<GpuBuffer> mPlaneIB;
			mutable SPtr<GpuBuffer> mPlaneVB;

			// Mesh information used for drawing a shadow frustum
			mutable SPtr<GpuBuffer> mFrustumIB;
			mutable SPtr<GpuBuffer> mFrustumVB;

			Vector<bool> mRenderableVisibility; // Transient
			Vector<ShadowMapOptions> mSpotLightShadowOptions; // Transient
			Vector<ShadowMapOptions> mRadialLightShadowOptions; // Transient
		};

		/* @} */
	} // namespace render
} // namespace b3d
