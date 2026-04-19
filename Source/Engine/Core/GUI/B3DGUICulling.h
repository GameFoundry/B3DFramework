//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIElement.h"
#include "Utility/B3DSpatialTree.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	struct GUIQuadTreeOptions
	{
		enum
		{
			LoosePadding = 16,
			MinimumElementsPerNode = 16,
			MaximumElementsPerNode = 64,
			MaximumDepth = 12
		};

		static simd::Area2 GetBounds(GUIElement* element, void* context)
		{
			const GUILayoutData& layoutData = element->GetLayoutData();

			const Vector2 relativePosition = layoutData.RelativePosition.To<i32>().To<float>();
			const Size2 size = layoutData.Size.To<float>();

			const Area2 area(relativePosition.X, relativePosition.Y, size.Width, size.Height);
			return simd::Area2(area);
		}

		static void SetElementId(GUIElement* element, const SpatialTreeElementId& id, void* context)
		{
			element->SetQuadTreeId(id);
		}
	};

	using GUIElementQuadTree = TQuadTree<GUIElement*, GUIQuadTreeOptions>;

	/** Object that maintains a list of GUI elements visible within a certain area. */
	class GUICulling
	{
		/** Information about GUI elements for culling purposes. */
		struct GUIElementCullInformation 
		{
			u8 LastVisibleQueryIndex = 255;
		};

	public:
		/**
		 * Builds a quad-tree from provided child elements and their current relative positions and size.
		 *
		 * @param	elements				Elements to register into the quad-tree.
		 * @param	maximumQuadTreeSize		Maximum allowed bounds of the internal quad tree.
		 */
		GUICulling(const TArrayView<GUIElement*>& elements, float maximumQuadTreeSize = 50000.0f);

		/** Recreates the internal quad-tree and adds all the provided elements. The elements should be the same as currently in the quad-tree. */
		void RebuildQuadTree(const TArrayView<GUIElement*>& elements);

		/** Traverses the built oct-tree and tags all elements within the provided bounds as visible, and the remainder as invisible. */
		void UpdateVisibleElements(const Area2& bounds);

		/** Sets up necessary information for culling the provided element. Should be called on every element added to the layout, if culling is enabled. */
		void RegisterElement(GUIElement* element);

		/** Removes the element from culling related data structures. Should be called before the element is removed from the layout, if culling is enabled. */
		void UnregisterElement(GUIElement* element);

		/** Clears quad-tree IDs and culling flags from all the child elements. */
		void ClearElements();

		/** Returns currently visible elements. */
		const TInlineArray<GUIElement*, 4>& GetVisibleElements() const { return mVisibleElements; }
		
	private:
		u8 mCullingQueryIndex = 0;
		UPtr<GUIElementQuadTree> mQuadTree;
		UnorderedMap<GUIElement*, GUIElementCullInformation> mNonCulledElements;
		TInlineArray<GUIElement*, 4> mVisibleElements;
		float mMaximumQuadTreeSize = 50000.0f;
	};

	/** @} */
} // namespace b3d
