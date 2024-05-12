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
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportName(Skybox)) CSkybox : public Component
	{
	public:
		CSkybox(const HSceneObject& parent);
		virtual ~CSkybox();

		/** @copydoc Skybox::GetTexture */
		B3D_SCRIPT_EXPORT(ExportName(Texture), Property(Getter))
		HTexture GetTexture() const { return mInternal->GetTexture(); }

		/** @copydoc Skybox::SetTexture */
		B3D_SCRIPT_EXPORT(ExportName(Texture), Property(Setter))
		void SetTexture(const HTexture& texture) { mInternal->SetTexture(texture); }

		/** @copydoc Skybox::SetBrightness */
		B3D_SCRIPT_EXPORT(ExportName(Brightness), Property(Setter))
		void SetBrightness(float brightness) { mInternal->SetBrightness(brightness); }

		/** @copydoc Skybox::GetBrightness */
		B3D_SCRIPT_EXPORT(ExportName(Brightness), Property(Getter))
		float GetBrightness() const { return mInternal->GetBrightness(); }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the skybox that this component wraps. */
		SPtr<Skybox> GetSkyboxInternal() const { return mInternal; }

		/** @} */

	protected:
		mutable SPtr<Skybox> mInternal;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnBeginPlay() override;
		void OnDestroyed() override;
		void Update() override {}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CSkyboxRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CSkybox(); // Serialization only
	};

	/** @} */
} // namespace bs
