//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsPostProcessing.h"
#include "BsObjectRendering.h"
#include "BsRenderQueue.h"
#include "BsRendererObject.h"
#include "BsBounds.h"
#include "BsConvexVolume.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(PerCameraParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector3, gViewDir)
		BS_PARAM_BLOCK_ENTRY(Vector3, gViewOrigin)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatView)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatProj)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvProj)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvViewProj)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatScreenToWorld)
		BS_PARAM_BLOCK_ENTRY(Vector2, gDeviceZToWorldZ)
		BS_PARAM_BLOCK_ENTRY(Vector2, gNDCZToWorldZ)
		BS_PARAM_BLOCK_ENTRY(Vector2, gNDCZToDeviceZ)
		BS_PARAM_BLOCK_ENTRY(Vector2, gNearFar)
		BS_PARAM_BLOCK_ENTRY(Vector4I, gViewportRectangle)
		BS_PARAM_BLOCK_ENTRY(Vector4, gClipToUVScaleOffset)
		BS_PARAM_BLOCK_ENTRY(float, gAmbientFactor)
	BS_PARAM_BLOCK_END

	extern PerCameraParamDef gPerCameraParamDef;

	BS_PARAM_BLOCK_BEGIN(SkyboxParamDef)
		BS_PARAM_BLOCK_ENTRY(Color, gClearColor)
	BS_PARAM_BLOCK_END

	extern SkyboxParamDef gSkyboxParamDef;

	/** Shader that renders a skybox using a cubemap or a solid color. */
	template<bool SOLID_COLOR>
	class SkyboxMat : public RendererMaterial<SkyboxMat<SOLID_COLOR>>
	{
		RMAT_DEF("Skybox.bsl");

	public:
		SkyboxMat();

		/** Binds the material for rendering and sets up any global parameters. */
		void bind(const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Updates the skybox texture & solid color used by the material. */
		void setParams(const SPtr<Texture>& texture, const Color& solidColor);
	private:
		GpuParamTexture mSkyTextureParam;
		SPtr<GpuParamBlockBuffer> mParamBuffer;
	};

	/** Data shared between RENDERER_VIEW_DESC and RendererViewProperties */
	struct RendererViewData
	{
		Matrix4 viewTransform;
		Matrix4 projTransform;
		Vector3 viewDirection;
		Vector3 viewOrigin;
		bool flipView;
		float nearPlane;
		float farPlane;
		ProjectionType projType;

		bool isOverlay : 1;
		bool isHDR : 1;
		bool noLighting : 1;
		bool triggerCallbacks : 1;
		bool runPostProcessing : 1;
		bool renderingReflections : 1;

		UINT64 visibleLayers;
		ConvexVolume cullFrustum;
	};

	/** Data shared between RENDERER_VIEW_TARGET_DESC and RendererViewTargetProperties */
	struct RendererViewTargetData
	{
		SPtr<RenderTarget> target;

		Rect2I viewRect;
		Rect2 nrmViewRect;
		UINT32 targetWidth;
		UINT32 targetHeight;
		UINT32 numSamples;

		UINT32 clearFlags;
		Color clearColor;
		float clearDepthValue;
		UINT16 clearStencilValue;
	};

	/** Set of properties describing the output render target used by a renderer view. */
	struct RENDERER_VIEW_TARGET_DESC : RendererViewTargetData
	{ };

	/** Set of properties used describing a specific view that the renderer can render. */
	struct RENDERER_VIEW_DESC : RendererViewData
	{
		RENDERER_VIEW_TARGET_DESC target;

		StateReduction stateReduction;
		Camera* sceneCamera;
	};

	/** Set of properties used describing a specific view that the renderer can render. */
	struct RendererViewProperties : RendererViewData
	{
		RendererViewProperties() {}
		RendererViewProperties(const RENDERER_VIEW_DESC& src);

		Matrix4 viewProjTransform;

		SPtr<RenderTarget> target;
		Rect2I viewRect;
		Rect2 nrmViewRect;
		UINT32 numSamples;

		UINT32 clearFlags;
		Color clearColor;
		float clearDepthValue;
		UINT16 clearStencilValue;
	};

	/** Information whether certain scene objects are visible in a view, per object type. */
	struct VisibilityInfo
	{
		Vector<bool> renderables;
	};

	/** Information used for culling an object against a view. */
	struct CullInfo
	{
		CullInfo(const Bounds& bounds, UINT64 layer = -1)
			:bounds(bounds), layer(layer)
		{ }

		Bounds bounds;
		UINT64 layer;
	};

	/**	Renderer information specific to a single render target. */
	struct RendererRenderTarget
	{
		SPtr<RenderTarget> target;
		Vector<Camera*> cameras;
	};

	/** Contains information about a single view into the scene, used by the renderer. */
	class RendererView
	{
	public:
		RendererView();
		RendererView(const RENDERER_VIEW_DESC& desc);

		/** Sets state reduction mode that determines how do render queues group & sort renderables. */
		void setStateReductionMode(StateReduction reductionMode);

		/** Updates the internal camera post-processing data. */
		void setPostProcessSettings(const SPtr<PostProcessSettings>& ppSettings);

		/** Updates the internal information with a new view transform. */
		void setTransform(const Vector3& origin, const Vector3& direction, const Matrix4& view,
						  const Matrix4& proj, const ConvexVolume& worldFrustum);

		/** Updates all internal information with new view information. */
		void setView(const RENDERER_VIEW_DESC& desc);

		/** Returns a structure describing the view. */
		const RendererViewProperties& getProperties() const { return mProperties; }

		/** Returns the scene camera this object is based of. This can be null for manually constructed renderer cameras. */
		Camera* getSceneCamera() const { return mCamera; }

		/** 
		 * Prepares render targets for rendering. When done call endRendering().
		 *
		 * @param[in]	useGBuffer			Set to true if you will be rendering to internal render targets containing the
		 *									GBuffer (retrieved via getRenderTargets()).
		 */
		void beginRendering(bool useGBuffer);

		/** Ends rendering and frees any acquired resources. */
		void endRendering();

		/** Returns the view's renderTargets. Only valid if called in-between beginRendering() and endRendering() calls. */
		SPtr<RenderTargets> getRenderTargets() const { return mRenderTargets; }

		/** 
		 * Returns a render queue containing all opaque objects. Make sure to call determineVisible() beforehand if view 
		 * or object transforms changed since the last time it was called.
		 */
		const SPtr<RenderQueue>& getOpaqueQueue() const { return mOpaqueQueue; }
		
		/** 
		 * Returns a render queue containing all transparent objects. Make sure to call determineVisible() beforehand if 
		 * view or object transforms changed since the last time it was called.
		 */
		const SPtr<RenderQueue>& getTransparentQueue() const { return mTransparentQueue; }

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
		void determineVisible(const Vector<RendererObject*>& renderables, const Vector<CullInfo>& cullInfos,
			Vector<bool>* visibility = nullptr);

		/**
		 * Culls the provided set of bounds against the current frustum and outputs a set of visibility flags determining
		 * which entry is or isn't visible by this view. Both inputs must be arrays of the same size.
		 */
		void calculateVisibility(const Vector<CullInfo>& cullInfos, Vector<bool>& visibility) const;

		/**
		* Culls the provided set of bounds against the current frustum and outputs a set of visibility flags determining
		* which entry is or isn't visible by this view. Both inputs must be arrays of the same size.
		*/
		void calculateVisibility(const Vector<Sphere>& bounds, Vector<bool>& visibility) const;

		/** Returns the visibility mask calculated with the last call to determineVisible(). */
		const VisibilityInfo& getVisibilityMasks() const { return mVisibility; }

		/** 
		 * Returns a structure containing information about post-processing effects. This structure will be modified and
		 * maintained by the post-processing system.
		 */
		PostProcessInfo& getPPInfo() { return mPostProcessInfo; }

		/** Updates the GPU buffer containing per-view information, with the latest internal data. */
		void updatePerViewBuffer();

		/** Returns a buffer that stores per-view parameters. */
		SPtr<GpuParamBlockBuffer> getPerViewBuffer() const { return mParamBuffer; }

		/**
		 * Extracts the necessary values from the projection matrix that allow you to transform device Z value (range [0, 1]
		 * into view Z value.
		 * 
		 * @param[in]	projMatrix	Projection matrix that was used to create the device Z value to transform.
		 * @return					Returns two values that can be used to transform device z to view z using this formula:
		 * 							z = (deviceZ + y) * x.
		 */
		static Vector2 getDeviceZToViewZ(const Matrix4& projMatrix);

		/**
		 * Extracts the necessary values from the projection matrix that allow you to transform NDC Z value (range depending
		 * on render API) into view Z value.
		 * 
		 * @param[in]	projMatrix	Projection matrix that was used to create the NDC Z value to transform.
		 * @return					Returns two values that can be used to transform NDC z to view z using this formula:
		 * 							z = (NDCZ + y) * x.
		 */
		static Vector2 getNDCZToViewZ(const Matrix4& projMatrix);

		/** 
		 * Returns a value that can be used for tranforming a depth value in NDC, to a depth value in device Z ([0, 1] 
		 * range using this formula: (NDCZ + y) * x. 
		 */
		static Vector2 getNDCZToDeviceZ();
	private:
		RendererViewProperties mProperties;
		RENDERER_VIEW_TARGET_DESC mTargetDesc;
		Camera* mCamera;

		SPtr<RenderQueue> mOpaqueQueue;
		SPtr<RenderQueue> mTransparentQueue;

		SPtr<RenderTargets> mRenderTargets;
		PostProcessInfo mPostProcessInfo;
		bool mUsingGBuffer;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		VisibilityInfo mVisibility;
	};

	/** @} */
}}