//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Controls GUI element layout options, possibly by overriding the default options specified in GUI element style. 
	 * These options control GUI element placement and size in a GUI layout.
	 */
	class BS_EXPORT GUIOption
	{
		/**	Type of GUI element options. */
		enum class Type
		{
			FixedWidth,
			FlexibleWidth,
			FixedHeight,
			FlexibleHeight,
			Position
		};

	public:
		GUIOption();

		/**
		 * Constructs a GUI option notifying the GUI layout that this element should be positioned at this offset from the
		 * parent GUI panel. This option is ignored if element is part of a layout since it controls its placement.
		 */
		static GUIOption position(INT32 x, INT32 y);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a fixed width. This will override the 
		 * width property set in element style.
		 */
		static GUIOption fixedWidth(UINT32 value);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a flexible width with optional min/max
		 * constraints (value of 0 means no constraint). This will override the width property set in element style.
		 */
		static GUIOption flexibleWidth(UINT32 min = 0, UINT32 max = 0);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a fixed height. This will override the
		 * height property set in element style.
		 */
		static GUIOption fixedHeight(UINT32 value);

		/**
		 * Constructs a GUI option notifying the GUI layout that this element has a flexible height with optional min/max 
		 * constraints (value of 0 means no constraint). This will override the height property set in element style.
		 */
		static GUIOption flexibleHeight(UINT32 min = 0, UINT32 max = 0);

	private:
		friend struct GUIDimensions;

		UINT32 min, max;
		Type type;
	};

	/**	Container for a list of options used for controlling GUI element properties. */
	class BS_EXPORT GUIOptions
	{
	public:
		GUIOptions() { }

		GUIOptions(const GUIOption& e0)
		{			
			mOptions.push_back(e0);
		}

		GUIOptions(const GUIOption& e0, const GUIOption& e1)
		{
			mOptions.push_back(e0);
			mOptions.push_back(e1);
		}

		GUIOptions(const GUIOption& e0, const GUIOption& e1, const GUIOption& e2)
		{
			mOptions.push_back(e0);
			mOptions.push_back(e1);
			mOptions.push_back(e2);
		}

		GUIOptions(const GUIOption& e0, const GUIOption& e1, const GUIOption& e2,
			const GUIOption& e3)
		{
			mOptions.push_back(e0);
			mOptions.push_back(e1);
			mOptions.push_back(e2);
			mOptions.push_back(e3);
		}

		GUIOptions(const GUIOption& e0, const GUIOption& e1, const GUIOption& e2,
			const GUIOption& e3, const GUIOption& e4)
		{
			mOptions.push_back(e0);
			mOptions.push_back(e1);
			mOptions.push_back(e2);
			mOptions.push_back(e3);
			mOptions.push_back(e4);
		}

		/**	Adds a new option to the options list.  */
		void addOption(const GUIOption& option)
		{
			mOptions.push_back(option);
		}

	private:
		friend struct GUIDimensions;

		Vector<GUIOption> mOptions;
	};

	/** @} */
}