//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsLightProbeVolume.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	LightProbeVolume
	 *
	 * @note	Wraps LightProbeVolume as a Component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Rendering,n:LightProbeVolume) CLightProbeVolume : public Component
	{
	public:
		CLightProbeVolume(const HSceneObject& parent, const AABox& volume = AABox::UNIT_BOX,
			const Vector3I& cellCount = Vector3I(1, 1, 1));
		virtual ~CLightProbeVolume();

		/** @copydoc LightProbeVolume::addProbe() */
		BS_SCRIPT_EXPORT()
		UINT32 AddProbe(const Vector3& position) { return mInternal->AddProbe(position); }

		/** @copydoc LightProbeVolume::setProbePosition() */
		BS_SCRIPT_EXPORT()
		void SetProbePosition(UINT32 handle, const Vector3& position) { mInternal->SetProbePosition(handle, position); }

		/** @copydoc LightProbeVolume::getProbePosition() */
		BS_SCRIPT_EXPORT()
		Vector3 GetProbePosition(UINT32 handle) const { return mInternal->GetProbePosition(handle); }

		/** @copydoc LightProbeVolume::removeProbe() */
		BS_SCRIPT_EXPORT()
		void RemoveProbe(UINT32 handle) { mInternal->RemoveProbe(handle); }

		/** @copydoc LightProbeVolume::getProbes() */
		BS_SCRIPT_EXPORT()
		Vector<LightProbeInfo> GetProbes() const;

		/** @copydoc LightProbeVolume::renderProbe() */
		BS_SCRIPT_EXPORT()
		void RenderProbe(UINT32 handle);

		/** @copydoc LightProbeVolume::renderProbes() */
		BS_SCRIPT_EXPORT()
		void RenderProbes();

		/** @copydoc LightProbeVolume::resize() */
		BS_SCRIPT_EXPORT()		
		void Resize(const AABox& volume, const Vector3I& cellCount = Vector3I(1, 1, 1)) { mInternal->Resize(volume, cellCount); }

		/** @copydoc LightProbeVolume::clip() */
		BS_SCRIPT_EXPORT()
		void Clip() { mInternal->Clip(); }

		/** @copydoc LightProbeVolume::reset() */
		BS_SCRIPT_EXPORT()
		void Reset() { mInternal->Reset(); }

		/** @copydoc LightProbeVolume::getGridVolume() */
		BS_SCRIPT_EXPORT(n:GridVolume,pr:getter)
		const AABox& GetGridVolume() const { return mVolume; }

		/** @copydoc LightProbeVolume::getCellCount() */
		BS_SCRIPT_EXPORT(n:CellCount,pr:getter)
		const Vector3I& GetCellCount() const { return mCellCount; }

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
		void OnInitialized() override;

		/** @copydoc Component::onDestroyed */
		void OnDestroyed() override;

		/** @copydoc Component::update */
		void Update() override { }

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
