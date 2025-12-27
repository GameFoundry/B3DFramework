//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInputTool.h"
#include "2D/B3DTextSprite.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Helper class for dealing with text selection for text input boxes and similar controls. */
	class B3D_EXPORT GUIInputSelection : public GUIInputTool
	{
	public:
		GUIInputSelection() = default;
		~GUIInputSelection();

		/**	Returns sprites representing the currently selected areas. */
		const Vector<ImageSprite*>& GetSprites() const { return mSprites; }

		/** Returns the bounds of the particular selection sprite, relative to the current parent GUI element. */
		Area2I GetBounds(u32 spriteIndex) const;

		/**	Recreates the selection clip sprites. */
		void UpdateSprite();

		/**
		 * Shows the selection using the specified anchor. By default this will select 0 characters so you must manually
		 * move the selection using MoveSelectionToCaret() before anything is considered selected.
		 *
		 * @param	anchorCaretPos	Anchor position which to initially select. Anchor position determines selection
		 *							area behavior when the input caret moves (determines whether left or right side of
		 *							the selection will move with the caret).
		 */
		void ShowSelection(u32 anchorCaretPos);

		/**
		 * Clears the currently active selection. Note this does not clear the internal selection range, just the
		 * selection sprites.
		 */
		void ClearSelectionVisuals();

		/**
		 * Moves the selection to caret. Selected area will be from the anchor provided in ShowSelection() to the caret
		 * position provided here.
		 */
		void MoveSelectionToCaret(u32 caretPos);

		/**	Checks is anything selected. */
		bool IsSelectionEmpty() const;

		/**	Selects all available text. */
		void SelectAll();

		/**
		 * Starts selection drag at the specified caret position. Call SelectionDragUpdate() and SelectionDragEnd() as the
		 * drag operation progresses.
		 */
		void SelectionDragStart(u32 caretPos);

		/**	Updates selection drag at the specified caret position. */
		void SelectionDragUpdate(u32 caretPos);

		/**	Stops selection drag. */
		void SelectionDragEnd();

		/**	Gets caret index of selection start. */
		u32 GetSelectionStart() const { return mSelectionStart; }

		/**	Gets caret index of selection end. */
		u32 GetSelectionEnd() const { return mSelectionEnd; }

	private:
		/** Returns rectangles describing the currently selected areas. Rectangles are relative to parent GUI element. */
		Vector<Area2I> GetSelectionRects() const;

	private:
		u32 mSelectionStart = 0;
		u32 mSelectionEnd = 0;
		u32 mSelectionAnchor = 0;
		u32 mSelectionDragAnchor = 0;

		Vector<Area2I> mSelectionRects;
		Vector<ImageSprite*> mSprites;
	};

	/** @} */
} // namespace b3d
