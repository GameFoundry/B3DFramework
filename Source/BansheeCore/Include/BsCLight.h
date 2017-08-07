//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsLight.h"
#include "BsComponent.h"

namespace bs 
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	Light
	 *
	 * Wraps Light as a Component.
	 */
	class BS_CORE_EXPORT CLight : public Component
	{
	public:
		CLight(const HSceneObject& parent, LightType type = LightType::Radial, Color color = Color::White, 
			float intensity = 100.0f, float range = 1.0f, bool castsShadows = false, Degree spotAngle = Degree(45), 
			Degree spotFalloffAngle = Degree(40));

		virtual ~CLight();

		/** @copydoc Light::getType */
		LightType getType() const { return mInternal->getType(); }

		/** @copydoc Light::setType */
		void setType(LightType type) { mInternal->setType(type); }

		/** @copydoc Light::getCastsShadow */
		bool getCastsShadow() const { return mInternal->getCastsShadow(); }

		/** @copydoc Light::setCastsShadow */
		void setCastsShadow(bool castsShadow) { mInternal->setCastsShadow(castsShadow); }

		/** @copydoc Light::setShadowBias */
		void setShadowBias(float bias) { mInternal->setShadowBias(bias); }

		/** @copydoc Light::setShadowBias() */
		float getShadowBias() const { return mInternal->getShadowBias(); }

		/** @copydoc Light::getColor */
		Color getColor() const { return mInternal->getColor(); }

		/** @copydoc Light::setColor */
		void setColor(const Color& color) { mInternal->setColor(color); }

		/** @copydoc Light::getAttenuationRadius */
		float getAttenuationRadius() const { return mInternal->getAttenuationRadius(); }

		/** @copydoc Light::setAttenuationRadius */
		void setAttenuationRadius(float radius) { mInternal->setAttenuationRadius(radius); }

		/** @copydoc Light::getSourceRadius */
		float getSourceRadius() const { return mInternal->getSourceRadius(); }

		/** @copydoc Light::setSourceRadius */
		void setSourceRadius(float radius) { mInternal->setSourceRadius(radius); }

		/** @copydoc Light::getIntensity */
		float getIntensity() const { return mInternal->getIntensity(); }

		/** @copydoc Light::setIntensity */
		void setIntensity(float intensity) { mInternal->setIntensity(intensity); }

		/** @copydoc Light::getSpotAngle */
		Degree getSpotAngle() const { return mInternal->getSpotAngle(); }

		/** @copydoc Light::setSpotAngle */
		void setSpotAngle(const Degree& spotAngle) { mInternal->setSpotAngle(spotAngle); }

		/** @copydoc Light::getSpotFalloffAngle */
		Degree getSpotFalloffAngle() const { return mInternal->getSpotFalloffAngle(); }

		/** @copydoc Light::setSpotFalloffAngle */
		void setSpotFalloffAngle(const Degree& spotAngle) { mInternal->setSpotFalloffAngle(spotAngle); }

		/** @copydoc Light::getUseAutoAttenuation  */
		bool getUseAutoAttenuation() const { return mInternal->getUseAutoAttenuation(); }

		/**  @copydoc Light::setUseAutoAttenuation  */
		void setUseAutoAttenuation(bool enabled) { mInternal->setUseAutoAttenuation(enabled); }

		/** @copydoc Light::getBounds  */
		Sphere getBounds() const;

		/** @name Internal 
		 *  @{
		 */

		 /** Returns the light that this component wraps. */
		SPtr<Light> _getLight() const { return mInternal; }

		/** @} */

	protected:
		mutable SPtr<Light> mInternal;

		// Only valid during construction
		LightType mType;
		Color mColor;
		float mIntensity; 
		float mRange; 
		bool mCastsShadows; 
		Degree mSpotAngle; 
		Degree mSpotFalloffAngle;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

	    /** @copydoc Component::onInitialized */
		void onInitialized() override;

	    /** @copydoc Component::onDestroyed */
		void onDestroyed() override;

		/** @copydoc Component::update */
		void update() override { }

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CLightRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CLight(); // Serialization only
	 };

	 /** @} */
}