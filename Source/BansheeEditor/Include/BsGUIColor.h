//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIElement.h"
#include "BsImageSprite.h"
#include "BsGUIContent.h"

namespace BansheeEngine
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/** GUI element that displays the set color. RGB and alpha values are displayed separately. */
	class GUIColor : public GUIElement
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI color element.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIColor* create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI color element.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIColor* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**	Sets the color to display. */
		void setColor(const Color& color);

		/**	Returns the currently displayed color. */
		Color getColor() const { return mValue; }

		Event<void()> onClicked; /**< Triggered when the user clicks on the GUI element. */

		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @} */
	protected:
		GUIColor(const String& styleName, const GUIDimensions& dimensions);
		virtual ~GUIColor();

		/** @copydoc GUIElement::_getNumRenderElements() */
		UINT32 _getNumRenderElements() const override;

		/** @copydoc GUIElement::_getMaterial() */
		const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const override;

		/** @copydoc GUIElement::_getMeshSize() */
		void _getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const override;

		/** @copydoc GUIElement::_fillBuffer() */
		void _fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset, 
			UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateRenderElementsInternal() */
		void updateRenderElementsInternal() override;

		/** @copydoc GUIElement::_mouseEvent() */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

	private:
		static const float ALPHA_SPLIT_POSITION;

		ImageSprite* mColorSprite;
		ImageSprite* mAlphaSprite;

		IMAGE_SPRITE_DESC mColorImageDesc;
		IMAGE_SPRITE_DESC mAlphaImageDesc;

		Color mValue;
	};

	/** @} */
}