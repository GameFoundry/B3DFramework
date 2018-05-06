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
		ReflectionProbeType getType() const { return mInternal->getType(); }

		/** @copydoc ReflectionProbe::setType */
		BS_SCRIPT_EXPORT(n:Type,pr:setter)
		void setType(ReflectionProbeType type) { mInternal->setType(type); }

		/** @copydoc ReflectionProbe::getRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:getter)
		float getRadius() const { return mInternal->getRadius(); }

		/** @copydoc ReflectionProbe::setRadius */
		BS_SCRIPT_EXPORT(n:Radius,pr:setter)
		void setRadius(float radius) { mInternal->setRadius(radius); }

		/** @copydoc ReflectionProbe::getExtents */
		BS_SCRIPT_EXPORT(n:Extents,pr:getter)
		Vector3 getExtents() const { return mInternal->getExtents(); }

		/** @copydoc ReflectionProbe::setExtents */
		BS_SCRIPT_EXPORT(n:Extents,pr:setter)
		void setExtents(const Vector3& extents) { mInternal->setExtents(extents); }

		/** Retrieves transition distance set by setTransitionDistance(). */
		float getTransitionDistance() const { return mInternal->getTransitionDistance(); }

		/** @copydoc ReflectionProbe::setTransitionDistance */
		void setTransitionDistance(float distance) { mInternal->setTransitionDistance(distance); }

		/** @copydoc ReflectionProbe::getCustomTexture */
		BS_SCRIPT_EXPORT(n:CustomTexture,pr:getter)
		HTexture getCustomTexture() const { return mInternal->getCustomTexture(); }

		/** @copydoc ReflectionProbe::setCustomTexture */
		BS_SCRIPT_EXPORT(n:CustomTexture,pr:setter)
		void setCustomTexture(const HTexture& texture) { mInternal->setCustomTexture(texture); }

		/** @copydoc ReflectionProbe::getBounds */
		Sphere getBounds() const;

		/** @copydoc ReflectionProbe::capture */
		BS_SCRIPT_EXPORT(n:Capture)
		void capture() { mInternal->capture(); }

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
		void onInitialized() override;

		/** @copydoc Component::onDestroyed */
		void onDestroyed() override;

		/** @copydoc Component::update */
		void update() override { }

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