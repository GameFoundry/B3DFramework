//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIScrollBar.h"

namespace BansheeEngine
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Specialization of a GUIScrollBar for horizontal scrolling. */
	class BS_EXPORT GUIScrollBarHorz : public GUIScrollBar
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new horizontal scroll bar.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIScrollBarHorz* create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new horizontal scroll bar.
		 *
		 * @param[in]	resizeable		If true the scrollbar will have additional handles that allow the scroll handle to
		 *								be resized. This allows you to adjust the size of the visible scroll area.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIScrollBarHorz* create(bool resizeable, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new horizontal scroll bar.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIScrollBarHorz* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new horizontal scroll bar.
		 *
		 * @param[in]	resizeable		If true the scrollbar will have additional handles that allow the scroll handle to
		 *								be resized. This allows you to adjust the size of the visible scroll area.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIScrollBarHorz* create(bool resizeable, const GUIOptions& options, 
			const String& styleName = StringUtil::BLANK);
	protected:
		GUIScrollBarHorz(bool resizeable, const String& styleName, const GUIDimensions& dimensions);
		~GUIScrollBarHorz();
	};

	/** @} */
}