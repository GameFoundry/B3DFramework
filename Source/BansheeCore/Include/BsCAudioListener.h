//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsAudioListener.h"
#include "BsComponent.h"

namespace BansheeEngine 
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	AudioListener
	 *
	 * Wraps AudioListener as a Component.
	 */
    class BS_CORE_EXPORT CAudioListener : public Component
    {
    public:
		CAudioListener(const HSceneObject& parent);
		virtual ~CAudioListener() {}
		
		/** @name Internal
		 *  @{
		 */

		/** Returns the AudioListener implementation wrapped by this component. */
		AudioListener* _getInternal() const { return mInternal.get(); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onInitialized() */
		void onInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void onDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void onDisabled() override;

		/** @copydoc Component::onEnabled() */
		void onEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void onTransformChanged(TransformChangedFlags flags) override;

		/** @copydoc Component::update() */
		void update() override;
    protected:
		using Component::destroyInternal;

		/** Creates the internal representation of the AudioListener and restores the values saved by the Component. */
		void restoreInternal();

		/** Destroys the internal AudioListener representation. */
		void destroyInternal();

		/** 
		 * Updates the transform of the internal AudioListener representation from the transform of the component's scene
		 * object.
		 */
		void updateTransform();

		SPtr<AudioListener> mInternal;
		Vector3 mLastPosition = Vector3::ZERO;
		Vector3 mVelocity = Vector3::ZERO;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CAudioListenerRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CAudioListener() {} // Serialization only
     };

	 /** @} */
}