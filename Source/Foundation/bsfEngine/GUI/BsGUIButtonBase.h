//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "2D/BsImageSprite.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUIContent.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Base class for a clickable GUI button element. */
	class B3D_EXPORT GUIButtonBase : public GUIElement
	{
	public:
		/**	Change content displayed by the button. */
		void SetContent(const GUIContent& content);

		/**	Triggered when button is clicked. */
		Event<void()> OnClick;

		/**	Triggered when pointer hovers over the button. */
		Event<void()> OnHover;

		/**	Triggered when pointer that was previously hovering leaves the button. */
		Event<void()> OnOut;

		/**	Triggered when button is clicked twice in rapid succession. */
		Event<void()> OnDoubleClick;

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

		Vector2I GetOptimalSize() const override;
		u32 GetRenderElementDepthRange() const override;

		/** @} */
	protected:
		GUIButtonBase(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions, GUIElementOptions options = GUIElementOption::AcceptsKeyFocus);
		virtual ~GUIButtonBase();

		void FillBuffer(u8* vertices, u32* indices, u32 vertexOffset, u32 indexOffset, const Vector2I& offset, u32 maxNumVerts, u32 maxNumIndices, u32 renderElementIdx) const override;
		void UpdateRenderElements() override;
		bool DoOnMouseEvent(const GUIMouseEvent& ev) override;
		bool DoOnCommandEvent(const GUICommandEvent& ev) override;
		String GetTooltip() const override;
		void StyleUpdated() override;

		/** Creates or destroys the content image sprite depending if there is a content image for the active state. */
		void RefreshContentSprite();

		/**	Gets the text sprite descriptor used for creating/updating the internal text sprite. */
		TEXT_SPRITE_DESC GetTextDesc() const;

		/**	Retrieves internal button state. */
		GUIElementState GetState() const { return mActiveState; }

		/**	Returns the active sprite texture, depending on the current state. */
		const HSpriteTexture& GetActiveTexture() const;

		/**	Returns the active text color, depending on the current state. */
		Color GetActiveTextColor() const;

	private:
		ImageSprite* mImageSprite;
		ImageSprite* mContentImageSprite = nullptr;
		TextSprite* mTextSprite;
		GUIElementState mActiveState = GUIElementState::Normal;

		IMAGE_SPRITE_DESC mImageDesc;
		GUIContent mContent;
		bool mHasFocus = false;
		float mContentAnimationStartTime = 0.0f;
	};

	/** @} */
} // namespace bs
