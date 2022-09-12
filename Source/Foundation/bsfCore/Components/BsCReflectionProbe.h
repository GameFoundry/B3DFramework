//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsReflectionProbe.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	ReflectionProbe
	 *
	 * @note	Wraps ReflectionProbe as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering,n:ReflectionProbe) CReflectionProbe : public Component
	{
	public:
		CReflectionProbe(const HSceneObject& parent);
		virtual ~CReflectionProbe();

		/** @copydoc ReflectionProbe::getType */
		BS_SCRIPT_EXPORT(n:Type,pr:getter)
		ReflectionProbeType GetType() const { return mInternal->GetType(); }

		/** @copydoc ReflectionProbe::setType */
		BS_SCRIPT_EXPORT(n:Type,pr:setter)
		void SetType(ReflectionProbeType type) { mInternal->SetType(type); }

		/** @copydoc ReflectionProbe::getRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:getter)
		float GetRadius() const { return mInternal->GetRadius(); }

		/** @copydoc ReflectionProbe::setRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:setter)
		void SetRadius(float radius) { mInternal->SetRadius(radius); }

		/** @copydoc ReflectionProbe::getExtents */
		BS_SCRIPT_EXPORT(n:Extents,pr:getter)
		Vector3 GetExtents() const { return mInternal->GetExtents(); }

		/** @copydoc ReflectionProbe::setExtents */
		BS_SCRIPT_EXPORT(n:Extents,pr:setter)
		void SetExtents(const Vector3& extents) { mInternal->SetExtents(extents); }

		/** Retrieves transition distance set by setTransitionDistance(). */
		float GetTransitionDistance() const { return mInternal->GetTransitionDistance(); }

		/** @copydoc ReflectionProbe::setTransitionDistance */
		void SetTransitionDistance(float distance) { mInternal->SetTransitionDistance(distance); }

		/** @copydoc ReflectionProbe::getCustomTexture */
		BS_SCRIPT_EXPORT(n:CustomTexture,pr:getter)
		HTexture GetCustomTexture() const { return mInternal->GetCustomTexture(); }

		/** @copydoc ReflectionProbe::setCustomTexture */
		BS_SCRIPT_EXPORT(n:CustomTexture,pr:setter)
		void SetCustomTexture(const HTexture& texture) { mInternal->SetCustomTexture(texture); }

		/** @copydoc ReflectionProbe::getBounds */
		Sphere GetBounds() const;

		/** @copydoc ReflectionProbe::capture */
		BS_SCRIPT_EXPORT(n:Capture)
		void Capture() { mInternal->Capture(); }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the reflection probe that this component wraps. */
		SPtr<ReflectionProbe> _getReflectionProbe() const { return mInternal; }

		/** @} */

	protected:
		mutable SPtr<ReflectionProbe> mInternal;

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
		friend class CReflectionProbeRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CReflectionProbe(); // Serialization only
	};

	/** @} */
}
