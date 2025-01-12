//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUILayoutData.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector2I.h"
#include "Utility/BsRectOffset.h"

namespace bs
{
	struct GUIStyleSheetRuleset;
	struct GUIStyleSheetStateRulesets;
	class IGUIVectorPathBuilder;

	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Flags that signal the state of the GUI element. */
	enum class GUIElementInternalStateFlag 
	{
		LayoutDirty = 1 << 0, /**< GUI element is requesting layout update. Set if e.g. element's optimal size changes. */
		Hidden = 1 << 1, /**< GUI element is not visible (but may still take up space in the layout). */
		Inactive = 1 << 2, /**< GUI element is not active (is not visible and will not take up space in the layout). */
		HiddenSelf = 1 << 3, /**< Same as Hidden, but set only on the element that was explicitly hidden, while Hidden will also be set on all children of such element. */
		InactiveSelf = 1 << 4, /**< Same as Inactive, but set only on the element that was explicitly made inactive, while Inactive will also be set on all children of such element. */
		Disabled = 1 << 5, /**< GUI element is grayed out and cannot be interacted with. */
		DisabledSelf = 1 << 6, /**< Same as Disabled, but set only on the element that was explicitly disabled, while Disabled will also be set on all children of such element. */
		AbsoluteCoordinatesDirty = 1 << 7 /**< GUI element is requesting update for absolute coordinates of all its children. Set if e.g. scroll area is scrolled. */
	};

	using GUIElementInternalStateFlags = Flags<GUIElementInternalStateFlag>;
	B3D_FLAGS_OPERATORS(GUIElementInternalStateFlag)

	/**
	 * Contains style sheet rule for a GUI element, along with state rule for the particular state the GUI element is currently in.
	 * If used for pseudo-elements, also contains the name of the pseudo element the rule is for.
	 */
	struct GUIStyleSheetRuleInformation
	{
		GUIStyleSheetRuleInformation(const char* pseudoElementName = nullptr):
			PseudoElementName(pseudoElementName)
		{ }

		const char* PseudoElementName = nullptr; /**< Name of the pseudo-element, if the rule is for a pseudo-element. */
		SPtr<const GUIStyleSheetStateRulesets> StateRulesets; /**< Rulesets for all states for a particular pseudo-element. */
		SPtr<const GUIStyleSheetRuleset> CurrentStateRuleset; /**< Ruleset for the currently active state. */

		static const GUIStyleSheetRuleInformation kInvalid;
	};

	/**
	 * Base class for all GUI elements. Provides general functionality such as element size/position, as well as handling child/parent relationships.

	 * @note: Does not provide ability to render and interact with GUI elements - those are implemented by derived classes (i.e. GUIRenderable and GUIInteractable).
	 */
	class B3D_EXPORT GUIElement : public IReflectable, public IScriptExportable
	{
	public:
		/**	Valid types of GUI base elements. */
		enum class Type
		{
			Layout,
			Interactable,
			FixedSpace,
			FlexibleSpace,
			Panel
		};

	protected:
	public:
		GUIElement() = default;
		GUIElement(const GUISizeConstraints& dimensions);
		virtual ~GUIElement() = default;

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
		 * Returns non-clipped bounds of the GUI element. Relative to a parent GUI panel or the provided parent element.
		 * The bounds includes the content area, element padding and element border, but excludes element margins.

		 *
		 * @param	relativeTo	Parent panel of the provided element relative to which to return the bounds. If null
		 *						the bounds relative to the first parent panel are returned. Behavior is undefined if
		 *						provided panel is not a parent of the element.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty, as it can trigger a re-layouting operation.
		 */
		Rect2I CalculateBoundsRelativeTo(GUIElement* relativeTo = nullptr);

		/**
		 * Returns non-clipped bounds of the GUI element. Relative to a parent GUI widget. The bounds includes the content area,
		 * element padding and element border, but excludes element margins.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty, as it can trigger a re-layouting operation.
		 */
		Rect2I GetAbsoluteBounds() const;

		/**
		 * Returns non-clipped bounds of the GUI element in screenspace. The bounds includes the content area,
		 * element padding and element border, but excludes element margins.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty, as it can trigger a re-layouting operation.
		 */
		Rect2I GetScreenBounds() const;

		/** Same as GetBounds(), but never triggers a re-layouting pass, and instead always returns values from last layouting pass. */
		Rect2I GetCachedAbsoluteBounds() const { return Rect2I(mAbsolutePosition, mLayoutData.Size); }

		/**
		 * Returns the position of the GUI element relative to the parent widget.
		 *
		 * @note	This value is only updated during layout and/or absolute coordinate pass, which happens at the end of frame before GUI is drawn.
		 *			This means this value may contain position that is from the previous frame, unless you manually request a layout update before
		 *			calling this method.
		 */
		const Vector2I& GetCachedAbsolutePosition() const { return mAbsolutePosition; }

