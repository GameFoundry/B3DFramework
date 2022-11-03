//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererLight.h"
#include "BsRendererView.h"
#include "BsRendererParticles.h"
#include "Shading/BsLightProbes.h"
#include "Utility/BsSamplerOverrides.h"

namespace bs
{
	struct EvaluatedAnimationData;
	struct ParticlePerFrameData;

	namespace ct
	{
		struct RendererDecal;
		class Decal;
		struct FrameInfo;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		// Limited by max number of array elements in texture for DX11 hardware
		constexpr u32 kMaxReflectionCubemaps = 2048 / 6;

		/** Contains most scene objects relevant to the renderer. */
		struct SceneInfo
		{
			// Cameras and render targets
			Vector<RendererRenderTarget> RenderTargets;
			Vector<RendererView*> Views;
			UnorderedMap<const Camera*, u32> CameraToView;

			// Renderables
			Vector<RendererRenderable*> Renderables;
			Vector<CullInfo> RenderableCullInfos;

			// Lights
			Vector<RendererLight> DirectionalLights;
			Vector<RendererLight> RadialLights;
			Vector<RendererLight> SpotLights;
			Vector<Sphere> RadialLightWorldBounds;
			Vector<Sphere> SpotLightWorldBounds;

			// Reflection probes
			Vector<RendererReflectionProbe> ReflProbes;
			Vector<Sphere> ReflProbeWorldBounds;
			Vector<bool> ReflProbeCubemapArrayUsedSlots;
			SPtr<Texture> ReflProbeCubemapsTex;

			// Light probes (indirect lighting)
			LightProbes LightProbes;

			// Particles
			Vector<RendererParticles> ParticleSystems;
			Vector<CullInfo> ParticleSystemCullInfos;

			// Decals
			Vector<RendererDecal> Decals;
			Vector<CullInfo> DecalCullInfos;

			// Sky
			Skybox* Skybox = nullptr;

			// Buffers for various transient data that gets rebuilt every frame
			//// Rebuilt every frame
			mutable Vector<bool> RenderableReady;
		};

		/** Contains information about the scene (e.g. renderables, lights, cameras) required by the renderer. */
		class RendererScene
		{
		public:
			RendererScene(const SPtr<RenderBeastOptions>& options);
			~RendererScene();

			/** Registers a new camera in the scene. */
			void RegisterCamera(Camera* camera);

			/** Updates information about a previously registered camera. */
			void UpdateCamera(Camera* camera, u32 updateFlag);

			/** Removes a camera from the scene. */
			void UnregisterCamera(Camera* camera);

			/** Registers a new light in the scene. */
			void RegisterLight(Light* light);

			/** Updates information about a previously registered light. */
			void UpdateLight(Light* light);

			/** Removes a light from the scene. */
			void UnregisterLight(Light* light);

			/** Registers a new renderable object in the scene. */
			void RegisterRenderable(Renderable* renderable);

			/** Updates information about a previously registered renderable object. */
			void UpdateRenderable(Renderable* renderable);

			/** Removes a renderable object from the scene. */
			void UnregisterRenderable(Renderable* renderable);

			/** Registers a new reflection probe in the scene. */
			void RegisterReflectionProbe(ReflectionProbe* probe);

			/** Updates information about a previously registered reflection probe. */
			void UpdateReflectionProbe(ReflectionProbe* probe, bool texture);

			/** Removes a reflection probe from the scene. */
			void UnregisterReflectionProbe(ReflectionProbe* probe);

			/** Updates the index at which the reflection probe's texture is stored at, in the global array. */
			void SetReflectionProbeArrayIndex(u32 probeIdx, u32 arrayIdx, bool markAsClean);

			/** Registers a new light probe volume in the scene. */
			void RegisterLightProbeVolume(LightProbeVolume* volume);

			/** Updates information about a previously registered light probe volume. */
			void UpdateLightProbeVolume(LightProbeVolume* volume);

			/** Removes a light probe volume from the scene. */
			void UnregisterLightProbeVolume(LightProbeVolume* volume);

			/**
			 * Rebuilds any light probe related information. Should be called once immediately before rendering. If no change
			 * is detected since the last call, the call does nothing.
			 */
			void UpdateLightProbes();

			/** Registers a new sky texture in the scene. */
			void RegisterSkybox(Skybox* skybox);

			/** Removes a skybox from the scene. */
			void UnregisterSkybox(Skybox* skybox);

			/** Registers a new particle system in the scene. */
			void RegisterParticleSystem(ParticleSystem* particleSystem);

