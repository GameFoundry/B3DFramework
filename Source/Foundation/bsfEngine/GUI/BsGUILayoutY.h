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

	/**	Represents a vertical layout that will layout out its child elements top to bottom. */
	class BS_EXPORT GUILayoutY final : public GUILayout
	{
	public:
		GUILayoutY() = default;
		GUILayoutY(const GUIDimensions& dimensions);
		~GUILayoutY() = default;

		/**	Creates a new vertical layout. */
		static GUILayoutY* create();

		/**
		 * Creates a new vertical layout.
		 *
		 * @param[in]	options		Options that allow you to control how is the element positioned and sized.
		 */
		static GUILayoutY* create(const GUIOptions& options);

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