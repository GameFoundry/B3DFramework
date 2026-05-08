//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "RenderState/B3DLightRenderState.h"
#include "RenderState/B3DDecalRenderState.h"
#include "RenderState/B3DReflectionProbeRenderState.h"
#include "Components/B3DDecal.h"
#include "Components/B3DParticleSystem.h"
#include "Components/B3DReflectionProbe.h"
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
		struct FrameInfo;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		// Limited by max number of array elements in texture for DX11 hardware
		constexpr u32 kMaxReflectionCubemaps = 2048 / 6;

		/**
		 * Maintains packed arrays containing renderer-specific data for renderables. Allows fast iteration over
		 * all renderables in a scene and provides data needed for their rendering and culling.
		 */
		class RenderableObjectStorage final : public RenderableObjectStorageBase
		{
		public:
			RenderableObjectStorage();

			void ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator) override;
			void CreateRenderState(TArrayView<const PackedRendererId> ids) override;
			void DestroyRenderState(TArrayView<const PackedRendererId> ids) override;
			void UpdateRenderState(TArrayView<const PackedRendererId> ids) override;

			/** Returns renderable at the provided index. Valid index is range [0, GetRenderableCount()). */
			RenderableRenderState* GetRenderable(u32 index) const { return mRenderables[index]; }

			/** Returns renderable cull info at the provided index. Valid index is range [0, GetRenderableCount()). */
			const CullInfo& GetRenderableCullInfo(u32 index) const { return mRenderableCullInfos[index]; }

			/**
			 * Performs necessary per-frame updates to a renderable. This must be called once every frame for every renderable.
			 *
			 * @param	id			Packed id of the renderable to prepare.
			 * @param	frameInfo	Global information describing the current frame.
			 */
			void PrepareRenderable(PackedRendererId id, const FrameInfo& frameInfo);

			/**
			 * Performs necessary steps to make a renderable ready for rendering. This must be called at least once every frame
			 * for every renderable that will be drawn. Multiple calls for the same renderable during a single frame will result
			 * in a no-op.
			 *
			 * @param	id			Packed id of the renderable to prepare.
			 * @param	frameInfo	Global information describing the current frame.
			 */
			void PrepareVisibleRenderable(PackedRendererId id, const FrameInfo& frameInfo);

			/** Returns the packed renderable array. */
			TChunkedArray<RenderableRenderState*>& GetRenderables() { return mRenderables; }

			/** @copydoc GetRenderables */
			const TChunkedArray<RenderableRenderState*>& GetRenderables() const { return mRenderables; }

			/** Returns the packed cull info array. */
			TChunkedArray<CullInfo>& GetRenderableCullInfos() { return mRenderableCullInfos; }

			/** @copydoc GetRenderableCullInfos */
			const TChunkedArray<CullInfo>& GetRenderableCullInfos() const { return mRenderableCullInfos; }

			/** Sets the owning RenderBeastScene. Called by RenderBeastScene::Initialize(). */
			void SetScene(RenderBeastScene& scene) { mRenderBeastScene = &scene; }

		private:
			TChunkedArray<RenderableRenderState*> mRenderables;
			TChunkedArray<CullInfo> mRenderableCullInfos;

			RenderBeastScene* mRenderBeastScene = nullptr;
		};

		/**
		 * Maintains packed arrays containing renderer-specific data for lights. Allows fast iteration over
		 * all lights in a scene, provides iteration over different light types, and provides data needed for
		 * their rendering and culling.
		 */
		class LightObjectStorage final : public LightObjectStorageBase
		{
		public:
			LightObjectStorage();

			void ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator) override;
			void CreateRenderState(TArrayView<const PackedRendererId> ids) override;
			void DestroyRenderState(TArrayView<const PackedRendererId> ids) override;
			void UpdateRenderState(TArrayView<const PackedRendererId> ids) override;

			/** Returns packed IDs of all directional lights in the scene. */
			const Vector<PackedRendererId>& GetDirectionalLights() const { return mDirectionalLightIds; }

			/** Returns packed IDs of all radial lights in the scene. */
			const Vector<PackedRendererId>& GetRadialLights() const { return mRadialLightIds; }

			/** Returns packed IDs of all spot lights in the scene. */
			const Vector<PackedRendererId>& GetSpotLights() const { return mSpotLightIds; }

			/** Returns an array holding the world space bounds of all radial lights. */
			const Vector<Sphere>& GetRadialLightWorldBounds() const { return mRadialLightWorldBounds; }

			/** Returns an array holding the world space bounds of all spot lights. */
			const Vector<Sphere>& GetSpotLightWorldBounds() const { return mSpotLightWorldBounds; }

			/** Returns the render state for the light at the given packed ID. */
			const LightRenderState& GetLightRenderState(PackedRendererId packedId) const { return mLightRenderStates[packedId]; }

		private:
			Vector<Sphere> mRadialLightWorldBounds;
			Vector<Sphere> mSpotLightWorldBounds;
			Vector<PackedRendererId> mDirectionalLightIds;
			Vector<PackedRendererId> mRadialLightIds;
			Vector<PackedRendererId> mSpotLightIds;
			TChunkedArray<LightRenderState> mLightRenderStates;
		};

		/**
		 * Maintains packed arrays containing renderer-specific data for decals. Allows fast iteration over
		 * all decals in a scene and provides data needed for their rendering and culling.
		 */
		class DecalObjectStorage final : public DecalObjectStorageBase
		{
		public:
			DecalObjectStorage();

			void ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator) override;
			void CreateRenderState(TArrayView<const PackedRendererId> ids) override;
			void DestroyRenderState(TArrayView<const PackedRendererId> ids) override;
			void UpdateRenderState(TArrayView<const PackedRendererId> ids) override;

			/** Returns the decal render state at the given packed ID. */
			DecalRenderState& GetDecalRenderState(PackedRendererId packedId) { return mDecals[packedId]; }

			/** Returns the decal render state at the given packed ID (const). */
			const DecalRenderState& GetDecalRenderState(PackedRendererId packedId) const { return mDecals[packedId]; }

			/** Returns the packed decal render state array. */
			TChunkedArray<DecalRenderState>& GetDecals() { return mDecals; }

			/** @copydoc GetDecals */
			const TChunkedArray<DecalRenderState>& GetDecals() const { return mDecals; }

			/** Returns the packed cull info array. */
			TChunkedArray<CullInfo>& GetDecalCullInfos() { return mDecalCullInfos; }

			/** @copydoc GetDecalCullInfos */
			const TChunkedArray<CullInfo>& GetDecalCullInfos() const { return mDecalCullInfos; }

			/** Sets the owning RenderBeastScene. Called by RenderBeastScene::Initialize(). */
			void SetScene(RenderBeastScene& scene) { mRenderBeastScene = &scene; }

		private:
			TChunkedArray<DecalRenderState> mDecals;
			TChunkedArray<CullInfo> mDecalCullInfos;

			RenderBeastScene* mRenderBeastScene = nullptr;
		};

		/**
		 * Maintains packed arrays containing renderer-specific data for particle systems. Allows fast iteration over
		 * all particle systems in a scene and provides data needed for their rendering and culling.
		 */
		class ParticleSystemObjectStorage final : public ParticleSystemObjectStorageBase
		{
		public:
			ParticleSystemObjectStorage();

			void ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator) override;
			void CreateRenderState(TArrayView<const PackedRendererId> ids) override;
			void DestroyRenderState(TArrayView<const PackedRendererId> ids) override;
			void UpdateRenderState(TArrayView<const PackedRendererId> ids) override;

			/** Returns the particle render state at the given packed ID. */
			ParticleRenderState& GetParticleRenderState(PackedRendererId packedId) { return mParticleRenderStates[packedId]; }

			/** Returns the particle render state at the given packed ID (const). */
			const ParticleRenderState& GetParticleRenderState(PackedRendererId packedId) const { return mParticleRenderStates[packedId]; }

			/** Returns the packed particle render state array. */
			TChunkedArray<ParticleRenderState>& GetParticleRenderStates() { return mParticleRenderStates; }

			/** @copydoc GetParticleRenderStates */
			const TChunkedArray<ParticleRenderState>& GetParticleRenderStates() const { return mParticleRenderStates; }

			/** Returns particle system cull info at the provided index. Valid index is range [0, GetParticleSystemCount()). */
			const CullInfo& GetParticleSystemCullInfo(PackedRendererId packedId) const { return mParticleSystemCullInfos[packedId]; }

			/** Returns the packed cull info array. */
			TChunkedArray<CullInfo>& GetParticleSystemCullInfos() { return mParticleSystemCullInfos; }

			/** @copydoc GetParticleSystemCullInfos */
			const TChunkedArray<CullInfo>& GetParticleSystemCullInfos() const { return mParticleSystemCullInfos; }

			/** Returns packed IDs of all GPU-simulated particle systems. */
			const Vector<PackedRendererId>& GetGpuSimulatedIds() const { return mGpuSimulatedIds; }

			/** Sets the owning RenderBeastScene. Called by RenderBeastScene::Initialize(). */
			void SetScene(RenderBeastScene& scene) { mRenderBeastScene = &scene; }

		private:
			TChunkedArray<ParticleRenderState> mParticleRenderStates;
			TChunkedArray<CullInfo> mParticleSystemCullInfos;
			Vector<PackedRendererId> mGpuSimulatedIds;

			RenderBeastScene* mRenderBeastScene = nullptr;
		};
		/**
		 * Maintains packed arrays containing renderer-specific data for reflection probes. Allows fast iteration over
		 * all reflection probes in a scene, manages cubemap array allocation, and provides data needed for their rendering.
		 */
		class ReflectionProbeObjectStorage final : public ReflectionProbeObjectStorageBase
		{
		public:
			ReflectionProbeObjectStorage();

			void ApplyCommands(const CommandBatch& commands, FrameAllocator& allocator) override;
			void CreateRenderState(TArrayView<const PackedRendererId> slotIds) override;
			void DestroyRenderState(TArrayView<const PackedRendererId> slotIds) override;
			void UpdateRenderState(TArrayView<const PackedRendererId> slotIds) override;
			void OnFilteredTextureUpdated(PackedRendererId slotId) override;

			/** Returns the reflection probe render states. */
			const TChunkedArray<ReflectionProbeRenderState>& GetReflectionProbeRenderStates() const { return mReflectionProbeRenderStates; }

			/** Returns the render state at the given packed ID. */
			const ReflectionProbeRenderState& GetReflectionProbeRenderState(PackedRendererId packedId) const { return mReflectionProbeRenderStates[packedId]; }

			/** Returns the world space bounds of all reflection probes. */
			const TChunkedArray<Sphere>& GetReflProbeWorldBounds() const { return mReflProbeWorldBounds; }

			/** Returns the cubemap array texture containing all reflection probe cubemaps. */
			const TShared<Texture>& GetReflProbeCubemapsTex() const { return mReflProbeCubemapsTex; }

			/** Returns the boolean array tracking which cubemap array slots are in use. */
			const Vector<bool>& GetReflProbeCubemapArrayUsedSlots() const { return mReflProbeCubemapArrayUsedSlots; }

			/** Updates the cubemap array index for the given probe and optionally marks it as clean. */
			void SetReflectionProbeArrayIndex(PackedRendererId packedId, u32 arrayIdx, bool markAsClean);

			/** Updates the cubemap array texture with changed probe textures. */
			void UpdateReflectionProbes(GpuCommandBuffer& commandBuffer);

			/** Sets the GPU device used for texture creation. Called by RenderBeastScene::Initialize(). */
			void SetGpuDevice(const TShared<GpuDevice>& gpuDevice) { mGpuDevice = gpuDevice; }

		private:
			TChunkedArray<ReflectionProbeRenderState> mReflectionProbeRenderStates;
			TChunkedArray<Sphere> mReflProbeWorldBounds;
			Vector<bool> mReflProbeCubemapArrayUsedSlots;
			TShared<Texture> mReflProbeCubemapsTex;
			TShared<GpuDevice> mGpuDevice;
		};

		/** Contains information about the scene (e.g. renderables, lights, cameras) required by the renderer. */
		class RenderBeastScene : public RendererScene
		{
		public:
			explicit RenderBeastScene(const TShared<RenderBeastOptions>& options);

			void RegisterCamera(Camera* camera) override;
			void UpdateCamera(Camera* camera, u32 updateFlag) override;
			void UnregisterCamera(Camera* camera) override;

			void RegisterLightProbeVolume(LightProbeVolume* volume) override;
			void UpdateLightProbeVolume(LightProbeVolume* volume) override;
			void UnregisterLightProbeVolume(LightProbeVolume* volume) override;

			void RegisterSkybox(Skybox* skybox) override;
			void UnregisterSkybox(Skybox* skybox) override;

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

			/** Updates scene according to the newly provided renderer options. */
			void SetOptions(const TShared<RenderBeastOptions>& options);

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

			/** Returns the object for managing uniform buffer allocations. */
			UniformBufferPools& GetUniformBufferPools() { return mUniformBufferPools; }

			/** Returns the renderable object storage. */
			RenderableObjectStorage& GetRenderableStorage() { return static_cast<RenderableObjectStorage&>(*mRenderableStorage.get()); }
			const RenderableObjectStorage& GetRenderableStorage() const { return static_cast<const RenderableObjectStorage&>(*mRenderableStorage.get()); }

			/** Returns the light object storage. */
			LightObjectStorage& GetLightStorage() { return static_cast<LightObjectStorage&>(*mLightStorage.get()); }
			const LightObjectStorage& GetLightStorage() const { return static_cast<const LightObjectStorage&>(*mLightStorage.get()); }

			/** Returns the decal object storage. */
			DecalObjectStorage& GetDecalStorage() { return static_cast<DecalObjectStorage&>(*mDecalStorage.get()); }
			const DecalObjectStorage& GetDecalStorage() const { return static_cast<const DecalObjectStorage&>(*mDecalStorage.get()); }

			/** Returns the particle system object storage. */
			ParticleSystemObjectStorage& GetParticleSystemStorage() { return static_cast<ParticleSystemObjectStorage&>(*mParticleSystemStorage.get()); }
			const ParticleSystemObjectStorage& GetParticleSystemStorage() const { return static_cast<const ParticleSystemObjectStorage&>(*mParticleSystemStorage.get()); }

			/** Returns the reflection probe object storage. */
			ReflectionProbeObjectStorage& GetReflectionProbeStorage() { return static_cast<ReflectionProbeObjectStorage&>(*mReflectionProbeStorage.get()); }
			const ReflectionProbeObjectStorage& GetReflectionProbeStorage() const { return static_cast<const ReflectionProbeObjectStorage&>(*mReflectionProbeStorage.get()); }

			/**
			 * @name Renderable accessors — convenience wrappers around RenderableObjectStorage
			 * @{
			 */
			u32 GetRenderableCount() const { return GetRenderableStorage().GetRenderableCount(); }
			RenderableRenderState* GetRenderable(u32 index) const { return GetRenderableStorage().GetRenderable(index); }
			const CullInfo& GetRenderableCullInfo(u32 index) const { return GetRenderableStorage().GetRenderableCullInfo(index); }
			const TChunkedArray<RenderableRenderState*>& GetRenderables() const { return GetRenderableStorage().GetRenderables(); }
			const TChunkedArray<CullInfo>& GetRenderableCullInfos() const { return GetRenderableStorage().GetRenderableCullInfos(); }
			/** @} */

			/**
			 * @name Light accessors — convenience wrappers around LightObjectStorage
			 * @{
			 */
			TArrayView<const PackedRendererId> GetDirectionalLights() const { return GetLightStorage().GetDirectionalLights(); }
			TArrayView<const PackedRendererId> GetRadialLights() const { return GetLightStorage().GetRadialLights(); }
			TArrayView<const PackedRendererId> GetSpotLights() const { return GetLightStorage().GetSpotLights(); }
			TArrayView<const Sphere> GetRadialLightWorldBounds() const { return GetLightStorage().GetRadialLightWorldBounds(); }
			TArrayView<const Sphere> GetSpotLightWorldBounds() const { return GetLightStorage().GetSpotLightWorldBounds(); }
			const LightProxy& GetLightProxy(PackedRendererId packedId) const { return GetLightStorage().GetLightProxy(packedId); }
			const LightRenderState& GetLightRenderState(PackedRendererId packedId) const { return GetLightStorage().GetLightRenderState(packedId); }
			/** @} */

			/**
			 * @name Decal accessors — convenience wrappers around DecalObjectStorage
			 * @{
			 */
			u32 GetDecalCount() const { return (u32)GetDecalStorage().GetDecals().size(); }
			const DecalRenderState& GetDecalRenderState(u32 index) const { return GetDecalStorage().GetDecals()[index]; }
			const CullInfo& GetDecalCullInfo(u32 index) const { return GetDecalStorage().GetDecalCullInfos()[index]; }
			const TChunkedArray<DecalRenderState>& GetDecals() const { return GetDecalStorage().GetDecals(); }
			const TChunkedArray<CullInfo>& GetDecalCullInfos() const { return GetDecalStorage().GetDecalCullInfos(); }
			/** @} */

			/**
			 * @name ParticleSystem accessors — convenience wrappers around ParticleSystemObjectStorage
			 * @{
			 */
			u32 GetParticleSystemCount() const { return GetParticleSystemStorage().GetParticleSystemCount(); }
			const render::ParticleSystemProxy& GetParticleSystemProxy(PackedRendererId packedId) const { return GetParticleSystemStorage().GetParticleSystemProxy(packedId); }
			const ParticleRenderState& GetParticleRenderState(u32 index) const { return GetParticleSystemStorage().GetParticleRenderState(index); }
			const CullInfo& GetParticleSystemCullInfo(u32 index) const { return GetParticleSystemStorage().GetParticleSystemCullInfo(index); }
			const TChunkedArray<ParticleRenderState>& GetParticleRenderStates() const { return GetParticleSystemStorage().GetParticleRenderStates(); }
			const TChunkedArray<CullInfo>& GetParticleSystemCullInfos() const { return GetParticleSystemStorage().GetParticleSystemCullInfos(); }
			/** @} */

			/**
			 * @name ReflectionProbe accessors — convenience wrappers around ReflectionProbeObjectStorage
			 * @{
			 */
			u32 GetReflectionProbeCount() const { return GetReflectionProbeStorage().GetReflectionProbeCount(); }
			const render::ReflectionProbeProxy& GetReflectionProbeProxy(PackedRendererId packedId) const { return GetReflectionProbeStorage().GetReflectionProbeProxy(packedId); }
			const ReflectionProbeRenderState& GetReflectionProbeRenderState(PackedRendererId packedId) const { return GetReflectionProbeStorage().GetReflectionProbeRenderState(packedId); }
			const TChunkedArray<ReflectionProbeRenderState>& GetReflectionProbes() const { return GetReflectionProbeStorage().GetReflectionProbeRenderStates(); }
			const TChunkedArray<Sphere>& GetReflectionProbeWorldBounds() const { return GetReflectionProbeStorage().GetReflProbeWorldBounds(); }
			const TShared<Texture>& GetReflectionProbeCubemapsTex() const { return GetReflectionProbeStorage().GetReflProbeCubemapsTex(); }
			/** @} */

			/**
			 * Generates sampler state overrides for the provided render element, or returns existing ones if they
			 * already exist for the element's material. Shared between renderables and decals.
			 */
			MaterialSamplerOverrides* AllocSamplerStateOverrides(DrawCommand& drawCommand);

			/** Releases sampler state overrides for the provided render element. */
			void FreeSamplerStateOverrides(DrawCommand& drawCommand);

			/**
			 * Checks all sampler overrides in case material sampler states changed, and updates them.
			 *
			 * @param[in]	force	If true, all sampler overrides will be updated, regardless of a change in the material
			 *						was detected or not.
			 */
			void RefreshSamplerOverrides(bool force = false);

			/** Resets renderable ready flags for a new frame. */
			void ResetRenderableReady();

			// Cameras and render targets
			const Vector<RendererRenderTarget>& GetRenderTargets() const { return mRenderTargets; }

			/** Returns the view associated with the given camera, or nullptr if the camera has no registered view. */
			RendererView* TryGetView(const Camera* camera) const
			{
				auto it = mCameraToView.find(camera);
				if(it != mCameraToView.end())
					return mViews[it->second];

				return nullptr;
			}

			// Light probes (indirect lighting)
			const LightProbes& GetLightProbes() const { return mLightProbes; }

			// Sky
			Skybox* GetSkybox() const { return mSkybox; }

		private:
			friend class RenderableObjectStorage;
			friend class LightObjectStorage;
			friend class DecalObjectStorage;
			friend class ParticleSystemObjectStorage;

			/** Creates a renderer view descriptor for the particular camera. */
			RendererViewCreateInformation CreateViewDesc(Camera* camera) const;

			/**
			 * Find the render target the camera belongs to and adds it to the relevant list. If the camera was previously
			 * registered with some other render target it will be removed from it and added to the new target.
			 */
			void UpdateCameraRenderTargets(Camera* camera, bool remove = false);

			TShared<GpuDevice> mGpuDevice;
			GpuBufferSuballocation mPerFrameSuballocation;
			UniformBufferPools mUniformBufferPools;
			TShared<RenderBeastOptions> mOptions;
			UnorderedMap<SamplerOverrideKey, MaterialSamplerOverrides*> mSamplerOverrides;

			// Cameras and render targets
			Vector<RendererRenderTarget> mRenderTargets;
			Vector<RendererView*> mViews;
			UnorderedMap<const Camera*, u32> mCameraToView;

			// Light probes (indirect lighting)
			LightProbes mLightProbes;

			// Sky
			Skybox* mSkybox = nullptr;

			// Transient per-frame data
			mutable Vector<bool> mRenderableReady;
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
