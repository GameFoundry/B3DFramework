//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUILayout.h"
#include "B3DGUIConstructionMethods.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Represents a horizontal layout that will layout out its child elements left to right. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUILayoutX final : public GUILayout, public TGUIConstructionMethodsWithoutContent<GUILayoutX>
	{
	public:
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct { };

		/** @} */

		GUILayoutX() = default;
		GUILayoutX(PrivatelyConstruct, const String& styleClass, const GUISizeConstraints& sizeConstraints);
		~GUILayoutX() = default;

		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& GetGuiTypeName();

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
		 * @param	layoutSize				Size of the parent layout area to position the child elements in.
		 * @param	outElementPositions		Array to hold output positions. Must be the same size as the number of child elements.
		 * @param	outElementSizes			Array to hold output areas. Must be the same size as the number of child elements.
		 * @param	elementCount			Size of the element positions/sizes arrays.
		 * @param	sizeRanges				Ranges of possible sizes used for the child elements. Array must be same size as elements array.
		 * @param	myOptimalSize			Optimal size of this element with all the child elements.
		 */
		void GetChildRelativeLayoutAreas(const GUILogicalSize& layoutSize, GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSizeRange>& sizeRanges, const GUILogicalSize& myOptimalSize) const;
	};

	/** @} */
} // namespace b3d
