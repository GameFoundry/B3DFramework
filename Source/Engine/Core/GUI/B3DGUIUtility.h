//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIUnits.h"
#include "B3DPrerequisites.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	struct GUIStyleSheetRules;

	/** @addtogroup GUI
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
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalUnit LogicalToPhysical(const GUILogicalUnit& value, float scale)
		{
			return Math::RoundToI32((float)(i32)value * scale);
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalUnitF LogicalToPhysical(const GUILogicalUnitF& value, float scale)
		{
			return (float)(value * scale);
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalPoint LogicalToPhysical(const GUILogicalPoint& value, float scale)
		{
			return GUIPhysicalPoint(LogicalToPhysical(value.X, scale), LogicalToPhysical(value.Y, scale));
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalPointF LogicalToPhysical(const GUILogicalPointF& value, float scale)
		{
			return GUIPhysicalPointF(LogicalToPhysical(value.X, scale), LogicalToPhysical(value.Y, scale));
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalSize LogicalToPhysical(const GUILogicalSize& value, float scale)
		{
			return GUIPhysicalSize(LogicalToPhysical(value.Width, scale), LogicalToPhysical(value.Height, scale));
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalSizeF LogicalToPhysical(const GUILogicalSizeF& value, float scale)
		{
			return GUIPhysicalSizeF(LogicalToPhysical(value.Width, scale), LogicalToPhysical(value.Height, scale));
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalArea LogicalToPhysical(const GUILogicalArea& value, float scale)
		{
			return GUIPhysicalArea(LogicalToPhysical(value.X, scale), LogicalToPhysical(value.Y, scale), LogicalToPhysical(value.Width, scale), LogicalToPhysical(value.Height, scale));
		}

		/**
		 * Converts a value from logical pixels to physical pixels.
		 * 
		 * @param value			Value in logical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in physical pixels.
		 */
		static GUIPhysicalAreaF LogicalToPhysical(const GUILogicalAreaF& value, float scale)
		{
			return GUIPhysicalAreaF(LogicalToPhysical(value.X, scale), LogicalToPhysical(value.Y, scale), LogicalToPhysical(value.Width, scale), LogicalToPhysical(value.Height, scale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalUnit PhysicalToLogical(const GUIPhysicalUnit& value, float scale)
		{
			if(Math::ApproxEquals(scale, 0.0f))
				return (i32)value;

			const float inversescale = 1.0f / scale;
			return Math::RoundToI32((float)(i32)value * inversescale);
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalUnitF PhysicalToLogical(const GUIPhysicalUnitF& value, float scale)
		{
			if(Math::ApproxEquals(scale, 0.0f))
				return (float)value;

			const float inversescale = 1.0f / scale;
			return (float)value * inversescale;
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalPoint PhysicalToLogical(const GUIPhysicalPoint& value, float scale)
		{
			return GUILogicalPoint(PhysicalToLogical(value.X, scale), PhysicalToLogical(value.Y, scale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalPointF PhysicalToLogical(const GUIPhysicalPointF& value, float scale)
		{
			return GUILogicalPointF(PhysicalToLogical(value.X, scale), PhysicalToLogical(value.Y, scale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalSize PhysicalToLogical(const GUIPhysicalSize& value, float scale)
		{
			return GUILogicalSize(PhysicalToLogical(value.Width, scale), PhysicalToLogical(value.Height, scale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalSizeF PhysicalToLogical(const GUIPhysicalSizeF& value, float scale)
		{
			return GUILogicalSizeF(PhysicalToLogical(value.Width, scale), PhysicalToLogical(value.Height, scale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalArea PhysicalToLogical(const GUIPhysicalArea& value, float scale)
		{
			return GUILogicalArea(PhysicalToLogical(value.X, scale), PhysicalToLogical(value.Y, scale), PhysicalToLogical(value.Width, scale), PhysicalToLogical(value.Height, scale));
		}

		/**
		 * Converts a value from physical pixels to logical pixels.
		 * 
		 * @param value			Value in physical pixels.
		 * @param scale			Scale applied to physical pixels. Normally the DPI scale, but can also include per-element scale depending on the requirements.
		 * @return				Value in logical pixels.
		 */
		static GUILogicalAreaF PhysicalToLogical(const GUIPhysicalAreaF& value, float scale)
		{
			return GUILogicalAreaF(PhysicalToLogical(value.X, scale), PhysicalToLogical(value.Y, scale), PhysicalToLogical(value.Width, scale), PhysicalToLogical(value.Height, scale));
		}

		/**
		 * Calculates size of the GUI element area based on the GUI content size. This is just the content area expanded by padding and border provided by the style.
		 *
		 * @param	contentSize		Size of the GUI element's content area.
		 * @param	styleSheetRule	Style to use when rendering the GUI element.
		 * @return					Size of the GUI element (including the border).
		 */
		static GUILogicalSize CalculateSizeWithPaddingAndBorder(const GUILogicalSize& contentSize, const GUIStyleSheetRules& styleSheetRule);

		/**
		 * Calculates optimal size for displaying particular GUI contents.
		 *
		 * @param	content			Content to calculate size for.
		 * @param	styleSheetRule	Style that the content will be displayed with.
		 * @param	wordWrapWidth	If non-zero, the width at which to perform word wrap. If not provided, all the text will be placed in a single line. Only relevant
		 *							if the contents contain text.
		 * @return					Optimal size of the GUI element, including content size, style padding and border width.
		 */
		static GUILogicalSize CalculateOptimalContentSizeWithPaddingAndBorder(const GUIContent& content, const GUIStyleSheetRules& styleSheetRule, GUILogicalUnit wordWrapWidth = 0);

		/**
		 * Calculates optimal size for displaying text.
		 *
		 * @param	text			Text to calculate size for.
		 * @param	styleSheetRule	Style that the content will be displayed with.
		 * @param	wordWrapWidth	If non-zero, the width at which to perform word wrap. If not provided, all the text will be placed in a single line.
		 * @return					Optimal size of the GUI element, including content size and the style padding (area within GUI elements border).
		 */
		static GUILogicalSize CalculateOptimalContentSizeWithPaddingAndBorder(const String& text, const GUIStyleSheetRules& styleSheetRule, GUILogicalUnit wordWrapWidth = 0);

		/**
		 * Calculates the content area based on the total element size (i.e. size as calculated by the layout). This is the layout area potentially
		 * offset/reduced by border/padding as specified in the style sheet rules.
		 */
		static GUILogicalArea RemovePaddingAndBorder(const GUILogicalSize& layoutSize, const GUIStyleSheetRules& styleSheetRules);

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
		static Size2I CalculateTextBounds(const String& text, const HFont& font, float fontSize);
	};

	/** @} */
} // namespace b3d
