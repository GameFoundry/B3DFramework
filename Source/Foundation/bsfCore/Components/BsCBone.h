//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * Component that maps animation for specific bone also be applied to the SceneObject this component is attached to.
	 * The component will attach to the first found parent Animation component.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(Bone)) CBone : public Component
	{
	public:
		CBone(const HSceneObject& parent);
		virtual ~CBone() = default;

		/** Determines the name of the bone the component is referencing. */
		BS_SCRIPT_EXPORT(ExportName(Name), Property(Setter))
		void SetBoneName(const String& name);

		/** @copydoc SetBoneName */
		BS_SCRIPT_EXPORT(ExportName(Name), Property(Getter))

		const String& GetBoneName() const { return mBoneName; }

		/** @name Internal
		 *  @{
		 */

		/**
		 * Changes the parent animation of this component.
		 *
		 * @param[in]	animation	New animation parent, can be null.
		 * @param[in]	isInternal	If true the bone will just be changed internally, but parent animation will not be
		 *							notified.
		 */
		void SetParentInternal(const HAnimation& animation, bool isInternal = false);

		/** @} */
	private:
		/** Attempts to find the parent Animation component and registers itself with it. */
		void UpdateParentAnimation();

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnDestroyed();
		void OnDisabled();
		void OnEnabled();
		void OnTransformChanged(TransformChangedFlags flags);

	protected:
		using Component::DestroyInternal;

		String mBoneName;
		HAnimation mParent;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CBoneRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const;

	protected:
		CBone(); // Serialization only
	};

	/** @} */
} // namespace bs
