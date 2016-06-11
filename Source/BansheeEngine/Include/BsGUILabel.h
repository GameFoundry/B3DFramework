//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElement.h"
#include "BsTextSprite.h"
#include "BsGUIContent.h"

namespace BansheeEngine
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

		/** @copydoc GUIElement::_getNumRenderElements */
		UINT32 _getNumRenderElements() const override;

		/** @copydoc GUIElement::_getMaterial */
		const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const override;

		/** @copydoc GUIElement::_getMeshSize() */
		void _getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const override;

		/** @copydoc GUIElement::_fillBuffer */
		void _fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
			UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateRenderElementsInternal */
		void updateRenderElementsInternal() override;

	private:
		GUILabel(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions);

		TextSprite* mTextSprite;
		GUIContent mContent;

		TEXT_SPRITE_DESC mDesc;
	};

	/** @} */
}