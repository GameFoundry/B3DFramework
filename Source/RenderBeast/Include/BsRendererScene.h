//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsObjectRendering.h"
#include "BsSamplerOverrides.h"
#include "BsLightRendering.h"
#include "BsRendererView.h"
#include "BsLight.h"

namespace bs 
{ 
	struct RendererAnimationData;

	namespace ct
	{
		struct FrameInfo;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Contains most scene objects relevant to the renderer. */
	struct SceneInfo
	{
		// Cameras and render targets
		Vector<RendererRenderTarget> renderTargets;
		UnorderedMap<const Camera*, RendererView*> views;
		
		// Renderables
		Vector<RendererObject*> renderables;
		Vector<CullInfo> renderableCullInfos;

		// Lights
		Vector<RendererLight> directionalLights;
		Vector<RendererLight> radialLights;
		Vector<RendererLight> spotLights;
		Vector<Sphere> radialLightWorldBounds;
		Vector<Sphere> spotLightWorldBounds;

		// Reflection probes
		Vector<RendererReflectionProbe> reflProbes;
		Vector<Sphere> reflProbeWorldBounds;

		// Buffers for various transient data that gets rebuilt every frame
		//// Rebuilt every frame
		mutable Vector<bool> renderableReady;

		//// Rebuilt for every set of views
		mutable Vector<bool> renderableVisibility;
		mutable Vector<bool> radialLightVisibility;
		mutable Vector<bool> spotLightVisibility;
	};

	/** Contains information about the scene (e.g. renderables, lights, cameras) required by the renderer. */
	class RendererScene
	{
	public:
		RendererScene(const SPtr<RenderBeastOptions>& options);
		~RendererScene();

		/** Registers a new camera in the scene. */
		void registerCamera(const Camera* camera);

		/** Updates information about a previously registered camera. */
		void updateCamera(const Camera* camera, UINT32 updateFlag);

		/** Removes a camera from the scene. */
		void unregisterCamera(const Camera* camera);

		/** Registers a new light in the scene. */
		void registerLight(Light* light);

		/** Updates information about a previously registered light. */
		void updateLight(Light* light);

		/** Removes a light from the scene. */
		void unregisterLight(Light* light);

		/** Registers a new renderable object in the scene. */
		void registerRenderable(Renderable* renderable);

		/** Updates information about a previously registered renderable object. */
		void updateRenderable(Renderable* renderable);

		/** Removes a renderable object from the scene. */
		void unregisterRenderable(Renderable* renderable);

		/** Registers a new reflection probe in the scene. */
		void registerReflectionProbe(ReflectionProbe* probe);

		/** Updates information about a previously registered reflection probe. */
		void updateReflectionProbe(ReflectionProbe* probe);

		/** Removes a reflection probe from the scene. */
		void unregisterReflectionProbe(ReflectionProbe* probe);

		/** Updates the index which maps the light to a particular shadow map in ShadowRendering. */
		void setLightShadowMapIdx(UINT32 lightIdx, LightType lightType, UINT32 shadowMapIndex);

		/** Updates or replaces the filtered reflection texture of the probe at the specified index. */
		void setReflectionProbeTexture(UINT32 probeIdx, const SPtr<Texture>& texture);

		/** Updates the index at which the reflection probe's texture is stored at, in the global array. */
		void setReflectionProbeArrayIndex(UINT32 probeIdx, UINT32 arrayIdx, bool markAsClean);

		/** Returns a container with all relevant scene objects. */
		const SceneInfo& getSceneInfo() const { return mInfo; }

		/** Updates scene according to the newly provided renderer options. */
		void setOptions(const SPtr<RenderBeastOptions>& options);

		/**
		 * Checks all sampler overrides in case material sampler states changed, and updates them.
		 *
		 * @param[in]	force	If true, all sampler overrides will be updated, regardless of a change in the material
		 *						was detected or not.
		 */
		void refreshSamplerOverrides(bool force = false);

		/**
		 * Performs necessary steps to make a renderable ready for rendering. This must be called at least once every frame,
		 * for every renderable that will be drawn. Multiple calls for the same renderable during a single frame will result
		 * in a no-op.
		 * 
		 * @param[in]	idx			Index of the renderable to prepare.
		 * @param[in]	frameInfo	Global information describing the current frame.
		 */
		void prepareRenderable(UINT32 idx, const FrameInfo& frameInfo);
	private:
		/** 
		 * Updates (or adds) renderer specific data for the specified camera. Should be called whenever camera properties
		 * change. 
		 *
		 * @param[in]	camera		Camera whose data to update.
		 * @param[in]	forceRemove	If true, the camera data will be removed instead of updated.
		 * @return					Renderer view object that represents the camera. Null if camera was removed.
		 */
		RendererView* updateCameraData(const Camera* camera, bool forceRemove = false);

		SceneInfo mInfo;
		UnorderedMap<SamplerOverrideKey, MaterialSamplerOverrides*> mSamplerOverrides;

		DefaultMaterial* mDefaultMaterial = nullptr;
		SPtr<RenderBeastOptions> mOptions;
	};


	/** @} */
}}
