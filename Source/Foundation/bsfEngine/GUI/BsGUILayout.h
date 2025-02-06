//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "Utility/BsSpatialTree.h"

namespace bs
{
	/** @addtogroup Implementation
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

		static simd::Rect2 GetBounds(GUIElement* element, void* context)
		{
			const Vector2 relativePosition = element->GetLayoutData().RelativePosition.To<float>();
			const Size2 size = element->GetLayoutData().Size.ToFloat();

			const Rect2 area(relativePosition.X, relativePosition.Y, size.Width, size.Height);
			return simd::Rect2(area);
		}

		static void SetElementId(GUIElement* element, const SpatialTreeElementId& id, void* context)
		{
			element->SetQuadTreeId(id);
		}
	};

	using GUIElementQuadTree = TQuadTree<GUIElement*, GUIQuadTreeOptions>;

	/**
	 * Base class for layout GUI element. Layout element positions and sizes any child elements according to element styles
	 * and layout options.
	 */
	class B3D_EXPORT GUILayout : public GUIElement
	{
		/** Information about GUI elements for culling purposes. */
		struct GUIElementCullInformation 
		{
			u8 LastVisibleQueryIndex = 255;
		};

		using Super = GUIElement;
	public:
		GUILayout(const GUISizeConstraints& dimensions);
		GUILayout() = default;
		virtual ~GUILayout() = default;

		/**	Creates a new element and adds it to the layout after all existing elements. */
		template <class Type, class... Args>
		Type* AddNewElement(Args&&... args)
		{
			Type* elem = Type::Create(std::forward<Args>(args)...);
			AddElement(elem);
			return elem;
		}

		/**	Creates a new element and inserts it before the element at the specified index. */
		template <class Type, class... Args>
		Type* InsertNewElement(u32 idx, Args&&... args)
		{
			Type* elem = Type::Create(std::forward<Args>(args)...);
			InsertElement(idx, elem);
			return elem;
		}

		/**	Adds a new element to the layout after all existing elements. */
		void AddElement(GUIElement* element);

		/**	Removes the specified element from the layout. */
		void RemoveElement(GUIElement* element);

		/**	Removes a child element at the specified index. */
		void RemoveElementAt(u32 idx);

		/**	Inserts a GUI element before the element at the specified index. */
		void InsertElement(u32 idx, GUIElement* element);

		/** Removes all child elements and destroys them. */
		void Clear();

		/**
		 * Enables/disables culling of child elements. If culling is enabled all child elements that are fully outside of the parent visible bounds will be marked as culled.
		 * Culled elements will never have their contents or mesh updated, their absolute coordinate will not be updated and they wont be drawn
		 * This is useful for layouts with a large amount of children, but comes with an overhead so it is disabled by default. Note this has no impact on layout update,
		 * which may still be expensive with many elements.
		 */
		void SetEnableCulling(bool enable);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		GUIConstrainedSize GetConstrainedSize() const override { return GetCachedConstrainedSize(); }

		/** Returns a size range that was cached during the last GUIElementBase::_updateOptimalLayoutSizes call. */
		GUIConstrainedSize GetCachedConstrainedSize() const { return mConstrainedSize; }

		/**
		 * Returns a size ranges for all children that was cached during the last GUIElementBase::_updateOptimalLayoutSizes
		 * call.
		 */
		const Vector<GUIConstrainedSize>& GetChildrenConstrainedSizes() const { return mChildrenConstrainedSizes; }

		Vector2I CalculateUnconstrainedOptimalSize() const override { return mConstrainedSize.Optimal; }
		Type GetType() const override { return GUIElement::Type::Layout; }

		/** @} */

	protected:
		/** Builds a quad-tree from all child elements and their current relative positions and size. If quad-tree already exists it is rebuilt. */
		void RebuildQuadTree();

		/** Sets up necessary information for culling the provided element. Should be called on every element added to the layout, if culling is enabled. */
		void RegisterElementForCulling(GUIElement* element);

		/** Removes the element from culling related data structures. Should be called before the element is removed from the layout, if culling is enabled. */
		void UnregisterElementFromCulling(GUIElement* element);

		/** Clears quad-tree IDs from all the child elements. */
		void ClearElementCullInformation();

		const TInlineArray<GUIElement*, 4>& GetVisibleChildren() const override { return mIsCullingEnabled ? mVisibleElements : Super::GetVisibleChildren(); }
		void RegisterChildElement(GUIElement* element) override;
		void UnregisterChildElement(GUIElement* element) override;
		void UpdateAbsoluteCoordinatesForChildren() override;

	protected:
		static constexpr float kMaximumQuadtreeSize = 50000.0f;

		Vector<GUIConstrainedSize> mChildrenConstrainedSizes;
		GUIConstrainedSize mConstrainedSize;
		bool mIsCullingEnabled = false;
		u8 mCullingQueryIndex = 0;
		UPtr<GUIElementQuadTree> mQuadTree;
		UnorderedMap<GUIElement*, GUIElementCullInformation> mNonCulledElements;
		TInlineArray<GUIElement*, 4> mVisibleElements;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUILayoutRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace bs
