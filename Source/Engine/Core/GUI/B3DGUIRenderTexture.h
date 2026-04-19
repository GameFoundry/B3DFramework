//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUITexture.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Allows you to display a render texture in the GUI. Has the same functionality as GUITexture, but also forwards any
	 * input to underlying GUI elements being rendered on the provided render texture.
	 */
	class B3D_EXPORT GUIRenderTexture : public GUITexture
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new element with the provided render texture.
		 *
		 * @param	texture			Render texture to display.
		 * @param	transparent		Determines should the texture be rendered with transparency active.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIRenderTexture* Create(const SPtr<RenderTexture>& texture, bool transparent, const String& styleName = StringUtility::kBlank);

		/**
		 * Creates a new element with the provided render texture.
		 *
		 * @param	texture			Render texture to display.
		 * @param	transparent		Determines should the texture be rendered with transparency active.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIRenderTexture* Create(const SPtr<RenderTexture>& texture, bool transparent, const GUIOptions& options, const String& styleName = StringUtility::kBlank);

		/**
		 * Creates a new element with the provided render texture.
		 *
		 * @param	texture			Render texture to display.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIRenderTexture* Create(const SPtr<RenderTexture>& texture, const String& styleName = StringUtility::kBlank);

		/**
		 * Creates a new element with the provided render texture.
		 *
		 * @param	texture			Render texture to display.
		 * @param	options			Options that allow you to control how is the element positioned and sized.
		 *							This will override any similar options set by style.
		 * @param	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIRenderTexture* Create(const SPtr<RenderTexture>& texture, const GUIOptions& options, const String& styleName = StringUtility::kBlank);

		/** Changes the active render texture whose contents to display in the GUI element. */
		void SetRenderTexture(const SPtr<RenderTexture>& texture);

	protected:
		GUIRenderTexture(const String& styleName, const SPtr<RenderTexture>& texture, bool transparent, const GUISizeConstraints& dimensions);
		virtual ~GUIRenderTexture();

		void UpdateRenderElements() override;

		SPtr<RenderTexture> mSourceTexture;
		bool mTransparent;
	};

	/** @} */
} // namespace b3d
