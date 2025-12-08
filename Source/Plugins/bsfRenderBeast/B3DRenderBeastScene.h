//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "B3DRendererLight.h"
#include "B3DRendererView.h"
#include "B3DRendererParticles.h"
#include "Renderer/B3DRendererScene.h"
#include "Shading/B3DLightProbes.h"
#include "Utility/B3DSamplerOverrides.h"
#include "Utility/B3DUniformBufferPools.h"

namespace b3d
{
	struct EvaluatedAnimationData;
	struct EvaluatedParticleData;

	namespace render
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
		class RenderBeastScene : public RendererScene
		{
		public:
			RenderBeastScene(const SPtr<RenderBeastOptions>& options);

			void RegisterCamera(Camera* camera) override;
			void UpdateCamera(Camera* camera, u32 updateFlag) override;
			void UnregisterCamera(Camera* camera) override;

			void RegisterLight(Light* light) override;
			void UpdateLight(Light* light) override;
			void UnregisterLight(Light* light) override;

			void RegisterRenderable(Renderable* renderable) override;
			void UpdateRenderable(Renderable* renderable) override;
			void UnregisterRenderable(Renderable* renderable) override;

			void RegisterReflectionProbe(ReflectionProbe* probe) override;
			void UpdateReflectionProbe(ReflectionProbe* probe, bool texture) override;
			void UnregisterReflectionProbe(ReflectionProbe* probe) override;

			void RegisterLightProbeVolume(LightProbeVolume* volume) override;
			void UpdateLightProbeVolume(LightProbeVolume* volume) override;
			void UnregisterLightProbeVolume(LightProbeVolume* volume) override;

			void RegisterSkybox(Skybox* skybox) override;
			void UnregisterSkybox(Skybox* skybox) override;

			void RegisterParticleSystem(ParticleSystem* particleSystem) override;
			void UpdateParticleSystem(ParticleSystem* particleSystem, bool tfrmOnly) override;
			void UnregisterParticleSystem(ParticleSystem* particleSystem) override;

			void RegisterDecal(Decal* decal) override;
			void UpdateDecal(Decal* decal) override;
			void UnregisterDecal(Decal* decal) override;

			void Initialize() override;
			void Destroy() override;

			/** Updates the index at which the reflection probe's texture is stored at, in the global array. */
			void SetReflectionProbeArrayIndex(u32 probeIdx, u32 arrayIdx, bool markAsClean);

			/**
			 * Rebuilds any light probe related information. Should be called once immediately before rendering. If no change
			 * is detected since the last call, the call does nothing.
			 */
			void UpdateLightProbes(GpuCommandBuffer& commandBuffer);

			/** Updates the global reflection probe cubemap array with changed probe textures. */
			void UpdateReflectionProbes(GpuCommandBuffer& commandBuffer);

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
			void UpdateParticleSystemBounds(const EvaluatedParticleData* particleRenderData);

			/** Returns a modifiable version of SceneInfo. Only to be used by friends who know what they are doing. */
			SceneInfo& GetSceneInfo() { return mInfo; }

			/** Returns the object for managing uniform buffer allocations. */
			UniformBufferPools& GetUniformBufferPools() { return mUniformBufferPools; }

		private:
			/** Creates a renderer view descriptor for the particular camera. */
			RendererViewCreateInformation CreateViewDesc(Camera* camera) const;

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

			SPtr<GpuDevice> mGpuDevice;
			SceneInfo mInfo;
			SPtr<GpuBuffer> mPerFrameParamBuffer;
			UnorderedMap<SamplerOverrideKey, MaterialSamplerOverrides*> mSamplerOverrides;
			UniformBufferPools mUniformBufferPools;

			SPtr<RenderBeastOptions> mOptions;
		};

		B3D_UNIFORM_BUFFER_BEGIN(PerFrameParamDef)
			B3D_UNIFORM_BUFFER_MEMBER(float, gTime)
		B3D_UNIFORM_BUFFER_END

		extern PerFrameParamDef gPerFrameParamDef;

		/** Basic shader that is used when no other is available. */
		class DefaultMaterial : public RendererMaterial<DefaultMaterial>
		{
			RMAT_DEF("Default.bsl");
		};

		/** @} */
	} // namespace render
} // namespace b3d
