//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Audio/BsAudioListener.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	AudioListener
	 *
	 * @note	Wraps AudioListener as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Audio),n:AudioListener) CAudioListener : public Component
	{
	public:
		CAudioListener(const HSceneObject& parent);
		virtual ~CAudioListener() = default;
		
		/** @name Internal
		 *  @{
		 */

		/** Returns the AudioListener implementation wrapped by this component. */
		AudioListener* GetInternalInternal() const { return mInternal.get(); }

		/** @} */

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		/** @copydoc Component::onInitialized() */
		void OnInitialized() override;

		/** @copydoc Component::onDestroyed() */
		void OnDestroyed() override;

		/** @copydoc Component::onDisabled() */
		void OnDisabled() override;

		/** @copydoc Component::onEnabled() */
		void OnEnabled() override;

		/** @copydoc Component::onTransformChanged() */
		void OnTransformChanged(TransformChangedFlags flags) override;

		/** @copydoc Component::update() */
		void Update() override;
	protected:
		using Component::DestroyInternal;

		/** Creates the internal representation of the AudioListener and restores the values saved by the Component. */
		void RestoreInternal();

		/** Destroys the internal AudioListener representation. */
		void DestroyInternal();

		/**
		 * Updates the transform of the internal AudioListener representation from the transform of the component's scene
		 * object.
		 */
		void UpdateTransform();

		SPtr<AudioListener> mInternal;
		Vector3 mLastPosition = Vector3::ZERO;
		Vector3 mVelocity = Vector3::ZERO;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CAudioListenerRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const ;

	protected:
		CAudioListener(); // Serialization only
	};

	 /** @} */
}
