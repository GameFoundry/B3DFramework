//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DSize2.h"
#include "B3DGUIUnits.h"

namespace b3d
{
	class GUIOption;
	struct GUIStyleSheetRules;

	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Contains valid size range for a GUI element, based on element's optimal size and size constraints. */
	struct B3D_EXPORT GUIConstrainedSizeRange
	{
		GUILogicalSize Optimal{kZeroTag}; /**< Optimal GUI element size, constrained by the size constraints. */
		GUILogicalSize Minimum{kZeroTag}; /**< In case of flexible size, minimum allowed size. Equivalent to Optimal if size is fixed. */
		GUILogicalSize Maximum{kZeroTag}; /**< In case of flexible size, maximum allowed size. Equivalent to Optimal if size is fixed. If 0, the dimension has no maximum limit. */

		/**
		 * Calculates GUI element size constrained by parent size. For example, if a GUI element optimal width is 100 units, but parent is only 50 units wide, then the optimal
		 * size may be reduced to 50 units, if the constraints permit it.
		 *
		 * @param	sizeConstraints	Size constrains this size range was originally created with. New size will respect these constraints (e.g. if width is fixed, parent width will have no impact).
		 * @param	parentSize		Size of the parent element to constrain the current size within.
		 */
		GUILogicalSize CalculateSizeConstrainedByParentSize(const GUISizeConstraints& sizeConstraints, const GUILogicalSize& parentSize) const;
	};

	/**	Flags that identify the type of data stored in a GUIDimensions structure. */
	enum class GUISizeConstraintFlag
	{
		None,
		FixedWidth = 1 << 0,
		FixedHeight = 1 << 1,
		WidthOverridenAtRuntime = 1 << 2,
		HeightOverridenAtRuntime = 1 << 3,
		ExpandingWidth = 1 << 4,
		ExpandingHeight = 1 << 5
	};

	using GUISizeConstraintFlags = Flags<GUISizeConstraintFlag>;
	B3D_FLAGS_OPERATORS(GUISizeConstraintFlag)

	/**	Options that control how is a GUI element size constrained */
	struct B3D_EXPORT GUISizeConstraints
	{
		/**	Creates new object with no constraints. */
		static GUISizeConstraints Create();

		/**	Creates new constraints from user defined options. */
		static GUISizeConstraints Create(const GUIOptions& options);

		/**	Creates new constraints from user defined options. */
		static GUISizeConstraints Create(const TInlineArray<GUIOption, 4>& options);

		GUISizeConstraints() = default;

		/** Updates constraints from the provided style sheet rule. If user has not manually set a specific constraint, that property will be inherited from the rule. */
		void UpdateWithStyleSheetRule(const GUIStyleSheetRules& rule);

		/** Constrains the provided optimal element size based on active constraints. */
		GUILogicalSize CalculateConstrainedOptimalSize(const GUILogicalSize& unconstrainedOptimalSize) const;

		/** Constrains the provided optimal element size based on active constraints. */
		GUIConstrainedSizeRange CalculateConstrainedSizeRange(const GUILogicalSize& unconstrainedOptimalSize) const;

		/**	Checks do the constraint contain fixed width. */
		bool IsWidthFixed() const { return Flags.IsSet(GUISizeConstraintFlag::FixedWidth); }

		/**	Checks do the constraint contains fixed height. */
		bool IsHeightFixed() const { return Flags.IsSet(GUISizeConstraintFlag::FixedHeight); }

		/** Returns true if the GUI element will attempt to expand to fill all available width. */
		bool IsWidthExpanding() const { return Flags.IsSet(GUISizeConstraintFlag::ExpandingWidth); }

		/** Returns true if the GUI element will attempt to expand to fill all available height. */
		bool IsHeightExpanding() const { return Flags.IsSet(GUISizeConstraintFlag::ExpandingHeight); }

		GUILogicalPoint ExplicitPosition; // TODO - Move position elsewhere

		GUILogicalUnit MinimumWidth = 0;
		GUILogicalUnit MaximumWidth = 0;
		GUILogicalUnit MinimumHeight = 0;
		GUILogicalUnit MaximumHeight = 0;
		float FlexibleWidthWeight = 1.0f;
		float FlexibleHeightWeight = 1.0f;
		GUISizeConstraintFlags Flags;
	};

	/** @} */
} // namespace b3d