		/**
		 * Returns the position and size of the GUI element, relative to the parent widget. The returned area is clipped by the visible
		 * area as specified by the parent GUI element (e.g. if the parent is a scroll area or similar, only some or none of the GUI element may
		 * be visible, if it's scrolled out of view).
		 * 
		 * @note	This value is only updated during layout and/or absolute coordinate pass, which happens at the end of frame before GUI is drawn.
		 *			This means this value may contain position that is from the previous frame, unless you manually request a layout update before
		 *			calling this method.
		 */
		const Rect2I& GetCachedAbsoluteClippedArea() const { return mAbsoluteClippedArea; }

		/** Same as GetCachedAbsoluteClippedArea(), except the area is made relative to this GUI element. */
		Rect2I GetCachedLocalClippedArea() const;

		/**
		 * Sets the bounds of the GUI element. Relative to a parent GUI panel. Equivalent to calling SetPosition(),
		 * setWidth() and setHeight().
		 */
		void SetBounds(const Rect2I& bounds);

		/**
		 * Destroy the element. Removes it from parent and widget, and queues it for deletion. Element memory will be
		 * released delayed, next frame.
		 */
		virtual void Destroy();

		/** Checks if element is queued for deletion. */
		bool IsPendingDestroy() const { return mIsPendingDestroy; }

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Calculates optimal sizes of all child elements, as determined by their style and layout options. This is performed recursively
		 * over all child elements, starting with the bottom-most child element. This should be called before UpdateLayoutRecursive().
		 */
		virtual void UpdateOptimalLayoutSizes();

		/**
		 * Calculates element positions and sizes based on their options and layout. This is an expensive operation that requires
		 * multiple passes over all child GUI elements. The operation is performed in three passes:
		 *  1. Optimal sizes for all elements are determined. This is done for the bottom-most child first, and then for parent elements.
		 *	2. Layout calculations are performed for all elements, starting with the top-most element. This determines relative positions and
		 *	   final element size.
		 *	3. Absolute coordinate calculations are performed for all elements, starting with the top-most element. This adds parent coordinates
		 *	   to the element's local coordinates, and calculates the visible area of the element (area of the element as clipped by the parent,
		 *	   e.g. for elements in a scroll area that might not be fully visible).
		 */
		virtual void UpdateLayout();

		/** Calculates sizes and relative positions for all child elements. Should be preceded with a call to UpdateOptimalLayoutSizes(). */
		virtual void UpdateLayoutForChildren() { }

		/**
		 * Updates the absolute coordinates of the GUI element using the currently assigned relative coordinates and the provided
		 * @p parentOrigin. Also calculates the visible area clip rectangle and marks culled elements if they have no visible area.
		 * This should be called after updating the layout (as layout update calculates the needed relative coordinates).
		 * This may also be called independently of layout update, which is useful for scroll areas that then do not require
		 * a full layout pass to scroll their children.
		 * 
		 * @param parentOrigin			Absolute origin to add to the relative coordinates, in order to determine the absolute element coordinates.
		 * @param parentVisibleArea		Absolute visible (clipped) area though which this element may be seen. This will be used for culling and clipping.
		 */
		virtual void UpdateAbsoluteCoordinates(const Vector2I& parentOrigin, const Rect2I& parentVisibleArea);