			/** Updates information about a previously registered particle system. */
			void UpdateParticleSystem(ParticleSystem* particleSystem, bool tfrmOnly);

			/** Removes a particle system from the scene. */
			void UnregisterParticleSystem(ParticleSystem* particleSystem);

			/** Registers a new decal object in the scene. */
			void RegisterDecal(Decal* decal);

			/** Updates information about a previously registered decal object. */
			void UpdateDecal(Decal* decal);

			/** Removes a decal object from the scene. */
			void UnregisterDecal(Decal* decal);

			/** Returns a container with all relevant scene objects. */
			const SceneInfo& GetSceneInfo() const { return mInfo; }

			/** Updates scene according to the newly provided renderer options. */
			void SetOptions(const SPtr<RenderBeastOptions>& options);

			/**
			 * Checks all sampler overrides in case material sampler states changed, and updates them.
			 *
			 * @param[in]	force	If true, all sampler overrides will be updated, regardless of a change in the material
			 *						was detected or not.
			 */
			void RefreshSamplerOverrides(bool force = false);

			/** Updates global per frame parameter buffers with new values. To be called at the start of every frame. */
			void SetParamFrameParams(float time);

			/**
			 * Performs necessary per-frame updates to a renderable. This must be called once every frame for every renderable.
			 *
			 * @param[in]	idx			Index of the renderable to prepare.
			 * @param[in]	frameInfo	Global information describing the current frame.
			 */
			void PrepareRenderable(u32 idx, const FrameInfo& frameInfo);

			/**
			 * Performs necessary steps to make a renderable ready for rendering. This must be called at least once every frame
			 * for every renderable that will be drawn. Multiple calls for the same renderable during a single frame will result
			 * in a no-op.
			 *
			 * @param[in]	idx			Index of the renderable to prepare.
			 * @param[in]	frameInfo	Global information describing the current frame.
			 */
			void PrepareVisibleRenderable(u32 idx, const FrameInfo& frameInfo);

			/**
			 * Performs necessary steps to make a particle system ready for rendering. This must be called at least once every
			 * frame for every particle system that will be drawn.
			 *
			 * @param[in]	idx			Index of the particle system to prepare.
			 * @param[in]	frameInfo	Global information describing the current frame.
			 */
			void PrepareParticleSystem(u32 idx, const FrameInfo& frameInfo);

			/**
			 * Performs necessary steps to make a decal ready for rendering. This must be called at least once every frame
			 * for every decal that will be drawn.
			 *
			 * @param[in]	idx			Index of the decal to prepare.
			 * @param[in]	frameInfo	Global information describing the current frame.
			 */
			void PrepareDecal(u32 idx, const FrameInfo& frameInfo);

			/** Updates the bounds for all the particle systems from the provided object. */
			void UpdateParticleSystemBounds(const ParticlePerFrameData* particleRenderData);

			/** Returns a modifiable version of SceneInfo. Only to be used by friends who know what they are doing. */
			SceneInfo& GetSceneInfoInternal() { return mInfo; }

		private:
			/** Creates a renderer view descriptor for the particular camera. */
			RENDERER_VIEW_DESC CreateViewDesc(Camera* camera) const;

			/**
			 * Find the render target the camera belongs to and adds it to the relevant list. If the camera was previously
			 * registered with some other render target it will be removed from it and added to the new target.
			 */
			void UpdateCameraRenderTargets(Camera* camera, bool remove = false);

			/**
			 * Allocates (or returns existing) set of sampler state overrides that can be used for the provided render
			 * element.
			 */
			MaterialSamplerOverrides* AllocSamplerStateOverrides(RenderElement& elem);

			/** Frees sampler state overrides previously allocated with allocSamplerStateOverrides(). */
			void FreeSamplerStateOverrides(RenderElement& elem);

			SceneInfo mInfo;
			SPtr<GpuParamBlockBuffer> mPerFrameParamBuffer;
			UnorderedMap<SamplerOverrideKey, MaterialSamplerOverrides*> mSamplerOverrides;

			SPtr<RenderBeastOptions> mOptions;
		};

		B3D_PARAM_BLOCK_BEGIN(PerFrameParamDef)
			B3D_PARAM_BLOCK_ENTRY(float, gTime)
		B3D_PARAM_BLOCK_END

		extern PerFrameParamDef gPerFrameParamDef;

		/** Basic shader that is used when no other is available. */
		class DefaultMaterial : public RendererMaterial<DefaultMaterial>
		{
			RMAT_DEF("Default.bsl");
		};

		/** @} */
	} // namespace ct
} // namespace bs
