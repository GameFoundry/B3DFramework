//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIConstructionMethods.h"
#include "B3DGUIContent.h"
#include "B3DGUISpriteHelper.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUITextLayout.h"
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
		virtual ~GUIInputBox();

		StringView GetStyleSheetElement() const override { return "inputbox"; }
		void UpdateRenderElements() override;
		bool DoOnMouseEvent(const GUIMouseEvent& ev) override;
		bool DoOnTextInputEvent(const GUITextInputEvent& ev) override;
		bool DoOnCommandEvent(const GUICommandEvent& ev) override;
		bool DoOnVirtualButtonEvent(const GUIVirtualButtonEvent& ev) override;
		u32 GetRenderElementDepthRange() const override;
		bool HasCustomCursor(const GUIPhysicalPoint& position, CursorType& type) const override;
		TShared<GUIContextMenu> GetContextMenu() const override;

	private:
		/** Returns the layout of the current text, refreshed if the text or its style has changed. */
		const GUITextLayout& GetTextLayout();

		/**
		 * Moves the caret and scrolls it into view.
		 *
		 * @param	position		Position to move the caret to.
		 * @param	extendSelection	If true the selection grows or shrinks along with the caret. Otherwise it collapses.
		 */
		void MoveCaret(const GUITextPosition& position, bool extendSelection);

		/**
		 * Moves the caret one line up or down, or to the start or end of the text if there is no such line.
		 *
		 * @param	moveUp			True to move up, false to move down.
		 * @param	extendSelection	If true the selection grows or shrinks along with the caret. Otherwise it collapses.
		 */
		void MoveCaretToAdjacentLine(bool moveUp, bool extendSelection);

		/**	Selects the entire text, with the caret at its end. Doesn't scroll the text. */
		void SelectAll();

		/**	Returns true if at least one character is selected. */
		bool HasSelection() const;

		/**	Returns the index of the first selected character. Equal to the caret's character index if nothing is selected. */
		u32 GetSelectionStart() const;

		/**	Returns the index of the character after the last selected one. Equal to the caret's character index if nothing is selected. */
		u32 GetSelectionEnd() const;

		/**
		 * Replaces a range of characters if the filter accepts the resulting text, and otherwise leaves everything as it is.
		 * On success the caret is placed after the new text and OnValueChanged is triggered.
		 *
		 * @param	startCharacterIndex		Index of the first character to replace.
		 * @param	endCharacterIndex		Index of the character after the last one to replace.
		 * @param	replacement				UTF-8 encoded text to insert in place of the range. Can be empty.
		 * @return							True if the text was replaced.
		 */
		bool ReplaceText(u32 startCharacterIndex, u32 endCharacterIndex, const String& replacement);

		/**	Destroys the sprites used for displaying the caret and the selection. */
		void DestroyEditingSprites();

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
		GUIPhysicalPoint mTextOffset{kZeroTag};
		bool mHasFocus = false;
		u64 mFocusGainedFrame = (u64)-1;
		bool mIsMouseOver = false;

		String mText;
		u32 mCharCount = 0;
		std::function<bool(const String&)> mFilter;

		GUITextLayout mTextLayout; // Released when the input box loses focus
		GUITextPosition mCaretPosition;
		u32 mSelectionAnchorIndex = 0; // Index of the character at the fixed end of the selection, while the caret is at the moving end
		ImageSprite* mCaretSprite = nullptr;
		Vector<ImageSprite*> mSelectionSprites;
		Vector<Area2I> mSelectionAreas; // One entry for each sprite in mSelectionSprites, relative to the text
	};

	/** @} */
} // namespace b3d
