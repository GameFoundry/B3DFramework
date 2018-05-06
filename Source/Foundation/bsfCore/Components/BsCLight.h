//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsLight.h"
#include "Scene/BsComponent.h"

namespace bs 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Light
	 *
	 * @note	Wraps Light as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering,n:Light) CLight : public Component
	{
	public:
		CLight(const HSceneObject& parent, LightType type = LightType::Radial, Color color = Color::White, 
			float intensity = 100.0f, float range = 1.0f, bool castsShadows = false, Degree spotAngle = Degree(45), 
			Degree spotFalloffAngle = Degree(40));

		virtual ~CLight();

		/** @copydoc Light::setType */
		BS_SCRIPT_EXPORT(n:Type,pr:setter)
		void setType(LightType type) { mInternal->setType(type); }

		/** @copydoc Light::getType */
		BS_SCRIPT_EXPORT(n:Type,pr:getter)
		LightType getType() const { return mInternal->getType(); }

		/** @copydoc Light::setCastsShadow */
		BS_SCRIPT_EXPORT(n:CastsShadow,pr:setter)
		void setCastsShadow(bool castsShadow) { mInternal->setCastsShadow(castsShadow); }

		/** @copydoc Light::getCastsShadow */
		BS_SCRIPT_EXPORT(n:CastsShadow,pr:getter)
		bool getCastsShadow() const { return mInternal->getCastsShadow(); }

		/** @copydoc Light::setShadowBias */
		BS_SCRIPT_EXPORT(n:ShadowBias,pr:setter)
		void setShadowBias(float bias) { mInternal->setShadowBias(bias); }

		/** @copydoc Light::setShadowBias() */
		BS_SCRIPT_EXPORT(n:ShadowBias,pr:getter)
		float getShadowBias() const { return mInternal->getShadowBias(); }

		/** @copydoc Light::setColor */
		BS_SCRIPT_EXPORT(n:Color,pr:setter)
		void setColor(const Color& color) { mInternal->setColor(color); }

		/** @copydoc Light::getColor */
		BS_SCRIPT_EXPORT(n:Color,pr:getter)
		Color getColor() const { return mInternal->getColor(); }

		/** @copydoc Light::setAttenuationRadius */
		BS_SCRIPT_EXPORT(n:AttenuationRadius,pr:setter)
		void setAttenuationRadius(float radius) { mInternal->setAttenuationRadius(radius); }

		/** @copydoc Light::getAttenuationRadius */
		BS_SCRIPT_EXPORT(n:AttenuationRadius,pr:getter)
		float getAttenuationRadius() const { return mInternal->getAttenuationRadius(); }

		/** @copydoc Light::setSourceRadius */
		BS_SCRIPT_EXPORT(n:SourceRadius,pr:setter)
		void setSourceRadius(float radius) { mInternal->setSourceRadius(radius); }

		/** @copydoc Light::getSourceRadius */
		BS_SCRIPT_EXPORT(n:SourceRadius,pr:getter)
		float getSourceRadius() const { return mInternal->getSourceRadius(); }

		/** @copydoc Light::setIntensity */
		BS_SCRIPT_EXPORT(n:Intensity,pr:setter)
		void setIntensity(float intensity) { mInternal->setIntensity(intensity); }

		/** @copydoc Light::getIntensity */
		BS_SCRIPT_EXPORT(n:Intensity,pr:getter)
		float getIntensity() const { return mInternal->getIntensity(); }

		/** @copydoc Light::setSpotAngle */
		BS_SCRIPT_EXPORT(n:SpotAngle,pr:setter)
		void setSpotAngle(const Degree& spotAngle) { mInternal->setSpotAngle(spotAngle); }

		/** @copydoc Light::getSpotAngle */
		BS_SCRIPT_EXPORT(n:SpotAngle,pr:getter)
		Degree getSpotAngle() const { return mInternal->getSpotAngle(); }

		/** @copydoc Light::setSpotFalloffAngle */
		BS_SCRIPT_EXPORT(n:SpotAngleFalloff,pr:setter)
		void setSpotFalloffAngle(const Degree& spotAngle) { mInternal->setSpotFalloffAngle(spotAngle); }

		/** @copydoc Light::getSpotFalloffAngle */
		BS_SCRIPT_EXPORT(n:SpotAngleFalloff,pr:getter)
		Degree getSpotFalloffAngle() const { return mInternal->getSpotFalloffAngle(); }

		/**  @copydoc Light::setUseAutoAttenuation */
		BS_SCRIPT_EXPORT(n:UseAutoAttenuation,pr:setter)
		void setUseAutoAttenuation(bool enabled) { mInternal->setUseAutoAttenuation(enabled); }

		/** @copydoc Light::getUseAutoAttenuation */
		BS_SCRIPT_EXPORT(n:UseAutoAttenuation,pr:getter)
		bool getUseAutoAttenuation() const { return mInternal->getUseAutoAttenuation(); }

		/** @copydoc Light::getBounds */
		BS_SCRIPT_EXPORT(n:Bounds,pr:getter)
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
		LightType mType = LightType::Radial;
		Color mColor = Color::White;
		float mIntensity = 100.0f;
		float mRange = 1.0f;
		bool mCastsShadows = false; 
		Degree mSpotAngle = Degree(45); 
		Degree mSpotFalloffAngle = Degree(40);

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