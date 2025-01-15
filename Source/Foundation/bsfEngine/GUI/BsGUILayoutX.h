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
		void GetChildRelativeLayoutAreas(const Size2UI& layoutSize, Vector2I* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const override;

		/** @} */

	protected:
		void UpdateLayoutForChildren() override;
	};

	/** @} */
} // namespace bs
