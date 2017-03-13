//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsReflectionProbe.h"
#include "BsComponent.h"

namespace bs 
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	ReflectionProbe
	 *
	 * Wraps ReflectionProbe as a Component.
	 */
    class BS_CORE_EXPORT CReflectionProbe : public Component
    {
    public:
		CReflectionProbe(const HSceneObject& parent);
		virtual ~CReflectionProbe();

	    /** @copydoc ReflectionProbe::getType */
		ReflectionProbeType getType() const { return mInternal->getType(); }

	    /** @copydoc ReflectionProbe::setType */
		void setType(ReflectionProbeType type) { mInternal->setType(type); }

	    /** @copydoc ReflectionProbe::getRadius */
		float getRadius() const { return mInternal->getRadius(); }

	    /** @copydoc ReflectionProbe::setRadius  */
		void setRadius(float radius) { mInternal->setRadius(radius); }

	    /** @copydoc ReflectionProbe::getExtents */
		Vector3 getExtents() const { return mInternal->getExtents(); }

	    /** @copydoc ReflectionProbe::setExtents */
		void setExtents(const Vector3& extents) { mInternal->setExtents(extents); }

        /** Retrieves transition distance set by setTransitionDistance(). */
        float getTransitionDistance() const { return mInternal->getTransitionDistance(); }

        /** @copydoc ReflectionProbe::setTransitionDistance */
        void setTransitionDistance(float distance) { mInternal->setTransitionDistance(distance); }

        /** @copydoc ReflectionProbe::getCustomTexture */
        HTexture getCustomTexture() const { return mInternal->getCustomTexture(); }

        /** @copydoc ReflectionProbe::setCustomTexture */
        void setCustomTexture(const HTexture& texture) { mInternal->setCustomTexture(texture); }

	    /** @copydoc ReflectionProbe::getBounds */
		Sphere getBounds() const;

        /** @copydoc ReflectionProbe::generate */
        void generate() { mInternal->generate(); }

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