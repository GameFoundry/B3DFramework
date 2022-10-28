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
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportName(LightProbeVolume)) CLightProbeVolume : public Component
	{
	public:
		CLightProbeVolume(const HSceneObject& parent, const AABox& volume = AABox::kUnitBox, const Vector3I& cellCount = Vector3I(1, 1, 1));
		virtual ~CLightProbeVolume();

		/** @copydoc LightProbeVolume::AddProbe() */
		BS_SCRIPT_EXPORT()
		u32 AddProbe(const Vector3& position) { return mInternal->AddProbe(position); }

		/** @copydoc LightProbeVolume::SetProbePosition() */
		BS_SCRIPT_EXPORT()
		void SetProbePosition(u32 handle, const Vector3& position) { mInternal->SetProbePosition(handle, position); }

		/** @copydoc LightProbeVolume::GetProbePosition() */
		BS_SCRIPT_EXPORT()
		Vector3 GetProbePosition(u32 handle) const { return mInternal->GetProbePosition(handle); }

		/** @copydoc LightProbeVolume::RemoveProbe() */
		BS_SCRIPT_EXPORT()
		void RemoveProbe(u32 handle) { mInternal->RemoveProbe(handle); }

		/** @copydoc LightProbeVolume::GetProbes() */
		BS_SCRIPT_EXPORT()
		Vector<LightProbeInfo> GetProbes() const;

		/** @copydoc LightProbeVolume::RenderProbe() */
		BS_SCRIPT_EXPORT()
		void RenderProbe(u32 handle);

		/** @copydoc LightProbeVolume::RenderProbes() */
		BS_SCRIPT_EXPORT()
		void RenderProbes();

		/** @copydoc LightProbeVolume::Resize() */
		BS_SCRIPT_EXPORT()
		void Resize(const AABox& volume, const Vector3I& cellCount = Vector3I(1, 1, 1)) { mInternal->Resize(volume, cellCount); }

		/** @copydoc LightProbeVolume::Clip() */
		BS_SCRIPT_EXPORT()
		void Clip() { mInternal->Clip(); }

		/** @copydoc LightProbeVolume::Reset() */
		BS_SCRIPT_EXPORT()
		void Reset() { mInternal->Reset(); }

		/** @copydoc LightProbeVolume::GetGridVolume() */
		BS_SCRIPT_EXPORT(ExportName(GridVolume), Property(Getter))
		const AABox& GetGridVolume() const { return mVolume; }

		/** @copydoc LightProbeVolume::GetCellCount() */
		BS_SCRIPT_EXPORT(ExportName(CellCount), Property(Getter))
		const Vector3I& GetCellCount() const { return mCellCount; }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the light probe volume that this component wraps. */
		SPtr<LightProbeVolume> GetInternalInternal() const { return mInternal; }

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

		void OnInitialized();
		void OnDestroyed();
		void Update() {}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CLightProbeVolumeRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;

	protected:
		CLightProbeVolume(); // Serialization only
	};

	/** @} */
} // namespace bs
