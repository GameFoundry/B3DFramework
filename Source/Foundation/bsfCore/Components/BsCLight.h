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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering),n:Light) CLight : public Component
	{
	public:
		CLight(const HSceneObject& parent, LightType type = LightType::Radial, Color color = Color::White,
			float intensity = 100.0f, float range = 1.0f, bool castsShadows = false, Degree spotAngle = Degree(45),
			Degree spotFalloffAngle = Degree(40));

		virtual ~CLight();

		/** @copydoc Light::setType */
		BS_SCRIPT_EXPORT(n:Type,pr:setter)
		void SetType(LightType type) { mInternal->SetType(type); }

		/** @copydoc Light::getType */
		BS_SCRIPT_EXPORT(n:Type,pr:getter)
		LightType GetType() const { return mInternal->GetType(); }

		/** @copydoc Light::setColor */
		BS_SCRIPT_EXPORT(n:Color,pr:setter)
		void SetColor(const Color& color) { mInternal->SetColor(color); }

		/** @copydoc Light::getColor */
		BS_SCRIPT_EXPORT(n:Color,pr:getter)
		Color GetColor() const { return mInternal->GetColor(); }

		/** @copydoc Light::setIntensity */
		BS_SCRIPT_EXPORT(n:Intensity,pr:setter)
		void SetIntensity(float intensity) { mInternal->SetIntensity(intensity); }

		/** @copydoc Light::getIntensity */
		BS_SCRIPT_EXPORT(n:Intensity,pr:getter)
		float GetIntensity() const { return mInternal->GetIntensity(); }

		/**  @copydoc Light::setUseAutoAttenuation */
		BS_SCRIPT_EXPORT(n:UseAutoAttenuation,pr:setter)
		void SetUseAutoAttenuation(bool enabled) { mInternal->SetUseAutoAttenuation(enabled); }

		/** @copydoc Light::getUseAutoAttenuation */
		BS_SCRIPT_EXPORT(n:UseAutoAttenuation,pr:getter)
		bool GetUseAutoAttenuation() const { return mInternal->GetUseAutoAttenuation(); }

		/** @copydoc Light::setAttenuationRadius */
		BS_SCRIPT_EXPORT(n:AttenuationRadius,pr:setter)
		void SetAttenuationRadius(float radius) { mInternal->SetAttenuationRadius(radius); }

		/** @copydoc Light::getAttenuationRadius */
		BS_SCRIPT_EXPORT(n:AttenuationRadius,pr:getter)
		float GetAttenuationRadius() const { return mInternal->GetAttenuationRadius(); }

		/** @copydoc Light::setSourceRadius */
		BS_SCRIPT_EXPORT(n:SourceRadius,pr:setter)
		void SetSourceRadius(float radius) { mInternal->SetSourceRadius(radius); }

		/** @copydoc Light::getSourceRadius */
		BS_SCRIPT_EXPORT(n:SourceRadius,pr:getter)
		float GetSourceRadius() const { return mInternal->GetSourceRadius(); }

		/** @copydoc Light::setSpotAngle */
		BS_SCRIPT_EXPORT(n:SpotAngle,pr:setter,range:[1,180],slider)
		void SetSpotAngle(const Degree& spotAngle) { mInternal->SetSpotAngle(spotAngle); }

		/** @copydoc Light::getSpotAngle */
		BS_SCRIPT_EXPORT(n:SpotAngle,pr:getter)
		Degree GetSpotAngle() const { return mInternal->GetSpotAngle(); }

		/** @copydoc Light::setSpotFalloffAngle */
		BS_SCRIPT_EXPORT(n:SpotAngleFalloff,pr:setter,range:[1,180],slider)
		void SetSpotFalloffAngle(const Degree& spotAngle) { mInternal->SetSpotFalloffAngle(spotAngle); }

		/** @copydoc Light::getSpotFalloffAngle */
		BS_SCRIPT_EXPORT(n:SpotAngleFalloff,pr:getter)
		Degree GetSpotFalloffAngle() const { return mInternal->GetSpotFalloffAngle(); }

		/** @copydoc Light::setCastsShadow */
		BS_SCRIPT_EXPORT(n:CastsShadow,pr:setter)
		void SetCastsShadow(bool castsShadow) { mInternal->SetCastsShadow(castsShadow); }

		/** @copydoc Light::getCastsShadow */
		BS_SCRIPT_EXPORT(n:CastsShadow,pr:getter)
		bool GetCastsShadow() const { return mInternal->GetCastsShadow(); }

		/** @copydoc Light::setShadowBias */
		BS_SCRIPT_EXPORT(n:ShadowBias,pr:setter,range:[-1,1],slider)
		void SetShadowBias(float bias) { mInternal->SetShadowBias(bias); }

		/** @copydoc Light::setShadowBias() */
		BS_SCRIPT_EXPORT(n:ShadowBias,pr:getter)
		float GetShadowBias() const { return mInternal->GetShadowBias(); }

		/** @copydoc Light::getBounds */
		BS_SCRIPT_EXPORT(n:Bounds,pr:getter)
		Sphere GetBounds() const;

		/** @name Internal
		 *  @{
		 */

		 /** Returns the light that this component wraps. */
		SPtr<Light> GetLightInternal() const { return mInternal; }

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
		void OnInitialized() override;

	    /** @copydoc Component::onDestroyed */
		void OnDestroyed() override;

		/** @copydoc Component::update */
		void Update() override { }

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CLightRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CLight(); // Serialization only
	 };

	 /** @} */
}
