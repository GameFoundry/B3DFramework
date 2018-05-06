//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsSkybox.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	Skybox
	 *
	 * @note	Wraps Skybox as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering,n:Skybox) CSkybox : public Component
	{
	public:
		CSkybox(const HSceneObject& parent);
		virtual ~CSkybox();

		/** @copydoc Skybox::getTexture */
		BS_SCRIPT_EXPORT(n:Texture,pr:getter)
		HTexture getTexture() const { return mInternal->getTexture(); }

		/** @copydoc Skybox::setTexture */
		BS_SCRIPT_EXPORT(n:Texture,pr:setter)
		void setTexture(const HTexture& texture) { mInternal->setTexture(texture); }

		/** @copydoc Skybox::setBrightness */
		BS_SCRIPT_EXPORT(n:Brightness,pr:setter)
		void setBrightness(float brightness) { mInternal->setBrightness(brightness); }

		/** @copydoc Skybox::getBrightness */
		BS_SCRIPT_EXPORT(n:Brightness,pr:getter)
		float getBrightness() const { return mInternal->getBrightness(); }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the skybox that this component wraps. */
		SPtr<Skybox> _getSkybox() const { return mInternal; }

		/** @} */

	protected:
		mutable SPtr<Skybox> mInternal;

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
		friend class CSkyboxRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		CSkybox(); // Serialization only
	};

	/** @} */
}