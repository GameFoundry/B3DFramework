//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUILayoutData.h"
#include "Math/B3DArea2.h"
#include "Utility/B3DRectOffset.h"
#include "Utility/B3DSpatialTree.h"
#include "B3DGUIUnits.h"

namespace b3d
{
	struct GUIStyleSheetRuleset;
	struct GUIStyleSheetStateRulesets;
	class IGUIVectorPathBuilder;

	/** @addtogroup GUI-Internal
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
		AbsoluteCoordinatesDirty = 1 << 7, /**< GUI element is requesting update for absolute coordinates of all its children. Set if e.g. scroll area is scrolled. */
		Culled = 1 << 8, /**< Element is not visible due to being culled by the parent's visible area. Content/mesh updates will be skipped, absolute coordinate update will be skipped, element won't be rendered. */
		CulledSelf = 1 << 9, /**< Same as Culled, but set only on the element that was explicitly marked as culled, while Culled will also be set on all children on such element. */
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

	/** @} */

	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * Base class for all GUI elements. Provides general functionality such as element size/position, as well as handling child/parent relationships.
	 *
	 * @note: Does not provide ability to render and interact with GUI elements - those are implemented by derived classes (i.e. GUIRenderable and GUIInteractable).
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIElement : public IReflectable, public IScriptExportable
	{
	public:
		GUIElement() = default;
		GUIElement(const GUISizeConstraints& sizeConstraints);
		virtual ~GUIElement() = default;

		/**
		 * Sets element position relative to parent GUI panel. Values should be provided in logical pixel units.
		 *
		 * Be aware that this value will be ignored if GUI element is part of a layout since then the layout controls its placement.
		 */
		B3D_SCRIPT_EXPORT()
		void SetPosition(GUILogicalUnit x, GUILogicalUnit y) { SetPosition(GUILogicalPoint(x, y));}

		/**
		 * Sets element position relative to parent GUI panel. Values should be provided in logical pixel units.
		 *
		 * Be aware that this value will be ignored if GUI element is part of a layout since then the layout controls its placement.
		 */
		B3D_SCRIPT_EXPORT()
		void SetPosition(const GUILogicalPoint& position);

		/**	Sets fixed element width. Value should be in logical pixel units. */
		B3D_SCRIPT_EXPORT()
		void SetWidth(GUILogicalUnit width);

		/**
		 * Sets flexible element width. Element will be resized according to its contents and parent layout but will
		 * always stay within the provided range. If maximum width is zero, the element is allowed to expand as much as
		 * it needs. Values should be in logical pixel units.
		 */
		B3D_SCRIPT_EXPORT()
		void SetFlexibleWidth(GUILogicalUnit minWidth = 0, GUILogicalUnit maxWidth = 0);

		/**	Sets fixed element height. Value should be in logical pixel units. */
		B3D_SCRIPT_EXPORT()
		void SetHeight(GUILogicalUnit height);

		/**
		 * Sets flexible element height. Element will be resized according to its contents and parent layout but will
		 * always stay within the provided range. If maximum height is zero, the element is allowed to expand as much as
		 * it needs. Values provided should be in logical pixel units.
		 */
		B3D_SCRIPT_EXPORT()
		void SetFlexibleHeight(GUILogicalUnit minHeight = 0, GUILogicalUnit maxHeight = 0);

		/** Sets fixed width and height of a GUI element. Values provided should be in logical pixel units. */
		B3D_SCRIPT_EXPORT()
		void SetSize(const GUILogicalSize& size);

		/**	Resets element size constraints to their initial values dictated by the element's style. */
		B3D_SCRIPT_EXPORT()
		virtual void ResetSizeConstraints();

		/** Sets the scale of the GUI element. Note that scale will not affect the size of this GUI element, but will rather scale all of its contents. */
		void SetScale(float scale);

		/**
		 * Hides or shows this element and recursively applies the same state to all the child elements. This will not
		 * remove the element from the layout, the room for it will still be reserved but it just won't be visible.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Hidden))
		void SetHidden(bool hidden);

		/**
		 * Activates or deactives this element and recursively applies the same state to all the child elements. This has
		 * the same effect as setVisible(), but when disabled it will also remove the element from the layout, essentially
		 * having the same effect is if you destroyed the element.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Active))
		void SetActive(bool active);

		/** Disables or enables the element. Disabled elements cannot be interacted with and have a faded out appearance. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Disabled))
		void SetDisabled(bool disabled);

		/**
		 * Returns width/height of the GUI element. This will be the fixed width/height if set by the user, or automatically
		 * determined by the layout update pass if not fixed. Size is provided in logical pixel units.
		 *
		 * Always returns value calculated by last layout update. This means out of date value may be returned if the
		 * layout has been dirtied since then.
		 */
		const GUILogicalSize& GetLayoutCalculatedSize() const { return mLayoutData.Size; }

		/**
		 * Returns width/height of the GUI element. This will be the fixed width/height if set by the user, or automatically
		 * determined by the layout update pass if not fixed. Size is provided in logical pixel units.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(LayoutCalculatedSize))
		GUILogicalSize CalculateSizeInLayout() const;

		/**
		 * Calculates position of the GUI element, relative to the provided parent element (or parent panel if null).
		 * The value is provided in logical pixel units.
		 *
		 * @param	relativeTo	Parent element of the provided element relative to which to return the position. If null
		 *						the position relative to parent panel is returned. Behavior is undefined if
		 *						provided parent is not a parent of the element.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.
		 */
		B3D_SCRIPT_EXPORT()
		GUILogicalPoint CalculatePositionRelativeTo(GUIElement* relativeTo = nullptr) const;

		/**
		 * Calculates bounds of the GUI element, relative to the provided parent element (or parent panel if null), with
		 * scaling applied. The values are provided in physical pixel units.
		 *
		 * @param	relativeTo	Parent element of the provided element relative to which to return the bounds. If null
		 *						the bounds relative to parent panel are returned. Behavior is undefined if
		 *						provided parent is not a parent of the element.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.
		 */
		B3D_SCRIPT_EXPORT()
		GUIPhysicalArea CalculateAbsoluteBoundsRelativeTo(GUIElement* relativeTo = nullptr);

		/**
		 * Calculates bounds of the GUI element, relative to the parent GUI widget, with scaling applied. 
		 * The values are provided in physical pixel units.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(AbsoluteBounds))
		GUIPhysicalArea CalculateAbsoluteBounds() const;

		/**
		 * Calculates bounds of the GUI element in screen space. 
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty, as it can trigger a layout update operation.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ScreenBounds))
		GUIPhysicalArea CalculateScreenBounds() const;

		/**
		 * Returns bounds of the GUI element, relative to the parent GUI widget. Absolute values represent the final
		 * position and size of the GUI element, affected by DPI scale, parent scale and self scale. The values are
		 * provided in physical pixel units.
		 *
		 * Always returns value calculated by last layout update. This means out of date value may be returned if the
		 * layout has been dirtied since then.
		 */
		GUIPhysicalArea GetAbsoluteBounds() const { return GUIPhysicalArea(mAbsolutePosition, mAbsoluteSize); }

		/** Converts a point relative to the parent widget, into a point relative to this element. */
		B3D_SCRIPT_EXPORT()
		GUILogicalPoint WidgetToElementSpace(const GUIPhysicalPoint& point) const;

		/** Converts a point relative to this element, into a point relative to the parent widget. */
		B3D_SCRIPT_EXPORT()
		GUIPhysicalPoint ElementToWidgetSpace(const GUILogicalPoint& point) const;

		/** Converts an area relative to the parent widget, into an area relative to this element. */
		B3D_SCRIPT_EXPORT()
		GUILogicalArea WidgetToElementSpace(const GUIPhysicalArea& area) const;

		/** Converts an area relative to this element, into an area relative to the parent widget. */
		B3D_SCRIPT_EXPORT()
		GUIPhysicalArea ElementToWidgetSpace(const GUILogicalArea& area) const;

		/**
		 * Returns the position of the GUI element, relative to the parent widget. Absolute values represent the final
		 * position of the GUI element, affected by DPI scale and parent scale. The values are provided in physical pixel units.
		 *
		 * Always returns value calculated by last layout update. This means out of date value may be returned if the
		 * layout has been dirtied since then.
		 */
		const GUIPhysicalPoint& GetAbsolutePosition() const { return mAbsolutePosition; }

		/** Combined local and parent scale. */
		float GetAbsoluteScale() const { return mAbsoluteScale; }

		/**
		 * Destroy the element. Removes it from parent and widget, and queues it for deletion. Element memory will be
		 * released delayed, next frame.
		 */
		B3D_SCRIPT_EXPORT()
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

		/** Triggers a layout update immediately if the layout has been dirtied. */
		B3D_SCRIPT_EXPORT()
		void UpdateLayoutIfDirty();

		/**
		 * Updates the absolute coordinates of the GUI element using the currently assigned relative coordinates and the provided
		 * @p parentOrigin. Also calculates the visible area clip rectangle and marks culled elements if they have no visible area.
		 * This should be called after updating the layout (as layout update calculates the needed relative coordinates).
		 * This may also be called independently of layout update, which is useful for scroll areas that then do not require
		 * a full layout pass to scroll their children.
		 *
		 * @param	parentOrigin		Absolute origin to add to the relative coordinates, in order to determine the absolute element coordinates.
		 * @param	parentScale			Scale of the parent GUI element.
		 * @param	parentVisibleArea	Absolute visible (clipped) area though which this element may be seen. This will be used for culling and clipping.
		 */
		virtual void UpdateAbsoluteCoordinates(const GUIPhysicalPointF& parentOrigin, float parentScale, const GUIPhysicalAreaF& parentVisibleArea);

		/** Calls UpdateAbsoluteCoordinates() on all child elements. */
		virtual void UpdateAbsoluteCoordinatesForChildren();

		/** Updates layout data that determines GUI elements relative position, size and depth in the GUI widget. */
		virtual void SetLayoutData(const GUILayoutData& data) { mLayoutData = data; }

		/** Retrieves layout data that determines GUI elements relative position, size and depth in the GUI widget. */
		const GUILayoutData& GetLayoutData() const { return mLayoutData; }

		/**	Sets a new parent for this element. */
		void SetParent(GUIElement* parent);

		/**	Returns number of child elements. */
		u32 GetChildCount() const { return (u32)mChildren.size(); }

		/**	Return the child element at the specified index.*/
		GUIElement* GetChild(u32 index) const { return mChildren[index]; }

		/**
		 * Returns all children that can be seen through the parent's visible area (i.e. all elements that are not culled or explicitly made invisible). Note this
		 * may return all child elements on GUI elements that do not support culling. Only well defined after layout update.
		 */
		virtual const TInlineArray<GUIElement*, 4>& GetVisibleChildren() const { return mChildren; }

		/**	Calculates the optimal size for the GUI element, ignoring size constraints. */
		virtual GUILogicalSize CalculateUnconstrainedOptimalSize() const = 0;

		/**	Returns the result of CalculateUnconstrainedOptimalSize() with size constrains applied. */
		GUILogicalSize CalculateConstrainedOptimalSize() const { return mSizeConstraints.CalculateConstrainedOptimalSize(CalculateUnconstrainedOptimalSize()); }

		/**	Returns size constraints that determine how is the GUI element allowed to be resized by the layout. */
		const GUISizeConstraints& GetSizeConstraints() const { return mSizeConstraints; }

		/**	Calculates element size based on its optimal size constrained by its size constraint options. */
		virtual GUIConstrainedSizeRange CalculateConstrainedSizeRange() const;

		/** Returns element size constrained by its size constraints. This is different from CalculateConstrainedSize() because this method may return cached size. */
		virtual GUIConstrainedSizeRange GetConstrainedSizeRange() const;

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

		/**	Returns parent GUI base element. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Parent))
		GUIElement* GetParent() const { return mParent; }

		/**	Returns parent GUI widget, can be null. */
		GUIWidget* GetParentWidget() const { return mParentWidget; }

		/**	Checks if element is explicitly hidden. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Hidden))
		bool IsHidden() const { return mFlags.IsSet(GUIElementInternalStateFlag::Hidden); }

		/**
		 * Checks if element is active or inactive. Inactive elements are not visible, don't take up space
		 * in their parent layouts, and can't be interacted with.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Active))
		bool IsActive() const { return !mFlags.IsSet(GUIElementInternalStateFlag::Inactive); }

		/** Checks if element is disabled. Disabled elements cannot be interacted with and have a faded out appearance. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Disabled))
		bool IsDisabled() const { return mFlags.IsSet(GUIElementInternalStateFlag::Disabled); }

		/** Returns true if the element is not visible due to being culled by its parent bounds. */
		bool IsCulled() const { return mFlags.IsSet(GUIElementInternalStateFlag::Culled); }

		/**	Checks if element is invisible. Either due to explicitly being hidden or due to being culled by the parent. */
		bool IsHiddenOrCulled() const { return mFlags.IsSetAny(GUIElementInternalStateFlag::Hidden | GUIElementInternalStateFlag::Culled); }

		/**
		 * Internal version of SetHidden() that doesn't modify local visibility, instead it is only meant to be called
		 * on child elements of the element whose visibility was modified.
		 */
		void SetHiddenRecursive(bool hidden);

		/**
		 * Internal version of SetActive() that doesn't modify local state, instead it is only meant to be called
		 * on child elements of the element whose state was modified.
		 *
		 * @copydoc SetActive
		 */
		void SetActiveRecursive(bool active);

		/**
		 * Internal version of SetDisabled() that doesn't modify local state, instead it is only meant to be called
		 * on child elements of the element whose state was modified.
		 *
		 * @copydoc SetDisabled
		 */
		void SetDisabledRecursive(bool disabled);

		/** Marks the object and all children as culled. */
		void SetCulled(bool culled);

		/**
		 * Changes the active GUI element widget. This allows you to move an element to a different viewport, or change
		 * element style by using a widget with a different skin. You are allowed to pass null here, but elements with no
		 * parent will be unmanaged. You will be responsible for deleting them manually, and they will not render anywhere.
		 */
		virtual void ChangeParentWidget(GUIWidget* widget);

		/**Registers a new child element. */
		virtual void RegisterChildElement(GUIElement* element);

		/**	Unregisters an existing child element. */
		virtual void UnregisterChildElement(GUIElement* element);

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

		/** ID of the element in a quad-tree managed by its direct parent. Quad-tree can be used for speeding up lookup for elements containing many children. */
		void SetQuadTreeId(const SpatialTreeElementId& id) { mQuadTreeId = id; }

		/** @copydoc SetQuadTreeId */
		const SpatialTreeElementId& GetQuadTreeId() const { return mQuadTreeId; }

		/**
		 * Returns the position and size of the GUI element, relative to the parent widget. The returned area is clipped by the visible
		 * area as specified by the parent GUI element (e.g. if the parent is a scroll area or similar, only some or none of the GUI element may
		 * be visible, if it's scrolled out of view). Provided value is in physical pixel units.
		 *
		 * Always returns value calculated by last layout update. This means out of date value may be returned if the
		 * layout has been dirtied since then.
		 */
		const GUIPhysicalArea& GetAbsoluteClippedArea() const { return mAbsoluteClippedArea; }

		/** Similar to GetAbsolutePosition(), but contains floating point data that is not rounded, useful primarily for calculating child absolute coordinates without losing precision. */
		const GUIPhysicalPointF& GetIntermediateAbsolutePosition() const { return mIntermediateAbsolutePosition; }

		/** Similar to GetAbsoluteClippedArea(), but contains floating point data that is not rounded, useful primarily for calculating child absolute coordinates without losing precision. */
		const GUIPhysicalAreaF& GetIntermediateAbsoluteClippedArea() const { return mIntermediateAbsoluteClippedArea; }

		/** Same as GetAbsoluteClippedArea(), except the area position is made relative to this GUI element. */
		GUIPhysicalArea GetClippedAreaTranslatedRelativeToParent() const;

		/** @} */

	protected:
		friend class GUISpriteHelper;

		/**
		 * Same as UpdateAbsoluteCoordinates, but allows the user to provide explicit size of this GUI element's contents. This is useful for e.g. scroll areas where the content size may be
		 * larger than the layout calculated area used by default.
		 */
		void UpdateAbsoluteCoordinatesWithExplicitContentSize(const GUIPhysicalPointF& parentOrigin, float parentScale, const GUIPhysicalAreaF& parentVisibleArea, const GUILogicalSize& contentSize);

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

		/** If the parent GUI element maintains a quad tree of its children, this will be ID of the element within the quad-tree. */
		SpatialTreeElementId mQuadTreeId;

		// User-defined layouting data
		GUISizeConstraints mSizeConstraints; /**< Constraints on the element size as set by the style sheet, or set explicitly at runtime. */
		float mScale = 1.0f; /**< Scale to apply to the GUI element and all children. */

		// Data calculated by layout update pass
		GUILayoutData mLayoutData; /**< Relative position (to parent), size, depth and other information, calculated during a layout update. */

		// Data calculated by absolute coordinate pass
		GUIPhysicalPoint mAbsolutePosition{kZeroTag}; /**< Absolute position of the GUI element (relative to parent GUI widget). Only valid after layout update & absolute coordinate update. */
		GUIPhysicalSize mAbsoluteSize{kZeroTag}; /**< Final size to use for the element. Same as GUILayoutData::Size, scaled by GUI element scale. */
		float mAbsoluteScale = 1.0f; /**< Combined local and parent scale. */
		GUIPhysicalArea mAbsoluteClippedArea; /**< Absolute area of the GUI element as clipped by the parent visible bounds (e.g. if a parent is a scroll area). Only valid after layout update & absolute coordinate update. */

		// Keep floating point copies of absolute position & clipped area in case we need to update child absolute positions, so we don't lose precision from converting to integer at every child
		GUIPhysicalPointF mIntermediateAbsolutePosition{kZeroTag};
		GUIPhysicalAreaF mIntermediateAbsoluteClippedArea;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIElementRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
