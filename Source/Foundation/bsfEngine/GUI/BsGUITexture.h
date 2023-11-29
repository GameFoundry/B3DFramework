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
	class B3D_EXPORT GUITexture : public GUIElement
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new GUI texture element.
		 *
		 * @param[in]	image			Image to display. If this is null then the image specified by the style will be used.
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	transparent		Determines should the texture be rendered with transparency active.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(const HSpriteImage& image, TextureScaleMode scale, bool transparent, const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element.
		 *
		 * @param[in]	image			Image to display. If this is null then the image specified by the style will be used.
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	transparent		Determines should the texture be rendered with transparency active.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(const HSpriteImage& image, TextureScaleMode scale, bool transparent, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element.
		 *
		 * @param[in]	image			Image to display. If this is null then the image specified by the style will be used.
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(const HSpriteImage& image, TextureScaleMode scale, const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element.
		 *
		 * @param[in]	image			Image to display. If this is null then the image specified by the style will be used.
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(const HSpriteImage& image, TextureScaleMode scale, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element. Uses the default StretchToFit scale mode.
		 *
		 * @param[in]	image			Image to display. If this is null then the texture specified by the style will be used.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(const HSpriteImage& image, const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element. Uses the default StretchToFit scale mode.
		 *
		 * @param[in]	image			Image to display. If this is null then the image specified by the style will be used.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(const HSpriteImage& image, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(TextureScaleMode scale, const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *
		 * @param[in]	scale			Scale mode to use when sizing the texture.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(TextureScaleMode scale, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 * Uses the default StretchToFit scale mode.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new GUI texture element. Uses the "normal" texture from the active GUI element style.
		 *			Uses the default StretchToFit scale mode.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITexture* Create(const String& styleName = StringUtil::kBlank);

		/**
		 * Changes the active image. If the provided image is null then the image specified by the style will be used.
		 */
		void SetImage(const HSpriteImage& image);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		ElementType GetElementType() const override { return ElementType::Texture; }
		Vector2I CalculateUnconstrainedOptimalSize() const override;
		const char* GetStyleSheetElement() const override { return "texture"; }

		/** @} */
	protected:
		GUITexture(const String& styleName, const HSpriteImage& image, TextureScaleMode scale, bool transparent, const GUISizeConstraints& dimensions);
		virtual ~GUITexture();

		void UpdateRenderElements() override;
		void NotifyStyleChanged() override;

		ImageSprite* mImageSprite;
		HSpriteImage mActiveImage;
		u32 mActiveImageWidth = 0;
		u32 mActiveImageHeight = 0;
		ImageSpriteInformation mDesc;
		TextureScaleMode mScaleMode;
		bool mTransparent;
		bool mUsingStyleTexture;
	};

	/** @} */
} // namespace bs
