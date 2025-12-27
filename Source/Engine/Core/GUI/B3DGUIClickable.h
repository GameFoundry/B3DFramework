//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "2D/B3DImageSprite.h"
#include "2D/B3DTextSprite.h"
#include "GUI/B3DGUIContent.h"
#include "GUI/B3DGUISpriteHelper.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Base class for a clickable GUI button element. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIClickable : public GUIInteractable
	{
	public:
		/**	Change content displayed by the button. */
		B3D_SCRIPT_EXPORT()
		void SetContent(const GUIContent& content);

		/**	Triggered when button is clicked. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnClick;

		/**	Triggered when pointer hovers over the button. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnHover;

		/**	Triggered when pointer that was previously hovering leaves the button. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnOut;

		/**	Triggered when button is clicked twice in rapid succession. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnDoubleClick;

		static constexpr const char* kElementType = "button";
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Change the button "on" state. This state determines whether the button uses normal or "on" fields specified in
		 * the GUI style.
		 */
		void SetOnInternal(bool on);

		/**
		 * Retrieves the button "on" state. This state determines whether the button uses normal or "on" fields specified
		 * in the GUI style.
		 */
		bool IsOnInternal() const;

		/**	Change the internal button state, changing the button look depending on set style. */
		void SetStateInternal(GUIElementState state);

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;
		u32 GetRenderElementDepthRange() const override;
		const char* GetStyleSheetElement() const override { return kElementType; }

		/** @} */
	protected:
		GUIClickable(const String& styleName, const GUIContent& content, const GUISizeConstraints& sizeConstraints, GUIElementOptions options = GUIElementOption::AcceptsKeyFocus);
		~GUIClickable() override = default;

		void UpdateRenderElements() override;
		bool DoOnMouseEvent(const GUIMouseEvent& event) override;
		bool DoOnCommandEvent(const GUICommandEvent& event) override;
		String GetTooltip() const override;
		void NotifyStyleChanged() override;

		/**	Retrieves internal button state. */
		GUIElementState GetState() const { return mActiveState; }

	protected:
		GUIBackgroundSprite mBackgroundSprite;
		GUIContentSprites mContentSprites;
		GUIElementState mActiveState = GUIElementState::Normal;

		GUIContent mContent;
		bool mHasFocus = false;
	};

	/** @} */
} // namespace b3d
