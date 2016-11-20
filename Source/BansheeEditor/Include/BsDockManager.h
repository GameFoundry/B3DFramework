//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIElementContainer.h"
#include "BsRect2I.h"

namespace bs
{
	/** @addtogroup EditorWindow-Internal
	 *  @{
	 */

	class DockOverlayRenderer;

	/**
	 * GUI element that allows editor widgets to be docked in it using arbitrary layouts. Docked widgets can be resized,
	 * undocked, maximizes or closed as needed.
	 */
	class DockManager : public GUIElementContainer
	{
	public:
		/**
		 * Contains information about a single dock area. Each container can be a parent to two other containers or may
		 * contain a widget, which results in a container hierarchy. Two children can be split vertically or horizontally
		 * at an user-defined point.
		 */
		class DockContainer
		{
		public:
			DockContainer(DockManager* manager);
			DockContainer(DockManager* manager, DockContainer* parent);
			~DockContainer();

			/**	Determines the position and size of the container, relative to the parent dock manager. */
			void setArea(INT32 x, INT32 y, UINT32 width, UINT32 height);

			/**
			 * Transforms the container from non-leaf (parent to other containers) to leaf (parent to widgets). This
			 * involves creating a widget container to which you can dock widgets to.
			 *
			 * @param[in]	parentWindow	Editor window of the parent dock manager.
			 */
			void makeLeaf(EditorWindowBase* parentWindow);

			/**
			 * Transforms the container from non-leaf (parent to other containers) to leaf (parent to widgets). Unlike the
			 * other overload this one accepts a previously created widget container.
			 *
			 * @param[in]	guiWidgetSO			Parent SceneObject of the GUIWidget used by the provided widget container.
			 * @param[in]	existingContainer	An existing widget container that may be used for docking widgets.
			 */
			void makeLeaf(const HSceneObject& guiWidgetSO, EditorWidgetContainer* existingContainer);

			/**
			 * Splits a leaf container containing a widget container (or may be empty in the case of root with no elements) 
			 * into a container parent to two other containers.
			 *
			 * @param[in]	horizontal			Whether the split is horizontal (true) or vertical (false.
			 * @param[in]	newChildIsFirst		Determines to which child should the widget container from this object be
			 *									moved to. If the new child is first, then bottom or right (for horizontal
			 *									and vertical respectively) will receive the current widget container, and
			 *									opposite if it's not first.
			 * @param[in]	splitPosition		Determines at what position(in percent) should this container be split. User
			 *									can modify this later via a dock slider.
			 */
			void splitContainer(bool horizontal, bool newChildIsFirst, float splitPosition = 0.5f);

			/**
			 * Splits a leaf container containing a widget container (or may be empty in the case of root with no elements)
			 * into a container parent to two other containers. Unlike "splitContainer" new containers aren't created 
			 * automatically but you must provide existing ones. If this container is non-leaf its widget container will be
			 * destroyed.
			 *
			 * @param[in]	first			Container to insert into the first child slot (left if vertical split, top if
			 *								horizontal split).
			 * @param[in]	second			Container to insert into the second child slot (right if vertical split, bottom
			 *								if horizontal split).
			 * @param[in]	horizontal		Whether the split is horizontal (true) or vertical (false.
			 * @param[in]	splitPosition	Determines at what position(in percent) should this container be split. User can
			 *								modify this later via a dock slider.
			 */
			void makeSplit(DockContainer* first, DockContainer* second, bool horizontal, float splitPosition);

			/**
			 * Adds a new widget to the left side of the container. If the container is leaf it will be split into two
			 * containers vertically.
			 */
			void addLeft(EditorWidgetBase* widget);

			/**
			 * Adds a new widget to the right side of the container. If the container is leaf it will be split into two
			 * containers vertically.
			 */
			void addRight(EditorWidgetBase* widget);

			/**
			 * Adds a new widget to the top side of the container. If the container is leaf it will be split into two
			 * containers horizontally.
			 */
			void addTop(EditorWidgetBase* widget);

			/**
			 * Adds a new widget to the bottom side of the container. If the container is leaf it will be split into two
			 * containers horizontally.
			 */
			void addBottom(EditorWidgetBase* widget);

			/**	Adds an existing widget to this leaf container. */
			void addWidget(EditorWidgetBase* widget);

			/** Attempts to find a widget with the specified name, opens it and adds it to this leaf container. */
			void addWidget(const String& name);

			/**	Update to be called once per frame. Calls updates on all child widgets. */
			void update();

			/**
			 * Attempts to find an existing leaf dock container with the specified widget container. Returns null if one
			 * cannot be found.
			 */
			DockContainer* find(EditorWidgetContainer* widgetContainer);

			/**
			 * Searches for a container at the specified position. Call this at this top-most container in order to search
			 * them all.
			 *
			 * @param[in]	pos	Position is relative to the container area. 
			 * @return			null if it fails, else the found DockContainer at position.
			 */
			DockContainer* findAtPos(const Vector2I& pos);

			/** Returns the bounds of the container that are to be considered dockable and interactable. */
			Rect2I getContentBounds() const;

			bool mIsLeaf;
			DockContainer* mChildren[2];
			DockContainer* mParent;
			DockManager* mManager;
			EditorWidgetContainer* mWidgets;
			HSceneObject mGUIWidgetSO;
			GUIDockSlider* mSlider;
			Rect2I mArea;
			float mSplitPosition;
			bool mIsHorizontal;

