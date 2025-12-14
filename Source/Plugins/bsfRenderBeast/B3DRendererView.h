//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRenderQueue.h"
#include "Renderer/B3DRenderSettings.h"
#include "Math/B3DBounds.h"
#include "Math/B3DConvexVolume.h"
#include "Shading/B3DLightGrid.h"
#include "Shading/B3DShadowRendering.h"
#include "B3DRendererRenderable.h"
#include "B3DRenderCompositor.h"
#include "B3DRendererParticles.h"
#include "B3DRendererDecal.h"
#include "Renderer/B3DRenderer.h"

namespace b3d
{
	namespace render
	{
		struct SceneInfo;
		class RendererLight;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		B3D_UNIFORM_BUFFER_BEGIN(PerCameraParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gViewDir)
			B3D_UNIFORM_BUFFER_MEMBER(Vector3, gViewOrigin)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatViewProj)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatView)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatProj)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatInvProj)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatInvViewProj)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatPrevViewProj)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gMatScreenToWorld)
			B3D_UNIFORM_BUFFER_MEMBER(Matrix4, gNDCToPrevNDC)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gDeviceZToWorldZ)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gNDCZToWorldZ)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gNDCZToDeviceZ)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2, gNearFar)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4I, gViewportRectangle)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gClipToUVScaleOffset)
			B3D_UNIFORM_BUFFER_MEMBER(Vector4, gUVToClipScaleOffset)
			B3D_UNIFORM_BUFFER_MEMBER(float, gAmbientFactor)
		B3D_UNIFORM_BUFFER_END

		extern PerCameraParamDef gPerCameraUniformDefinition;

		B3D_UNIFORM_BUFFER_BEGIN(SkyboxUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Color, gClearColor)
		B3D_UNIFORM_BUFFER_END

		extern SkyboxUniformDefinition gSkyboxUniformDefinition;

		/** Shader that renders a skybox using a cubemap or a solid color. */
		class SkyboxMaterial : public RendererMaterial<SkyboxMaterial>
		{
			RMAT_DEF("Skybox.bsl");

			/** Helper method used for initializing variations of this material. */
			template <bool color>
			static const ShaderVariationParameters& GetVariation()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters(
					{ ShaderVariationParameter("SOLID_COLOR", color) });

				return variation;
			}

		public:
			SkyboxMaterial() = default;
			void Initialize() override;

			/** Binds the material for rendering and sets up any parameters. */
			void Bind(GpuCommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perCamera, const SPtr<Texture>& texture, const Color& solidColor);

			/**
			 * Returns the material variation matching the provided parameters.
			 *
			 * @param	color	When true the material will use a solid color to render a skybox. When false a user
			 *					provided texture will be used instead.
			 */
			static SkyboxMaterial* GetVariation(bool color);

		private:
			GpuParameterUniformBuffer mUniformBufferParameter;
			GpuParameterSampledTexture mSkyTextureParameter;
		};

		/** Data shared between RendererViewCreateInformation and RendererViewProperties */
		struct RendererViewInformation
		{
			RendererViewInformation();

			Matrix4 ViewTransform;
			Matrix4 ProjTransform;
			Vector3 ViewDirection;
			Vector3 ViewOrigin;
			bool FlipView;
			float NearPlane;
			float FarPlane;
			ProjectionType ProjType;

			/**
			 * Determines does this view output to the final render target. If false the view is usually used for some
			 * sort of helper rendering.
			 */
			bool MainView;

			/**
			 * When enabled, renderer extension callbacks will be triggered, allowing other systems to inject their own
			 * render operations into the view.
			 */
			bool TriggerCallbacks : 1;

			/** When enabled, post-processing effects (like tonemapping) will be executed. */
			bool RunPostProcessing : 1;

			/**
			 * Determines if the view is currently rendering reflection probes. This ensures the systems can disable refl.
			 * probe reads in order to prevent incorrect rendering (since probes won't yet have any data).
			 */
			bool CapturingReflections : 1;

			/**
			 * When enabled the alpha channel of the final render target will be populated with an encoded depth value.
			 * Parameters @p depthEncodeNear and @p depthEncodeFar control which range of the depth buffer to encode.
			 */
			bool EncodeDepth : 1;

			/** If true the view will only be rendered when requested, otherwise it will be rendered every frame. */
			bool OnDemand : 1;

			/**
			 * Controls at which position to start encoding depth, in view space. Only relevant with @p encodeDepth is enabled.
			 * Depth will be linearly interpolated between this value and @p depthEncodeFar.
			 */
			float DepthEncodeNear = 0.0f;

			/**
			 * Controls at which position to stop encoding depth, in view space. Only relevant with @p encodeDepth is enabled.
			 * Depth will be linearly interpolated between @p depthEncodeNear and this value.
			 */
			float DepthEncodeFar = 0.0f;

			u64 VisibleLayers;
			ConvexVolume CullFrustum;
		};

		/** Data shared between RendererViewTargetCreateInformation and RendererViewTargetProperties */
		struct RendererViewTargetInformation
		{
			SPtr<RenderTarget> Target;

			Area2I ViewRect;
			Area2 NrmViewRect;
			u32 TargetWidth;
			u32 TargetHeight;
			u32 NumSamples;

			u32 ClearFlags;
			Color ClearColor;
			float ClearDepthValue;
			u16 ClearStencilValue;
		};

		/** Set of properties describing the output render target used by a renderer view. */
		struct RendererViewTargetCreateInformation : RendererViewTargetInformation
		{};

		/** Set of properties used describing a specific view that the renderer can render. */
		struct RendererViewCreateInformation : RendererViewInformation
		{
			RendererViewTargetCreateInformation Target;

			StateReduction StateReduction;
			Camera* SceneCamera;
		};

		/** Set of properties used describing a specific view that the renderer can render. */
		struct RendererViewProperties : RendererViewInformation
		{
			RendererViewProperties() {}

			RendererViewProperties(const RendererViewCreateInformation& src);

			Matrix4 ViewProjTransform;
			Matrix4 PrevViewProjTransform;
			Matrix4 ProjTransformNoAa;
			Vector2 TemporalJitter{ BsZero };
			u32 FrameIdx;

			RendererViewTargetInformation Target;
		};

		/** Information whether certain scene objects are visible in a view, per object type. */
		struct VisibilityInfo
		{
			Vector<bool> Renderables;
			Vector<bool> RadialLights;
			Vector<bool> SpotLights;
			Vector<bool> ReflProbes;
			Vector<bool> ParticleSystems;
			Vector<bool> Decals;
		};

		/** Information used for culling an object against a view. */
		struct CullInfo
		{
			CullInfo(const Bounds& bounds, u64 layer = -1, float cullDistanceFactor = 1.0f)
				: Layer(layer), Bounds(bounds), CullDistanceFactor(cullDistanceFactor)
			{}

			u64 Layer;
			Bounds Bounds;
			float CullDistanceFactor;
		};

		/**	Renderer information specific to a single render target. */
		struct RendererRenderTarget
		{
			SPtr<RenderTarget> Target;
			Vector<Camera*> Cameras;
		};

		/** Returns the reason why is a RendererView being redrawn. */
		enum class RendererViewRedrawReason
		{
			/** This particular view isn't on-demand and is redrawn every frame. */
			PerFrame,

			/** Draws on demand and on-demand drawing was triggered this frame. */
			OnDemandThisFrame,

			/**
			 * Draws on demand and on-demand drawing was triggered during an earlier frame but a multi-frame effect is
			 * requiring the view to get redrawn in later frames.
			 */
			OnDemandLingering
		};

		/** Contains information about a single view into the scene, used by the renderer. */
		class RendererView
		{
		public:
			RendererView();
			RendererView(const RendererViewCreateInformation& desc);

			/** Sets state reduction mode that determines how do render queues group & sort renderables. */
			void SetStateReductionMode(StateReduction reductionMode);

			/** Updates the internal camera render settings. */
			void SetRenderSettings(const SPtr<RenderSettings>& settings);

			/** Updates the internal information with a new view transform. */
			void SetTransform(const Vector3& origin, const Vector3& direction, const Matrix4& view, const Matrix4& proj, const ConvexVolume& worldFrustum);

			/** Updates all internal information with new view information. */
			void SetView(const RendererViewCreateInformation& desc);

			/** Returns a structure describing the view. */
			const RendererViewProperties& GetProperties() const { return mProperties; }

			/** Returns the scene camera this object is based of. This can be null for manually constructed renderer cameras. */
			Camera* GetSceneCamera() const { return mCamera; }

			/** Prepares render targets for rendering. When done call endFrame(). */
			void BeginFrame(const FrameInfo& frameInfo);

			/** Ends rendering and frees any acquired resources. */
			void EndFrame();

			/**
			 * Returns a render queue containing all opaque objects for the specified pipeline. Make sure to call
			 * determineVisible() beforehand if view or object transforms changed since the last time it was called. If @p
			 * forward is true then opaque objects using the forward pipeline are returned, otherwise deferred pipeline objects
			 * are returned.
			 */
			const SPtr<RenderQueue>& GetOpaqueQueue(bool forward) const { return forward ? mForwardOpaqueQueue : mDeferredOpaqueQueue; }

			/**
			 * Returns a render queue containing all transparent objects. Make sure to call determineVisible() beforehand if
			 * view or object transforms changed since the last time it was called.
			 */
			const SPtr<RenderQueue>& GetTransparentQueue() const { return mTransparentQueue; }

			/**
			 * Returns a render queue containing all decal renderable objects. Make sure to call determineVisible() beforehand
			 * if view or object transforms changed since the last time it was called.
			 */
			const SPtr<RenderQueue>& GetDecalQueue() const { return mDecalQueue; }

			/** Returns the compositor in charge of rendering for this view. */
			const RenderCompositor& GetCompositor() const { return mCompositor; }

			/**
			 * Populates view render queues by determining visible renderable objects.
			 *
			 * @param[in]	renderables			A set of renderable objects to iterate over and determine visibility for.
			 * @param[in]	cullInfos			A set of world bounds & other information relevant for culling the provided
			 *									renderable objects. Must be the same size as the @p renderables array.
			 * @param[out]	visibility			Output parameter that will have the true bit set for any visible renderable
			 *									object. If the bit for an object is already set to true, the method will never
			 *									change it to false which allows the same bitfield to be provided to multiple
			 *									renderer views. Must be the same size as the @p renderables array.
			 *
			 *									As a side-effect, per-view visibility data is also calculated and can be
			 *									retrieved by calling getVisibilityMask().
			 */
			void DetermineVisible(const Vector<RendererRenderable*>& renderables, const Vector<CullInfo>& cullInfos, Vector<bool>* visibility = nullptr);

			/**
			 * Populates view render queues by determining visible particle systems.
			 *
			 * @param[in]	particleSystems		A set of particle systems to iterate over and determine visibility for.
			 * @param[in]	cullInfos			A set of world bounds & other information relevant for culling the provided
			 *									renderable objects. Must be the same size as the @p particleSystems array.
			 * @param[out]	visibility			Output parameter that will have the true bit set for any visible particle system
			 *									object. If the bit for an object is already set to true, the method will never
			 *									change it to false which allows the same bitfield to be provided to multiple
			 *									renderer views. Must be the same size as the @p particleSystems array.
			 *
			 *									As a side-effect, per-view visibility data is also calculated and can be
			 *									retrieved by calling getVisibilityMask().
			 */
			void DetermineVisible(const Vector<RendererParticles>& particleSystems, const Vector<CullInfo>& cullInfos, Vector<bool>* visibility = nullptr);

			/**
			 * Populates view render queues by determining visible decals.
			 *
			 * @param[in]	decals				A set of decals to iterate over and determine visibility for.
			 * @param[in]	cullInfos			A set of world bounds & other information relevant for culling the provided
			 *									renderable objects. Must be the same size as the @p decals array.
			 * @param[out]	visibility			Output parameter that will have the true bit set for any visible decal
			 *									object. If the bit for an object is already set to true, the method will never
			 *									change it to false which allows the same bitfield to be provided to multiple
			 *									renderer views. Must be the same size as the @p decals array.
			 *
			 *									As a side-effect, per-view visibility data is also calculated and can be
			 *									retrieved by calling getVisibilityMask().
			 */
			void DetermineVisible(const Vector<RendererDecal>& decals, const Vector<CullInfo>& cullInfos, Vector<bool>* visibility = nullptr);

			/**
			 * Calculates the visibility masks for all the lights of the provided type.
			 *
			 * @param[in]	lights				A set of lights to determine visibility for.
			 * @param[in]	bounds				Bounding sphere for each provided light. Must be the same size as the @p lights
			 *									array.
			 * @param[in]	type				Type of all the lights in the @p lights array.
			 * @param[out]	visibility			Output parameter that will have the true bit set for any visible light. If the
			 *									bit for a light is already set to true, the method will never change it to false
			 *									which allows the same bitfield to be provided to multiple renderer views. Must
			 *									be the same size as the @p lights array.
			 *
			 *									As a side-effect, per-view visibility data is also calculated and can be
			 *									retrieved by calling getVisibilityMask().
			 */
			void DetermineVisible(const Vector<RendererLight>& lights, const Vector<Sphere>& bounds, LightType type, Vector<bool>* visibility = nullptr);

			/**
			 * Culls the provided set of bounds against the current frustum and outputs a set of visibility flags determining
			 * which entry is or isn't visible by this view. Both inputs must be arrays of the same size.
			 */
			void CalculateVisibility(const Vector<CullInfo>& cullInfos, Vector<bool>& visibility) const;

			/**
			 * Culls the provided set of bounds against the current frustum and outputs a set of visibility flags determining
			 * which entry is or isn't visible by this view. Both inputs must be arrays of the same size.
			 */
			void CalculateVisibility(const Vector<Sphere>& bounds, Vector<bool>& visibility) const;

			/**
			 * Culls the provided set of bounds against the current frustum and outputs a set of visibility flags determining
			 * which entry is or isn't visible by this view. Both inputs must be arrays of the same size.
			 */
			void CalculateVisibility(const Vector<AABox>& bounds, Vector<bool>& visibility) const;

			/**
			 * Inserts all visible renderable elements into render queues. Assumes visibility has been calculated beforehand
			 * by calling determineVisible(). After the call render elements can be retrieved from the queues using
			 * getOpaqueQueue or getTransparentQueue() calls.
			 */
			void QueueRenderElements(const SceneInfo& sceneInfo);

			/** Returns the visibility mask calculated with the last call to determineVisible(). */
			const VisibilityInfo& GetVisibilityMasks() const { return mVisibility; }

			/** Returns per-view settings that control rendering. */
			const RenderSettings& GetRenderSettings() const { return *mRenderSettings; }

			/**
			 * Retrieves a hash value that is updated whenever render settings change. This can be used by external systems
			 * to detect when they need to update.
			 */
			u64 GetRenderSettingsHash() const { return mRenderSettingsHash; }

			/** Updates the GPU buffer containing per-view information, with the latest internal data. */
			void UpdatePerViewBuffer();

			/** Returns a buffer that stores per-view parameters. */
			SPtr<GpuBuffer> GetPerViewBuffer() const { return mUniformBuffer; }

			/**
			 * Returns information about visible lights, in the form of a light grid, used for forward rendering. Only valid
			 * after a call to updateLightGrid().
			 */
			const LightGrid& GetLightGrid() const { return mLightGrid; }

			/** Updates the light grid used for forward rendering. */
			void UpdateLightGrid(GpuCommandBuffer& commandBuffer, const VisibleLightData& visibleLightData, const VisibleReflectionProbeData& visibleReflProbeData);

			/**
			 * Returns a value that can be used for transforming x, y coordinates from NDC into UV coordinates that can be used
			 * for sampling a texture projected on the view.
			 *
			 * @return	Returns two 2D values that can be used to transform the coordinate as such: UV = NDC * xy + zw.
			 */
			Vector4 GetNdcToUv() const;

			/** Returns an index of this view within the parent view group. */
			u32 GetViewIdx() const { return mViewIdx; }

			/** Determines if a view should be rendered this frame. */
			bool ShouldDraw() const;

			/** Determines if view's 3D geometry should be rendered this frame. */
			bool ShouldDraw3D() const { return !mRenderSettings->OverlayOnly && ShouldDraw(); }

			/** Returns true if the view should write to the velocity buffer. */
			bool RequiresVelocityWrites() const;

			/**
			 * Determines if any async operations have completed executing and finalizes them. Should be called once
			 * per frame.
			 */
			void UpdateAsyncOperations();

			/**
			 * Returns the reason that explains why is the view getting redrawn this frame. Only relevant if shouldDraw() returned
			 * true previously during the frame.
			 */
			RendererViewRedrawReason GetRedrawReason() const;

			/**
			 * Gets the current exposure of the view, used for transforming scene light values from HDR in a range that can be
			 * displayed on a display device.
			 */
			float GetCurrentExposure() const;

			/** Assigns a view index to the view. To be called by the parent view group when the view is added to it. */
			void SetViewIdxInternal(u32 viewIdx) { mViewIdx = viewIdx; }

			/** Lets an on-demand view know that it should be redrawn this frame. */
			void NotifyNeedsRedraw();

			/**
			 * Notifies the view that the render target the compositor is rendering to has changed. Note that this does not
			 * mean the final render target, rather the current intermediate target as set by the renderer during the
			 * rendering of a single frame. This should be set to null if the renderer is not currently rendering the
			 * view.
			 */
			void NotifyCompositorTargetChangedInternal(const SPtr<RenderTarget>& target) const { mCurrentRenderTarget = target; }

			/**
			 * Returns the render target that is currently being rendered to by the render compositor.
			 */
			SPtr<RenderTarget> GetCompositorRenderTarget() const { return mCurrentRenderTarget; }

			/**
			 * Notifies the view that a new average luminance is being calculated on the provided command buffer. The results
			 * will be read from the provided texture when the command buffer finishes executing.
			 */
			void NotifyLuminanceUpdated(u64 frameIdx, SPtr<GpuCommandBuffer> cb, SPtr<PooledRenderTexture> texture) const;

			/**
			 * Extracts the necessary values from the projection matrix that allow you to transform device Z value (range [0, 1]
			 * into view Z value.
			 *
			 * @param[in]	projMatrix	Projection matrix that was used to create the device Z value to transform.
			 * @return					Returns two values that can be used to transform device z to view z using this formula:
			 * 							z = (deviceZ + y) * x.
			 */
			static Vector2 GetDeviceZToViewZ(const Matrix4& projMatrix);

			/**
			 * Extracts the necessary values from the projection matrix that allow you to transform NDC Z value (range depending
			 * on render API) into view Z value.
			 *
			 * @param[in]	projMatrix	Projection matrix that was used to create the NDC Z value to transform.
			 * @return					Returns two values that can be used to transform NDC z to view z using this formula:
			 * 							z = (NDCZ + y) * x.
			 */
			static Vector2 GetNdczToViewZ(const Matrix4& projMatrix);

			/**
			 * Returns a value that can be used for tranforming a depth value in NDC, to a depth value in device Z ([0, 1]
			 * range using this formula: (NDCZ + y) * x.
			 */
			static Vector2 GetNdczToDeviceZ();

		private:
			struct LuminanceUpdate
			{
				LuminanceUpdate(u64 frameIdx, TAsyncOp<SPtr<PixelData>> readbackAsyncOp, SPtr<PooledRenderTexture> outputTexture)
					: FrameIdx(frameIdx), OutputTexture(std::move(outputTexture)), ReadbackAsyncOp(std::move(readbackAsyncOp))
				{}

				u64 FrameIdx;
				SPtr<GpuCommandBuffer> CommandBuffer;
				SPtr<PooledRenderTexture> OutputTexture;
				TAsyncOp<SPtr<PixelData>> ReadbackAsyncOp;
			};

			RendererViewProperties mProperties;
			mutable SPtr<RenderTarget> mCurrentRenderTarget;
			Camera* mCamera;

			SPtr<RenderQueue> mDeferredOpaqueQueue;
			SPtr<RenderQueue> mForwardOpaqueQueue;
			SPtr<RenderQueue> mTransparentQueue;
			SPtr<RenderQueue> mDecalQueue;

			RenderCompositor mCompositor;
			SPtr<RenderSettings> mRenderSettings;
			u32 mRenderSettingsHash;

			SPtr<GpuBuffer> mUniformBuffer;
			VisibilityInfo mVisibility;
			LightGrid mLightGrid;
			u32 mViewIdx;

			// Temporal anti-aliasing
			u32 mTemporalPositionIdx;

			// On-demand drawing
			bool mRedrawThisFrame = false;
			float mRedrawForSeconds = 0.0f;
			u32 mRedrawForFrames = 0;
			u64 mWaitingOnAutoExposureFrame = std::numeric_limits<u64>::max();
			mutable Vector<LuminanceUpdate> mLuminanceUpdates;

			// Exposure
			float mPreviousEyeAdaptation = 0.0f;
			float mCurrentEyeAdaptation = 0.0f;

			// Current frame info
			FrameTimings mFrameTimings;
			bool mAsyncAnim = false;
		};

		/** Contains one or multiple RendererView%s that are in some way related. */
		class RendererViewGroup
		{
		public:
			RendererViewGroup(RendererView** views, u32 numViews, bool mainPass, u32 shadowMapSize = 2048);

			/**
			 * Updates the internal list of views. This is more efficient than always constructing a new instance of this class
			 * when views change, as internal buffers don't need to be re-allocated.
			 */
			void SetViews(RendererView** views, u32 numViews);

			/** Returns a view at the specified index. Index must be less than the value returned by getNumViews(). */
			RendererView* GetView(u32 idx) const { return mViews[idx]; }

			/** Returns the total number of views in the group. */
			u32 GetViewCount() const { return (u32)mViews.size(); }

			/** Determines is this the primary rendering pass for this frame. There can only be one primary pass per frame. */
			bool IsMainPass() const { return mIsMainPass; }

			/**
			 * Returns information about visibility of various scene objects, from the perspective of all the views in the
			 * group (visibility will be true if the object is visible from any of the views. determineVisibility() must be
			 * called whenever the scene or view information changes (usually every frame).
			 */
			const VisibilityInfo& GetVisibilityInfo() const { return mVisibility; }

			/**
			 * Returns information about lights visible from this group of views. Only valid after a call to
			 * determineVisibility().
			 */
			const VisibleLightData& GetVisibleLightData() const { return mVisibleLightData; }

			/**
			 * Returns information about refl. probes visible from this group of views. Only valid after a call to
			 * determineVisibility().
			 */
			const VisibleReflectionProbeData& GetVisibleReflProbeData() const { return mVisibleReflProbeData; }

			/** Returns the object responsible for rendering shadows for this view group. */
			ShadowRendering& GetShadowRenderer() { return mShadowRenderer; }

			/** Returns the object responsible for rendering shadows for this view group. */
			const ShadowRendering& GetShadowRenderer() const { return mShadowRenderer; }

			/**
			 * Updates visibility information for the provided scene objects, from the perspective of all views in this group,
			 * and updates the render queues of each individual view. Use getVisibilityInfo() to retrieve the calculated
			 * visibility information.
			 */
			void DetermineVisibility(GpuCommandBuffer& commandBuffer, const SceneInfo& sceneInfo);

		private:
			Vector<RendererView*> mViews;
			VisibilityInfo mVisibility;
			bool mIsMainPass = false;

			VisibleLightData mVisibleLightData;
			VisibleReflectionProbeData mVisibleReflProbeData;

			// Note: Ideally we would want to keep this global, so all views share it. This way each view group renders its
			// own set of shadows, but there might be shadows that are shared, and therefore we could avoid rendering them
			// multiple times. Since non-primary view groups are used for pre-processing tasks exclusively (at the moment)
			// this isn't an issue right now.
			ShadowRendering mShadowRenderer;
		};

		/** @} */
	} // namespace render
} // namespace b3d
