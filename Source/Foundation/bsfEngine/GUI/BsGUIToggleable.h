//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIClickable.h"
#include "GUI/BsGUIToggleGroup.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUIContent.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Structure describing contents of a GUIToggle element. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIToggleContent
	{
		GUIToggleContent() = default;
		GUIToggleContent(const GUIContent& content, const SPtr<GUIToggleGroup>& toggleGroup = nullptr)
			: GeneralContent(content), ToggleGroup(toggleGroup)
		{ }

		GUIContent GeneralContent;
		SPtr<GUIToggleGroup> ToggleGroup;
	};

	/**	GUI element representing a toggleable button. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIToggleable : public GUIClickable
	{
		using Super = GUIClickable;
	public:
		/** Checks or unchecks the toggle. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(IsToggled))
		void SetIsToggled(bool isToggled) { SetIsToggled(isToggled, false); }

		/**	Checks is the toggle currently on. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(IsToggled))
		bool IsToggled() const { return mIsToggled; }

		/**	Triggered whenever the button is toggled on or off. */
		B3D_SCRIPT_EXPORT()
		Event<void(bool)> OnToggled;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Sets a toggle group of the toggle button. Toggling one button in a group will automatically untoggle others. */
		void SetToggleGroupInternal(SPtr<GUIToggleGroup> toggleGroup);

		/** Sets an interface that constructs the vector path used for drawing the GUI element checkmark. */
		void SetCheckmarkPathBuilder(const IGUIVectorPathBuilder* pathBuilder) { mCheckmarkPathBuilder = pathBuilder; }

		/** Checks or unchecks the toggle, and optionally triggers the OnToggled event. */
		virtual void SetIsToggled(bool isToggled, bool triggerEvent);

		Vector2I CalculateUnconstrainedOptimalSize() const override;

		/** @} */
	protected:
		GUIToggleable(const GUIToggleContent& contents, const String& styleName, const GUISizeConstraints& dimensions);
		virtual ~GUIToggleable();

		void UpdateRenderElements() override;
		bool DoOnMouseEvent(const GUIMouseEvent& event) override;
		bool DoOnCommandEvent(const GUICommandEvent& event) override;

		/**
		 * Calculates the bounds of the checkmark sprite, based on the current optimal size.
		 *
		 * @param	elementOptimalSize		Unconstrained optimal size for the GUI element. If zero, default checkmark size will be used.
		 */
		Size2UI CalculateCheckmarkSize(const Size2UI& elementOptimalSize) const;

		static constexpr i32 kCheckmarkContentSpacing = 3; /**< Spacing between the checkmark and contents, in pixels. */
		static constexpr Size2UI kDefaultCheckmarkSize = Size2UI(12, 12);
	protected:
		ImageSprite* mCheckmarkSprite = nullptr;
		ImageSpriteInformation mCheckmarkSpriteInformation;
		const IGUIVectorPathBuilder* mCheckmarkPathBuilder = nullptr;
		u32 mCheckmarkPseudoElementIndex = ~0u;

		SPtr<GUIToggleGroup> mToggleGroup;
		bool mIsToggled;
	};

	/** @} */
} // namespace bs
