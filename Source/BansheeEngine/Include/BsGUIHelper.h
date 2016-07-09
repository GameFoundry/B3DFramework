//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsTextSprite.h"
#include "BsGUIContent.h"

namespace BansheeEngine
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Contains various helper methods used by GUI. */
	class BS_EXPORT GUIHelper
	{
	public:
		/**
		 * Calculates optimal content size by returning the nearest valid size to the provided value.
		 *
		 * @param[in]	contentSize		Wanted content size. This will be limited by minimal constraints of the style and 
		 *								layout options.
		 * @param[in]	style			Style to use for determining size constraints.
		 * @param[in]	dimensions		Dimension constraints of a GUI element.
		 */
		static Vector2I calcOptimalContentsSize(const Vector2I& contentSize, const GUIElementStyle& style, 
			const GUIDimensions& dimensions);

		/**
		 * Calculates optimal content size for the provided content using the provided style and layout options for 
		 * constraints.
		 *
		 * @param[in]	content			Content to calculate size for.
		 * @param[in]	style			Style to use for determining size constraints.
		 * @param[in]	dimensions		Dimension constraints of a GUI element.
		 * @param[in]	state			State of the GUI element in case the content changes according to state.
		 */
		static Vector2I calcOptimalContentsSize(const GUIContent& content, const GUIElementStyle& style, 
			const GUIDimensions& dimensions, GUIElementState state = GUIElementState::Normal);

		/**
		 * Calculates optimal content size for the provided text using the provided style and layout options for 
		 * constraints.
		 *
		 * @param[in]	text			Text to calculate size for.
		 * @param[in]	style			Style to use for determining size constraints.
		 * @param[in]	dimensions		Dimension constraints of a GUI element.
		 */
		static Vector2I calcOptimalContentsSize(const WString& text, const GUIElementStyle& style, 
			const GUIDimensions& dimensions);

		/**
		 * Calculates optimal content size for the provided text using the provided font and size. Size is calculated
		 * without word wrap.
		 *
		 * @param[in]	text			Text to calculate the size for.
		 * @param[in]	font			Font to use for rendering the text.
		 * @param[in]	fontSize		Size of individual characters in the font, in points.
		 * @return						Width/height required to display the text, in pixels.
		 */
		static Vector2I calcTextSize(const WString& text, const HFont& font, UINT32 fontSize);
	};

	/** @} */
}