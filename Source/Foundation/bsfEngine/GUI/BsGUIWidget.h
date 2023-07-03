//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "Math/BsMatrix4.h"
#include "Utility/BsEvent.h"
#include "BsGUIMeshBatches.h"

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
	class B3D_EXPORT GUIWidget
	{
	public:
		virtual ~GUIWidget();

		/** Sets the skin used for all GUI elements in the widget. This will update the look of all current elements. */
		void SetSkin(const HGUISkin& skin);

		/**	Returns the currently active GUI skin. */
		const GUISkin& GetSkin() const;

		/**	Returns the currently active GUI skin resource. */
		const HGUISkin& GetSkinResource() const { return mSkin; }

		/** Returns the root GUI panel for the widget. */
		GUIPanel* GetPanel() const { return mPanel; }

		/**
		 * Returns the depth to render the widget at. If two widgets overlap the widget with the lower depth will be
		 * rendered in front.
		 */
		u8 GetDepth() const { return mDepth; }

		/**
		 * Changes the depth to render the widget at. If two widgets overlap the widget with the lower depth will be
		 * rendered in front.
		 */
		void SetDepth(u8 depth);

		/**
		 * Checks are the specified coordinates within widget bounds. Coordinates should be relative to the parent window.
		 */
		bool InBounds(const Vector2I& position) const;

		/** Returns bounds of the widget, relative to the parent window. */
		const Rect2I& GetBounds() const { return mBounds; }

		/**
		 * Rebuilds any dirty data required for GUI element rendering and returns the data required for updating the GUI
		 * renderer.
		 */
		GUIDrawGroupRenderDataUpdate RebuildDirtyRenderData();

		/**	Returns the viewport that this widget will be rendered on. */
		Viewport* GetTarget() const;

		/**	Returns the camera this widget is being rendered to. */
		SPtr<Camera> GetCamera() const { return mCamera; }

		/** Changes to which camera does the widget output its contents. */
		void SetCamera(const SPtr<Camera>& camera);

		/**	Returns a list of all elements parented to this widget. */
		const Vector<GUIElement*>& GetElements() const { return mElements; }

		/** Returns the world transform that all GUI elements beloning to this widget will be transformed by. */
		const Matrix4 GetWorldTfrm() const { return mTransform; }

		/**	Checks whether the widget should be rendered or not. */
		bool GetIsActive() const { return mIsActive; }

		/**	Sets whether the widget should be rendered or not. */
		void SetIsActive(bool active);

		/**	Creates a new GUI widget that will be rendered on the provided camera. */
		static SPtr<GUIWidget> Create(const SPtr<Camera>& camera);

		/**	Creates a new GUI widget that will be rendered on the provided camera. */
		static SPtr<GUIWidget> Create(const HCamera& camera);

		/**	Triggered when the widget's viewport size changes. */
		Event<void()> OnOwnerTargetResized;

		/**	Triggered when the parent window gained or lost focus. */
		Event<void()> OnOwnerWindowFocusChanged;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Registers a new element as a child of the widget. */
		void RegisterElement(GUIElementBase* guiElementBase);

		/**
		 * Unregisters an element from the widget. Usually called when the element is destroyed, or reparented to another
		 * widget.
		 */
		void UnregisterElement(GUIElementBase* guiElementBase);

		/** Called when a registered GUI element is hidden or made visible. Only needs to be called if visibility changes after registration. */
		void NotifyElementVisibilityChanged(GUIElementBase* guiElementBase, bool isVisible);

		/**
		 * Returns the default navigation group assigned to all elements of this widget that don't have an explicit nav-
		 * group. See GUIElement::setNavGroup().
		 */
		SPtr<GUINavGroup> GetDefaultNavGroupInternal() const { return mDefaultNavGroup; }

		/**
		 * Marks the widget mesh dirty requiring a mesh rebuild. Provided element is the one that requested the mesh update.
		 */
		void MarkMeshDirty(GUIElementBase* elem);

		/**
		 * Marks the elements content as dirty, meaning its internal mesh will need to be rebuilt (this implies the entire
		 * widget mesh will be rebuilt as well).
		 */
		void MarkContentDirty(GUIElementBase* elem);

		/**	Updates the layout of all child elements, repositioning and resizing them as needed. */
		void UpdateLayoutInternal();

		/**	Updates the layout of the provided element, and queues content updates. */
		void UpdateLayoutInternal(GUIElementBase* elem);

		/**
		 * Updates internal transform values from the specified scene object, in case that scene object's transform changed
		 * since the last call.
		 *
		 * @note	Assumes the same scene object will be provided every time.
		 */
		void UpdateTransformInternal(const HSceneObject& parent);

		/**
		 * Checks if the render target of the destination camera changed, and updates the widget with new information if
		 * it has. Should be called every frame.
		 */
		void UpdateRTInternal();

		/** Destroys the GUI widget and all child GUI elements. This is called automatically when GUIWidget is deleted. */
		void DestroyInternal();

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
		void Construct(const SPtr<Camera>& camera);

		/**	Called when the parent window gained or lost focus. */
		virtual void OwnerWindowFocusChanged();

	private:
		struct GUIGroupElement
		{
			GUIGroupElement() = default;

			GUIGroupElement(GUIElement* element, u32 renderElement)
				: Element(element), RenderElement(renderElement)
			{}

			GUIElement* Element = nullptr;
			u32 RenderElement = 0;
		};

		/** Holds information about a set of GUI elements that can be drawn together. */
		struct GUIDrawGroup
		{
			u32 Id = 0;
			u32 DepthRange = 0;
			u32 MinDepth = 0;
			Rect2I Bounds;
			Vector<GUIGroupElement> CachedElements;
			Vector<GUIGroupElement> NonCachedElements;
		};

		/**	Calculates widget bounds using the bounds of all child elements. */
		void UpdateBounds() const;

		/**	Updates the size of the primary GUI panel based on the viewport. */
		void UpdateRootPanel();

		SPtr<Camera> mCamera;
		Vector<GUIElement*> mElements;
		GUIMeshBatches mBatches;
		GUIPanel* mPanel = nullptr;
		u8 mDepth = 128;
		bool mIsActive = true;
		SPtr<GUINavGroup> mDefaultNavGroup;

		Vector3 mPosition = BsZero;
		Quaternion mRotation = BsIdentity;
		Vector3 mScale = Vector3::kOne;
		Matrix4 mTransform = BsIdentity;

		Set<GUIElement*> mDirtyContents;
		Set<GUIElement*> mDirtyContentsTemp;

		mutable u64 mCachedRTId = 0;
		mutable bool mWidgetIsDirty = false;
		mutable Rect2I mBounds;

		HGUISkin mSkin;
	};

	/** @} */
} // namespace bs
