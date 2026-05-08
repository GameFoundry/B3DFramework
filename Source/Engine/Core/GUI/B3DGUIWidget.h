//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIMeshBatches.h"
#include "B3DGUIUnits.h"
#include "B3DPrerequisites.h"
#include "Scene/B3DComponent.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	class GUINavGroup;
	class GUIRenderable;
	class GUIStyleSheetCascade;
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * A top level container for all types of GUI elements. Every GUI element, layout or area must be assigned to a widget
	 * in order to be rendered.
	 *
	 * Widgets are the only GUI objects that may be arbitrarily transformed, allowing you to create 3D interfaces.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) GUIWidget : public Component
	{
	public:
		virtual ~GUIWidget() = default;

		/** Determines the style sheets that all GUI elements part of this widget will lookup styles in. */
		const GUIStyleSheetCascade& GetStyleSheetCascade() const;

		/** @copydoc SetStyleSheetCascade */
		const TShared<const GUIStyleSheetCascade>& GetStyleSheetCascadeAsShared() const { return mStyleSheetCascade; }

		/** @copydoc SetStyleSheetCascade */
		void SetStyleSheetCascade(const TShared<const GUIStyleSheetCascade>& styleSheetCascade);

		/** Returns the root GUI panel for the widget. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Panel))
		GUIPanel* GetPanel() const { return mPanel; }

		/**
		 * Determines the depth to render the widget at. If two widgets overlap the widget with the lower depth will be
		 * rendered in front.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Depth))
		void SetDepth(u8 depth);

		/** @copydoc SetDepth */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Depth))
		u8 GetDepth() const { return mDepth; }

		/** Checks are the specified coordinates within widget bounds. Coordinates should be relative to the parent window. */
		B3D_SCRIPT_EXPORT()
		bool InBounds(const GUIPhysicalPoint& position) const;

		/** Returns bounds of the widget, relative to the parent window. */
		B3D_SCRIPT_EXPORT()
		const GUIPhysicalArea& GetBounds() const { return mBounds; }

		/**
		 * Returns currently set DPI scale. Scale of 1.0 corresponds to 96 DPI.
		 *  physical pixel = logical pixel * DPI scale
		 *  logical pixel = physical pixel / DPI ccale;
		 */
		float GetDPIScale() const { return mDPIScale; }

		/** Changes the DPI scale of all the GUI elements in the widget. Triggers a full GUI rebuild. */
		void SetDPIScale(float dpiScale);

		/** @copydoc GUIWidget::GetTarget */
		Viewport* GetTarget() const;

		/** Changes to which camera does the widget output its contents. */
		void SetCamera(const HCamera& camera);

		/**	Returns the camera this widget is being rendered to. */
		HCamera GetCamera() const { return mCamera; }

		/**	Returns a list of all elements parented to this widget. */
		const Vector<GUIRenderable*>& GetElements() const { return mElements; }

		/**	Triggered when the widget's viewport size changes. */
		Event<void()> OnOwnerTargetResized;

		/**	Triggered when the parent window gained or lost focus. */
		Event<void()> OnOwnerWindowFocusChanged;
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Registers a new element as a child of the widget. */
		void RegisterElement(GUIElement* guiElement);

		/**
		 * Unregisters an element from the widget. Usually called when the element is destroyed, or reparented to another
		 * widget.
		 */
		void UnregisterElement(GUIElement* guiElementBase);

		/** Called when a registered GUI element is hidden, culled or visible. Only needs to be called if visibility changes after registration. */
		void NotifyElementVisibilityChanged(GUIElement* guiElement, bool isVisible);

		/**
		 * Returns the default navigation group assigned to all elements of this widget that don't have an explicit nav-
		 * group. See GUIElement::setNavGroup().
		 */
		TShared<GUINavGroup> GetDefaultNavGroupInternal() const { return mDefaultNavGroup; }

		/**
		 * Marks the widget mesh dirty requiring a mesh rebuild. Provided element is the one that requested the mesh update.
		 */
		void MarkMeshDirty(GUIElement* elem);

		/**
		 * Marks the elements content as dirty, meaning its internal mesh will need to be rebuilt (this implies the entire
		 * widget mesh will be rebuilt as well).
		 */
		void MarkContentDirty(GUIElement* elem);

		/**
		 * Marks the element layout as dirty. This means layout for the element and all child elements will be re-calculated.
		 *
		 * Note you almost always want to call this method on a parent of the GUI element whose layout needs to update. In particular,
		 * you want to call it on the top-most parent that doesn't have a fixed size. This is because size changes in a child element
		 * can affect its siblings as well as parents, if those elements are using automatic layouts.
		 *
		 * If @p element is null, then entire widget's layout will be marked as dirty.
		 */
		void MarkLayoutDirty(GUIElement* element) { mDirtyLayoutOrAbsoluteCoordinates.insert(element); }

		/**
		 * Marks the element's absolute coordinates as dirty. This will trigger a recalculation of absolute coordinates for
		 * all the children of @p element. You should call this when a GUI element moves, or when the area its children
		 * are viewed through changes (e.g. scroll area is scrolled).
		 */
		void MarkAbsoluteCoordinatesDirty(GUIElement* element) { mDirtyLayoutOrAbsoluteCoordinates.insert(element); }

		/**	Updates the layout of all child elements, repositioning and resizing them as needed. */
		void UpdateLayout();

		/**	Updates the layout of the provided element, and queues content updates. */
		void UpdateLayout(GUIElement* element);

		/**
		 * Checks if the render target of the destination camera changed, and updates the widget with new information if
		 * it has. Should be called every frame.
		 */
		void UpdateRenderTarget();

		/**
		 * Rebuilds any dirty data required for GUI element rendering and returns the data required for updating the GUI
		 * renderer.
		 */
		GUIDrawGroupRenderDataUpdate RebuildDirtyRenderData();

		/** @} */

	protected:
		friend class SceneObject;
		friend class GUIElement;
		friend class GUIManager;

		/**
		 * Constructs a new GUI widget attached to the specified parent scene object. Widget elements will be rendered on
		 * the provided camera.
		 */
		GUIWidget(const HSceneObject& parent, const HCamera& camera);

		void Update() override;
		void OnCreated() override;
		void OnDestroyed() override;
		void OnTransformChanged(TransformChangedFlags flags) override;

		/**	Called when the viewport size changes and widget elements need to be updated. */
		virtual void OwnerTargetResized() {}

		/**	Called when the parent window gained or lost focus. */
		virtual void OwnerWindowFocusChanged() {}

	private:
		GUIWidget(GUIWidget&&) = delete;
		GUIWidget(const GUIWidget&) = delete;

		/**	Calculates widget bounds using the bounds of all child elements. */
		void UpdateBounds() const;

		/**	Updates the size of the primary GUI panel based on the viewport. */
		void UpdateRootPanel();

		HCamera mCamera;
		Vector<GUIRenderable*> mElements;
		GUIMeshBatches mBatches;
		GUIPanel* mPanel = nullptr;
		u8 mDepth = 128;
		TShared<GUINavGroup> mDefaultNavGroup;

		float mDPIScale = 1.0f; // TODO - This should be grabbed from the destination render target

		Set<GUIRenderable*> mDirtyContents;
		Set<GUIRenderable*> mDirtyContentsTemp;

		UnorderedSet<GUIElement*> mDirtyLayoutOrAbsoluteCoordinates;

		mutable u64 mCachedRTId = 0;
		mutable bool mWidgetIsDirty = false;
		mutable GUIPhysicalArea mBounds;

		TShared<const GUIStyleSheetCascade> mStyleSheetCascade;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIWidgetRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

		GUIWidget(); // Serialization only
	};

	/** @} */
} // namespace b3d
