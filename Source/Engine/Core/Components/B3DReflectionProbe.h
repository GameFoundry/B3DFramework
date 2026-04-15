//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "CoreObject/B3DCoreObject.h"
#include "Scene/B3DComponent.h"
#include "Math/B3DTransform.h"
#include "Renderer/B3DRendererId.h"
#include "Renderer/B3DRendererObjectECSUtility.h"
#include "Renderer/B3DRendererObjectStorage.h"
#include "Scene/B3DSceneObject.h"

namespace b3d
{
	class ReflectionProbeObjectStorageBase;
	struct ReflectionProbeFullUpdateChannel;
	struct ReflectionProbeTransformUpdateChannel;

	namespace render { class RendererTask; }

	/** @addtogroup Rendering
	 *  @{
	 */

	/** Probe type that determines the shape of the probe and how is it interpreted by the renderer. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) ReflectionProbeType
	{
		/**
		 * Reflection probe cubemap is generated, and box extents are used for calculating influence ranges and box
		 * geometry.
		 */
		Box,
		/**
		 * Reflection probe cubemap is generated, but sphere is used for calculating the influence radius and
		 * proxy geometry.
		 */
		Sphere
	};

	/** @} */

	/** @addtogroup Rendering-Internal
	 *  @{
	 */

	/** Common data used by both main and render thread variants of ReflectionProbe. */
	template <bool IsRenderProxy>
	struct B3D_EXPORT TReflectionProbeData
	{
		using TextureType = CoreVariantType<Texture, IsRenderProxy>;

		ReflectionProbeType Type = ReflectionProbeType::Box; /**< Type of probe that determines how are the rest of the parameters interpreted. */
		float Radius = 1.0f; /**< Radius used for sphere reflection probes. */
		Vector3 Extents = { 1.0f, 1.0f, 1.0f }; /**< Extents used by box reflection probe. */
		float TransitionDistance = 0.1f; /**< Extra distance to used for fading out box probes. */
		Sphere Bounds = { Vector3::kZero, 1.0f }; /**< Sphere that bounds the probe area of influence. */
		SPtr<TextureType> FilteredTexture; /**< Pre-filtered texture generated from custom texture or scene capture. */

		/** Computes the world space bounding sphere for a reflection probe and updates the Bounds field. */
		void ComputeBounds(const Transform& transform);
	};

	/** CRTP getter interface providing shared read access for reflection probe data to both ReflectionProbe and render::ReflectionProbeProxy. */
	template <typename Derived, bool IsRenderProxy>
	class TReflectionProbeGetters
	{
		using TextureType = CoreVariantType<Texture, IsRenderProxy>;

	public:
		/**	Determines the type of the probe. */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Getter))
		ReflectionProbeType GetType() const { return GetData().Type; }

		/** Returns the radius of a sphere reflection probe (unscaled). */
		float GetRadius() const { return GetData().Radius; }

		/** Returns the extents of a box reflection probe (unscaled). */
		const Vector3& GetExtents() const { return GetData().Extents; }

		/**	Returns world space bounds that completely encompass the probe's area of influence. */
		Sphere GetBounds() const { return GetData().Bounds; }

		/**
		 * Determines a distance that will be used for fading out the box reflection probe with distance. By default it
		 * is equal to one, and can never be less than one. Only relevant for box probes.
		 */
		float GetTransitionDistance() const { return GetData().TransitionDistance; }

		/** Returns a pre-filtered texture that is generated either from the provided custom texture, or from scene capture. */
		const SPtr<TextureType>& GetFilteredTexture() const { return GetData().FilteredTexture; }

	private:
		const TReflectionProbeData<IsRenderProxy>& GetData() const
		{
			return static_cast<const Derived*>(this)->GetReflectionProbeData();
		}
	};

	class RendererScene;

	namespace ecs
	{
		class ECSReflectionProbeRTTI;

		/** ECS fragment storing reflection probe visual data (type, radius, extents, etc.). */
		struct B3D_EXPORT ReflectionProbe : TReflectionProbeData<false>, IReflectable
		{
			struct FullSyncPacket;
			struct TransformSyncPacket;

			HTexture CustomTexture; /**< Optional custom cubemap texture. When set, this is filtered instead of capturing the scene. */
			SPtr<render::RendererTask> PendingTask; /**< Transient (not serialized). In-flight capture/filter task, if any. */

			friend class ECSReflectionProbeRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/** ECS fragment storing the persistent render object ID for a reflection probe. */
		struct ReflectionProbeId
		{
			RendererId Id = kInvalidRendererId;
		};

		/** Tag indicating a ReflectionProbe needs to sync all of its properties to its render proxy. */
		struct ReflectionProbeDirty {};

		/** Tag indicating a ReflectionProbe needs to sync transform to its render proxy. */
		struct ReflectionProbeTransformDirty {};

		/** Creates all ReflectionProbe data fragments, a world transform, and allocates a renderer ID. Entity is ready for rendering after this call. Returns the ReflectionProbe data fragment. */
		ReflectionProbe& CreateReflectionProbe(Registry& registry, Entity entity, const SPtr<RendererScene>& rendererScene, const Transform& transform = Transform::kIdentity);

		/** Removes all ReflectionProbe fragments. Cleanup (ID deallocation, dirty tags, task cancellation) is handled by the associated RendererScene when it is notified the fragment has been removed. */
		void DestroyReflectionProbe(Registry& registry, Entity entity);

		/** ECS utility for ReflectionProbe renderer objects. Provides fragment creation, renderer registration, dirty marking, and scene migration. */
		using ReflectionProbeECSUtility = TRendererObjectECSUtility<
			ReflectionProbeId,
			ReflectionProbeDirty, ReflectionProbeTransformDirty,
			&RendererScene::AllocateReflectionProbeId, &RendererScene::DeallocateReflectionProbeId,
			ReflectionProbe>;
	}

	/** Free utility functions for managing reflection probe capture and filtering without requiring a Component. */
	class ReflectionProbeUtility
	{
	public:
		/**
		 * Captures the scene at the probe's position and generates a filtered reflection cubemap.
		 * No action if the fragment has a custom texture set. Requires ecs::ReflectionProbe,
		 * ecs::WorldTransform, and ecs::ReflectionProbeId on the entity.
		 */
		static void Capture(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene);

		/**
		 * Filters the custom texture set on the fragment. No action if no custom texture is set.
		 * Requires ecs::ReflectionProbe, ecs::WorldTransform, and ecs::ReflectionProbeId on the entity.
		 */
		static void Filter(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene);

	private:
		/**
		 * Performs the actual capture and/or filter operation, creating a RendererTask.
		 * Cancels any existing in-flight task for this probe via RendererScene::SetPendingCaptureTask.
		 */
		static void CaptureAndFilter(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene);
	};

	/** @} */

	/** @addtogroup Rendering
	 *  @{
	 */

	/**
	 * Specifies a location at which a pre-computed texture containing scene radiance will be generated. This texture will
	 * then be used by the renderer to provide specular reflections.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) ReflectionProbe : public Component, public TReflectionProbeGetters<ReflectionProbe, false>, public CoreObject
	{
		friend class TReflectionProbeGetters<ReflectionProbe, false>;
	public:
		ReflectionProbe(const HSceneObject& parent);

		/**	@copydoc TReflectionProbeGetters::GetType */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Setter))
		void SetType(ReflectionProbeType type);

		/** Determines the radius of a sphere reflection probe. */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius);

		/** Determines the extents of a box reflection probe. Determines range of influence. */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Setter))
		void SetExtents(const Vector3& extents);

		/**
		 * Determines a distance that will be used for fading out the box reflection probe with distance. By default it
		 * is equal to one, and can never be less than one. Only relevant for box probes.
		 */
		void SetTransitionDistance(float distance);

		/**
		 * Allows you assign a custom texture to use as a reflection map. This will disable automatic generation of
		 * reflections. To re-enable auto-generation call this with a null parameter.
		 */
		B3D_SCRIPT_EXPORT(ExportName(CustomTexture), Property(Setter))
		void SetCustomTexture(const HTexture& texture);

		/** @copydoc SetCustomTexture */
		B3D_SCRIPT_EXPORT(ExportName(CustomTexture), Property(Getter))
		HTexture GetCustomTexture() const { return GetFragment().CustomTexture; }

		/** Returns the radius of a sphere reflection probe, scaled by scene object transform. */
		B3D_SCRIPT_EXPORT(ExportName(WorldRadius), Property(Getter))
		float GetWorldRadius() const;

		/** Returns the extents of a box reflection probe, scaled by scene object transform. */
		B3D_SCRIPT_EXPORT(ExportName(WorldExtents), Property(Getter))
		Vector3 GetWorldExtents() const;

		/**
		 * Captures the scene at the current location and generates a filtered reflection cubemap. No action is taken
		 * if a custom texture is set.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Capture))
		void Capture();

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void Initialize() override;
		void OnCreated() override;
		void OnEnabled() override;
		void OnDisabled() override;
		void OnDestroyed() override;
		void OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity) override;
		void OnTransformChanged(TransformChangedFlags flags) override;
		void Update() override {}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ReflectionProbeRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		ReflectionProbe(); // Serialization only

	private:
		/** Marks the reflection probe as dirty, adding the appropriate ECS dirty tag. */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		/** Returns a reference to the reflection probe data for the CRTP getter interface. */
		const TReflectionProbeData<false>& GetReflectionProbeData() const;

		/** Returns a mutable reference to the ECS reflection probe fragment. */
		ecs::ReflectionProbe& GetFragment();

		/** Returns a const reference to the ECS reflection probe fragment. */
		const ecs::ReflectionProbe& GetFragment() const;

		/** Updates the internal bounds for the probe. Call this whenever a property affecting the bounds changes. */
		void UpdateBounds();
	};

	/** @} */

	/**
	 * @addtogroup Renderer-Internal
	 * @{
	 */

	namespace render
	{
		/** Render-thread representation of a reflection probe, stored in packed arrays in ReflectionProbeObjectStorageBase. */
		class B3D_EXPORT ReflectionProbeProxy : public TReflectionProbeGetters<ReflectionProbeProxy, true>
		{
			friend class TReflectionProbeGetters<ReflectionProbeProxy, true>;
			friend class b3d::ReflectionProbeObjectStorageBase;
			friend struct b3d::ReflectionProbeFullUpdateChannel;
			friend struct b3d::ReflectionProbeTransformUpdateChannel;

		public:
			/** Returns the world space transform for the probe. */
			const Transform& GetWorldTransform() const { return mTransform; }

			/** Returns the radius of a sphere reflection probe, scaled by world transform. */
			float GetWorldRadius() const
			{
				Vector3 scale = mTransform.GetScale();
				return mData.Radius * std::max(std::max(scale.X, scale.Y), scale.Z);
			}

			/** Returns the extents of a box reflection probe, scaled by world transform. */
			Vector3 GetWorldExtents() const { return mData.Extents * mTransform.GetScale(); }

			/** Sets the renderer-assigned packed slot ID. */
			void SetRendererId(PackedRendererId id) { mRendererId = id; }

			/** Returns the renderer-assigned packed slot ID. */
			PackedRendererId GetRendererId() const { return mRendererId; }

			/** Returns the raw reflection probe data (unscaled). */
			const TReflectionProbeData<true>& GetReflectionProbeData() const { return mData; }

		protected:
			TReflectionProbeData<true> mData;
			Transform mTransform;
			PackedRendererId mRendererId = kInvalidPackedRendererId;
		};
	} // namespace render

	/**
	 * Contains render thread representation of reflection probe objects, stored in packed arrays accessible by PackedRendererId.
	 * Implements main -> render thread synchronization logic for reflection probe objects.
	 */
	class B3D_EXPORT ReflectionProbeObjectStorageBase : public RendererObjectStorage
	{
	public:
		void* SyncRead(ecs::Registry& registry, FrameAllocator& allocator) override;
		void SyncWrite(void* batchData, FrameAllocator& allocator) override;

		/** Returns the reflection probe proxy at the given slot. */
		render::ReflectionProbeProxy& GetReflectionProbeProxy(PackedRendererId slotId) { return mReflectionProbeProxies[slotId]; }

		/** Returns the reflection probe proxy at the given slot (const). */
		const render::ReflectionProbeProxy& GetReflectionProbeProxy(PackedRendererId slotId) const { return mReflectionProbeProxies[slotId]; }

		/** Returns the total number of reflection probes. */
		u32 GetReflectionProbeCount() const { return (u32)mReflectionProbeProxies.size(); }

		/**
		 * Updates the filtered texture for a reflection probe identified by persistent RendererId.
		 * Resolves the ID to a packed ID (with version check for safety), updates the proxy's filtered texture,
		 * and calls OnFilteredTextureUpdated so the renderer can mark the cubemap array slot dirty.
		 */
		void UpdateFilteredTexture(RendererId probeId, const SPtr<render::Texture>& texture);

		/**
		 * Called once per frame for each new reflection probe being added, or a probe whose data needs to be rebuilt
		 * after a significant update (in which case DestroyRenderState will be called first).
		 */
		virtual void CreateRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/**
		 * Called once per frame for each reflection probe that is being removed, or a probe that needs to be rebuilt
		 * after a significant update (in which case CreateRenderState will be called right after).
		 */
		virtual void DestroyRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/**
		 * Called once per frame for each reflection probe that needs to be updated after a minor update (e.g. one that doesn't
		 * require a full render state rebuild, such as a transform change).
		 */
		virtual void UpdateRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/** Called when a reflection probe's filtered texture has been updated. Override to mark cubemap array slot dirty. */
		virtual void OnFilteredTextureUpdated(PackedRendererId slotId) {}

	protected:
		TChunkedArray<render::ReflectionProbeProxy> mReflectionProbeProxies;
	};

	/** @} */
} // namespace b3d
