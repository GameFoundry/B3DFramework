//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIReflectable.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsColor.h"
#include "BsSphere.h"
#include "BsCoreObject.h"

namespace bs
{
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	/** Light type that determines how is light information parsed by the renderer and other systems. */
	enum class LightType
	{
		Directional, 
		Radial, 
		Spot,

		Count // Keep at end
	};

	/**	Signals which portion of a light is dirty. */
	enum class LightDirtyFlag
	{
		Transform = 0x01,
		Everything = 0x02,
		Mobility = 0x04
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Base class for both sim and core thread Light implementations. */
	class BS_CORE_EXPORT LightBase
	{
	public:
		LightBase();
		LightBase(LightType type, Color color, float intensity, float attRadius, float srcRadius,
			bool castsShadows, Degree spotAngle, Degree spotFalloffAngle);

		virtual ~LightBase() { }

		/**	Returns the position of the light, in world space. */
		Vector3 getPosition() const { return mPosition; }

		/**	Sets the position of the light, in world space. */
		void setPosition(const Vector3& position) 
			{ mPosition = position; _markCoreDirty(LightDirtyFlag::Transform); updateBounds(); }

		/**	Returns the rotation of the light, in world space. */
		Quaternion getRotation() const { return mRotation; }

		/**	Sets the rotation of the light, in world space. */
		void setRotation(const Quaternion& rotation) 
			{ mRotation = rotation; _markCoreDirty(LightDirtyFlag::Transform); updateBounds(); }

		/**	Returns the type of the light. */
		LightType getType() const { return mType; }

		/**	Changes the type of the light. */
		void setType(LightType type) { mType = type; _markCoreDirty(); updateBounds(); }

		/**	Checks does this light cast shadows when rendered. */
		bool getCastsShadow() const { return mCastsShadows; }

		/**	Sets whether this light will cast shadows when rendered. */
		void setCastsShadow(bool castsShadow) { mCastsShadows = castsShadow; _markCoreDirty(); }

		/** 
		 * Shadow bias determines shadow accuracy. Low bias values mean that shadows start closer near their caster surface
		 * but will result in more shadowing artifacts (shadow acne). Larger values reduce shadow acne but caster may appear
		 * as floating on air as nearby part of the shadow is cut off (peter paning).
		 * 
		 * Default value is 0.5. Must be in range [0, 1].
		 */
		void setShadowBias(float bias) { mShadowBias = std::max(std::min(bias, 1.0f), 0.0f); _markCoreDirty(); }

		/** @copydoc setShadowBias() */
		float getShadowBias() const { return mShadowBias; }

		/**	Returns the color emitted from the light. */
		Color getColor() const { return mColor; }

		/**	Sets the color emitted from the light. */
		void setColor(const Color& color) { mColor = color; _markCoreDirty(); }

		/**	@see setAttenuationRadius */
		float getAttenuationRadius() const { return mAttRadius; }

		/**
		 * Range at which the light contribution fades out to zero. Use setUseAutoAttenuation to provide a radius
		 * automatically dependant on light intensity. The radius will cut-off light contribution and therefore manually set
		 * very small radius can end up being very physically incorrect.
		 */
		void setAttenuationRadius(float radius);

		/**	@see setSourceRadius */
		float getSourceRadius() const { return mSourceRadius; }

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

		/** @see setUseAutoAttenuation */
		bool getUseAutoAttenuation() const { return mAutoAttenuation; }

		/** 
		 * If enabled the attenuation radius will automatically be controlled in order to provide reasonable light radius, 
		 * depending on its intensity.
		 */
		void setUseAutoAttenuation(bool enabled);

		/** @see setIntensity */
		float getIntensity() const { return mIntensity; }

		/**
		 * Determines the power of the light source. This will be luminous flux for radial & spot lights, 
		 * luminance for directional lights with no area, and illuminance for directional lights with area (non-zero source
		 * radius).
		 */
		void setIntensity(float intensity);

		/**	Gets the total angle covered by a spot light. */
		Degree getSpotAngle() const { return mSpotAngle; }

		/**	Sets the total angle covered by a spot light. */
		void setSpotAngle(const Degree& spotAngle) { mSpotAngle = spotAngle; _markCoreDirty(); updateBounds(); }

