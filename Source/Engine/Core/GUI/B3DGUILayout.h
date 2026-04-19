//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIElement.h"
#include "GUI/B3DGUICulling.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Base class for layout GUI element. Layout element positions and sizes any child elements according to element styles
	 * and layout options.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUILayout : public GUIElement
	{
		using Super = GUIElement;
	public:
		GUILayout() = default;
		GUILayout(const String& styleClass, const GUISizeConstraints& dimensions);
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
		Type* InsertNewElement(u32 index, Args&&... args)
		{
			Type* elem = Type::Create(std::forward<Args>(args)...);
			InsertElement(index, elem);
			return elem;
		}

		/**	Adds a new element to the layout after all existing elements. */
		B3D_SCRIPT_EXPORT()
		void AddElement(GUIElement* element);

		/**	Removes the specified element from the layout. */
		B3D_SCRIPT_EXPORT()
		void RemoveElement(GUIElement* element);

		/**	Removes a child element at the specified index. */
		B3D_SCRIPT_EXPORT()
		void RemoveElementAt(u32 index);

		/**	Inserts a GUI element before the element at the specified index. */
		B3D_SCRIPT_EXPORT()
		void InsertElement(u32 index, GUIElement* element);

		/** Returns the number of children in the layout. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ChildCount))
		u32 GetChildCount() const { return (u32)mChildren.Size(); }

		/** Returns a child element at the specified index, or null if the index is not valid. */
		B3D_SCRIPT_EXPORT()
		GUIElement* GetChild(u32 index) const { return index <= (u32)mChildren.Size() ? mChildren[index] : nullptr; }

		/** Removes all child elements and destroys them. */
		B3D_SCRIPT_EXPORT()
		void Clear();

		/**
		 * Enables/disables culling of child elements. If culling is enabled all child elements that are fully outside of the parent visible bounds will be marked as culled.
		 * Culled elements will never have their contents or mesh updated, their absolute coordinate will not be updated and they wont be drawn
		 * This is useful for layouts with a large amount of children, but comes with an overhead so it is disabled by default. Note this has no impact on layout update,
		 * which may still be expensive with many elements.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(EnableCulling))
		void SetEnableCulling(bool enable);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		GUIConstrainedSizeRange GetConstrainedSizeRange() const override { return mConstrainedSizeRange; }

		/**
		 * Returns a size ranges for all children that was cached during the last GUIElementBase::_updateOptimalLayoutSizes
		 * call.
		 */
		const Vector<GUIConstrainedSizeRange>& GetChildrenConstrainedSizes() const { return mChildConstrainedSizeRanges; }

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override { return mConstrainedSizeRange.Optimal; }
		void UpdateAbsoluteCoordinates(const GUIPhysicalPointF& parentOrigin, float parentScale, const GUIPhysicalAreaF& parentVisibleArea) override;
		const TInlineArray<GUIElement*, 4>& GetVisibleChildren() const override { return mCulling != nullptr ? mCulling->GetVisibleElements() : Super::GetVisibleChildren(); }

		/** @} */

	protected:
		void RegisterChildElement(GUIElement* element) override;
		void UnregisterChildElement(GUIElement* element) override;
		void UpdateAbsoluteCoordinatesForChildren() override;

	protected:
		Vector<GUIConstrainedSizeRange> mChildConstrainedSizeRanges;
		GUIConstrainedSizeRange mConstrainedSizeRange;
		UPtr<GUICulling> mCulling;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUILayoutRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
