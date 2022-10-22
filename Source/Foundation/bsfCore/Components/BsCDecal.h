//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsDecal.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/** Wraps Decal as a Component. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering),n:Decal) CDecal : public Component
	{
	public:
		CDecal(const HSceneObject& parent);
		virtual ~CDecal();

		/** @copydoc Decal::setMaterial */
		BS_SCRIPT_EXPORT(n:Material,pr:setter)
		void SetMaterial(const HMaterial& material) { mInternal->SetMaterial(material); }

		/** @copydoc setMaterial */
		BS_SCRIPT_EXPORT(n:Material,pr:getter)
		const HMaterial& GetMaterial() const { return mInternal->GetMaterial(); }

		/** @copydoc Decal::setSize */
		BS_SCRIPT_EXPORT(n:Size,pr:setter)
		void SetSize(const Vector2& size) { mInternal->SetSize(size); }

		/** @copydoc setSize */
		BS_SCRIPT_EXPORT(n:Size,pr:getter)
		Vector2 GetSize() const { return mInternal->GetSize(); }

		/** @copydoc Decal::setMaxDistance */
		BS_SCRIPT_EXPORT(n:MaxDistance,pr:setter)
		void SetMaxDistance(float distance) { mInternal->SetMaxDistance(distance); }

		/** @copydoc getSize */
		BS_SCRIPT_EXPORT(n:MaxDistance,pr:getter)
		float GetMaxDistance() const { return mInternal->GetMaxDistance(); }

		/** @copydoc Decal::setLayer */
		BS_SCRIPT_EXPORT(n:Layer,pr:setter)
		void SetLayer(u64 layer) { mInternal->SetLayer(layer); }

		/** @copydoc setLayer() */
		BS_SCRIPT_EXPORT(n:Layer,pr:getter)
		u64 GetLayer() const { return mInternal->GetLayer(); }

		BS_SCRIPT_EXPORT(n:LayerMask,pr:setter)
		/** @copydoc Decal::setLayerMask */
		void SetLayerMask(u32 mask) { mInternal->SetLayerMask(mask); }

		BS_SCRIPT_EXPORT(n:LayerMask,pr:getter)
		/** @copydoc setLayerMask */
		u32 GetLayerMask() const { return mInternal->GetLayerMask(); }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the internal decal object that this component wraps. */
		SPtr<Decal> GetInternalInternal() const { return mInternal; }

		/** @} */

	protected:
		mutable SPtr<Decal> mInternal;

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
		friend class CDecalRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CDecal(); // Serialization only
	};

	/** @} */
}
