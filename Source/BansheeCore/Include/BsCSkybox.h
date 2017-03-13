//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsSkybox.h"
#include "BsComponent.h"

namespace bs 
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * @copydoc	Skybox
	 *
	 * Wraps Skybox as a Component.
	 */
    class BS_CORE_EXPORT CSkybox : public Component
    {
    public:
        CSkybox(const HSceneObject& parent);
		virtual ~CSkybox();

        /** @copydoc Skybox::getTexture */
        HTexture getTexture() const { return mInternal->getTexture(); }

        /** @copydoc Skybox::setTexture */
        void setTexture(const HTexture& texture) { mInternal->setTexture(texture); }

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