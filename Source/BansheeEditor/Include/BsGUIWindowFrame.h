//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUITexture.h"
#include "BsImageSprite.h"

namespace bs
{
	/** @addtogroup GUI-Editor-Internal
	 *  @{
	 */

	/**	GUI element used for displaying a border on an editor window edge. */
	class GUIWindowFrame : public GUITexture
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI window frame element.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIWindowFrame* create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI window frame element.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIWindowFrame* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**	Sets whether the frame should be displayed in focus or unfocused state. */
		void setFocused(bool focused);
	protected:
		~GUIWindowFrame();
		GUIWindowFrame(const String& styleName, const GUIDimensions& dimensions);
	};

	/** @} */
}