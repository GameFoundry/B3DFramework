//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "2D/BsImageSprite.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	A GUI element that displays a texture. */
	class BS_EXPORT GUITexture : public GUIElement
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI texture element.
		 *
		 * @param[in]	texture			Texture element to display. If this is null then the texture specified by the style
		 *								will be used.
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	transparent		Determines should the texture be rendered with transparency active.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, TextureScaleMode scale, bool transparent,
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element.
		 *
		 * @param[in]	texture			Texture element to display. If this is null then the texture specified by the style
		 *								will be used.
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	transparent		Determines should the texture be rendered with transparency active.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, TextureScaleMode scale, bool transparent,
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element.
		 *
		 * @param[in]	texture			Texture element to display. If this is null then the texture specified by the style
		 *								will be used.
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, TextureScaleMode scale,
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element.
		 *
		 * @param[in]	texture			Texture element to display. If this is null then the texture specified by the style
		 *								will be used.
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, TextureScaleMode scale,
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element. Uses the default StretchToFit scale mode.
		 *
		 * @param[in]	texture			Texture element to display. If this is null then the texture specified by the style
		 *								will be used.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture,
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element. Uses the default StretchToFit scale mode.
		 *
		 * @param[in]	texture			Texture element to display. If this is null then the texture specified by the style
		 *								will be used.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(const HSpriteTexture& texture, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(TextureScaleMode scale, const GUIOptions& options,
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(TextureScaleMode scale, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 * Uses the default StretchToFit scale mode.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *			Uses the default StretchToFit scale mode.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* create(const String& styleName = StringUtil::BLANK);

		/**
		 * Changes the active texture. If the provided texture is null then the texture specified by the style will be used.
		 */
		void setTexture(const HSpriteTexture& texture);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElement::_getElementType */
		ElementType _getElementType() const override { return ElementType::Texture; }

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @} */
	protected:
		GUITexture(const String& styleName, const HSpriteTexture& texture, TextureScaleMode scale,
			bool transparent, const GUIDimensions& dimensions);
		virtual ~GUITexture();

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

		/** @copydoc GUIElement::styleUpdated */
		void styleUpdated() override;

		ImageSprite* mImageSprite;
		Vector2I mImageSpriteOffset;
		HSpriteTexture mActiveTexture;
		UINT32 mActiveTextureWidth = 0;
		UINT32 mActiveTextureHeight = 0;
		IMAGE_SPRITE_DESC mDesc;
		TextureScaleMode mScaleMode;
		bool mTransparent;
		bool mUsingStyleTexture;
	};

	/** @} */
}
