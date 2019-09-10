//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "2D/BsImageSprite.h"
#include "2D/BsTextSprite.h"
#include "Input/BsVirtualInput.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Input box is a GUI element that accepts Unicode textual input. It can be single or multi-line and handles various
	 * types of text manipulation.
	 */
	class BS_EXPORT GUIInputBox : public GUIElement
	{
		/**	Possible visual states the input box can be in. */
		enum class State
		{
			Normal,
			Hover,
			Focused
		};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new input box.
		 *
		 * @param[in]	multiline		If true the input box can be of arbitrary height and will accept multiple lines of
		 *								text.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style for this element
		 *								is used.
		 */
		static GUIInputBox* create(bool multiline = false, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new input box.
		 *
		 * @param[in]	multiline		If true the input box can be of arbitrary height and will accept multiple lines of
		 *								text.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIInputBox* create(bool multiline, const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new single-line input box.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIInputBox* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**	Returns the text currently entered in the input box. */
		const String& getText() const { return mText; }

		/**	Sets the text inside the input box. This will replace any current text. */
		void setText(const String& text);

		/**
		 * Sets an optional filter that can control what is allowed to be entered into the input box. Filter should return
		 * true if the provided string is valid and false otherwise. Set the filter to null to deactivate filtering.
		 */
		void setFilter(std::function<bool(const String&)> filter) { mFilter = filter; }

		/**	Triggered whenever input text has changed. */
		Event<void(const String&)> onValueChanged;

		/**	Triggered when the user hits the Enter key with the input box in focus. */
		Event<void()> onConfirm;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElement::_getElementType */
		ElementType _getElementType() const override { return ElementType::InputBox; }

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @} */
	protected:
		GUIInputBox(const String& styleName, const GUIDimensions& dimensions, bool multiline);
		virtual ~GUIInputBox();

		/** @copydoc GUIElement::_fillBuffer() */
		void _fillBuffer(
			UINT8* vertices,
			UINT32* indices,
			UINT32 vertexOffset,
			UINT32 indexOffset,
			const Vector2I& offset,
			UINT32 maxNumVerts,
			UINT32 maxNumIndices,
			UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateRenderElementsInternal() */
		void updateRenderElementsInternal() override;

		/** @copydoc GUIElement::updateClippedBounds() */
		void updateClippedBounds() override;

		/** @copydoc GUIElement::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

		/** @copydoc GUIElement::_textInputEvent */
		bool _textInputEvent(const GUITextInputEvent& ev) override;

		/** @copydoc GUIElement::_commandEvent */
		bool _commandEvent(const GUICommandEvent& ev) override;

		/** @copydoc GUIElement::_virtualButtonEvent */
		bool _virtualButtonEvent(const GUIVirtualButtonEvent& ev) override;

		/**
		 * Returns how much to offset text due to scrolling.
		 *
		 * @note	
		 * This is used when text is larger than the input box itself. As the caret moves the text will scroll so that the
		 * caret remains visible, and how much scroll is applied is determined by this value.
		 */
		Vector2I _getTextInputOffset() const override;

		/** Returns rectangle in which the text can be displayed, in local coordinates (text will start at 0, 0). */
		Rect2I _getTextInputRect() const override;

		/** @copydoc GUIElement::_getRenderElementDepthRange */
		UINT32 _getRenderElementDepthRange() const override;

		/** @copydoc GUIElement::_hasCustomCursor */
		bool _hasCustomCursor(const Vector2I position, CursorType& type) const override;

		/** @copydoc GUIElement::_getContextMenu */
		SPtr<GUIContextMenu> _getContextMenu() const override;
	private:
		/**
		 * Retrieves a sprite from a render element index, and a local render element index that represents render element
		 * within the returned sprite.
		 */
		Sprite* renderElemToSprite(UINT32 renderElemIdx, UINT32& localRenderElemIdx) const;

		/**
		 * Returns offset at which is the element with the provided render element index. Offset is relative to parent
		 * widget.
		 */
		Vector2I renderElemToOffset(UINT32 renderElemIdx) const;

		/**
		 * Returns a clip rectangle that can be used for clipping the render element with the provided index. Rectangle is
		 * in local coordiantes relative to element origin.
		 */
		Rect2I renderElemToClipRect(UINT32 renderElemIdx) const;

		/** Inserts a new string into the current text at the specified index. */
		void insertString(UINT32 charIdx, const String& string);

		/**	Inserts a new character into the current text at the specified index. */
		void insertChar(UINT32 charIdx, UINT32 charCode);

		/**	Erases a single character at the specified index. */
		void eraseChar(UINT32 charIdx);

		/**
		 * Deletes text that is currently selected.
		 * 			
		 * @param[in]	internal	If internal no filter will be applied after the text is deleted, and no event will be
		 * 							triggered either.
		 */
		void deleteSelectedText(bool internal = false);

		/**	Returns currently selected text. */
		String getSelectedText();

		/**	Shows the input caret. You must position the caret manually after showing it. */
		void showCaret();

		/**	Hides the input caret. */
		void hideCaret();

		/**
		 * Shows selection with the specified anchor position. You must position selection start and end before selection
		 * will actually render. Anchor position determines selection behavior as the user moves the selection with the
		 * keyboard.
		 */
		void showSelection(UINT32 anchorCaretPos);

		/**	Removes any active selection. */
		void clearSelection();

		/**	Adjusts the text offset (scroll) so that the caret is visible. */
		void scrollTextToCaret();

		/** Clamps the text offset (scroll)	so that the text fits in the provided bounds nicely with minimal white space. */
		void clampScrollToBounds(Rect2I unclippedTextBounds);

		/**	Returns offset at which to render the text. Relative to parent widget. */
		Vector2I getTextOffset() const;

		/**	Returns rectangle used for clipping the text. Relative to element. */
		Rect2I getTextClipRect() const;

		/**	Returns text sprite descriptor determining how is text sprite created. */
		TEXT_SPRITE_DESC getTextDesc() const;

		/**	Returns currently active input box texture, depending on active state. */
		const HSpriteTexture& getActiveTexture() const;
		
		/**	Returns currently active input box text color, depending on active state. */
		Color getActiveTextColor() const;

		/**	Cuts currently selected text to clipboard. */
		void cutText();

		/**	Copies currently selected text to clipboard. */
		void copyText();

		/**	Inserts text from clipboard to current caret location. */
		void pasteText();

	private:
		static VirtualButton mCopyVB;
		static VirtualButton mPasteVB;
		static VirtualButton mCutVB;
		static VirtualButton mSelectAllVB;

		// Sprites
		ImageSprite* mImageSprite;
		TextSprite* mTextSprite;
		bool mIsMultiline;
		Vector2I mTextOffset;
		bool mHasFocus = false;
		UINT64 mFocusGainedFrame = (UINT64)-1;
		bool mIsMouseOver = false;
		State mState = State::Normal;

		IMAGE_SPRITE_DESC mImageDesc;
		String mText;
		UINT32 mNumChars = 0;
		std::function<bool(const String&)> mFilter;

		bool mCaretShown = false;
		bool mSelectionShown = false;
		bool mDragInProgress = false;
	};

	/** @} */
}
