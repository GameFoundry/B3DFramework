//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "RenderState/B3DLightRenderState.h"
#include "B3DRendererView.h"
#include "RenderState/B3DParticleRenderState.h"
#include "Renderer/B3DRendererScene.h"
#include "Renderer/B3DRendererObjectStorage.h"
#include "Shading/B3DLightProbes.h"
#include "Utility/B3DSamplerOverrides.h"
#include "Utility/B3DUniformBufferPools.h"

namespace b3d
{
	struct EvaluatedAnimationData;
	struct EvaluatedParticleData;

	class RenderableObjectStorageBase;

	namespace render
	{
		class RenderableObjectStorage;
		struct DecalRenderState;
		class Decal;
		struct FrameInfo;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		// Limited by max number of array elements in texture for DX11 hardware
		constexpr u32 kMaxReflectionCubemaps = 2048 / 6;

		/**
		 * Concrete renderable object storage for RenderBeast. Owns the packed renderable and cull-info arrays,
		 * and implements register/unregister/update/slot-replay operations.
		 */
		class RenderableObjectStorage final : public RenderableObjectStorageBase
		{
		public:
			RenderableObjectStorage();

			void ProcessCommands(TArrayView<const RendererIdCommand> deallocations, TArrayView<const RendererIdCommand> allocations) override;
			void CreateRenderState(TArrayView<const PackedRendererId> slotIds) override;
			void DestroyRenderState(TArrayView<const PackedRendererId> slotIds) override;
			void UpdateRenderState(TArrayView<const PackedRendererId> slotIds) override;

			/** Returns renderable at the provided index. Valid index is range [0, GetRenderableCount()). */
			RenderableRenderState* GetRenderable(u32 index) const { return mRenderables[index]; }

			/** Returns renderable cull info at the provided index. Valid index is range [0, GetRenderableCount()). */
			const CullInfo& GetRenderableCullInfo(u32 index) const { return mRenderableCullInfos[index]; }

			/**
			 * Performs necessary per-frame updates to a renderable. This must be called once every frame for every renderable.
			 *
			 * @param	id			Slot ID of the renderable to prepare.
			 * @param	frameInfo	Global information describing the current frame.
			 */
			void PrepareRenderable(PackedRendererId id, const FrameInfo& frameInfo);

			/**
			 * Performs necessary steps to make a renderable ready for rendering. This must be called at least once every frame
			 * for every renderable that will be drawn. Multiple calls for the same renderable during a single frame will result
			 * in a no-op.
			 *
			 * @param	id			Slot ID of the renderable to prepare.
			 * @param	frameInfo	Global information describing the current frame.
			 */
			void PrepareVisibleRenderable(PackedRendererId id, const FrameInfo& frameInfo);

			/** Returns the packed renderable array. */
			Vector<RenderableRenderState*>& GetRenderables() { return mRenderables; }

			/** @copydoc GetRenderables */
			const Vector<RenderableRenderState*>& GetRenderables() const { return mRenderables; }

			/** Returns the packed cull info array. */
			Vector<CullInfo>& GetRenderableCullInfos() { return mRenderableCullInfos; }

			/** @copydoc GetRenderableCullInfos */
			const Vector<CullInfo>& GetRenderableCullInfos() const { return mRenderableCullInfos; }

			/** Sets the owning RenderBeastScene. Called by RenderBeastScene::Initialize(). */
			void SetScene(RenderBeastScene& scene) { mRenderBeastScene = &scene; }

		private:
			Vector<RenderableRenderState*> mRenderables;
			Vector<CullInfo> mRenderableCullInfos;

			RenderBeastScene* mRenderBeastScene = nullptr;
		};

		/** Contains most scene objects relevant to the renderer. */
		struct SceneInfo
		{
			// Cameras and render targets
			Vector<RendererRenderTarget> RenderTargets;
			Vector<RendererView*> Views;
			UnorderedMap<const Camera*, u32> CameraToView;

			// Renderables — pointers to arrays in RenderableObjectStorage
			const Vector<RenderableRenderState*>* Renderables = nullptr;
			const Vector<CullInfo>* RenderableCullInfos = nullptr;

