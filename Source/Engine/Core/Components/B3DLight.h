//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "CoreObject/B3DCoreObject.h"
#include "Image/B3DColor.h"
#include "Math/B3DTransform.h"
#include "Renderer/B3DRendererId.h"
#include "Renderer/B3DRendererObjectECSUtility.h"
#include "Renderer/B3DRendererObjectStorage.h"

namespace b3d
{
	class LightObjectStorageBase;
	struct LightFullUpdateChannel;
	struct LightTransformUpdateChannel;

	/** @addtogroup Rendering
	 *  @{
	 */

	/** Light type that determines how is light information parsed by the renderer and other systems. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) LightType
	{
		Directional,
		Radial,
		Spot,

		Count B3D_SCRIPT_EXPORT(Exclude(true)) // Keep at end
	};

	/** @} */

	/** @addtogroup Rendering-Internal
	 *  @{
	 */

	/** Common data used by both main and render thread variants of Light. */
	template <bool IsRenderProxy>
	struct B3D_EXPORT TLightData
	{
		LightType Type = LightType::Radial; /**< Type of light that determines how are the rest of the parameters interpreted. */
		bool CastsShadows = false; /**< Determines whether the light casts shadows. */
		Color LightColor = Color::kWhite; /**< Color of the light. */
		float AttRadius = 10.0f; /**< Radius at which light intensity falls off to zero. */
		float SourceRadius = 0.0f; /**< Radius of the light source. If > 0 the light is an area light. */
		float Intensity = 100.0f; /**< Power of the light source. @see Light::SetIntensity. */
		Degree SpotAngle { 45 }; /**< Total angle covered by a spot light. */
		Degree SpotFalloffAngle { 35.0f }; /**< Spot light angle at which falloff starts. Must be smaller than total angle. */
		bool AutoAttenuation = false; /**< Determines is attenuation radius is automatically determined. */
		Sphere Bounds; /**< Sphere that bounds the light area of influence. */
		float ShadowBias = 0.5f; /**< See Light::SetShadowBias() */

		/** Computes the world space bounding sphere for a light and updates the Bounds field. */
		void ComputeBounds(const Transform& transform);

		/**
		 * Returns the luminance of the light source. This is the value that should be used in lighting equations.
		 *
		 * @note
		 * For point light sources this returns luminous intensity and not luminance. We use the same method for both
		 * as a convenience since in either case its used as a measure of intensity in lighting equations.
		 */
		float ComputeLuminance() const;

		/** Calculates maximum light range based on light intensity and updates AttRadius and Bounds. */
		void ComputeAttenuationRange(const Transform& transform);
	};

