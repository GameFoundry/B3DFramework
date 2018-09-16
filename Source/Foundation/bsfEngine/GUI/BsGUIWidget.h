//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "Math/BsMatrix4.h"
#include "Utility/BsEvent.h"

namespace bs
{
	class GUINavGroup;

	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * A top level container for all types of GUI elements. Every GUI element, layout or area must be assigned to a widget
	 * in order to be rendered.
	 *
	 * Widgets are the only GUI objects that may be arbitrarily transformed, allowing you to create 3D interfaces.
	 */
	class BS_EXPORT GUIWidget 
	{
	public:
		virtual ~GUIWidget();

		/** Sets the skin used for all GUI elements in the widget. This will update the look of all current elements. */
		void setSkin(const HGUISkin& skin);

		/**	Returns the currently active GUI skin. */
		const GUISkin& getSkin() const;

		/**	Returns the currently active GUI skin resource. */
		const HGUISkin& getSkinResource() const { return mSkin; }

		/** Returns the root GUI panel for the widget. */
		GUIPanel* getPanel() const { return mPanel; }

		/**
		 * Returns the depth to render the widget at. If two widgets overlap the widget with the lower depth will be 
		 * rendered in front.
		 */
		UINT8 getDepth() const { return mDepth; }

		/**
		 * Changes the depth to render the widget at. If two widgets overlap the widget with the lower depth will be 
		 * rendered in front.
		 */
		void setDepth(UINT8 depth);

		/**
		 * Checks are the specified coordinates within widget bounds. Coordinates should be relative to the parent window.
		 */
		bool inBounds(const Vector2I& position) const;

		/** Returns bounds of the widget, relative to the parent window. */
		const Rect2I& getBounds() const { return mBounds; }

		/**
		 * Return true if widget or any of its elements are dirty.
		 *
		 * @param[in]	cleanIfDirty	If true, all dirty elements will be updated and widget will be marked as clean.
		 * @return						True if dirty, false if not. If "cleanIfDirty" is true, the returned state is the 
		 *								one before cleaning.
		 */
		bool isDirty(bool cleanIfDirty);

		/**	Returns the viewport that this widget will be rendered on. */
		Viewport* getTarget() const;

		/**	Returns the camera this widget is being rendered to. */
		SPtr<Camera> getCamera() const { return mCamera; }

		/** Changes to which camera does the widget output its contents. */
		void setCamera(const SPtr<Camera>& camera);

		/**	Returns a list of all elements parented to this widget. */
		const Vector<GUIElement*>& getElements() const { return mElements; }

		/** Returns the world transform that all GUI elements beloning to this widget will be transformed by. */
		const Matrix4 getWorldTfrm() const { return mTransform; }

		/**	Checks whether the widget should be rendered or not. */
		bool getIsActive() const { return mIsActive; }

		/**	Sets whether the widget should be rendered or not. */
		void setIsActive(bool active);

		/**	Creates a new GUI widget that will be rendered on the provided camera. */
		static SPtr<GUIWidget> create(const SPtr<Camera>& camera);

		/**	Creates a new GUI widget that will be rendered on the provided camera. */
		static SPtr<GUIWidget> create(const HCamera& camera);

		/**	Triggered when the widget's viewport size changes. */
		Event<void()> onOwnerTargetResized;

		/**	Triggered when the parent window gained or lost focus. */
		Event<void()> onOwnerWindowFocusChanged;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Registers a new element as a child of the widget. */
		void _registerElement(GUIElementBase* elem);
		
		/**
		 * Unregisters an element from the widget. Usually called when the element is destroyed, or reparented to another 
		 * widget.
		 */
		void _unregisterElement(GUIElementBase* elem);

		/** 
		 * Returns the default navigation group assigned to all elements of this widget that don't have an explicit nav-
		 * group. See GUIElement::setNavGroup().
		 */
		SPtr<GUINavGroup> _getDefaultNavGroup() const { return mDefaultNavGroup; }

		/**
		 * Marks the widget mesh dirty requiring a mesh rebuild. Provided element is the one that requested the mesh update.
		 */
		void _markMeshDirty(GUIElementBase* elem);

		/**
		 * Marks the elements content as dirty, meaning its internal mesh will need to be rebuilt (this implies the entire
		 * widget mesh will be rebuilt as well).
		 */
		void _markContentDirty(GUIElementBase* elem);

		/**	Updates the layout of all child elements, repositioning and resizing them as needed. */
		void _updateLayout();

		/**	Updates the layout of the provided element, and queues content updates. */
		void _updateLayout(GUIElementBase* elem);

		/**
		 * Updates internal transform values from the specified scene object, in case that scene object's transform changed
		 * since the last call.
		 *
		 * @note	Assumes the same scene object will be provided every time.
		 */
		void _updateTransform(const HSceneObject& parent);

		/** 
		 * Checks if the render target of the destination camera changed, and updates the widget with new information if 
		 * it has. Should be called every frame. 
		 */
		void _updateRT();

		/** Destroys the GUI widget and all child GUI elements. This is called automatically when GUIWidget is deleted. */
		void _destroy();

		/** @} */

	protected:
		friend class SceneObject;
		friend class GUIElementBase;
		friend class GUIManager;
		friend class CGUIWidget;

		/**	Constructs a new GUI widget that will be rendered on the provided camera. */
		GUIWidget(const SPtr<Camera>& camera);

		/**	Constructs a new GUI widget that will be rendered on the provided camera. */
		GUIWidget(const HCamera& camera);

		/**	Common code for constructors. */
		void construct(const SPtr<Camera>& camera);

		/**	Called when the viewport size changes and widget elements need to be updated. */
		virtual void ownerTargetResized();

		/**	Called when the parent window gained or lost focus. */
		virtual void ownerWindowFocusChanged();
	private:
		/**	Calculates widget bounds using the bounds of all child elements. */
		void updateBounds() const;

		/**	Updates the size of the primary GUI panel based on the viewport. */
		void updateRootPanel();

		SPtr<Camera> mCamera;
		Vector<GUIElement*> mElements;
		GUIPanel* mPanel;
		UINT8 mDepth;
		bool mIsActive;
		SPtr<GUINavGroup> mDefaultNavGroup;

		Vector3 mPosition;
		Quaternion mRotation;
		Vector3 mScale;
		Matrix4 mTransform;

		HEvent mOwnerTargetResizedConn;

		Set<GUIElement*> mDirtyContents;
		Set<GUIElement*> mDirtyContentsTemp;

		mutable UINT64 mCachedRTId;
		mutable bool mWidgetIsDirty;
		mutable Rect2I mBounds;

		HGUISkin mSkin;
	};

	/** @} */
}