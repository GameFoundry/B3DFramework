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

	/** Represents a horizontal layout that will layout out its child elements left to right. */
	class BS_EXPORT GUILayoutX final : public GUILayout
	{
	public:
		GUILayoutX() = default;
		GUILayoutX(const GUIDimensions& dimensions);
		~GUILayoutX() = default;

		/**	Creates a new horizontal layout. */
		static GUILayoutX* Create();

		/**
		 * Creates a new horizontal layout.
		 *
		 * @param[in]	options		Options that allow you to control how is the element positioned and sized.
		 */
		static GUILayoutX* Create(const GUIOptions& options);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**	Calculate optimal sizes of all child layout elements. */
		void UpdateOptimalLayoutSizesInternal() override;

		/** @copydoc GUIElementBase::_calculateLayoutSizeRange */
		LayoutSizeRange CalculateLayoutSizeRangeInternal() const override;

		/** @copydoc GUILayout::_getElementAreas */
		void GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const override;

		/** @} */

	protected:
		/** @copydoc GUIElementBase::_updateLayoutInternal */
		void UpdateLayoutInternalInternal(const GUILayoutData& data) override;
	};

	/** @} */
} // namespace bs
