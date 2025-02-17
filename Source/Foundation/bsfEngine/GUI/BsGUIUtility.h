//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGUIUnits.h"
#include "BsPrerequisites.h"
#include "Math/BsRect2I.h"

namespace bs
{
	struct GUIStyleSheetRules;

	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Helper class that performs various operations related to GUI. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI), Static) GUIUtility
	{
	public:
		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalUnit LogicalToPhysical(const GUILogicalUnit& value, float DPIScale)
		{
			return (i32)((float)(i32)value * DPIScale);
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalUnitF LogicalToPhysical(const GUILogicalUnitF& value, float DPIScale)
		{
			return (float)(value * DPIScale);
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalPoint LogicalToPhysical(const GUILogicalPoint& value, float DPIScale)
		{
			return GUIPhysicalPoint(LogicalToPhysical(value.X, DPIScale), LogicalToPhysical(value.Y, DPIScale));
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalPointF LogicalToPhysical(const GUILogicalPointF& value, float DPIScale)
		{
			return GUIPhysicalPointF(LogicalToPhysical(value.X, DPIScale), LogicalToPhysical(value.Y, DPIScale));
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalSize LogicalToPhysical(const GUILogicalSize& value, float DPIScale)
		{
			return GUIPhysicalSize(LogicalToPhysical(value.Width, DPIScale), LogicalToPhysical(value.Height, DPIScale));
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalSizeF LogicalToPhysical(const GUILogicalSizeF& value, float DPIScale)
		{
			return GUIPhysicalSizeF(LogicalToPhysical(value.Width, DPIScale), LogicalToPhysical(value.Height, DPIScale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalUnit PhysicalToLogical(const GUIPhysicalUnit& value, float DPIScale)
		{
			if(Math::ApproxEquals(DPIScale, 0.0f))
				return (i32)value;

			const float inverseDPIScale = 1.0f / DPIScale;
			return (i32)((float)(i32)value * inverseDPIScale);
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalUnitF PhysicalToLogical(const GUIPhysicalUnitF& value, float DPIScale)
		{
			if(Math::ApproxEquals(DPIScale, 0.0f))
				return (float)value;

			const float inverseDPIScale = 1.0f / DPIScale;
			return (float)value * inverseDPIScale;
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalPoint PhysicalToLogical(const GUIPhysicalPoint& value, float DPIScale)
		{
			return GUILogicalPoint(PhysicalToLogical(value.X, DPIScale), PhysicalToLogical(value.Y, DPIScale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalPointF PhysicalToLogical(const GUIPhysicalPointF& value, float DPIScale)
		{
			return GUILogicalPointF(PhysicalToLogical(value.X, DPIScale), PhysicalToLogical(value.Y, DPIScale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalSize PhysicalToLogical(const GUIPhysicalSize& value, float DPIScale)
		{
			return GUILogicalSize(PhysicalToLogical(value.Width, DPIScale), PhysicalToLogical(value.Height, DPIScale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param DPIScale		DPI scale applied to physical pixels. This is calculated as DPIScale = DPI / 96.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalSizeF PhysicalToLogical(const GUIPhysicalSizeF& value, float DPIScale)
		{
			return GUILogicalSizeF(PhysicalToLogical(value.Width, DPIScale), PhysicalToLogical(value.Height, DPIScale));
		}

		/**
		 * Calculates size of the GUI element area based on the GUI content size. This is just the content area expanded by padding and border provided by the style.
		 *
		 * @param	contentSize		Size of the GUI element's content area.
		 * @param	styleSheetRule	Style to use when rendering the GUI element.
		 * @return					Size of the GUI element (including the border).
		 */
		static Size2UI CalculateSizeWithPaddingAndBorder(const Size2UI& contentSize, const GUIStyleSheetRules& styleSheetRule);

		/**
		 * Calculates optimal size for displaying particular GUI contents.
		 *
		 * @param	content			Content to calculate size for.
		 * @param	styleSheetRule	Style that the content will be displayed with.
		 * @param	wordWrapWidth	If non-zero, the width at which to perform word wrap. If not provided, all the text will be placed in a single line. Only relevant
		 *							if the contents contain text.
		 * @return					Optimal size of the GUI element, including content size, style padding and border width.
		 */
		static Size2UI CalculateOptimalContentSizeWithPaddingAndBorder(const GUIContent& content, const GUIStyleSheetRules& styleSheetRule, u32 wordWrapWidth = 0);

		/**
		 * Calculates optimal size for displaying text.
		 *
		 * @param	text			Text to calculate size for.
		 * @param	styleSheetRule	Style that the content will be displayed with.
		 * @param	wordWrapWidth	If non-zero, the width at which to perform word wrap. If not provided, all the text will be placed in a single line.
		 * @return					Optimal size of the GUI element, including content size and the style padding (area within GUI elements border).
		 */
		static Size2UI CalculateOptimalContentSizeWithPaddingAndBorder(const String& text, const GUIStyleSheetRules& styleSheetRule, u32 wordWrapWidth = 0);

		/**
		 * Calculates the content area based on the total element size (i.e. size as calculated by the layout). This is the layout area potentially
		 * offset/reduced by border/padding as specified in the style sheet rules.
		 */
		static Rect2I CalculateContentArea(const Size2UI& layoutSize, const GUIStyleSheetRules& styleSheetRules);

		/**
		 * Calculates optimal content size for the provided text using the provided font and size. Size is calculated
		 * without word wrap.
		 *
		 * @param	text			Text to calculate the size for.
		 * @param	font			Font to use for rendering the text.
		 * @param	fontSize		Size of individual characters in the font, in points.
		 * @return					Width/height required to display the text, in pixels.
		 */
		B3D_SCRIPT_EXPORT()
		static Vector2I CalculateTextBounds(const String& text, const HFont& font, float fontSize);
	};

	/** @} */
} // namespace bs
