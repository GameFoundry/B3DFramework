//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUICulling.h"
#include "GUI/B3DGUISpriteHelper.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Base class for layout GUI element. Layout element positions and sizes any child elements according to element styles
	 * and layout options.
	 *
	 * A layout without a style class only arranges its children: it draws nothing, receives no input, is never registered
	 * with the parent widget and never consults the style sheet (see IsLayoutOnly()). Giving it a style class, or making it
	 * interactive, turns it into a `container` element in the style sheet, which draws a background, border, radius and
	 * shadow, insets its children by its padding and border, and can be restyled per state.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUILayout : public GUIInteractable
	{
		using Super = GUIInteractable;
	public:
		GUILayout();
		GUILayout(const String& styleClass, const GUISizeConstraints& dimensions);
		virtual ~GUILayout() = default;

		/** Layouts stay class-less when no style class is provided, which is what keeps them layout only. */
		static constexpr bool kUseTypeNameAsDefaultStyleClass = false;

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

		/**
		 * Makes the layout take part in pointer hit-testing, so that it tracks the `:hover`, `:active` and `:focus` states
		 * and raises OnClick, OnDoubleClick and OnContextMenu. Elements inside the layout are offered pointer events first,
		 * and the layout only receives the clicks none of them handled. 
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Interactive))
		void SetInteractive(bool interactive);

		/** @copydoc SetInteractive */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Interactive))
		bool IsInteractive() const { return mIsInteractive; }

		/** Toggles the `:checked` state of the layout, whether or not it is interactive. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Checked))
		void SetChecked(bool value);

		/** @copydoc SetChecked */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Checked))
		bool IsChecked() const { return mStateFlags.IsSet(GUIElementState::Checked); }

		/** Triggered when an interactive layout is clicked, unless an element inside it handled the click. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnClick;

		/** Triggered when an interactive layout is double clicked, unless an element inside it handled the click. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnDoubleClick;

		/** Triggered when an interactive layout is right clicked, unless an element inside it handled the click. */
		B3D_SCRIPT_EXPORT()
		Event<void()> OnContextMenu;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		StringView GetStyleSheetElement() const override { return "container"; }
		bool IsLayoutOnly() const override { return !mIsInteractive && mStyleClasses.Empty(); }

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
		void UpdateRenderElements() override;
		bool DoOnMouseEvent(const GUIMouseEvent& event) override;
		bool DoOnCommandEvent(const GUICommandEvent& event) override;
		void NotifyStyleChanged() override;

		/**
		 * Returns the area the child elements are laid out in, relative to the layout origin. This is the whole layout,
		 * minus the style padding and border if the layout is styled.
		 */
		GUILogicalArea GetChildLayoutArea() const;

		/** Grows the size of the child elements by the style padding and border, giving the size of the layout itself. */
		GUILogicalSize AddStylePaddingAndBorder(const GUILogicalSize& childrenSize) const;

		/**
		 * Returns the layout data to hand down to the child elements. This is the layout's own data, except that a styled
		 * layout moves its children one step towards the viewer, so that its own background is drawn behind them rather
		 * than at the same depth.
		 */
		GUILayoutData GetChildLayoutData() const;

	protected:
		Vector<GUIConstrainedSizeRange> mChildConstrainedSizeRanges;
		GUIConstrainedSizeRange mConstrainedSizeRange;
		TUnique<GUICulling> mCulling;

		GUIBackgroundSprite mBackgroundSprite;
		bool mIsInteractive = false;

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
