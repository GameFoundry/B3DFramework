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

	/** Represents a horizontal layout that will layout out its child elements left to right. */
	class BS_EXPORT GUILayoutX : public GUILayout
	{
	public:
		GUILayoutX(const GUIDimensions& dimensions);
		GUILayoutX() {};
		~GUILayoutX() {};

		/**	Creates a new horizontal layout. */
		static GUILayoutX* create();

		/**
		 * Creates a new horizontal layout.
		 *
		 * @param[in]	options		Options that allow you to control how is the element positioned and sized.
		 */
		static GUILayoutX* create(const GUIOptions& options);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**	Calculate optimal sizes of all child layout elements. */
		void _updateOptimalLayoutSizes() override;

		/** @copydoc GUIElementBase::_calculateLayoutSizeRange */
		LayoutSizeRange _calculateLayoutSizeRange() const override;

		/** @copydoc GUILayout::_getElementAreas */
		void _getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
			const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const override;

		/** @} */

	protected:
		/** @copydoc GUIElementBase::_updateLayoutInternal */
		void _updateLayoutInternal(const GUILayoutData& data) override;
	};

	/** @} */
}