			static const UINT32 SLIDER_SIZE;
			static const UINT32 MIN_CHILD_SIZE;

		private:
			/**	Updates sizes and positions of all child containers. Normally called when parent area changes. */
			void updateChildAreas();

			/**	Triggered whenever the user drags the GUI slider belonging to this container. */
			void sliderDragged(const Vector2I& delta);

			/**	Triggered whenever the user closes or undocks a widget belonging to this container. */
			void widgetRemoved();

			/**	Triggered when the maximize button in the container's title bar is clicked. */
			void maximizeClicked();
		};

		/**	Available dock locations for the dock manager. */
		enum class DockLocation
		{
			Top,
			Bottom,
			Left,
			Right,
			None
		};
	public:
		/**	Creates a new dock manager for the specified window. */
		static DockManager* create(EditorWindowBase* parentWindow);

		/**	Internal method. Called once every frame. */
		void update();

		/**
		 * Inserts a new widget at the specified location.
		 *
		 * @param[in]	relativeTo		Container relative to which to insert the widget. Can be null in which case the
		 *								widget is inserted at the root.
		 * @param[in]	widgetToInsert	Widget we want to insert into the dock layout.
		 * @param[in]	location		Location to insert the widget at, relative to @p relativeTo container. 
		 *								If @p relativeTo is null this is ignored.
		 */
		void insert(EditorWidgetContainer* relativeTo, EditorWidgetBase* widgetToInsert, DockLocation location);

		/** Returns a saved layout of all the currently docked widgets and their positions and areas. */
		SPtr<DockManagerLayout> getLayout() const;

		/**
		 * Sets a previously saved layout of docked widgets. This will close all currently active widgets and open and
		 * position new ones according to the layout.
		 */
		void setLayout(const SPtr<DockManagerLayout>& layout);

		/**	Changes the position and size of the dock manager. */
		void setArea(INT32 x, INT32 y, UINT32 width, UINT32 height);

		/**	Closes all docked widgets. */
		void closeAll();

	private:
		DockManager(EditorWindowBase* parentWindow, const GUIDimensions& dimensions);
		~DockManager();

		/** Updates the dock overlay mesh that is displayed when user is dragging a widget over a certain area. */
		void updateDropOverlay(INT32 x, INT32 y, UINT32 width, UINT32 height);

		/**
		 * Checks is the provided point inside the provided polygon.
		 *
		 * @param[i]	polyPoints	Points of the polygon to test against.
		 * @param[i]	numPoints	Number of points in "polyPoints".
		 * @param[i]	point		Point to check if it's in the polygon.
		 * @return					True if the point is in the polygon.
		 */
		bool insidePolygon(Vector2* polyPoints, UINT32 numPoints, Vector2 point) const;

		/**	Initializes the renderer used for displaying the dock overlay. */
		void initializeOverlayRenderer(const SPtr<MaterialCore>& initData);

		/**	Destroys the dock overlay renderer. */
		void destroyOverlayRenderer(DockOverlayRenderer* core);

		/** @copydoc GUIElementBase::updateClippedBounds */
		void updateClippedBounds() override;

		/**
		 * Maximizes or restored the specified container. If any container is previously maximized it needs to be toggled
		 * back to restored state before maximizing another.
		 */
		void toggleMaximize(DockContainer* container);

		/** @copydoc GUIElementBase::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& event) override;

		std::atomic<DockOverlayRenderer*> mCore;

		EditorWindowBase* mParentWindow;
		DockContainer mRootContainer;
		Rect2I mArea;

		HMesh mDropOverlayMesh;
		Rect2I mLastOverlayBounds;

		bool mIsMaximized;
		DockContainer* mMaximizedContainer;
		SPtr<DockManagerLayout> mRestoredLayout;

		DockContainer* mMouseOverContainer;
		DockLocation mHighlightedDropLoc;
		bool mShowOverlay;
		Vector2* mTopDropPolygon;
		Vector2* mBotDropPolygon;
		Vector2* mLeftDropPolygon;
		Vector2* mRightDropPolygon;
	};

	/**
	 * Handles rendering of the dock overlay, on the core thread.
	 * 			
	 * @note	Core thread only.
	 */
	class DockOverlayRenderer
	{
	public:
		DockOverlayRenderer();
		~DockOverlayRenderer();

	private:
		friend class DockManager;

		/**
		 * Initializes the object. Must be called right after construction and before any use.
		 *
		 * @param[in]	material	Material used for drawing the dock overlay.
		 */
		void initialize(const SPtr<MaterialCore>& material);

		/**
		 * Updates the grid mesh to render.
		 * 			
		 * @param[in]	camera		Camera to render the dock overlay to.
		 * @param[in]	mesh		Overlay mesh to render.
		 * @param[in]	active		Should the overlay be shown or not.
		 * @param[in]	location	Highlighted location of the overlay.
		 */
		void updateData(const SPtr<CameraCore>& camera, const SPtr<MeshCore>& mesh, bool active,
			DockManager::DockLocation location);

		/**	Callback triggered by the renderer, actually draws the dock overlay. */
		void render();
		
		SPtr<CameraCore> mCamera;
		SPtr<MaterialCore> mMaterial;
		SPtr<GpuParamsSetCore> mParams;
		SPtr<MeshCore> mMesh;
		DockManager::DockLocation mHighlightedDropLoc;
		bool mShowOverlay;

		static const Color TINT_COLOR;
		static const Color HIGHLIGHT_COLOR;
	};

	/** @} */
}