			// Lights
			Vector<LightRenderState> DirectionalLights;
			Vector<LightRenderState> RadialLights;
			Vector<LightRenderState> SpotLights;
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
			Vector<ParticleRenderState> ParticleSystems;
			Vector<CullInfo> ParticleSystemCullInfos;

			// Decals
			Vector<DecalRenderState> Decals;
			Vector<CullInfo> DecalCullInfos;

			// Sky
			Skybox* Skybox = nullptr;

			// Buffers for various transient data that gets rebuilt every frame
			//// Rebuilt every frame
			mutable Vector<bool> RenderableReady;
			// Per-frame uniform buffer suballocation (updated each frame)
			const GpuBufferSuballocation* PerFrameSuballocation = nullptr;

			// Renderable accessors — convenience wrappers around the storage-owned arrays
			u32 GetRenderableCount() const { return (u32)Renderables->size(); }
			RenderableRenderState* GetRenderable(u32 idx) const { return (*Renderables)[idx]; }
			const CullInfo& GetRenderableCullInfo(u32 idx) const { return (*RenderableCullInfos)[idx]; }
		};

		/** Contains information about the scene (e.g. renderables, lights, cameras) required by the renderer. */
		class RenderBeastScene : public RendererScene
		{
		public:
			explicit RenderBeastScene(const SPtr<RenderBeastOptions>& options);

			void RegisterCamera(Camera* camera) override;
			void UpdateCamera(Camera* camera, u32 updateFlag) override;
			void UnregisterCamera(Camera* camera) override;

			void RegisterLight(Light* light) override;
			void UpdateLight(Light* light) override;
			void UnregisterLight(Light* light) override;

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

			/** Updates global per frame parameter buffers with new values. To be called at the start of every frame. */
			void SetParamFrameParams(float time);

			/** Returns the current per-frame uniform buffer suballocation. Valid after SetParamFrameParams() is called. */
			const GpuBufferSuballocation& GetPerFrameSuballocation() const { return mPerFrameSuballocation; }

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

			/** Returns the concrete renderable object storage. */
			RenderableObjectStorage& GetRenderableStorage() { return static_cast<RenderableObjectStorage&>(*mRenderableStorage.get()); }
			const RenderableObjectStorage& GetRenderableStorage() const { return static_cast<const RenderableObjectStorage&>(*mRenderableStorage.get()); }

			/**
			 * Generates sampler state overrides for the provided render element, or returns existing ones if they
			 * already exist for the element's material. Shared between renderables and decals.
			 */
			MaterialSamplerOverrides* AllocSamplerStateOverrides(DrawCommand& elem);

			/** Releases sampler state overrides for the provided render element. */
			void FreeSamplerStateOverrides(DrawCommand& elem);

			/**
			 * Checks all sampler overrides in case material sampler states changed, and updates them.
			 *
			 * @param[in]	force	If true, all sampler overrides will be updated, regardless of a change in the material
			 *						was detected or not.
			 */
			void RefreshSamplerOverrides(bool force = false);

		private:
			friend class RenderableObjectStorage;

			/** Creates a renderer view descriptor for the particular camera. */
			RendererViewCreateInformation CreateViewDesc(Camera* camera) const;

			/**
			 * Find the render target the camera belongs to and adds it to the relevant list. If the camera was previously
			 * registered with some other render target it will be removed from it and added to the new target.
			 */
			void UpdateCameraRenderTargets(Camera* camera, bool remove = false);

			SPtr<GpuDevice> mGpuDevice;
			SceneInfo mInfo;
			GpuBufferSuballocation mPerFrameSuballocation;
			UniformBufferPools mUniformBufferPools;

			SPtr<RenderBeastOptions> mOptions;
			UnorderedMap<SamplerOverrideKey, MaterialSamplerOverrides*> mSamplerOverrides;
		};

		B3D_UNIFORM_BUFFER_BEGIN(PerFrameUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(float, gTime)
		B3D_UNIFORM_BUFFER_END

		extern PerFrameUniformDefinition gPerFrameUniformDefinition;

		/** Basic shader that is used when no other is available. */
		class DefaultMaterial : public RendererMaterial<DefaultMaterial>
		{
			RMAT_DEF("Default.bsl");
		};

		/** @} */
	} // namespace render
} // namespace b3d
