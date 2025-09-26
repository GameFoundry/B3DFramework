//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"

namespace b3d
{
	/** @addtogroup Components
	 *  @{
	 */

	/**
	 * Component that maps animation for specific bone also be applied to the SceneObject this component is attached to.
	 * The component will attach to the first found parent Animation component.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) Bone : public Component
	{
	public:
		Bone(const HSceneObject& parent);
		virtual ~Bone() = default;

		/** Determines the name of the bone the component is referencing. */
		B3D_SCRIPT_EXPORT(ExportName(Name), Property(Setter))
		void SetBoneName(const String& name);

		/** @copydoc SetBoneName */
		B3D_SCRIPT_EXPORT(ExportName(Name), Property(Getter))
		const String& GetBoneName() const { return mBoneName; }

		/** @name Internal
		 *  @{
		 */

		/**
		 * Changes the parent animation of this component.
		 *
		 * @param	animation	New animation parent, can be null.
		 * @param	isInternal	If true the bone will just be changed internally, but parent animation will not be
		 *						notified.
		 */
		void SetParentAnimation(const HAnimation& animation, bool isInternal = false);

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
		String mBoneName;
		HAnimation mParent;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class BoneRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		Bone(); // Serialization only
	};

	/** @} */
} // namespace b3d
