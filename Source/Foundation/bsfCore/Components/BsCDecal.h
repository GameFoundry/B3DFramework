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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportName(Decal)) CDecal : public Component
	{
	public:
		CDecal(const HSceneObject& parent);
		virtual ~CDecal();

		/** @copydoc Decal::SetMaterial */
		BS_SCRIPT_EXPORT(ExportName(Material), Property(Setter))
		void SetMaterial(const HMaterial& material) { mInternal->SetMaterial(material); }

		/** @copydoc SetMaterial */
		BS_SCRIPT_EXPORT(ExportName(Material), Property(Getter))
		const HMaterial& GetMaterial() const { return mInternal->GetMaterial(); }

		/** @copydoc Decal::SetSize */
		BS_SCRIPT_EXPORT(ExportName(Size), Property(Setter))
		void SetSize(const Vector2& size) { mInternal->SetSize(size); }

		/** @copydoc SetSize */
		BS_SCRIPT_EXPORT(ExportName(Size), Property(Getter))
		Vector2 GetSize() const { return mInternal->GetSize(); }

		/** @copydoc Decal::SetMaxDistance */
		BS_SCRIPT_EXPORT(ExportName(MaxDistance), Property(Setter))
		void SetMaxDistance(float distance) { mInternal->SetMaxDistance(distance); }

		/** @copydoc GetSize */
		BS_SCRIPT_EXPORT(ExportName(MaxDistance), Property(Getter))
		float GetMaxDistance() const { return mInternal->GetMaxDistance(); }

		/** @copydoc Decal::SetLayer */
		BS_SCRIPT_EXPORT(ExportName(Layer), Property(Setter))
		void SetLayer(u64 layer) { mInternal->SetLayer(layer); }

		/** @copydoc SetLayer() */
		BS_SCRIPT_EXPORT(ExportName(Layer), Property(Getter))
		u64 GetLayer() const { return mInternal->GetLayer(); }

		/** @copydoc Decal::SetLayerMask */
		BS_SCRIPT_EXPORT(ExportName(LayerMask), Property(Setter))
		void SetLayerMask(u32 mask) { mInternal->SetLayerMask(mask); }

		/** @copydoc SetLayerMask */
		BS_SCRIPT_EXPORT(ExportName(LayerMask), Property(Getter))
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

		void OnInitialized() override;
		void OnDestroyed() override;
		void Update() override {}

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
} // namespace bs
