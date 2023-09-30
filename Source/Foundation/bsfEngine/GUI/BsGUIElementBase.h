//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUILayoutData.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector2I.h"
#include "Utility/BsRectOffset.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Base class for all GUI elements (visible or layout). */
	class B3D_EXPORT GUIElementBase
	{
	public:
		/**	Valid types of GUI base elements. */
		enum class Type
		{
			Layout,
			Element,
			FixedSpace,
			FlexibleSpace,
			Panel
		};

	protected:
		/**	Flags that signal the state of the GUI element. */
		enum GUIElementFlags
		{
			GUIElem_Dirty = 0x01,
			GUIElem_Hidden = 0x02,
			GUIElem_Inactive = 0x04,
			GUIElem_HiddenSelf = 0x08,
			GUIElem_InactiveSelf = 0x10,
			GUIElem_Disabled = 0x20,
			GUIElem_DisabledSelf = 0x40
		};

	public:
		GUIElementBase() = default;
		GUIElementBase(const GUIDimensions& dimensions);
		virtual ~GUIElementBase();

		/**
		 * Sets element position relative to parent GUI panel.
		 *
		 * @note
		 * Be aware that this value will be ignored if GUI element is part of a layout since then the layout controls its
		 * placement.
		 */
		void SetPosition(i32 x, i32 y);

		/**	Sets element width in pixels.  */
		void SetWidth(u32 width);

		/**
		 * Sets element width in pixels. Element will be resized according to its contents and parent layout but will
		 * always stay within the provided range. If maximum width is zero, the element is allowed to expand as much as
		 * it needs.
		 */
		void SetFlexibleWidth(u32 minWidth = 0, u32 maxWidth = 0);

		/**	Sets element height in pixels. */
		void SetHeight(u32 height);

		/** Sets width and height of a GUI element in pixels. */
		void SetSize(u32 width, u32 height);

		/**
		 * Sets element height in pixels. Element will be resized according to its contents and parent layout but will
		 * always stay within the provided range. If maximum height is zero, the element is allowed to expand as much as
		 * it needs.
		 */
		void SetFlexibleHeight(u32 minHeight = 0, u32 maxHeight = 0);

		/**	Resets element dimensions to their initial values dictated by the element's style. */
		virtual void ResetDimensions();

		/**
		 * Hides or shows this element and recursively applies the same state to all the child elements. This will not
		 * remove the element from the layout, the room for it will still be reserved but it just won't be visible.
		 */
		void SetVisible(bool visible);

		/**
		 * Activates or deactives this element and recursively applies the same state to all the child elements. This has
		 * the same effect as setVisible(), but when disabled it will also remove the element from the layout, essentially
		 * having the same effect is if you destroyed the element.
		 */
		void SetActive(bool active);

		/** Disables or enables the element. Disabled elements cannot be interacted with and have a faded out appearance. */
		void SetDisabled(bool disabled);

		/**
		 * Returns non-clipped bounds of the GUI element. Relative to a parent GUI panel.
		 *
		 * @param[in]	relativeTo	Parent panel of the provided element relative to which to return the bounds. If null
		 *							the bounds relative to the first parent panel are returned. Behavior is undefined if
		 *							provided panel is not a parent of the element.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty.
		 */
		Rect2I GetBounds(GUIPanel* relativeTo = nullptr);

		/**
		 * Sets the bounds of the GUI element. Relative to a parent GUI panel. Equivalent to calling SetPosition(),
		 * setWidth() and setHeight().
		 */
		void SetBounds(const Rect2I& bounds);

		/**
		 * Returns non-clipped bounds of the GUI element. Relative to a parent GUI widget.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty.
		 */
		Rect2I GetGlobalBounds();

		/**
		 * Returns non-clipped bounds of the GUI element in screenspace.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty.
		 */
		Rect2I GetScreenBounds() const;

		/**
		 * Returns non-clipped visible bounds of the GUI element (bounds exclude the margins). Relative to the parent GUI
		 * panel.
		 *
		 * @note	This call can be potentially expensive as the bounds need to be calculated based on current GUI state.
		 */
		virtual Rect2I GetVisibleBounds();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Updates child elements positions, sizes, clip rectangles and depths so they fit into the provided bounds, while
		 * respecting their layout options.
		 *
		 * @param[in]	data	Layout data containing the necessary bounds and restrictions to use for calculating the
		 *						child element layout data.
		 */
		virtual void UpdateLayout(const GUILayoutData& data);

		/** Calculates optimal sizes of all child elements, as determined by their style and layout options. */
		virtual void UpdateOptimalLayoutSizes();

		/** @copydoc UpdateLayout */
		virtual void UpdateLayoutRecursive(const GUILayoutData& data);

		/**
		 * Calculates positions & sizes of all elements in the layout. This method expects a pre-allocated array to store
		 * the data in.
		 *
		 * @param[in]	layoutArea		Parent layout area to position the child elements in.
		 * @param[out]	elementAreas	Array to hold output areas. Must be the same size as the number of child elements.
		 * @param[in]	numElements		Size of the element areas array.
		 * @param[in]	sizeRanges		Ranges of possible sizes used for the child elements. Array must be same size as
		 *								elements array.
		 * @param[in]	mySizeRange		Size range of this element.
		 */
		virtual void GetChildLayoutAreas(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const;

		/** Updates layout data that determines GUI elements final position & depth in the GUI widget. */
		virtual void SetLayoutData(const GUILayoutData& data) { mLayoutData = data; }

		/** Retrieves layout data that determines GUI elements final position & depth in the GUI widget. */
		const GUILayoutData& GetLayoutData() const { return mLayoutData; }

		/**	Sets a new parent for this element. */
		void SetParent(GUIElementBase* parent);

		/**	Returns number of child elements. */
		u32 GetChildCount() const { return (u32)mChildren.size(); }

		/**	Return the child element at the specified index.*/
		GUIElementBase* GetChild(u32 idx) const { return mChildren[idx]; }

		/**	Returns previously calculated optimal size for this element. */
		virtual Vector2I GetOptimalSize() const = 0;

		/**	Returns layout options that determine how is the element positioned and sized. */
		const GUIDimensions& GetDimensions() const { return mDimensions; }

		/**	Calculates element size range constrained by its layout options. */
		virtual LayoutSizeRange CalculateLayoutSizeRange() const;

		/**
		 * Returns element size range constrained by its layout options. This is different from CalculateLayoutSizeRangeInternal()
		 * because this method may return cached size range.
		 */
		virtual LayoutSizeRange GetLayoutSizeRange() const;

		/**
		 * Returns element padding that determines how far apart to space out this element from other elements in a layout.
		 */
		virtual const RectOffset& GetPadding() const = 0;

		/**	Returns specific sub-type of this object. */
		virtual Type GetType() const = 0;

		/**	Returns parent GUI base element. */
		GUIElementBase* GetParent() const { return mParent; }

		/**
		 * Returns the parent element whose layout needs to be updated when this elements contents change.
		 *
		 * @note
		 * Due to the nature of the GUI system, when a child element bounds or contents change, its parents and siblings
		 * usually need their layout bound updated. This function returns the first parent of all the elements that require
		 * updating. This parent usually has fixed bounds or some other property that allows its children to be updated
		 * independently from the even higher-up elements.
		 */
		GUIElementBase* GetUpdateParent() const { return mLayoutUpdateParent; }

		/**	Returns parent GUI widget, can be null. */
		GUIWidget* GetParentWidget() const { return mParentWidget; }

		/**	Checks if element is visible or hidden. */
		bool IsVisible() const { return (mFlags & GUIElem_Hidden) == 0; }

		/**
		 * Checks if element is active or inactive. Inactive elements are not visible, don't take up space
		 * in their parent layouts, and can't be interacted with.
		 */
		bool IsActive() const { return (mFlags & GUIElem_Inactive) == 0; }

		/** Checks if element is disabled. Disabled elements cannot be interacted with and have a faded out appearance. */
		bool IsDisabled() const { return (mFlags & GUIElem_Disabled) != 0; }

		/**
		 * Internal version of setVisible() that doesn't modify local visibility, instead it is only meant to be called
		 * on child elements of the element whose visibility was modified.
		 */
		void SetVisibleRecursive(bool visible);

		/**
		 * Internal version of setActive() that doesn't modify local state, instead it is only meant to be called
		 * on child elements of the element whose state was modified.
		 *
		 * @copydoc SetActive
		 */
		void SetActiveRecursive(bool active);

		/**
		 * Internal version of setDisabled() that doesn't modify local state, instead it is only meant to be called
		 * on child elements of the element whose state was modified.
		 *
		 * @copydoc SetDisabled
		 */
		void SetDisabledRecursive(bool disabled);

		/**
		 * Changes the active GUI element widget. This allows you to move an element to a different viewport, or change
		 * element style by using a widget with a different skin. You are allowed to pass null here, but elements with no
		 * parent will be unmanaged. You will be responsible for deleting them manually, and they will not render anywhere.
		 */
		virtual void ChangeParentWidget(GUIWidget* widget);

		/**Registers a new child element. */
		void RegisterChildElement(GUIElementBase* element);

		/**	Unregisters an existing child element. */
		void UnregisterChildElement(GUIElementBase* element);

		/**	Checks if element has been destroyed and is queued for deletion. */
		virtual bool IsDestroyed() const { return false; }

		/**	Marks the element's dimensions as dirty, triggering a layout rebuild. */
		void MarkLayoutAsDirty();

		/**	Marks the element's contents as dirty, which causes the sprite meshes to be recreated from scratch. */
		void MarkContentAsDirty();

		/**
		 * Mark only the elements that operate directly on the sprite mesh without requiring the mesh to be recreated as
		 * dirty. This includes position, depth and clip rectangle. This will cause the parent widget mesh to be rebuilt
		 * from its child element's meshes.
		 */
		void MarkMeshAsDirty();

		/**	Returns true if elements contents have changed since last update. */
		bool IsDirty() const { return (mFlags & GUIElem_Dirty) != 0; }

		/**	Marks the element contents to be up to date (meaning it's processed by the GUI system). */
		void MarkAsClean();

		/** @} */

	protected:
		/**	Finds anchor and update parents and recursively assigns them to all children. */
		void UpdatePanelAndLayoutUpdateParents();

		/**	Refreshes update parents of all child elements. */
		void RefreshLayoutUpdateParentsForChildren();

		/**
		 * Finds the first parent element whose size doesn't depend on child sizes.
		 *
		 * @note
		 * This allows us to optimize layout updates and trigger them only on such parents when their child elements
		 * contents change, compared to doing them on the entire GUI hierarchy.
		 */
		GUIElementBase* FindLayoutUpdateParent();

		/**
		 * Helper method for recursion in UpdateAUParentsInternal(). Sets the provided anchor parent for all children recursively.
		 * Recursion stops when a child anchor is detected.
		 */
		void SetPanelParent(GUIPanel* panelParent);

		/**
		 * Helper method for recursion in UpdateAUParentsInternal(). Sets the provided update parent for all children recursively.
		 * Recursion stops when a child update parent is detected.
		 */
		void SetLayoutUpdateParent(GUIElementBase* layoutUpdateParent);

		/** Unregisters and destroys all child elements. */
		void DestroyChildElements();

		GUIWidget* mParentWidget = nullptr;
		GUIPanel* mPanelParent = nullptr; /**< First panel in the parent hierarchy, if any. */
		GUIElementBase* mLayoutUpdateParent = nullptr; /**< Parent on which we need to call layout update if this element's size changes. This will be the first parent GUI element that doesn't have fixed bounds. */

		GUIElementBase* mParent = nullptr; /**< Direct parent of this element. */
		TInlineArray<GUIElementBase*, 4> mChildren;

		u8 mFlags = GUIElem_Dirty;

		GUIDimensions mDimensions; /**< Wanted position and size, as requested by the style, or set explicitly at runtime. */
		GUILayoutData mLayoutData; /**< Calculated position, size, depth and other information, valid after a layout update. */
	};

	/** @} */
} // namespace bs
