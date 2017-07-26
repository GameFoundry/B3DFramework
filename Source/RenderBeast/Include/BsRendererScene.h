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

	// Limited by max number of array elements in texture for DX11 hardware
	constexpr UINT32 MaxReflectionCubemaps = 2048 / 6;

	/** Contains information about the skybox in the current scene. */
	struct SkyInfo
	{
		Skybox* skybox = nullptr;

		SPtr<Texture> radiance;
		SPtr<Texture> filteredReflections;
		SPtr<Texture> irradiance;
	};

	/** Contains most scene objects relevant to the renderer. */
	struct SceneInfo
	{
		// Cameras and render targets
		Vector<RendererRenderTarget> renderTargets;
		Vector<RendererView*> views;
		UnorderedMap<const Camera*, UINT32> cameraToView;
		
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
		Vector<bool> reflProbeCubemapArrayUsedSlots;
		SPtr<Texture> reflProbeCubemapsTex;

		// Sky
		SkyInfo sky;

		// Buffers for various transient data that gets rebuilt every frame
		//// Rebuilt every frame
		mutable Vector<bool> renderableReady;
	};

	/** Contains information about the scene (e.g. renderables, lights, cameras) required by the renderer. */
	class RendererScene
	{
	public:
		RendererScene(const SPtr<RenderBeastOptions>& options);
		~RendererScene();

		/** Registers a new camera in the scene. */
		void registerCamera(Camera* camera);

		/** Updates information about a previously registered camera. */
		void updateCamera(Camera* camera, UINT32 updateFlag);

		/** Removes a camera from the scene. */
		void unregisterCamera(Camera* camera);

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

		/** Updates or replaces the filtered reflection texture of the probe at the specified index. */
		void setReflectionProbeTexture(UINT32 probeIdx, const SPtr<Texture>& texture);

		/** Updates the index at which the reflection probe's texture is stored at, in the global array. */
		void setReflectionProbeArrayIndex(UINT32 probeIdx, UINT32 arrayIdx, bool markAsClean);

		/** Registers a new sky texture in the scene. */
		void registerSkybox(Skybox* skybox);

		/** Updates information about the previously registered skybox. */
		void updateSkybox(Skybox* skybox);

		/** Removes a skybox from the scene. */
		void unregisterSkybox(Skybox* skybox);

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

		/** Returns a modifiable version of SceneInfo. Only to be used by friends who know what they are doing. */
		SceneInfo& _getSceneInfo() { return mInfo; }
	private:
		/** Creates a renderer view descriptor for the particular camera. */
		RENDERER_VIEW_DESC createViewDesc(Camera* camera) const;

		/** 
		 * Find the render target the camera belongs to and adds it to the relevant list. If the camera was previously
		 * registered with some other render target it will be removed from it and added to the new target.
		 */
		void updateCameraRenderTargets(Camera* camera, bool remove = false);

		SceneInfo mInfo;
		UnorderedMap<SamplerOverrideKey, MaterialSamplerOverrides*> mSamplerOverrides;

		DefaultMaterial* mDefaultMaterial = nullptr;
		SPtr<RenderBeastOptions> mOptions;
	};

	/** @} */
}}
