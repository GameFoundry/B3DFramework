//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Math/BsVector2I.h"

namespace bs
{
	struct GUIStyleSheetStateRule;
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Contains valid size range for a GUI element in a GUI layout. */
	struct B3D_EXPORT LayoutSizeRange
	{
		Vector2I Optimal;
		Vector2I Min;
		Vector2I Max;
	};

	/**	Flags that identify the type of data stored in a GUIDimensions structure. */
	enum GUIDimensionFlags
	{
		GUIDF_FixedWidth = 0x01,
		GUIDF_FixedHeight = 0x02,
		GUIDF_WidthOverridenAtRuntime = 0x04,
		GUIDF_HeightOverridenAtRuntime = 0x08
	};

	/**	Options that control how an element is positioned and sized. */
	struct B3D_EXPORT GUIDimensions
	{
		/**	Creates new default layout options. */
		static GUIDimensions Create();

		/**	Creates layout options with user defined options. */
		static GUIDimensions Create(const GUIOptions& options);

		GUIDimensions() = default;

		/**
		 * Updates layout options from the provided style. If user has not manually set a specific layout property, that
		 * property will be inherited from style.
		 */
		void UpdateWithStyle(const GUIElementStyle* style);

		/**
		 * Updates layout options from the provided style. If user has not manually set a specific layout property, that
		 * property will be inherited from style.
		 */
		void UpdateWithStyle(const GUIStyleSheetStateRule& style);

		/**
		 * Calculates size range for a GUI element using this layout.
		 *
		 * @param[in]	optimal	Preferred size of the GUI element.
		 */
		LayoutSizeRange CalculateSizeRange(const Vector2I& optimal) const;

		/**	Checks do the dimensions override the style height. */
		bool IsHeightOverridenAtRuntime() const { return (Flags & GUIDF_HeightOverridenAtRuntime) != 0; }

		/**	Checks do the dimensions override the style width. */
		bool IsWidthOverridenAtRuntime() const { return (Flags & GUIDF_WidthOverridenAtRuntime) != 0; }

		/**	Checks do the dimensions contain fixed width. */
		bool FixedWidth() const { return (Flags & GUIDF_FixedWidth) != 0; }

		/**	Checks do the dimensions contain fixed height. */
		bool FixedHeight() const { return (Flags & GUIDF_FixedHeight) != 0; }

		i32 X = 0;
		i32 Y = 0;

		u32 MinWidth = 0;
		u32 MaxWidth = 0;
		u32 MinHeight = 0;
		u32 MaxHeight = 0;
		u32 Flags = 0;
	};

	/** @} */
} // namespace bs
