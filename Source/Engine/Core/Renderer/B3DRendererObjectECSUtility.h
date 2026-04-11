//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "ECS/B3DRegistry.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DComponent.h"
#include "Scene/B3DSceneInstance.h"

namespace b3d
{
	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * Provides generic ECS utility methods for renderer object types. Handles the common pattern of
	 * creating data fragments, registering/unregistering with the renderer, managing dirty tags, and migrating
	 * fragments across scenes.
	 *
	 * Fragment removal cleanup (ID deallocation, dirty tag removal) is handled automatically by OnWillRemove events
	 * subscribed in RendererScene. This template only needs to handle creation, registration, and migration.
	 *
	 * @tparam IdFragment			The ID fragment type (e.g., ecs::LightId).
	 * @tparam DirtyTag				The full-dirty tag type (e.g., ecs::LightDirty).
	 * @tparam TransformDirtyTag	The transform-dirty tag type (e.g., ecs::LightTransformDirty).
	 * @tparam AllocateFn			Pointer-to-member for RendererScene::Allocate*Id.
	 * @tparam DeallocateFn			Pointer-to-member for RendererScene::Deallocate*Id.
	 * @tparam DataFragments		The ECS data fragment types managed by this renderer object.
	 */
	template
	<
		typename IdFragment,
		typename DirtyTag,
		typename TransformDirtyTag,
		RendererId(RendererScene::*AllocateFn)(ecs::Registry&, ecs::Entity),
		void(RendererScene::*DeallocateFn)(ecs::Registry&, ecs::Entity),
		typename... DataFragments
	>
	struct TRendererObjectECSUtility
	{
		static_assert(sizeof...(DataFragments) > 0, "At least one data fragment type is required");

		/** Ensures all data fragments exist on the entity. Returns true if any fragment was newly created. */
		static bool CreateFragmentsIfMissing(ecs::Registry& registry, ecs::Entity entity)
		{
			return (AddIfMissing<DataFragments>(registry, entity) | ...);
		}

		/**
		 * Allocates a renderer ID and marks the entity dirty. This ensures a render proxy for the entity is created
		 * on the render thread, and the entity can be rendered. Use Mark*Dirty() methods when changing data fragment properties
		 * to ensure render proxy has up-to-date information.
		 */
		static void RegisterWithRenderer(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene)
		{
			(rendererScene.get()->*AllocateFn)(registry, entity);
			MarkDirty(registry, entity);
		}

		/**
		 * Deallocates the renderer ID and removes dirty tags. This deallocated the render proxy for the entity on the render thread,
		 * and the entity will no longer be rendered. Does not remove data fragments — the entity stays but is deactivated.
		 */
		static void UnregisterFromRenderer(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene)
		{
			(rendererScene.get()->*DeallocateFn)(registry, entity);
			registry.RemoveComponents<DirtyTag>(entity);
			registry.RemoveComponents<TransformDirtyTag>(entity);
		}

		/**
		 * Removes all data fragments from the entity. Cleanup (ID deallocation, dirty tag removal) is handled
		 * by the associated RendererScene when it is notified the fragment has been removed.
		 */
		static void RemoveFragments(ecs::Registry& registry, ecs::Entity entity)
		{
			registry.RemoveComponents<DataFragments...>(entity);
		}

		/**
		 * Handles fragment migration when the entity moves between scenes. Migrates data fragments
		 * from old entity to new (removal from old triggers OnWillRemove → auto-cleanup on old scene).
		 * Optionally registers in new renderer scene if @p registerWithRenderer is true (e.g. can be false if entity exists, but is currently not active/disabled).
		 */
		static void ChangeScene(SceneInstance* oldScene, ecs::Entity oldEntity, SceneInstance& newScene, ecs::Entity newEntity, bool registerWithRenderer)
		{
			ecs::Registry& newRegistry = newScene.GetECSRegistry();

			if(oldScene != nullptr)
			{
				ecs::Registry& oldRegistry = oldScene->GetECSRegistry();
				(MigrateSingle<DataFragments>(oldRegistry, oldEntity, newRegistry, newEntity), ...);
			}

			if(registerWithRenderer)
			{
				const SPtr<RendererScene>& rendererScene = newScene.GetRendererScene();
				(rendererScene.get()->*AllocateFn)(newRegistry, newEntity);
				MarkDirty(newRegistry, newEntity);
			}
		}

		/** Marks the entity as fully dirty, requiring a full sync to the render thread. */
		static void MarkDirty(ecs::Registry& registry, ecs::Entity entity)
		{
			registry.AddTag<DirtyTag>(entity);
		}

		/** Marks only the transform as dirty. No-op if the entity is already fully dirty. */
		static void MarkTransformDirty(ecs::Registry& registry, ecs::Entity entity)
		{
			if(!registry.HasAllOf<DirtyTag>(entity))
				registry.AddTag<TransformDirtyTag>(entity);
		}

		/**
		 * Adds the appropriate dirty tag based on the flag type. Transform-only changes use TransformDirtyTag
		 * (unless already full-dirty). Everything else uses DirtyTag.
		 */
		static void MarkDirty(ecs::Registry& registry, ecs::Entity entity, ComponentDirtyFlag flag)
		{
			if(flag == ComponentDirtyFlag::Transform)
				MarkTransformDirty(registry, entity);
			else
				MarkDirty(registry, entity);
		}

	private:
		template<typename T>
		static bool AddIfMissing(ecs::Registry& registry, ecs::Entity entity)
		{
			if(!registry.HasAllOf<T>(entity))
			{
				T data;
				registry.AddComponent<T>(entity, std::move(data));
				return true;
			}

			return false;
		}

		template<typename T>
		static void MigrateSingle(ecs::Registry& oldRegistry, ecs::Entity oldEntity, ecs::Registry& newRegistry, ecs::Entity newEntity)
		{
			if(oldRegistry.HasAllOf<T>(oldEntity))
			{
				T copy = oldRegistry.GetComponents<T>(oldEntity);
				newRegistry.AddComponent<T>(newEntity, std::move(copy));
				oldRegistry.RemoveComponents<T>(oldEntity);
			}
		}
	};

	/** @} */
} // namespace b3d
