//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUILayout.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Represents a GUI panel that you can use for free placement of GUI elements within its bounds. */
	class BS_EXPORT GUIPanel : public GUILayout
	{
	public:
		GUIPanel(INT16 depth, UINT16 depthRangeMin, UINT16 depthRangeMax, const GUIDimensions& dimensions);
		GUIPanel() {};
		~GUIPanel() {};

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
		void setDepthRange(INT16 depth = 0, UINT16 depthRangeMin = -1, UINT16 depthRangeMax = -1);

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
		static GUIPanel* create(INT16 depth = 0, UINT16 depthRangeMin = -1, UINT16 depthRangeMax = -1);

		/**
		 * Creates a new GUI panel.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 */
		static GUIPanel* create(const GUIOptions& options);

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
		static GUIPanel* create(INT16 depth, UINT16 depthRangeMin, UINT16 depthRangeMax, const GUIOptions& options);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementBase::_getType */
		virtual Type _getType() const override { return GUIElementBase::Type::Panel; }

		/**	Calculate optimal sizes of all child layout elements. */
		void _updateOptimalLayoutSizes() override;

		/** @copydoc GUIElementBase::_calculateLayoutSizeRange */
		virtual LayoutSizeRange _calculateLayoutSizeRange() const override;

		/** @copydoc GUILayout::_getElementAreas */
		void _getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
			const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const override;

		/** Calculates the size of the provided child within this layout with the provided dimensions. */
		Rect2I _getElementArea(const Rect2I& layoutArea, const GUIElementBase* element, const LayoutSizeRange& sizeRange) const;

		/**
		 * Calculates an element size range for the provided child of the GUI panel. Will return cached bounds so make sure
		 * to update optimal size ranges before calling.
		 */
		LayoutSizeRange _getElementSizeRange(const GUIElementBase* element) const;

		/** Assigns the specified layout information to a child element of a GUI panel. */
		void _updateChildLayout(GUIElementBase* element, const GUILayoutData& data);

		/** @copydoc GUIElementBase::_updateLayoutInternal */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/**
		 * Updates the provided depth range by taking into consideration the depth range of the panel. This depth range
		 * should be passed on to child elements of the panel.
		 */
		void _updateDepthRange(GUILayoutData& data);

		/** @} */

	protected:
		INT16 mDepthOffset;
		UINT16 mDepthRangeMin;
		UINT16 mDepthRangeMax;
	};

	/** @} */
}