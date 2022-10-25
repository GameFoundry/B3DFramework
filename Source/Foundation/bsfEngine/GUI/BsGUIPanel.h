//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUILayout.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Represents a GUI panel that you can use for free placement of GUI elements within its bounds. */
	class BS_EXPORT GUIPanel final : public GUILayout
	{
	public:
		GUIPanel() = default;
		GUIPanel(i16 depth, u16 depthRangeMin, u16 depthRangeMax, const GUIDimensions& dimensions);
		~GUIPanel() = default;

		/**
		 * Changes values that control at which depth is GUI panel and its children rendered.
		 *
		 * @param[in]	depth			Determines rendering order of the GUI panel. Panels with lower depth will be
		 *								rendered in front of panels with higher depth. Provided depth is relative to depth
		 *								of the parent GUI panel (if any).
		 * @param[in]	depthRangeMin	Minimum range of depths that children of this GUI panel can have. If any panel has
		 *								depth outside of the range [depth - depthRangeMin, depth + depthRangeMax] it will
		 *								be clamped to nearest extreme. Value of -1 means infinite range.
		 * @param[in]	depthRangeMax	Maximum range of depths that children of this GUI panel can have. If any panel has
		 *								depth outside of the range [depth - depthRangeMin, depth + depthRangeMax] it will
		 *								be clamped to nearest extreme. Value of -1 means infinite range.
		 */
		void SetDepthRange(i16 depth = 0, u16 depthRangeMin = -1, u16 depthRangeMax = -1);

		/**
		 * Creates a new GUI panel.
		 *
		 * @param[in]	depth			Determines rendering order of the GUI panel. Panels with lower depth will be
		 *								rendered in front of panels with higher depth. Provided depth is relative to depth
		 *								of the parent GUI panel (if any).
		 * @param[in]	depthRangeMin	Minimum range of depths that children of this GUI panel can have. If any panel has
		 *								depth outside of the range [depth - depthRangeMin, depth + depthRangeMax] it will
		 *								be clamped to nearest extreme. Value of -1 means infinite range.
		 * @param[in]	depthRangeMax	Maximum range of depths that children of this GUI panel can have. If any panel has
		 *								depth outside of the range [depth - depthRangeMin, depth + depthRangeMax] it will
		 *								be clamped to nearest extreme. Value of -1 means infinite range.
		 */
		static GUIPanel* Create(i16 depth = 0, u16 depthRangeMin = -1, u16 depthRangeMax = -1);

		/**
		 * Creates a new GUI panel.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 */
		static GUIPanel* Create(const GUIOptions& options);

		/**
		 * Creates a new GUI panel.
		 *
		 * @param[in]	depth			Determines rendering order of the GUI panel. Panels with lower depth will be
		 *								rendered in front of panels with higher depth. Provided depth is relative to depth
		 *								of the parent GUI panel (if any).
		 * @param[in]	depthRangeMin	Minimum range of depths that children of this GUI panel can have. If any panel has
		 *								depth outside of the range [depth - depthRangeMin, depth + depthRangeMax] it will be
		 *								clamped to nearest extreme. Value of -1 means infinite range.
		 * @param[in]	depthRangeMax	Maximum range of depths that children of this GUI panel can have. If any panel has
		 *								depth outside of the range [depth - depthRangeMin, depth + depthRangeMax] it will be
		 *								clamped to nearest extreme. Value of -1 means infinite range.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 */
		static GUIPanel* Create(i16 depth, u16 depthRangeMin, u16 depthRangeMax, const GUIOptions& options);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementBase::_getType */
		Type GetTypeInternal() const override { return GUIElementBase::Type::Panel; }

		/**	Calculate optimal sizes of all child layout elements. */
		void UpdateOptimalLayoutSizesInternal() override;

		/** @copydoc GUIElementBase::_calculateLayoutSizeRange */
		LayoutSizeRange CalculateLayoutSizeRangeInternal() const override;

		/** @copydoc GUILayout::_getElementAreas */
		void GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const override;

		/** Calculates the size of the provided child within this layout with the provided dimensions. */
		Rect2I GetElementAreaInternal(const Rect2I& layoutArea, const GUIElementBase* element, const LayoutSizeRange& sizeRange) const;

		/**
		 * Calculates an element size range for the provided child of the GUI panel. Will return cached bounds so make sure
		 * to update optimal size ranges before calling.
		 */
		LayoutSizeRange GetElementSizeRangeInternal(const GUIElementBase* element) const;

		/** Assigns the specified layout information to a child element of a GUI panel. */
		void UpdateChildLayoutInternal(GUIElementBase* element, const GUILayoutData& data);

		/** @copydoc GUIElementBase::_updateLayoutInternal */
		void UpdateLayoutInternalInternal(const GUILayoutData& data) override;

		/**
		 * Updates the provided depth range by taking into consideration the depth range of the panel. This depth range
		 * should be passed on to child elements of the panel.
		 */
		void UpdateDepthRangeInternal(GUILayoutData& data);

		/** @} */

	protected:
		i16 mDepthOffset;
		u16 mDepthRangeMin;
		u16 mDepthRangeMax;
	};

	/** @} */
} // namespace bs