		/**
		 * Gets the falloff angle covered by a spot light. Falloff angle determines at what point does light intensity 
		 * starts quadratically falling off as the angle approaches the total spot angle.
		 */
		Degree getSpotFalloffAngle() const { return mSpotFalloffAngle; }

		/**
		 * Sets the falloff angle covered by a spot light. Falloff angle determines at what point does light intensity 
		 * starts quadratically falling off as the angle approaches the total spot angle.
		 */
		void setSpotFalloffAngle(const Degree& spotFallofAngle) 
		{ mSpotFalloffAngle = spotFallofAngle; _markCoreDirty(); updateBounds(); }

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

		/**	Checks whether the light should be rendered or not. */
		bool getIsActive() const { return mIsActive; }

		/**	Sets whether the light should be rendered or not. */
		void setIsActive(bool active) { mIsActive = active; _markCoreDirty(); }

		/**
		 * Sets the mobility of a scene object. This is used primarily as a performance hint to engine systems. Objects
		 * with more restricted mobility will result in higher performance. Some mobility constraints will be enforced by
		 * the engine itself, while for others the caller must be sure not to break the promise he made when mobility was
		 * set. By default scene object's mobility is unrestricted.
		 */
		void setMobility(ObjectMobility mobility);

		/** 
		 * Gets the mobility setting for this scene object. See setMobility(); 
		 */
		ObjectMobility getMobility() const { return mMobility; }

		/** 
		 * Marks the simulation thread object as dirty and notifies the system its data should be synced with its core 
		 * thread counterpart. 
		 */
		virtual void _markCoreDirty(LightDirtyFlag flag = LightDirtyFlag::Everything) { }

	protected:
		/** Updates the internal bounds for the light. Call this whenever a property affecting the bounds changes. */
		void updateBounds();

		/** Calculates maximum light range based on light intensity. */
		void updateAttenuationRange();

		Vector3 mPosition; /**< World space position. */
		Quaternion mRotation; /**< World space rotation. */

		LightType mType; /**< Type of light that determines how are the rest of the parameters interpreted. */
		bool mCastsShadows; /**< Determines whether the light casts shadows. */
		Color mColor; /**< Color of the light. */
		float mAttRadius; /**< Radius at which light intensity falls off to zero. */
		float mSourceRadius; /**< Radius of the light source. If > 0 the light is an area light. */
		float mIntensity; /**< Power of the light source. @see setIntensity. */
		Degree mSpotAngle; /**< Total angle covered by a spot light. */
		Degree mSpotFalloffAngle; /**< Spot light angle at which falloff starts. Must be smaller than total angle. */
		bool mIsActive; /**< Whether the light should be rendered or not. */
		Sphere mBounds; /**< Sphere that bounds the light area of influence. */
		bool mAutoAttenuation; /**< Determines is attenuation radius is automatically determined. */
		ObjectMobility mMobility; /**< Determines if there are any restrictions placed on light movement. */
		float mShadowBias; /**< See setShadowBias() */
	};

	/** @} */
	/** @addtogroup Renderer-Internal
	 *  @{
	 */

	namespace ct { class Light; }

	/** Illuminates a portion of the scene covered by a light. */
	class BS_CORE_EXPORT Light : public IReflectable, public CoreObject, public LightBase
	{
	public:
		/**	Retrieves an implementation of the light usable only from the core thread. */
		SPtr<ct::Light> getCore() const;

		/** Returns the hash value that can be used to identify if the internal data needs an update. */
		UINT32 _getLastModifiedHash() const { return mLastUpdateHash; }

		/**	Sets the hash value that can be used to identify if the internal data needs an update. */
		void _setLastModifiedHash(UINT32 hash) { mLastUpdateHash = hash; }

		/**
		 * Updates internal transform values from the specified scene object, in case that scene object's transform changed
		 * since the last call.
		 *
		 * @note	Assumes the same scene object will be provided every time.
		 */
		void _updateTransform(const HSceneObject& parent);

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
		void _markCoreDirty(LightDirtyFlag flag = LightDirtyFlag::Everything) override;

		/** @copydoc CoreObject::syncToCore */
		CoreSyncData syncToCore(FrameAlloc* allocator) override;

		/**	Creates a light with without initializing it. Used for serialization. */
		static SPtr<Light> createEmpty();

		UINT32 mLastUpdateHash;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class LightRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		Light(); // Serialization only
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