	/** CRTP getter interface providing shared read access for light data to both Light and render::LightProxy. */
	template <typename Derived, bool IsRenderProxy>
	class TLightGetters
	{
	public:
		/**	Determines the type of the light. */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Getter))
		LightType GetType() const { return GetData().Type; }

		/** Determines does this light cast shadows when rendered. */
		B3D_SCRIPT_EXPORT(ExportName(CastsShadow), Property(Getter))
		bool GetCastsShadow() const { return GetData().CastsShadows; }

		/**
		 * Shadow bias determines offset at which the shadows are rendered from the shadow caster. Bias value of 0 means
		 * the shadow will be renderered exactly at the casters position. If your geometry has thin areas this will
		 * produce an artifact called shadow acne, in which case you can increase the shadow bias value to eliminate it.
		 * Note that increasing the shadow bias will on the other hand make the shadow be offset from the caster and may
		 * make the caster appear as if floating (Peter Panning artifact). Neither is perfect, so it is preferable to ensure
		 * all your geometry has thickness and keep the bias at zero, or even at negative values.
		 *
		 * Default value is 0.5. Should be in rough range [-1, 1].
		 */
		B3D_SCRIPT_EXPORT(ExportName(ShadowBias), Property(Getter))
		float GetShadowBias() const { return GetData().ShadowBias; }

		/** Determines the color emitted by the light. */
		B3D_SCRIPT_EXPORT(ExportName(Color), Property(Getter))
		Color GetColor() const { return GetData().LightColor; }

		/**
		 * Range at which the light contribution fades out to zero. Use SetUseAutoAttenuation to provide a radius
		 * automatically dependant on light intensity. The radius will cut-off light contribution and therefore manually set
		 * very small radius can end up being very physically incorrect.
		 */
		B3D_SCRIPT_EXPORT(ExportName(AttenuationRadius), Property(Getter))
		float GetAttenuationRadius() const { return GetData().AttRadius; }

		/**
		 * Radius of the light source. If non-zero then this light represents an area light, otherwise it is a punctual
		 * light. Area lights have different attenuation then punctual lights, and their appearance in specular reflections
		 * is realistic. Shape of the area light depends on light type:
		 *  - For directional light the shape is a disc projected on the hemisphere on the sky. This parameter
		 *    represents angular radius (in degrees) of the disk and should be very small (think of how much space the Sun
		 *    takes on the sky - roughly 0.25 degree radius).
		 *  - For radial light the shape is a sphere and the source radius is the radius of the sphere.
		 *  - For spot lights the shape is a disc oriented in the direction of the spot light and the source radius is the
		 *    radius of the disc.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SourceRadius), Property(Getter))
		float GetSourceRadius() const { return GetData().SourceRadius; }

		/**
		 * If enabled the attenuation radius will automatically be controlled in order to provide reasonable light radius,
		 * depending on its intensity.
		 */
		B3D_SCRIPT_EXPORT(ExportName(UseAutoAttenuation), Property(Getter))
		bool GetUseAutoAttenuation() const { return GetData().AutoAttenuation; }

		/**
		 * Determines the power of the light source. This will be luminous flux for radial & spot lights,
		 * luminance for directional lights with no area, and illuminance for directional lights with area (non-zero source
		 * radius).
		 */
		B3D_SCRIPT_EXPORT(ExportName(Intensity), Property(Getter))
		float GetIntensity() const { return GetData().Intensity; }

		/**	Determines the total angle covered by a spot light. */
		B3D_SCRIPT_EXPORT(ExportName(SpotAngle), Property(Getter), UIValueRange([ 1, 180 ]), UI(AsSlider))
		Degree GetSpotAngle() const { return GetData().SpotAngle; }

		/**
		 * Determines the falloff angle covered by a spot light. Falloff angle determines at what point does light intensity
		 * starts quadratically falling off as the angle approaches the total spot angle.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SpotAngleFalloff), Property(Getter), UIValueRange([ 1, 180 ]), UI(AsSlider))
		Degree GetSpotFalloffAngle() const { return GetData().SpotFalloffAngle; }

		/**	Returns world space bounds that completely encompass the light's area of influence. */
		B3D_SCRIPT_EXPORT(ExportName(Bounds), Property(Getter))
		Sphere GetBounds() const { return GetData().Bounds; }

	private:
		const TLightData<IsRenderProxy>& GetData() const
		{
			return static_cast<const Derived*>(this)->GetLightData();
		}
	};

	class RendererScene;

	namespace ecs
	{
		class ECSLightRTTI;

		/** ECS fragment storing light visual data (type, color, intensity, bounds, etc.). */
		struct B3D_EXPORT Light : TLightData<false>, IReflectable
		{
			struct FullSyncPacket;
			struct TransformSyncPacket;

			friend class ECSLightRTTI;
			static RTTIType* GetRttiStatic();
			RTTIType* GetRtti() const override;
		};

		/** ECS fragment storing the persistent render object ID for a light. */
		struct LightId
		{
			RendererId Id = kInvalidRendererId;
		};

		/** Tag indicating a Light needs to sync all of its properties to its render proxy. */
		struct LightDirty {};

		/** Tag indicating a Light needs to sync transform to its render proxy. */
		struct LightTransformDirty {};

		/** Creates all Light data fragments, a world transform, and allocates a renderer ID. Entity is ready for rendering after this call. Returns the Light data fragment. */
		Light& CreateLight(Registry& registry, Entity entity, const SPtr<RendererScene>& rendererScene, const Transform& transform = Transform::kIdentity);

		/** Removes all Light fragments. Cleanup (ID deallocation, dirty tags) is handled by the associated RendererScene when it is notified the fragment has been removed. */
		void DestroyLight(Registry& registry, Entity entity);

		/** ECS utility for Light renderer objects. Provides fragment creation, renderer registration, dirty marking, and scene migration. */
		using LightECSUtility = TRendererObjectECSUtility<
			LightId,
			LightDirty, LightTransformDirty,
			&RendererScene::AllocateLightId, &RendererScene::DeallocateLightId,
			Light>;
	}

	/** @} */

	/** @addtogroup Rendering
	 *  @{
	 */

	/**
	 * @copydoc	Light
	 *
	 * @note	Wraps Light as a Component.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Light : public Component, public TLightGetters<Light, false>, public CoreObject
	{
		friend class TLightGetters<Light, false>;
	public:
		Light(const HSceneObject& parent);

		/** @copydoc TLightGetters::GetType */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Setter))
		void SetType(LightType type);

		/** @copydoc TLightGetters::GetCastsShadow */
		B3D_SCRIPT_EXPORT(ExportName(CastsShadow), Property(Setter))
		void SetCastsShadow(bool castsShadow);

		/** @copydoc TLightGetters::GetShadowBias */
		B3D_SCRIPT_EXPORT(ExportName(ShadowBias), Property(Setter), UIValueRange([ -1, 1 ]), UI(AsSlider))
		void SetShadowBias(float bias);

		/** @copydoc TLightGetters::GetColor */
		B3D_SCRIPT_EXPORT(ExportName(Color), Property(Setter))
		void SetColor(const Color& color);

		/** @copydoc TLightGetters::GetAttenuationRadius */
		B3D_SCRIPT_EXPORT(ExportName(AttenuationRadius), Property(Setter))
		void SetAttenuationRadius(float radius);

		/** @copydoc TLightGetters::GetSourceRadius */
		B3D_SCRIPT_EXPORT(ExportName(SourceRadius), Property(Setter))
		void SetSourceRadius(float radius);

		/** @copydoc TLightGetters::GetUseAutoAttenuation */
		B3D_SCRIPT_EXPORT(ExportName(UseAutoAttenuation), Property(Setter))
		void SetUseAutoAttenuation(bool enabled);

		/** @copydoc TLightGetters::GetIntensity */
		B3D_SCRIPT_EXPORT(ExportName(Intensity), Property(Setter))
		void SetIntensity(float intensity);

		/** @copydoc TLightGetters::GetSpotAngle */
		B3D_SCRIPT_EXPORT(ExportName(SpotAngle), Property(Setter), UIValueRange([ 1, 180 ]), UI(AsSlider))
		void SetSpotAngle(const Degree& spotAngle);

		/** @copydoc TLightGetters::GetSpotFalloffAngle */
		B3D_SCRIPT_EXPORT(ExportName(SpotAngleFalloff), Property(Setter), UIValueRange([ 1, 180 ]), UI(AsSlider))
		void SetSpotFalloffAngle(const Degree& spotFallofAngle);

		/** @copydoc TLightData::ComputeLuminance */
		float GetLuminance() const;

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

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class LightRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Light(); // Serialization only

	private:
		/** @copydoc CoreObject::MarkRenderProxyDataDirty */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		/** Returns a reference to the light data for the CRTP getter interface. */
		const TLightData<false>& GetLightData() const;

		/** Returns a mutable reference to the ECS light fragment. */
		ecs::Light& GetFragment();

		/** Returns a const reference to the ECS light fragment. */
		const ecs::Light& GetFragment() const;

		/** Updates the internal bounds for the light. Call this whenever a property affecting the bounds changes. */
		void UpdateBounds();

		/** Calculates maximum light range based on light intensity. */
		void UpdateAttenuationRange();
	};

	/** @} */

	/** @addtogroup Renderer
	 *  @{
	 */

	namespace render
	{
		/** Render-thread representation of a light, stored in packed arrays in LightObjectStorageBase. */
		class B3D_EXPORT LightProxy : public TLightGetters<LightProxy, true>
		{
			friend class TLightGetters<LightProxy, true>;
			friend struct b3d::LightFullUpdateChannel;
			friend struct b3d::LightTransformUpdateChannel;

		public:
			/** Returns the world space transform for the light. */
			const Transform& GetWorldTransform() const { return mTransform; }

			/** Returns world space bounds that encompass the light's area of influence. */
			Sphere GetBounds() const { return mBounds; }

			/** Returns the luminance of the light source. */
			float GetLuminance() const { return mData.ComputeLuminance(); }

			/** Sets the renderer-assigned packed slot ID. */
			void SetRendererId(PackedRendererId id) { mRendererId = id; }

			/** Returns the renderer-assigned packed slot ID. */
			PackedRendererId GetRendererId() const { return mRendererId; }

		protected:
			const TLightData<true>& GetLightData() const { return mData; }

			TLightData<true> mData;
			Transform mTransform;
			Sphere mBounds;
			PackedRendererId mRendererId = kInvalidPackedRendererId;
		};
	} // namespace render

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/**
	 * Contains render thread representation of light objects, stored in packed arrays accessible by PackedRendererId.
	 * Implements main -> render thread synchronization logic for light objects.
	 */
	class B3D_EXPORT LightObjectStorageBase : public RendererObjectStorage
	{
	public:
		void* SyncRead(ecs::Registry& registry, FrameAllocator& allocator) override;
		void SyncWrite(void* batchData, FrameAllocator& allocator) override;

		/** Returns the light proxy at the given slot. */
		render::LightProxy& GetLightProxy(PackedRendererId slotId) { return mLightProxies[slotId]; }

		/** Returns the light proxy at the given slot (const). */
		const render::LightProxy& GetLightProxy(PackedRendererId slotId) const { return mLightProxies[slotId]; }

		/** Returns the total number of lights. */
		u32 GetLightCount() const { return (u32)mLightProxies.size(); }

		/**
		 * Called once per frame for each new light being added, or a light whose data needs to be rebuilt
		 * after a significant update (in which case DestroyRenderState will be called first).
		 */
		virtual void CreateRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/**
		 * Called once per frame for each light that is being removed, or a light that needs to be rebuilt
		 * after a significant update (in which case CreateRenderState will be called right after).
		 */
		virtual void DestroyRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

		/**
		 * Called once per frame for each light that needs to be updated after a minor update (e.g. one that doesn't
		 * require a full render state rebuild, such as a transform change).
		 */
		virtual void UpdateRenderState(TArrayView<const PackedRendererId> slotIds) = 0;

	protected:
		TChunkedArray<render::LightProxy> mLightProxies;
	};

	/** @} */
} // namespace b3d
