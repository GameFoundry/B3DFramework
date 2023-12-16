//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Type of GUI element options. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIOptionType
	{
		FixedWidth,
		FlexibleWidth,
		FixedHeight,
		FlexibleHeight,
		Position
	};

	/**
	 * Controls GUI element layout options, possibly by overriding the default options specified in GUI element style.
	 * These options control GUI element placement and size in a GUI layout.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIOption
	{

	public:
		GUIOption() = default;

		/**
		 * Constructs a GUI option notifying the GUI layout that this element should be positioned at this offset from the
		 * parent GUI panel. This option is ignored if element is part of a layout since it controls its placement.
		 */
		static GUIOption Position(i32 x, i32 y);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a fixed width. This will override the
		 * width property set in element style.
		 */
		static GUIOption FixedWidth(u32 value);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a flexible width with optional min/max
		 * constraints (value of 0 means no constraint). This will override the width property set in element style.
		 */
		static GUIOption FlexibleWidth(u32 min = 0, u32 max = 0);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a fixed height. This will override the
		 * height property set in element style.
		 */
		static GUIOption FixedHeight(u32 value);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a flexible height with optional min/max
		 * constraints (value of 0 means no constraint). This will override the height property set in element style.
		 */
		static GUIOption FlexibleHeight(u32 min = 0, u32 max = 0);

	private:
		friend struct GUISizeConstraints;

		u32 min = 0;
		u32 max = 0;
		GUIOptionType type = GUIOptionType::FixedWidth;
	};

	/**	Container for a list of options used for controlling GUI element properties. */
	class B3D_EXPORT GUIOptions
	{
	public:
		GUIOptions() = default;

		GUIOptions(const GUIOption& e0)
		{
			mOptions.Add(e0);
		}

		GUIOptions(const GUIOption& e0, const GUIOption& e1)
		{
			mOptions.Add(e0);
			mOptions.Add(e1);
		}

		GUIOptions(const GUIOption& e0, const GUIOption& e1, const GUIOption& e2)
		{
			mOptions.Add(e0);
			mOptions.Add(e1);
			mOptions.Add(e2);
		}

		GUIOptions(const GUIOption& e0, const GUIOption& e1, const GUIOption& e2, const GUIOption& e3)
		{
			mOptions.Add(e0);
			mOptions.Add(e1);
			mOptions.Add(e2);
			mOptions.Add(e3);
		}

		GUIOptions(const GUIOption& e0, const GUIOption& e1, const GUIOption& e2, const GUIOption& e3, const GUIOption& e4)
		{
			mOptions.Add(e0);
			mOptions.Add(e1);
			mOptions.Add(e2);
			mOptions.Add(e3);
			mOptions.Add(e4);
		}

		/**	Adds a new option to the options list.  */
		void AddOption(const GUIOption& option)
		{
			mOptions.Add(option);
		}

	private:
		friend struct GUISizeConstraints;

		TInlineArray<GUIOption, 4> mOptions;
	};

	/** @} */
} // namespace bs
