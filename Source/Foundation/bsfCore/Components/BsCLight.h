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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportName(Light)) CLight : public Component
	{
	public:
		CLight(const HSceneObject& parent, LightType type = LightType::Radial, Color color = Color::kWhite, float intensity = 100.0f, float range = 1.0f, bool castsShadows = false, Degree spotAngle = Degree(45), Degree spotFalloffAngle = Degree(40));

		virtual ~CLight();

		/** @copydoc Light::SetType */
		BS_SCRIPT_EXPORT(ExportName(Type), Property(Setter))
		void SetType(LightType type) { mInternal->SetType(type); }

		/** @copydoc Light::GetType */
		BS_SCRIPT_EXPORT(ExportName(Type), Property(Getter))

		LightType GetType() const { return mInternal->GetType(); }

		/** @copydoc Light::SetColor */
		BS_SCRIPT_EXPORT(ExportName(Color), Property(Setter))
		void SetColor(const Color& color) { mInternal->SetColor(color); }

		/** @copydoc Light::GetColor */
		BS_SCRIPT_EXPORT(ExportName(Color), Property(Getter))
		Color GetColor() const { return mInternal->GetColor(); }

		/** @copydoc Light::SetIntensity */
		BS_SCRIPT_EXPORT(ExportName(Intensity), Property(Setter))
		void SetIntensity(float intensity) { mInternal->SetIntensity(intensity); }

		/** @copydoc Light::GetIntensity */
		BS_SCRIPT_EXPORT(ExportName(Intensity), Property(Getter))
		float GetIntensity() const { return mInternal->GetIntensity(); }

		/**  @copydoc Light::SetUseAutoAttenuation */
		BS_SCRIPT_EXPORT(ExportName(UseAutoAttenuation), Property(Setter))
		void SetUseAutoAttenuation(bool enabled) { mInternal->SetUseAutoAttenuation(enabled); }

		/** @copydoc Light::GetUseAutoAttenuation */
		BS_SCRIPT_EXPORT(ExportName(UseAutoAttenuation), Property(Getter))
		bool GetUseAutoAttenuation() const { return mInternal->GetUseAutoAttenuation(); }

		/** @copydoc Light::SetAttenuationRadius */
		BS_SCRIPT_EXPORT(ExportName(AttenuationRadius), Property(Setter))
		void SetAttenuationRadius(float radius) { mInternal->SetAttenuationRadius(radius); }

		/** @copydoc Light::GetAttenuationRadius */
		BS_SCRIPT_EXPORT(ExportName(AttenuationRadius), Property(Getter))
		float GetAttenuationRadius() const { return mInternal->GetAttenuationRadius(); }

		/** @copydoc Light::SetSourceRadius */
		BS_SCRIPT_EXPORT(ExportName(SourceRadius), Property(Setter))
		void SetSourceRadius(float radius) { mInternal->SetSourceRadius(radius); }

		/** @copydoc Light::GetSourceRadius */
		BS_SCRIPT_EXPORT(ExportName(SourceRadius), Property(Getter))
		float GetSourceRadius() const { return mInternal->GetSourceRadius(); }

		/** @copydoc Light::SetSpotAngle */
		BS_SCRIPT_EXPORT(ExportName(SpotAngle), Property(Setter), UIValueRange([ 1, 180 ]), UI(AsSlider))
		void SetSpotAngle(const Degree& spotAngle) { mInternal->SetSpotAngle(spotAngle); }

		/** @copydoc Light::GetSpotAngle */
		BS_SCRIPT_EXPORT(ExportName(SpotAngle), Property(Getter))
		Degree GetSpotAngle() const { return mInternal->GetSpotAngle(); }

		/** @copydoc Light::SetSpotFalloffAngle */
		BS_SCRIPT_EXPORT(ExportName(SpotAngleFalloff), Property(Setter), UIValueRange([ 1, 180 ]), UI(AsSlider))
		void SetSpotFalloffAngle(const Degree& spotAngle) { mInternal->SetSpotFalloffAngle(spotAngle); }

		/** @copydoc Light::GetSpotFalloffAngle */
		BS_SCRIPT_EXPORT(ExportName(SpotAngleFalloff), Property(Getter))
		Degree GetSpotFalloffAngle() const { return mInternal->GetSpotFalloffAngle(); }

		/** @copydoc Light::SetCastsShadow */
		BS_SCRIPT_EXPORT(ExportName(CastsShadow), Property(Setter))
		void SetCastsShadow(bool castsShadow) { mInternal->SetCastsShadow(castsShadow); }

		/** @copydoc Light::GetCastsShadow */
		BS_SCRIPT_EXPORT(ExportName(CastsShadow), Property(Getter))
		bool GetCastsShadow() const { return mInternal->GetCastsShadow(); }

		/** @copydoc Light::SetShadowBias */
		BS_SCRIPT_EXPORT(ExportName(ShadowBias), Property(Setter), UIValueRange([ -1, 1 ]), UI(AsSlider))
		void SetShadowBias(float bias) { mInternal->SetShadowBias(bias); }

		/** @copydoc Light::SetShadowBias() */
		BS_SCRIPT_EXPORT(ExportName(ShadowBias), Property(Getter))
		float GetShadowBias() const { return mInternal->GetShadowBias(); }

		/** @copydoc Light::GetBounds */
		BS_SCRIPT_EXPORT(ExportName(Bounds), Property(Getter))
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
		Color mColor = Color::kWhite;
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

		void OnInitialized() override;
		void OnDestroyed() override;
		void Update() override {}

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
} // namespace bs
