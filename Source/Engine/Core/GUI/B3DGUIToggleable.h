//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIClickable.h"
#include "GUI/B3DGUIToggleGroup.h"
#include "2D/B3DImageSprite.h"
#include "GUI/B3DGUIContent.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Structure describing contents of a GUIToggle element. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIToggleContent
	{
		GUIToggleContent() = default;
		GUIToggleContent(const GUIContent& content, const TShared<GUIToggleGroup>& toggleGroup = nullptr)
			: GeneralContent(content), ToggleGroup(toggleGroup)
		{ }

		GUIContent GeneralContent;
		TShared<GUIToggleGroup> ToggleGroup;
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
		void SetToggleGroupInternal(TShared<GUIToggleGroup> toggleGroup);

		/** Sets an interface that constructs the vector path used for drawing the GUI element checkmark. */
		void SetCheckmarkPathBuilder(const IGUIVectorPathBuilder* pathBuilder) { mCheckmarkPathBuilder = pathBuilder; }

		/** Checks or unchecks the toggle, and optionally triggers the OnToggled event. */
		virtual void SetIsToggled(bool isToggled, bool triggerEvent);

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;

		/** @} */
	protected:
		GUIToggleable(const GUIToggleContent& contents, const String& styleName, const GUISizeConstraints& dimensions);
		virtual ~GUIToggleable();

		void UpdateRenderElements() override;
		bool DoOnMouseEvent(const GUIMouseEvent& event) override;
		bool DoOnCommandEvent(const GUICommandEvent& event) override;
		void NotifyStyleChanged() override;

		/**
		 * Calculates the bounds of the content are in which the checkmark will be placed.
		 *
		 * @param	elementOptimalSize	Unconstrained optimal size for the GUI element. Will be used to derive checkmark area
		 * 								size if explicit size is not provided in the style sheet.
		 */
		GUILogicalSize CalculateCheckmarkContentAreaSize(const GUILogicalSize& elementOptimalSize) const;

		static constexpr GUILogicalUnit kCheckmarkContentSpacing = 3; /**< Spacing between the checkmark and contents, in pixels. */
	protected:
		ImageSprite* mCheckmarkSprite = nullptr;
		ImageSpriteInformation mCheckmarkSpriteInformation;
		const IGUIVectorPathBuilder* mCheckmarkPathBuilder = nullptr;
		u32 mCheckmarkPseudoElementIndex = ~0u;

		TShared<GUIToggleGroup> mToggleGroup;
		bool mIsToggled;

		GUISizeConstraints mCheckmarkSizeConstraints;
	};

	/** @} */
} // namespace b3d
