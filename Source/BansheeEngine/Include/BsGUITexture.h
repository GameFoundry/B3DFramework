//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElement.h"
#include "BsImageSprite.h"

namespace BansheeEngine
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

		/** @copydoc GUIElement::_getNumRenderElements */
		UINT32 _getNumRenderElements() const override;

		/** @copydoc GUIElement::_getMaterial */
		const SpriteMaterialInfo& _getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const override;

		/** @copydoc GUIElement::_getMeshInfo() */
		void _getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const override;

		/** @copydoc GUIElement::_fillBuffer */
		void _fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
			UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateRenderElementsInternal */
		void updateRenderElementsInternal() override;

		/** @copydoc GUIElement::styleUpdated */
		void styleUpdated() override;

		ImageSprite* mImageSprite;
		HSpriteTexture mActiveTexture;
		IMAGE_SPRITE_DESC mDesc;
		TextureScaleMode mScaleMode;
		bool mTransparent;
		bool mUsingStyleTexture;
	};

	/** @} */
}