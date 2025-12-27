//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIConstructionMethods.h"
#include "B3DGUIContent.h"
#include "B3DGUISpriteHelper.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "2D/B3DImageSprite.h"
#include "2D/B3DTextSprite.h"
#include "Input/B3DVirtualInput.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Structure describing contents of a GUIInputBox element. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIInputBoxContent
	{
		GUIInputBoxContent() = default;
		GUIInputBoxContent(bool allowMultiline)
			: AllowMultiline(allowMultiline)
		{ }

		bool AllowMultiline = false; /**< If true, allows multiline input. */
	};

	/**
	 * Input box is a GUI element that accepts Unicode textual input. It can be single or multi-line and handles various
	 * types of text manipulation.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIInputBox : public GUIInteractable, public TGUIConstructionMethods<GUIInputBox, GUIInputBoxContent>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**	Determines the text inside the input box. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Text))
		void SetText(const String& text);

		/** @copydoc SetText */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Text))
		const String& GetText() const { return mText; }

		/**
		 * Sets an optional filter that can control what is allowed to be entered into the input box. Filter should return
		 * true if the provided string is valid and false otherwise. Set the filter to null to deactivate filtering.
		 */
		void SetFilter(std::function<bool(const String&)> filter) { mFilter = filter; }

		/**	Triggered whenever input text has changed. */
		B3D_SCRIPT_EXPORT()
		Event<void(const String&)> OnValueChanged;

		/**	Triggered when the user hits the Enter key with the input box in focus. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnConfirm;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct { };
		GUIInputBox(PrivatelyConstruct, const GUIInputBoxContent& content, const String& styleName, const GUISizeConstraints& sizeConstraints);

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;

		/** @} */
	protected:
		virtual ~GUIInputBox() = default;

		const char* GetStyleSheetElement() const override { return "inputbox"; }
		void UpdateRenderElements() override;
		bool DoOnMouseEvent(const GUIMouseEvent& ev) override;
		bool DoOnTextInputEvent(const GUITextInputEvent& ev) override;
		bool DoOnCommandEvent(const GUICommandEvent& ev) override;
		bool DoOnVirtualButtonEvent(const GUIVirtualButtonEvent& ev) override;
		u32 GetRenderElementDepthRange() const override;
		bool HasCustomCursor(const GUIPhysicalPoint& position, CursorType& type) const override;
		SPtr<GUIContextMenu> GetContextMenu() const override;

	private:
		/** Inserts a new string into the current text at the specified index. */
		void InsertString(u32 charIndex, const String& string);

		/**	Inserts a new character into the current text at the specified index. */
		void InsertChar(u32 charIndex, u32 charCode);

		/**	Erases a single character at the specified index. */
		void EraseChar(u32 charIndex);

		/**
		 * Deletes text that is currently selected.
		 *
		 * @param	internal	If internal no filter will be applied after the text is deleted, and no event will be
		 * 						triggered either.
		 */
		void DeleteSelectedText(bool internal = false);

		/**	Returns currently selected text. */
		String GetSelectedText();

		/**	Shows the input caret. You must position the caret manually after showing it. */
		void ShowCaret();

		/**	Hides the input caret. */
		void HideCaret();

		/**
		 * Shows selection with the specified anchor position. You must position selection start and end before selection
		 * will actually render. Anchor position determines selection behavior as the user moves the selection with the
		 * keyboard.
		 */
		void ShowSelection(u32 anchorCaretPos);

		/**	Removes any active selection. */
		void ClearSelection();

		/**	Adjusts the text offset (scroll) so that the caret is visible. */
		void ScrollTextToCaret();

		/** Clamps the text offset (scroll)	so that the text fits in the provided bounds nicely with minimal white space. */
		void ClampScrollToBounds(const GUIPhysicalArea& unclippedTextBounds);

		/**	Returns offset at which to render the text. Relative to parent widget. */
		GUIPhysicalPoint GetTextOffset() const;

		/**	Cuts currently selected text to clipboard. */
		void CutText();

		/**	Copies currently selected text to clipboard. */
		void CopyText();

		/**	Inserts text from clipboard to current caret location. */
		void PasteText();

		/** Builds a structure describing the text, using the current text. */
		TextSpriteInformation BuildTextSpriteInformation() const;

	private:
		static VirtualButton mCopyVB;
		static VirtualButton mPasteVB;
		static VirtualButton mCutVB;
		static VirtualButton mSelectAllVB;

		// Sprites
		GUIBackgroundSprite mBackgroundSprite;
		GUIContentSprites mTextSprite;
		bool mIsMultiline;
		GUIPhysicalPoint mTextOffset{BsZero};
		bool mHasFocus = false;
		u64 mFocusGainedFrame = (u64)-1;
		bool mIsMouseOver = false;
		GUIElementState mState = GUIElementState::Normal;

		String mText;
		u32 mCharCount = 0;
		std::function<bool(const String&)> mFilter;

		bool mCaretShown = false;
		bool mSelectionShown = false;
		bool mDragInProgress = false;
	};

	/** @} */
} // namespace b3d
