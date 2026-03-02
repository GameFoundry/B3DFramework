//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DRendererId.h"
#include "B3DRendererExtension.h"
#include "CoreObject/B3DCoreObject.h"
#include "CoreObject/B3DRenderProxy.h"
#include "ECS/B3DEntity.h"

namespace b3d
{
	class FrameAllocator;
	class RenderableObjectStorageBase;

	namespace ecs { class Registry; }
	namespace render
	{
		class RendererScene;
	}

	/** @addtogroup Renderer
	 *  @{
	 */

	/** Frame-allocated data produced by RendererScene::SyncRead, consumed by render::RendererScene::SyncWrite. */
	struct RendererSceneSyncData
	{
		void* RenderableBatchData = nullptr;
	};

	/** Contains information about the scene (e.g. renderables, lights, cameras) required by the renderer. */
	class RendererScene : public CoreObject
	{
	public:
		~RendererScene() override;

		/** Creates a new renderer scene. */
		static SPtr<RendererScene> Create();

		/** Allocates a persistent render object ID for a renderable and adds the ecs::RenderableId fragment. */
		RendererId AllocateRenderableId(ecs::Registry& registry, ecs::Entity entity);

		/** Removes the ecs::RenderableId fragment and deallocates the persistent render object ID. */
		void DeallocateRenderableId(ecs::Registry& registry, ecs::Entity entity);

		/**
		 * Reads dirty ECS data on the main thread and posts a command to write the changes to the render thread,
		 * for all RenderableObjectStorage objects.
		 */
		void SyncToRenderThread(ecs::Registry& registry, FrameAllocator& allocator);

	protected:
		void Initialize() override;
		SPtr<render::RenderProxy> CreateRenderProxy() const override;

		/**
		 * Reads dirty ECS data for all sync handlers in this scene into a frame-allocated RendererSceneSyncData.
		 * Returns nullptr if no data is dirty.
		 */
		RendererSceneSyncData* SyncRead(ecs::Registry& registry, FrameAllocator& allocator);
	private:
		SPtr<RenderableObjectStorageBase> mRenderableStorage;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Renderere
		 *  @{
		 */

		/** Contains information about the scene (e.g. renderables, lights, cameras) required by the renderer. */
		class B3D_EXPORT RendererScene : public RenderProxy
		{
		public:
			virtual ~RendererScene() = default;

			RendererScene(const RendererScene&) = delete;
			RendererScene& operator=(const RendererScene&) = delete;

		protected:
			RendererScene() = default;

		public:
			/** Returns the renderable object storage for this scene. */
			const SPtr<RenderableObjectStorageBase>& GetRenderableStorage() const { return mRenderableStorage; }

			/** Applies sync data from SyncRead to render-thread representations and frees frame-allocated memory. */
			void SyncWrite(RendererSceneSyncData& batchData, FrameAllocator& allocator);

			/** Registers a new camera in the scene. */
			virtual void RegisterCamera(Camera* camera) = 0;

			/** Updates information about a previously registered camera. */
			virtual void UpdateCamera(Camera* camera, u32 updateFlag) = 0;

			/** Removes a camera from the scene. */
			virtual void UnregisterCamera(Camera* camera) = 0;

			/** Registers a new light in the scene. */
			virtual void RegisterLight(Light* light) = 0;

			/** Updates information about a previously registered light. */
			virtual void UpdateLight(Light* light) = 0;

			/** Removes a light from the scene. */
			virtual void UnregisterLight(Light* light) = 0;

			/** Registers a new reflection probe in the scene. */
			virtual void RegisterReflectionProbe(ReflectionProbe* probe) = 0;

			/** Updates information about a previously registered reflection probe. */
			virtual void UpdateReflectionProbe(ReflectionProbe* probe, bool texture) = 0;

			/** Removes a reflection probe from the scene. */
			virtual void UnregisterReflectionProbe(ReflectionProbe* probe) = 0;

			/** Registers a new light probe volume in the scene. */
			virtual void RegisterLightProbeVolume(LightProbeVolume* volume) = 0;

			/** Updates information about a previously registered light probe volume. */
			virtual void UpdateLightProbeVolume(LightProbeVolume* volume) = 0;

			/** Removes a light probe volume from the scene. */
			virtual void UnregisterLightProbeVolume(LightProbeVolume* volume) = 0;

			/** Registers a new sky texture in the scene. */
			virtual void RegisterSkybox(Skybox* skybox) = 0;

			/** Removes a skybox from the scene. */
			virtual void UnregisterSkybox(Skybox* skybox) = 0;

			/** Registers a new particle system in the scene. */
			virtual void RegisterParticleSystem(ParticleSystem* particleSystem) = 0;

			/** Updates information about a previously registered particle system. */
			virtual void UpdateParticleSystem(ParticleSystem* particleSystem, bool tfrmOnly) = 0;

			/** Removes a particle system from the scene. */
			virtual void UnregisterParticleSystem(ParticleSystem* particleSystem) = 0;

			/** Registers a new decal object in the scene. */
			virtual void RegisterDecal(Decal* decal) = 0;

			/** Updates information about a previously registered decal object. */
			virtual void UpdateDecal(Decal* decal) = 0;

			/** Removes a decal object from the scene. */
			virtual void UnregisterDecal(Decal* decal) = 0;

			/**
			 * Registers an extension object that will be called every frame, for view in this scene. Allows external code to perform
			 * custom rendering interleaved with the renderer's output.
			 */
			void AddExtension(RendererExtension* extension) { mRendererExtensions.insert(extension); mCombinedRendererExtensionsDirty = true; }

			/** Unregisters an extension registered with AddRendererExtension(). */
			void RemoveExtension(RendererExtension* extension) { mRendererExtensions.erase(extension); mCombinedRendererExtensionsDirty = true; }

			/**
			 * Updates the combined extension list if required. Combined extension list contains extensions specific to the scene and global renderer ones. This will rebuild
			 * the internal list if the per-scene extensions have changed since the last call, or if @p forceUpdate is true. @p forceUpdate should be true if @p globalRendererExtensions
			 * has changed since the last time this method was called.
			 */
			void UpdateCombinedRendererExtensionsIfNeeded(const Set<RendererExtension*, RendererExtension::SortFunction>& globalRendererExtensions, bool forceUpdate = false);

			/**
			 * Returns a list of renderer extensions that includes both the global renderer extensions, and the per-scene extensions.
			 * Make sure to call UpdateCombinedRendererExtensionsIfNeeded() before this method, if extension list has been modified.
			 */
			const Set<RendererExtension*, RendererExtension::SortFunction>& GetCombinedRendererExtensions() const { return mCombinedRendererExtensions; }

		protected:
			SPtr<RenderableObjectStorageBase> mRenderableStorage;

			Set<RendererExtension*, RendererExtension::SortFunction> mRendererExtensions;
			Set<RendererExtension*, RendererExtension::SortFunction> mCombinedRendererExtensions; /**< Transient set of per-scene and global renderer extensions. */
			bool mCombinedRendererExtensionsDirty = true;
		};

		/** @} */
	} // namespace render
} // namespace b3d
