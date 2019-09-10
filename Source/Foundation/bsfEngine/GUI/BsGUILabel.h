//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "2D/BsTextSprite.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUIContent.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	GUI element that displays text and optionally a content image. */
	class BS_EXPORT GUILabel : public GUIElement
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new label with the specified text.
		 *
		 * @param[in]	text		Text to display.
		 * @param[in]	styleName	Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUILabel* create(const HString& text, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new label with the specified text.
		 *
		 * @param[in]	text			Text to display.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUILabel* create(const HString& text, const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new label with the specified content (text + optional image).
		 *
		 * @param[in]	content			Content to display.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUILabel* create(const GUIContent& content, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new label with the specified content (text + optional image).
		 *
		 * @param[in]	content			Content to display.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUILabel* create(const GUIContent& content, const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/** Changes the active content of the label. */
		void setContent(const GUIContent& content);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @copydoc GUIElement::_getElementType */
		ElementType _getElementType() const override { return ElementType::Label; }

		/** @} */
	protected:
		~GUILabel();

		/** @copydoc GUIElement::_getRenderElementDepthRange */
		UINT32 _getRenderElementDepthRange() const override;

		/** @copydoc GUIElement::_fillBuffer */
		void _fillBuffer(
			UINT8* vertices,
			UINT32* indices,
			UINT32 vertexOffset,
			UINT32 indexOffset,
			const Vector2I& offset,
			UINT32 maxNumVerts,
			UINT32 maxNumIndices,
			UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateRenderElementsInternal */
		void updateRenderElementsInternal() override;

	private:
		GUILabel(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions);

		GUIContent mContent;

		TextSprite* mTextSprite;
		ImageSprite* mImageSprite;
		
		TEXT_SPRITE_DESC mDesc;
		IMAGE_SPRITE_DESC mImageDesc;
	};

	/** @} */
}
