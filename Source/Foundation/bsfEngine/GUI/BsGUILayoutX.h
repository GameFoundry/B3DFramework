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
	class B3D_EXPORT GUILayoutX final : public GUILayout
	{
	public:
		GUILayoutX() = default;
		GUILayoutX(const GUISizeConstraints& dimensions);
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

		void UpdateOptimalLayoutSizes() override;

		/** @} */

	protected:
		void UpdateLayoutForChildren() override;

		/**
		 * Calculates positions & sizes of all elements in the layout. This method expects a pre-allocated array to store
		 * the data in.
		 *
		 * @param	layoutSize			Size of the parent layout area to position the child elements in.
		 * @param	outElementPositions	Array to hold output positions. Must be the same size as the number of child elements.
		 * @param	outElementSizes		Array to hold output areas. Must be the same size as the number of child elements.
		 * @param	elementCount		Size of the element positions/sizes arrays.
		 * @param	sizeRanges			Ranges of possible sizes used for the child elements. Array must be same size as elements array.
		 * @param	mySizeRange			Size range of this element.
		 */
		void GetChildRelativeLayoutAreas(const Size2UI& layoutSize, GUILogicalPoint* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const;
	};

	/** @} */
} // namespace bs
