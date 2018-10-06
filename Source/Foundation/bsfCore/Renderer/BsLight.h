//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsIReflectable.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "Image/BsColor.h"
#include "Math/BsSphere.h"
#include "CoreThread/BsCoreObject.h"
#include "Scene/BsSceneActor.h"

namespace bs
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** Light type that determines how is light information parsed by the renderer and other systems. */
	enum BS_SCRIPT_EXPORT(m:Rendering) class LightType
	{
		Directional, 
		Radial, 
		Spot,

		Count			BS_SCRIPT_EXPORT(ex:true) // Keep at end
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class for both sim and core thread Light implementations. */
	class BS_CORE_EXPORT LightBase : public SceneActor
	{
	public:
		LightBase();
		LightBase(LightType type, Color color, float intensity, float attRadius, float srcRadius,
			bool castsShadows, Degree spotAngle, Degree spotFalloffAngle);

		virtual ~LightBase() { }

		/**	Determines the type of the light. */
		LightType getType() const { return mType; }

		/** @copydoc getType() */
		void setType(LightType type) { mType = type; _markCoreDirty(); updateBounds(); }

		/**	Determines does this light cast shadows when rendered. */
		void setCastsShadow(bool castsShadow) { mCastsShadows = castsShadow; _markCoreDirty(); }

		/** @copydoc setCastsShadow */
		bool getCastsShadow() const { return mCastsShadows; }

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
		void setShadowBias(float bias) { mShadowBias = bias; _markCoreDirty(); }

		/** @copydoc setShadowBias() */
		float getShadowBias() const { return mShadowBias; }

		/** Determines the color emitted by the light. */
		void setColor(const Color& color) { mColor = color; _markCoreDirty(); }

		/** @copydoc setColor() */
		Color getColor() const { return mColor; }

		/**
		 * Range at which the light contribution fades out to zero. Use setUseAutoAttenuation to provide a radius
		 * automatically dependant on light intensity. The radius will cut-off light contribution and therefore manually set
		 * very small radius can end up being very physically incorrect.
		 */
		void setAttenuationRadius(float radius);

		/**	@copydoc setAttenuationRadius */
		float getAttenuationRadius() const { return mAttRadius; }

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
		void setSourceRadius(float radius);

		/**	@copydoc setSourceRadius */
		float getSourceRadius() const { return mSourceRadius; }

		/** 
		 * If enabled the attenuation radius will automatically be controlled in order to provide reasonable light radius, 
		 * depending on its intensity.
		 */
		void setUseAutoAttenuation(bool enabled);

		/** @copydoc setUseAutoAttenuation */
		bool getUseAutoAttenuation() const { return mAutoAttenuation; }

		/**
		 * Determines the power of the light source. This will be luminous flux for radial & spot lights, 
		 * luminance for directional lights with no area, and illuminance for directional lights with area (non-zero source
		 * radius).
		 */
		void setIntensity(float intensity);

		/** @copydoc setIntensity */
		float getIntensity() const { return mIntensity; }

		/**	Determines the total angle covered by a spot light. */
		void setSpotAngle(const Degree& spotAngle) { mSpotAngle = spotAngle; _markCoreDirty(); updateBounds(); }

		/** @copydoc setSpotAngle */
		Degree getSpotAngle() const { return mSpotAngle; }

		/**
		 * Determines the falloff angle covered by a spot light. Falloff angle determines at what point does light intensity 
		 * starts quadratically falling off as the angle approaches the total spot angle.
		 */
		void setSpotFalloffAngle(const Degree& spotFallofAngle) 
		{ mSpotFalloffAngle = spotFallofAngle; _markCoreDirty(); updateBounds(); }

		/** @copydoc setSpotFalloffAngle */
		Degree getSpotFalloffAngle() const { return mSpotFalloffAngle; }

		/**	Returns world space bounds that completely encompass the light's area of influence. */
		Sphere getBounds() const { return mBounds; }

		/**
		 * Returns the luminance of the light source. This is the value that should be used in lighting equations.
		 *
		 * @note
		 * For point light sources this method returns luminous intensity and not luminance. We use the same method for both
		 * as a convenience since in either case its used as a measure of intensity in lighting equations.
		 */
		float getLuminance() const;

		/** Enumerates all the fields in the type and executes the specified processor action for each field. */
		template<class P>
		void rttiEnumFields(P p);

	protected:
		/** Updates the internal bounds for the light. Call this whenever a property affecting the bounds changes. */
		void updateBounds();

		/** Calculates maximum light range based on light intensity. */
		void updateAttenuationRange();

		/** @copydoc SceneActor::setTransform */
		void setTransform(const Transform& transform) override;

		LightType mType; /**< Type of light that determines how are the rest of the parameters interpreted. */
		bool mCastsShadows; /**< Determines whether the light casts shadows. */
		Color mColor; /**< Color of the light. */
		float mAttRadius; /**< Radius at which light intensity falls off to zero. */
		float mSourceRadius; /**< Radius of the light source. If > 0 the light is an area light. */
		float mIntensity; /**< Power of the light source. @see setIntensity. */
		Degree mSpotAngle; /**< Total angle covered by a spot light. */
		Degree mSpotFalloffAngle; /**< Spot light angle at which falloff starts. Must be smaller than total angle. */
		Sphere mBounds; /**< Sphere that bounds the light area of influence. */
		bool mAutoAttenuation; /**< Determines is attenuation radius is automatically determined. */
		float mShadowBias; /**< See setShadowBias() */
	};

	/** @} */
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct { class Light; }

	/** Illuminates a portion of the scene covered by the light. */
	class BS_CORE_EXPORT Light : public IReflectable, public CoreObject, public LightBase
	{
	public:
		/**	Retrieves an implementation of the light usable only from the core thread. */
		SPtr<ct::Light> getCore() const;

		/**
		 * Creates a new light with provided settings.
		 *
		 * @param[in]	type				Type of light that determines how are the rest of the parameters interpreted.
		 * @param[in]	color				Color of the light.
		 * @param[in]	intensity			Power of the light source. This will be luminous flux for radial & spot lights, 
		 *									luminance for directional lights with no area, and illuminance for directional 
		 *									lights with area (non-zero source radius).
		 * @param[in]	attRadius			Radius at which light's influence fades out to zero.
		 * @param[in]	castsShadows		Determines whether the light casts shadows.
		 * @param[in]	spotAngle			Total angle covered by a spot light.
		 * @param[in]	spotFalloffAngle	Spot light angle at which falloff starts. Must be smaller than total angle.
		 */
		static SPtr<Light> create(LightType type = LightType::Radial, Color color = Color::White,
			float intensity = 100.0f, float attRadius = 10.0f, bool castsShadows = false,
			Degree spotAngle = Degree(45), Degree spotFalloffAngle = Degree(40));

	protected:
		Light(LightType type, Color color, float intensity, float attRadius, float srcRadius, 
			bool castsShadows, Degree spotAngle, Degree spotFalloffAngle);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/** @copydoc LightBase::_markCoreDirty */
		void _markCoreDirty(ActorDirtyFlag flag = ActorDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/**	Creates a light with without initializing it. Used for serialization. */
		static SPtr<Light> createEmpty();

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class LightRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		Light() = default; // Serialization only
	};

	namespace ct
	{
	/** Core thread usable version of bs::Light. */
	class BS_CORE_EXPORT Light : public CoreObject, public LightBase
	{
	public:
		~Light();

		/**	Sets an ID that can be used for uniquely identifying this object by the renderer. */
		void setRendererId(UINT32 id) { mRendererId = id; }

		/**	Retrieves an ID that can be used for uniquely identifying this object by the renderer. */
		UINT32 getRendererId() const { return mRendererId; }

		static const UINT32 LIGHT_CONE_NUM_SIDES;
		static const UINT32 LIGHT_CONE_NUM_SLICES;
	protected:
		friend class bs::Light;

		Light(LightType type, Color color, float intensity, float attRadius, float srcRadius, bool castsShadows, 
			  Degree spotAngle, Degree spotFalloffAngle);

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc CoreObject::syncToCore */
		void syncToCore(const CoreSyncData& data) override;

		UINT32 mRendererId;
	};
	}

	/** @} */
}
