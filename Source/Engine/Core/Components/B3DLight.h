//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "CoreObject/B3DCoreObject.h"
#include "Image/B3DColor.h"
#include "Scene/B3DTransform.h"

namespace b3d
{
	/** @addtogroup Renderer-Internal
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

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class for both main and render thread Light implementations. */
	template<bool IsRenderProxy>
	class B3D_EXPORT TLight : public CoreVariantType<CoreObject, IsRenderProxy>
	{
		using Super = CoreVariantType<CoreObject, IsRenderProxy>;
	public:
		TLight();
		TLight(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle);
		virtual ~TLight() = default;

		/**	Determines the type of the light. */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Setter))
		void SetType(LightType type)
		{
			mType = type;
			MarkRenderProxyDataDirty();
			UpdateBounds();
		}

		/** @copydoc SetType() */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Getter))
		LightType GetType() const { return mType; }

		/**	Determines does this light cast shadows when rendered. */
		B3D_SCRIPT_EXPORT(ExportName(CastsShadow), Property(Setter))
		void SetCastsShadow(bool castsShadow)
		{
			mCastsShadows = castsShadow;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetCastsShadow */
		B3D_SCRIPT_EXPORT(ExportName(CastsShadow), Property(Getter))
		bool GetCastsShadow() const { return mCastsShadows; }

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
		B3D_SCRIPT_EXPORT(ExportName(ShadowBias), Property(Setter), UIValueRange([ -1, 1 ]), UI(AsSlider))
		void SetShadowBias(float bias)
		{
			mShadowBias = bias;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetShadowBias() */
		B3D_SCRIPT_EXPORT(ExportName(ShadowBias), Property(Getter))
		float GetShadowBias() const { return mShadowBias; }

		/** Determines the color emitted by the light. */
		B3D_SCRIPT_EXPORT(ExportName(Color), Property(Setter))
		void SetColor(const Color& color)
		{
			mColor = color;
			MarkRenderProxyDataDirty();
		}

		/** @copydoc SetColor() */
		B3D_SCRIPT_EXPORT(ExportName(Color), Property(Getter))
		Color GetColor() const { return mColor; }

		/**
		 * Range at which the light contribution fades out to zero. Use SetUseAutoAttenuation to provide a radius
		 * automatically dependant on light intensity. The radius will cut-off light contribution and therefore manually set
		 * very small radius can end up being very physically incorrect.
		 */
		B3D_SCRIPT_EXPORT(ExportName(AttenuationRadius), Property(Setter))
		void SetAttenuationRadius(float radius);

		/**	@copydoc SetAttenuationRadius */
		B3D_SCRIPT_EXPORT(ExportName(AttenuationRadius), Property(Getter))
		float GetAttenuationRadius() const { return mAttRadius; }

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
		B3D_SCRIPT_EXPORT(ExportName(SourceRadius), Property(Setter))
		void SetSourceRadius(float radius);

		/**	@copydoc SetSourceRadius */
		B3D_SCRIPT_EXPORT(ExportName(SourceRadius), Property(Getter))
		float GetSourceRadius() const { return mSourceRadius; }

		/**
		 * If enabled the attenuation radius will automatically be controlled in order to provide reasonable light radius,
		 * depending on its intensity.
		 */
		B3D_SCRIPT_EXPORT(ExportName(UseAutoAttenuation), Property(Setter))
		void SetUseAutoAttenuation(bool enabled);

		/** @copydoc SetUseAutoAttenuation */
		B3D_SCRIPT_EXPORT(ExportName(UseAutoAttenuation), Property(Getter))
		bool GetUseAutoAttenuation() const { return mAutoAttenuation; }

		/**
		 * Determines the power of the light source. This will be luminous flux for radial & spot lights,
		 * luminance for directional lights with no area, and illuminance for directional lights with area (non-zero source
		 * radius).
		 */
		B3D_SCRIPT_EXPORT(ExportName(Intensity), Property(Setter))
		void SetIntensity(float intensity);

		/** @copydoc SetIntensity */
		B3D_SCRIPT_EXPORT(ExportName(Intensity), Property(Getter))
		float GetIntensity() const { return mIntensity; }

		/**	Determines the total angle covered by a spot light. */
		B3D_SCRIPT_EXPORT(ExportName(SpotAngle), Property(Setter), UIValueRange([ 1, 180 ]), UI(AsSlider))
		void SetSpotAngle(const Degree& spotAngle)
		{
			mSpotAngle = spotAngle;
			MarkRenderProxyDataDirty();
			UpdateBounds();
		}

		/** @copydoc SetSpotAngle */
		B3D_SCRIPT_EXPORT(ExportName(SpotAngle), Property(Getter))
		Degree GetSpotAngle() const { return mSpotAngle; }

		/**
		 * Determines the falloff angle covered by a spot light. Falloff angle determines at what point does light intensity
		 * starts quadratically falling off as the angle approaches the total spot angle.
		 */
		B3D_SCRIPT_EXPORT(ExportName(SpotAngleFalloff), Property(Setter), UIValueRange([ 1, 180 ]), UI(AsSlider))
		void SetSpotFalloffAngle(const Degree& spotFallofAngle)
		{
			mSpotFalloffAngle = spotFallofAngle;
			MarkRenderProxyDataDirty();
			UpdateBounds();
		}

		/** @copydoc SetSpotFalloffAngle */
		B3D_SCRIPT_EXPORT(ExportName(SpotAngleFalloff), Property(Getter))
		Degree GetSpotFalloffAngle() const { return mSpotFalloffAngle; }

		/**	Returns world space bounds that completely encompass the light's area of influence. */
		B3D_SCRIPT_EXPORT(ExportName(Bounds), Property(Getter))
		Sphere GetBounds() const { return mBounds; }

		/**
		 * Returns the luminance of the light source. This is the value that should be used in lighting equations.
		 *
		 * @note
		 * For point light sources this method returns luminous intensity and not luminance. We use the same method for both
		 * as a convenience since in either case its used as a measure of intensity in lighting equations.
		 */
		float GetLuminance() const;

	protected:
		/** Updates the internal bounds for the light. Call this whenever a property affecting the bounds changes. */
		void UpdateBounds();

		/** Calculates maximum light range based on light intensity. */
		void UpdateAttenuationRange();

		/** @copydoc CoreObject::MarkRenderProxyDataDirty */
		void MarkRenderProxyDataDirty(ComponentDirtyFlag flag = ComponentDirtyFlag::Everything);

		/** Returns the world space transform of the object. */
		const Transform& GetTransform() const;

		LightType mType = LightType::Radial; /**< Type of light that determines how are the rest of the parameters interpreted. */
		bool mCastsShadows = false; /**< Determines whether the light casts shadows. */
		Color mColor = Color::kWhite; /**< Color of the light. */
		float mAttRadius = 10.0f; /**< Radius at which light intensity falls off to zero. */
		float mSourceRadius = 0.0f; /**< Radius of the light source. If > 0 the light is an area light. */
		float mIntensity = 100.0f; /**< Power of the light source. @see SetIntensity. */
		Degree mSpotAngle { 45 }; /**< Total angle covered by a spot light. */
		Degree mSpotFalloffAngle { 35.0f }; /**< Spot light angle at which falloff starts. Must be smaller than total angle. */
		Sphere mBounds; /**< Sphere that bounds the light area of influence. */
		bool mAutoAttenuation = false; /**< Determines is attenuation radius is automatically determined. */
		float mShadowBias = 0.5f; /**< See SetShadowBias() */
	};

	/** @} */

	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	Light
	 *
	 * @note	Wraps Light as a Component.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Light : public Component, public TLight<false>
	{
	public:
		Light(const HSceneObject& parent);

	protected:
		friend class render::Light;
		struct FullSyncPacket;
		struct TransformSyncPacket;

		SPtr<render::RenderProxy> CreateRenderProxy() const override;
		RenderProxySyncPacket* CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags) override;

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
	};

	/** @} */

	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace render
	{
		/** Render thread counterpart of b3d::Light. */
		class B3D_EXPORT Light : public TLight<true>
		{
		public:
			~Light();

			/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
			void SetRendererId(u32 id) { mRendererId = id; }

			/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
			u32 GetRendererId() const { return mRendererId; }

			/** Returns the world space transform for the decal. */
			const Transform& GetWorldTransform() const { return mTransform; }

			static const u32 kLightConeNumSides;
			static const u32 kLightConeNumSlices;

		protected:
			friend class b3d::Light;

			Light(const SPtr<SceneInstance>& scene, LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, Degree spotAngle, Degree spotFalloffAngle);

			void Initialize() override;
			void SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator) override;

			u32 mRendererId;
			Transform mTransform;
			bool mActive = true;
			SPtr<SceneInstance> mSceneInstance;
		};
	} // namespace render

	/** @} */
} // namespace b3d
