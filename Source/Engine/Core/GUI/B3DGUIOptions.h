//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGUIUnits.h"

namespace b3d
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
		ExpandingWidth,
		ExpandingHeight,
		Position,
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
		static GUIOption Position(GUILogicalUnit x, GUILogicalUnit y);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a fixed width. This will override the
		 * width property set in element style.
		 */
		static GUIOption FixedWidth(GUILogicalUnit value);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a flexible width with optional min/max
		 * constraints (value of 0 means no constraint). This will override the width property set in element style.
		 */
		static GUIOption FlexibleWidth(GUILogicalUnit minimum = 0, GUILogicalUnit maximum = 0);

		/**
		 * Same as FlexibleWidth, but the width will attempt to expand to the maximum available space, rather than
		 * using the optimal width.
		 */
		static GUIOption ExpandingWidth(GUILogicalUnit minimum = 0, GUILogicalUnit maximum = 0);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a fixed height. This will override the
		 * height property set in element style.
		 */
		static GUIOption FixedHeight(GUILogicalUnit value);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a flexible height with optional min/max
		 * constraints (value of 0 means no constraint). This will override the height property set in element style.
		 */
		static GUIOption FlexibleHeight(GUILogicalUnit minimum = 0, GUILogicalUnit maximum = 0);

		/**
		 * Same as FlexibleHeight, but the height will attempt to expand to the maximum available space, rather than
		 * using the optimal height.
		 */
		static GUIOption ExpandingHeight(GUILogicalUnit minimum = 0, GUILogicalUnit maximum = 0);

	private:
		friend struct GUISizeConstraints;
		friend class ScriptGUIOption;

		GUILogicalUnit mMinimum = 0;
		GUILogicalUnit mMaximum = 0;
		GUIOptionType mType = GUIOptionType::FixedWidth;
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

		operator const TInlineArray<GUIOption, 4>&() const { return mOptions; }

	private:
		friend struct GUISizeConstraints;

		TInlineArray<GUIOption, 4> mOptions;
	};

	/** @} */
} // namespace b3d
