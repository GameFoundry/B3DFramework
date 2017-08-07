//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsLightProbeVolume.h"
#include "BsComponent.h"

namespace bs 
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	LightProbeVolume
	 *
	 * Wraps LightProbeVolume as a Component.
	 */
	class BS_CORE_EXPORT CLightProbeVolume : public Component
	{
	public:
		CLightProbeVolume(const HSceneObject& parent, const AABox& volume = AABox::UNIT_BOX, 
			const Vector3I& cellCount = {1, 1, 1});
		virtual ~CLightProbeVolume();

		/** @copydoc LightProbeVolume::addProbe() */
		UINT32 addProbe(const Vector3& position) { return mInternal->addProbe(position); }

		/** @copydoc LightProbeVolume::setProbePosition() */
		void setProbePosition(UINT32 handle, const Vector3& position) { mInternal->setProbePosition(handle, position); }

		/** @copydoc LightProbeVolume::getProbePosition() */
		Vector3 getProbePosition(UINT32 handle) const { return mInternal->getProbePosition(handle); }

		/** @copydoc LightProbeVolume::removeProbe() */
		void removeProbe(UINT32 handle) { mInternal->removeProbe(handle); }

		/** @copydoc LightProbeVolume::renderProbe() */
		void renderProbe(UINT32 handle);

		/** @copydoc LightProbeVolume::renderProbes() */
		void renderProbes();

		/** @copydoc LightProbeVolume::resize() */
		void resize(const AABox& volume, const Vector3I& cellCount = { 1, 1, 1 }) { mInternal->resize(volume, cellCount); }

		/** @copydoc LightProbeVolume::clip() */
		void clip() { mInternal->clip(); }

		/** @copydoc LightProbeVolume::reset() */
		void reset() { mInternal->reset(); }

		/** @name Internal 
		 *  @{
		 */

		/**	Returns the light probe volume that this component wraps. */
		SPtr<LightProbeVolume> _getInternal() const { return mInternal; }

		/** @} */

	protected:
		mutable SPtr<LightProbeVolume> mInternal;

		// Only valid during construction
		AABox mVolume;
		Vector3I mCellCount;

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
		friend class CLightProbeVolumeRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CLightProbeVolume(); // Serialization only
	};

	 /** @} */
}