//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DIGUIVectorPathBuilder.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Builds a vector path that draws a rectangle (optionally rounded) with a border. */
	class B3D_EXPORT GUIBackgroundVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

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
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

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
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

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
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

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
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

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
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIScrollHandleVectorPathBuilder* Get()
		{
			static const GUIScrollHandleVectorPathBuilder instance;
			return &instance;
		}
	};

	/** Builds a vector path that draws a resizable vertical scroll handle. */
	class B3D_EXPORT GUIResizableVerticalScrollHandleVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIResizableVerticalScrollHandleVectorPathBuilder* Get()
		{
			static const GUIResizableVerticalScrollHandleVectorPathBuilder instance;
			return &instance;
		}

		static constexpr float kResizableHandleSize = 6;
		static constexpr float kResizableHandlePadding = 2;
	};

	/** Builds a vector path that draws a resizable horizontal scroll handle. */
	class B3D_EXPORT GUIResizableHorizontalScrollHandleVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUIResizableHorizontalScrollHandleVectorPathBuilder* Get()
		{
			static const GUIResizableHorizontalScrollHandleVectorPathBuilder instance;
			return &instance;
		}

		static constexpr float kResizableHandleSize = 6;
		static constexpr float kResizableHandlePadding = 2;
	};

	/** Builds a vector path that draws a separator. */
	class B3D_EXPORT GUISeparatorVectorPathBuilder : public IGUIVectorPathBuilder
	{
	public:
		HVectorPath BuildPath(const Size2I& size, const GUIStyleSheetRules& styleSheetRule) const override;

		/** Returns a singleton instance of this builder. */
		static const GUISeparatorVectorPathBuilder* Get()
		{
			static const GUISeparatorVectorPathBuilder instance;
			return &instance;
		}
	};


	/** @} */
} // namespace b3d
