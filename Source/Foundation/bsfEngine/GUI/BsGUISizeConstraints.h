//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Math/BsVector2I.h"

namespace bs
{
	class GUIOption;
	struct GUIStyleSheetRules;
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Contains valid size range for a GUI element, based on element's optimal size and size constraints. */
	struct B3D_EXPORT GUIConstrainedSize
	{
		Vector2I Optimal; /**< Optimal GUI element size, constrained by the size constraints. */
		Vector2I Min; /**< In case of flexible size, minimum allowed size. Equivalent to Optimal if size is fixed. */
		Vector2I Max; /**< In case of flexible size, maximum allowed size. Equivalent to Optimal if size is fixed. If 0, the dimension has no maximum limit. */
	};

	/**	Flags that identify the type of data stored in a GUIDimensions structure. */
	enum class GUISizeConstraintFlag
	{
		None,
		FixedWidth = 1 << 0,
		FixedHeight = 1 << 1,
		WidthOverridenAtRuntime = 1 << 2,
		HeightOverridenAtRuntime = 1 << 3,
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

		/** Updates constraints from the provided style. If user has not manually set a specific constraint, that property will be inherited from style. */
		void UpdateWithStyle(const GUIElementStyle* style); // TODO: Deprecated

		/** Updates constraints from the provided style sheet rule. If user has not manually set a specific constraint, that property will be inherited from the rule. */
		void UpdateWithStyleSheetRule(const GUIStyleSheetRules& rule);

		/** Constrains the provided optimal element size based on active constraints. */
		GUIConstrainedSize CalculateConstrainedSize(const Vector2I& optimalSize) const;

		/**	Checks if the user has overriden the height at runtime. This means that width options set by the style will be ignored. */
		bool IsHeightOverridenAtRuntime() const { return Flags.IsSet(GUISizeConstraintFlag::HeightOverridenAtRuntime); }

		/**	Checks if the user has overriden the width at runtime. This means that height options set by the style will be ignored. */
		bool IsWidthOverridenAtRuntime() const { return Flags.IsSet(GUISizeConstraintFlag::WidthOverridenAtRuntime); }

		/**	Checks do the dimensions contain fixed width. */
		bool IsWidthFixed() const { return Flags.IsSet(GUISizeConstraintFlag::FixedWidth); }

		/**	Checks do the dimensions contain fixed height. */
		bool IsHeightFixed() const { return Flags.IsSet(GUISizeConstraintFlag::FixedHeight); }

		i32 X = 0; // TODO - Move position elsewhere
		i32 Y = 0;

		u32 MinWidth = 0;
		u32 MaxWidth = 0;
		u32 MinHeight = 0;
		u32 MaxHeight = 0;
		GUISizeConstraintFlags Flags;
	};

	/** @} */
} // namespace bs
