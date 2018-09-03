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
	class BS_EXPORT GUIElementBase
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
		void setPosition(INT32 x, INT32 y);

		/**	Sets element width in pixels.  */
		void setWidth(UINT32 width);

		/**
		 * Sets element width in pixels. Element will be resized according to its contents and parent layout but will 
		 * always stay within the provided range. If maximum width is zero, the element is allowed to expand as much as
		 * it needs.
		 */
		void setFlexibleWidth(UINT32 minWidth = 0, UINT32 maxWidth = 0);

		/**	Sets element height in pixels. */
		void setHeight(UINT32 height);

		/** Sets width and height of a GUI element in pixels. */
		void setSize(UINT32 width, UINT32 height);

		/**
		 * Sets element height in pixels. Element will be resized according to its contents and parent layout but will 
		 * always stay within the provided range. If maximum height is zero, the element is allowed to expand as much as
		 * it needs.
		 */
		void setFlexibleHeight(UINT32 minHeight = 0, UINT32 maxHeight = 0);

		/**	Resets element dimensions to their initial values dictated by the element's style. */
		virtual void resetDimensions();

		/**
		 * Hides or shows this element and recursively applies the same state to all the child elements. This will not
		 * remove the element from the layout, the room for it will still be reserved but it just won't be visible.
		 */
		void setVisible(bool visible);

		/**
		 * Activates or deactives this element and recursively applies the same state to all the child elements. This has 
		 * the same effect as setVisible(), but when disabled it will also remove the element from the layout, essentially
		 * having the same effect is if you destroyed the element.
		 */
		void setActive(bool active);

		/** Disables or enables the element. Disabled elements cannot be interacted with and have a faded out appearance. */
		void setDisabled(bool disabled);

		/**
		 * Returns non-clipped bounds of the GUI element. Relative to a parent GUI panel.
		 *
		 * @param[in]	relativeTo	Parent panel of the provided element relative to which to return the bounds. If null 
		 *							the bounds relative to the first parent panel are returned. Behavior is undefined if 
		 *							provided panel is not a parent of the element.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty.
		 */
		Rect2I getBounds(GUIPanel* relativeTo = nullptr);

		/**
		 * Sets the bounds of the GUI element. Relative to a parent GUI panel. Equivalent to calling setPosition(), 
		 * setWidth() and setHeight().
		 */
		void setBounds(const Rect2I& bounds);

		/**
		 * Returns non-clipped bounds of the GUI element. Relative to a parent GUI widget.
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty.
		 */
		Rect2I getGlobalBounds();

		/** 
		 * Returns non-clipped bounds of the GUI element in screenspace. 
		 *
		 * @note	This call can be potentially expensive if the GUI state is dirty.
		 */
		Rect2I getScreenBounds() const;

		/**
		 * Returns non-clipped visible bounds of the GUI element (bounds exclude the margins). Relative to the parent GUI 
		 * panel.
		 *
		 * @note	This call can be potentially expensive as the bounds need to be calculated based on current GUI state.
		 */
		virtual Rect2I getVisibleBounds();

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
		virtual void _updateLayout(const GUILayoutData& data);

		/** Calculates optimal sizes of all child elements, as determined by their style and layout options. */
		virtual void _updateOptimalLayoutSizes();

		/** @copydoc _updateLayout */
		virtual void _updateLayoutInternal(const GUILayoutData& data);

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
		virtual void _getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
			const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const;

		/** Updates layout data that determines GUI elements final position & depth in the GUI widget. */
		virtual void _setLayoutData(const GUILayoutData& data) { mLayoutData = data; }

		/** Retrieves layout data that determines GUI elements final position & depth in the GUI widget. */
		const GUILayoutData& _getLayoutData() const { return mLayoutData; }

		/**	Sets a new parent for this element. */
		void _setParent(GUIElementBase* parent);

		/**	Returns number of child elements. */
		UINT32 _getNumChildren() const { return (UINT32)mChildren.size(); }

		/**	Return the child element at the specified index.*/
		GUIElementBase* _getChild(UINT32 idx) const { return mChildren[idx]; }

		/**	Returns previously calculated optimal size for this element. */
		virtual Vector2I _getOptimalSize() const = 0;

		/**	Returns layout options that determine how is the element positioned and sized. */
		const GUIDimensions& _getDimensions() const { return mDimensions; }

		/**	Calculates element size range constrained by its layout options. */
		virtual LayoutSizeRange _calculateLayoutSizeRange() const ;

		/**
		 * Returns element size range constrained by its layout options. This is different from _calculateLayoutSizeRange()
		 * because this method may return cached size range. 
		 */
		virtual LayoutSizeRange _getLayoutSizeRange() const;

		/**
		 * Returns element padding that determines how far apart to space out this element from other elements in a layout.
		 */
		virtual const RectOffset& _getPadding() const = 0;

		/**	Returns specific sub-type of this object. */
		virtual Type _getType() const = 0;

		/**	Returns parent GUI base element. */
		GUIElementBase* _getParent() const { return mParentElement; }

		/**
		 * Returns the parent element whose layout needs to be updated when this elements contents change.
		 *
		 * @note	
		 * Due to the nature of the GUI system, when a child element bounds or contents change, its parents and siblings
		 * usually need their layout bound updated. This function returns the first parent of all the elements that require
		 * updating. This parent usually has fixed bounds or some other property that allows its children to be updated
		 * independently from the even higher-up elements.
		 */
		GUIElementBase* _getUpdateParent() const { return mUpdateParent; }

		/**	Returns parent GUI widget, can be null. */
		GUIWidget* _getParentWidget() const { return mParentWidget; }

		/**	Checks if element is visible or hidden. */
		bool _isVisible() const { return (mFlags & GUIElem_Hidden) == 0; }

		/**
		 * Checks if element is active or inactive. Inactive elements are not visible, don't take up space
		 * in their parent layouts, and can't be interacted with. 
		 */
		bool _isActive() const { return (mFlags & GUIElem_Inactive) == 0; }

		/** Checks if element is disabled. Disabled elements cannot be interacted with and have a faded out appearance. */
		bool _isDisabled() const { return (mFlags & GUIElem_Disabled) != 0; }

		/**
		 * Internal version of setVisible() that doesn't modify local visibility, instead it is only meant to be called
		 * on child elements of the element whose visibility was modified.
		 */
		void _setVisible(bool visible);

		/**
		 * Internal version of setActive() that doesn't modify local state, instead it is only meant to be called
		 * on child elements of the element whose state was modified.
		 *  
		 * @copydoc setActive
		 */
		void _setActive(bool active);

		/**
		 * Internal version of setDisabled() that doesn't modify local state, instead it is only meant to be called
		 * on child elements of the element whose state was modified.
		 *  
		 * @copydoc setDisabled
		 */
		void _setDisabled(bool disabled);

		/**
		 * Changes the active GUI element widget. This allows you to move an element to a different viewport, or change
		 * element style by using a widget with a different skin. You are allowed to pass null here, but elements with no
		 * parent will be unmanaged. You will be responsible for deleting them manually, and they will not render anywhere.
		 */
		virtual void _changeParentWidget(GUIWidget* widget);

		/**Registers a new child element. */
		void _registerChildElement(GUIElementBase* element);

		/**	Unregisters an existing child element. */
		void _unregisterChildElement(GUIElementBase* element);

		/**	Checks if element has been destroyed and is queued for deletion. */
		virtual bool _isDestroyed() const { return false; }

		/**	Marks the element's dimensions as dirty, triggering a layout rebuild. */
		void _markLayoutAsDirty();

		/**	Marks the element's contents as dirty, which causes the sprite meshes to be recreated from scratch. */
		void _markContentAsDirty();

		/**
		 * Mark only the elements that operate directly on the sprite mesh without requiring the mesh to be recreated as 
		 * dirty. This includes position, depth and clip rectangle. This will cause the parent widget mesh to be rebuilt
		 * from its child element's meshes.
		 */
		void _markMeshAsDirty();

		/**	Returns true if elements contents have changed since last update. */
		bool _isDirty() const { return (mFlags & GUIElem_Dirty) != 0; }

		/**	Marks the element contents to be up to date (meaning it's processed by the GUI system). */
		void _markAsClean();

		/** @} */

	protected:
		/**	Finds anchor and update parents and recursively assigns them to all children. */
		void _updateAUParents();

		/**	Refreshes update parents of all child elements. */
		void refreshChildUpdateParents();

		/**
		 * Finds the first parent element whose size doesn't depend on child sizes.
		 *			
		 * @note	
		 * This allows us to optimize layout updates and trigger them only on such parents when their child elements 
		 * contents change, compared to doing them on the entire GUI hierarchy.
		 */
		GUIElementBase* findUpdateParent();

		/**
		 * Helper method for recursion in _updateAUParents(). Sets the provided anchor parent for all children recursively.
		 * Recursion stops when a child anchor is detected.
		 */
		void setAnchorParent(GUIPanel* anchorParent);

		/**
		 * Helper method for recursion in _updateAUParents(). Sets the provided update parent for all children recursively.
		 * Recursion stops when a child update parent is detected.
		 */
		void setUpdateParent(GUIElementBase* updateParent);

		/** Unregisters and destroys all child elements. */
		void destroyChildElements();

		GUIWidget* mParentWidget = nullptr;
		GUIPanel* mAnchorParent = nullptr;
		GUIElementBase* mUpdateParent = nullptr;
		GUIElementBase* mParentElement = nullptr;

		Vector<GUIElementBase*> mChildren;	
		UINT8 mFlags = GUIElem_Dirty;

		GUIDimensions mDimensions;
		GUILayoutData mLayoutData;
	};

	/** @} */
}