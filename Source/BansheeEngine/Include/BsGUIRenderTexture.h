//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUITexture.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Allows you to display a render texture in the GUI. Has the same functionality as GUITexture, but also forwards any 
	 * input to underlying GUI elements being rendered on the provided render texture.
	 */
	class BS_EXPORT GUIRenderTexture : public GUITexture
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/**
		 * Creates a new element with the provided render texture.
		 *
		 * @param[in]	texture			Render texture to display.
		 * @param[in]	transparent		Determines should the texture be rendered with transparency active.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the 
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIRenderTexture* create(const SPtr<RenderTexture>& texture, bool transparent, 
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new element with the provided render texture.
		 *
		 * @param[in]	texture			Render texture to display.
		 * @param[in]	transparent		Determines should the texture be rendered with transparency active.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the 
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIRenderTexture* create(const SPtr<RenderTexture>& texture, bool transparent, const GUIOptions& options, 
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new element with the provided render texture.
		 *
		 * @param[in]	texture			Render texture to display.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the 
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIRenderTexture* create(const SPtr<RenderTexture>& texture, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new element with the provided render texture.
		 *
		 * @param[in]	texture			Render texture to display.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the 
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIRenderTexture* create(const SPtr<RenderTexture>& texture, const GUIOptions& options, 
			const String& styleName = StringUtil::BLANK);

		/** Changes the active render texture whose contents to display in the GUI element. */
		void setRenderTexture(const SPtr<RenderTexture>& texture);

	protected:
		GUIRenderTexture(const String& styleName, const SPtr<RenderTexture>& texture, bool transparent, 
			const GUIDimensions& dimensions);
		virtual ~GUIRenderTexture();

		/** @copydoc GUIElement::updateRenderElementsInternal */
		virtual void updateRenderElementsInternal() override;

		SPtr<RenderTexture> mSourceTexture;
		bool mTransparent;
	};

	/** @} */
}