		/** Calls UpdateAbsoluteCoordinates() on all child elements. */
		virtual void UpdateAbsoluteCoordinatesForChildren();

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
		virtual void GetChildRelativeLayoutAreas(const Size2UI& layoutSize, Vector2I* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const;

		/** Updates layout data that determines GUI elements relative position, size and depth in the GUI widget. */
		virtual void SetLayoutData(const GUILayoutData& data) { mLayoutData = data; }

		/** Resets the absolute clipped area to full width/height of the GUI element. */
		void ResetAbsoluteClippedArea() { mAbsoluteClippedArea = Rect2I(mAbsolutePosition.X, mAbsolutePosition.Y, mLayoutData.Size.Width, mLayoutData.Size.Height); }

		/** Retrieves layout data that determines GUI elements relative position, size and depth in the GUI widget. */
		const GUILayoutData& GetLayoutData() const { return mLayoutData; }

		/**	Sets a new parent for this element. */
		void SetParent(GUIElement* parent);

		/**	Returns number of child elements. */
		u32 GetChildCount() const { return (u32)mChildren.size(); }

		/**	Return the child element at the specified index.*/
		GUIElement* GetChild(u32 idx) const { return mChildren[idx]; }

		/**
		 * Returns all children that can be seen through the parent's visible area (i.e. all elements that are not culled or explicitly made invisible). Note this
		 * may return all child elements on GUI elements that do not support culling.
		 */
		virtual const TInlineArray<GUIElement*, 4>& GetVisibleChildren() const { return mChildren; }

		/**	Calculates the optimal size for the GUI element, ignoring size constraints. */
		virtual Vector2I CalculateUnconstrainedOptimalSize() const = 0;

		/**	Returns size constraints that determine how is the GUI element allowed to be resized by the layout. */
		const GUISizeConstraints& GetSizeConstraints() const { return mSizeConstraints; }

		/**	Calculates element size based on its optimal size constrained by its size constraint options. */
		virtual GUIConstrainedSize CalculateConstrainedSize() const;

		/** Returns element size constrained by its size constraints. This is different from CalculateConstrainedSize() because this method may return cached size. */
		virtual GUIConstrainedSize GetConstrainedSize() const;

		/**
		 * Returns GUI element margins. Margins are modified by changing element style and determines minimum distance
		 * between GUI element border and surrounding GUI elements.
		 */
		virtual const RectOffset& GetMargins() const;

		/**
		 * Returns GUI element padding. Padding is modified by changing element style and determines minimum distance
		 * between GUI element border and contents.
		 */
		virtual const RectOffset& GetPadding() const;

		/**	Returns specific sub-type of this object. */
		virtual Type GetType() const = 0;

		/**	Returns parent GUI base element. */
		GUIElement* GetParent() const { return mParent; }

		/**	Returns parent GUI widget, can be null. */
		GUIWidget* GetParentWidget() const { return mParentWidget; }

		/**	Checks if element is visible or hidden. */
		bool IsVisible() const { return !mFlags.IsSet(GUIElementInternalStateFlag::Hidden); }

		/**
		 * Checks if element is active or inactive. Inactive elements are not visible, don't take up space
		 * in their parent layouts, and can't be interacted with.
		 */
		bool IsActive() const { return !mFlags.IsSet(GUIElementInternalStateFlag::Inactive); }

		/** Checks if element is disabled. Disabled elements cannot be interacted with and have a faded out appearance. */
		bool IsDisabled() const { return mFlags.IsSet(GUIElementInternalStateFlag::Disabled); }

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
		void RegisterChildElement(GUIElement* element);

		/**	Unregisters an existing child element. */
		void UnregisterChildElement(GUIElement* element);

		/**	Marks the element's dimensions as dirty, triggering a layout rebuild. */
		void MarkLayoutAsDirty();

		/**
		 * Marks the element's absolute coordinates as dirty. This is usually true when the parent moves but relative
		 * positions/sizes of children remain unchanged. Or when area through which children are seen changes. Both
		 * cases are common for scroll areas.
		 */
		void MarkAbsoluteCoordinatesAsDirty();

		/**	Marks the element's contents as dirty, which causes the sprite meshes to be recreated from scratch. */
		void MarkContentAsDirty();

		/**
		 * Mark only the elements that operate directly on the sprite mesh without requiring the mesh to be recreated as
		 * dirty. This includes position, depth and clip rectangle. This will cause the parent widget mesh to be rebuilt
		 * from its child element's meshes.
		 */
		void MarkMeshAsDirty();

		/**	Returns true if the element requires layout update (e.g. if its optimal size has changed). */
		bool IsLayoutDirty() const { return mFlags.IsSet(GUIElementInternalStateFlag::LayoutDirty); }

		/** Returns true if absolute coordinates of child elements need to be updated (e.g. element is a scroll area and it has been scrolled). */
		bool AreAbsoluteCoordinatesDirty() const { return mFlags.IsSet(GUIElementInternalStateFlag::AbsoluteCoordinatesDirty); }

		/**	Marks the element contents to be up to date (meaning it's processed by the GUI system). */
		void MarkAsClean();

		/** @} */

	protected:
		friend class GUISpriteHelper;

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
		GUIElement* FindLayoutUpdateParent();

		/**
		 * Helper method for recursion in UpdatePanelAndLayoutUpdateParents(). Sets the provided anchor parent for all children recursively.
		 * Recursion stops when a child anchor is detected.
		 */
		void SetPanelParent(GUIPanel* panelParent);

		/**
		 * Helper method for recursion in UpdatePanelAndLayoutUpdateParents(). Sets the provided update parent for all children recursively.
		 * Recursion stops when a child update parent is detected.
		 */
		void SetLayoutUpdateParent(GUIElement* layoutUpdateParent);

		/** Unregisters and destroys all child elements. */
		void DestroyChildElements();

		GUIWidget* mParentWidget = nullptr;
		GUIPanel* mPanelParent = nullptr; /**< First panel in the parent hierarchy, if any. */
		GUIElement* mLayoutUpdateParent = nullptr; /**< Parent on which we need to call layout update if this element's size changes. This will be the first parent GUI element that doesn't have fixed bounds. */

		GUIElement* mParent = nullptr; /**< Direct parent of this element. */
		TInlineArray<GUIElement*, 4> mChildren;

		GUIElementInternalStateFlags mFlags = GUIElementInternalStateFlag::LayoutDirty;
		bool mIsPendingDestroy = false;

		GUISizeConstraints mSizeConstraints; /**< Constraints on the element size as set by the style sheet, or set explicitly at runtime. */
		GUILayoutData mLayoutData; /**< Relative position (to parent), size, depth and other information, calculated during a layout update. */
		Vector2I mAbsolutePosition; /**< Absolute position of the GUI element (relative to parent GUI widget). Only valid after layout update & absolute coordinate update. */
		Rect2I mAbsoluteClippedArea; /**< Absolute area of the GUI element as clipped by the parent visible bounds (e.g. if a parent is a scroll area). Only valid after layout update & absolute coordinate update. */

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIElementRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace bs
