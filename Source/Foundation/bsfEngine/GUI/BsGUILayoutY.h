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
	class B3D_EXPORT GUILayoutY final : public GUILayout
	{
	public:
		GUILayoutY() = default;
		GUILayoutY(const GUISizeConstraints& sizeConstraints);
		~GUILayoutY() = default;

		/**	Creates a new vertical layout. */
		static GUILayoutY* Create();

		/**
		 * Creates a new vertical layout.
		 *
		 * @param[in]	options		Options that allow you to control how is the element positioned and sized.
		 */
		static GUILayoutY* Create(const GUIOptions& options);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		void UpdateOptimalLayoutSizes() override;
		GUIConstrainedSize CalculateConstrainedSize() const override;
		void GetChildRelativeLayoutAreas(const Size2UI& layoutSize, Vector2I* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const override;
		void UpdateLayoutForChildren() override;

		/** @} */
	};

	/** @} */
} // namespace bs
