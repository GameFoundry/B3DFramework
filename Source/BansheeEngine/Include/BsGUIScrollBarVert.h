//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIScrollBar.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Specialization of a GUIScrollBar for vertical scrolling. */
	class BS_EXPORT GUIScrollBarVert : public GUIScrollBar
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName(bool resizeable);

		/**
		 * Creates a new vertical scroll bar.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIScrollBarVert* create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new vertical scroll bar.
		 *
		 * @param[in]	resizable		If true the scrollbar will have additional handles that allow the scroll handle to
		 *								be resized. This allows you to adjust the size of the visible scroll area.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIScrollBarVert* create(bool resizable, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new vertical scroll bar.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIScrollBarVert* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new vertical scroll bar.
		 *
		 * @param[in]	resizable		If true the scrollbar will have additional handles that allow the scroll handle to
		 *								be resized. This allows you to adjust the size of the visible scroll area.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIScrollBarVert* create(bool resizable, const GUIOptions& options, const String& styleName = StringUtil::BLANK);
	protected:
		GUIScrollBarVert(bool resizable, const String& styleName, const GUIDimensions& dimensions);
		~GUIScrollBarVert();
	};

	/** @} */
}