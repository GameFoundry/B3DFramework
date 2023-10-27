//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsIGUIVectorPathBuilder.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Builds a vector path that draws a rectangle (optionally rounded) with a border. */
	class B3D_EXPORT GUIBackgroundVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIBackgroundVectorPathBuilder* Get()
		{
			static const GUIBackgroundVectorPathBuilder instance;
			return &instance;
		}
	};

	/** Builds a vector path that draws a checkmark. */
	class B3D_EXPORT GUICheckmarkVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUICheckmarkVectorPathBuilder* Get()
		{
			static const GUICheckmarkVectorPathBuilder instance;
			return &instance;
		}
	};

	/** Builds a vector path that draws a tab button background. */
	class B3D_EXPORT GUITabBackgroundVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUITabBackgroundVectorPathBuilder* Get()
		{
			static const GUITabBackgroundVectorPathBuilder instance;
			return &instance;
		}
	};

	/** Builds a vector path that draws a drop down arrow. */
	class B3D_EXPORT GUIDropDownArrowVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIDropDownArrowVectorPathBuilder* Get()
		{
			static const GUIDropDownArrowVectorPathBuilder instance;
			return &instance;
		}
	};

	/** Builds a vector path that draws a scroll arrow pointing down. */
	class B3D_EXPORT GUIScrollArrowVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIScrollArrowVectorPathBuilder* Get()
		{
			static const GUIScrollArrowVectorPathBuilder instance;
			return &instance;
		}
	};

	/** Builds a vector path that draws a scroll handle. */
	class B3D_EXPORT GUIScrollHandleVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2UI& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIScrollHandleVectorPathBuilder* Get()
		{
			static const GUIScrollHandleVectorPathBuilder instance;
			return &instance;
		}
	};


	/** @} */
} // namespace bs
