//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Math/BsRect2I.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "Math/BsMatrix4.h"
#include "Utility/BsEvent.h"
#include "2D/BsSpriteMaterial.h"
#include "RenderAPI/BsSubMesh.h"

namespace bs
{
	class GUINavGroup;

	/** @addtogroup Implementation
	 *  @{
	 */

	/**	Data required for rendering a single batch of GUI elements.  */
	struct GUIMeshRenderData
	{
		SubMesh subMesh;
		SPtr<ct::Texture> texture;
		SPtr<ct::SpriteTexture> spriteTexture;
		SpriteMaterial* material;
		Color tint;
		float animationStartTime;
		SPtr<SpriteMaterialExtraInfo> additionalData;
		bool isLine;
		UINT32 bufferIdx;
	};

	/** Data required for rendering all the batches in a single GUI draw group. */
	struct GUIDrawGroupRenderData
	{
		UINT32 id = 0;
		SPtr<ct::RenderTexture> destination;
		Rect2I bounds;
		SubMesh subMesh;
		bool requiresRedraw = true;
		UINT32 bufferIdx = 0;

		Vector<GUIMeshRenderData> cachedElements;
		Vector<GUIMeshRenderData> nonCachedElements;
	};

	/**
	 * Contains data about which draw group needs to be redrawn, as well as a set of new draw groups if
	 * draw groups were updated.
	 */
	struct GUIDrawGroupRenderDataUpdate
	{
		Vector<GUIDrawGroupRenderData> newDrawGroups;
		Vector<bool> groupDirtyState;
		SPtr<ct::Mesh> triangleMesh;
		SPtr<ct::Mesh> lineMesh;
	};

	/**
	 * Organizes elements within a GUIWidget into groups that can be drawn together, as well as cached into the same
	 * output texture.
	 **/
	class BS_EXPORT GUIDrawGroups
	{
		/** Flags signaling which part of a GUIElement is dirty. */
		enum DirtyFlags
		{
			DirtyMesh = 1 << 0,
			DirtyContent = 1 << 1
		};
	public:
		GUIDrawGroups();
		
		/** Iterates over all the render elements in the GUI elements and adds them to suitable draw groups. */
		void add(GUIElement* element);

		/** Removes all render elements in the provided GUI element from their current set of draw groups. */
		void remove(GUIElement* element);

		/** Rebuilds any dirty internal data and returns the data structure required for updating the GUI renderer. */
		GUIDrawGroupRenderDataUpdate rebuildDirty(bool forceRebuildMeshes);

		/** Notifies the system that element's contents were marked as dirty. */
		void notifyContentDirty(GUIElement* element);

		/** Notifies the system that element's mesh was marked as dirty. */
		void notifyMeshDirty(GUIElement* element);
		
	private:
		/** Single render element in a GUIDrawGroup */
		struct GUIGroupRenderElement
		{
			GUIGroupRenderElement() = default;
			GUIGroupRenderElement(GUIElement* element, UINT32 renderElementIdx)
				:element(element), renderElementIdx(renderElementIdx)
			{ }

			GUIElement* element = nullptr;
			UINT32 renderElementIdx = 0;
		};

		/** Contains information about all draw groups a GUI element is part of. */
		struct GUIGroupElement
		{
			GUIElement* element = nullptr;
			SmallVector<INT32, 4> groups;
			Rect2I bounds;
		};
		
		/** Data required for rendering a single GUI mesh. */
		struct GUIMesh
		{
			UINT32 indexOffset = 0;
			UINT32 indexCount = 0;
			SpriteMaterial* material;
			SpriteMaterialInfo matInfo;
			bool isLine;
		};

		/** Holds information about a set of GUI elements that can be drawn together. */
		struct GUIDrawGroup
		{
			INT32 id = 0;
			UINT32 depthRange = 0;
			UINT32 minDepth = 0;
			bool dirtyBounds = true;
			bool needsRedraw = true;
			Rect2I bounds;
			Vector<GUIGroupRenderElement> cachedElements;
			Vector<GUIGroupRenderElement> nonCachedElements;
			Vector<GUIMesh> meshes;
			SPtr<RenderTexture> outputTexture;
		};

		/** Splits the provided draw group at the specified depth. Returns the second half of the group. */
		GUIDrawGroup& split(UINT32 groupIdx, UINT32 depth);

		/** Rebuilds the GUI element meshes. */
		void rebuildMeshes();

		/**
		 * Adds a specific render element of a GUI element to the specified draw group. Caller is responsible for
		 * ensuring the element is a valid match for the group.
		 */
		void add(GUIGroupElement& element, UINT32 renderElementIdx, UINT32 groupIdx);

		/** Adds a specific render element of a GUI element and adds it to a suitable draw group. */
		void add(GUIGroupElement& element, UINT32 renderElementIdx);

		/**
		 * Removes a specific render element in the provided GUI element from the provided draw group. Caller is
		 * responsible for ensuring the provided draw group is the element's current draw group.
		 */
		void remove(GUIGroupElement& element, UINT32 renderElementIdx, UINT32 groupIdx);

		/** Removes a specific render element in the provided GUI element from their current draw group. */
		void remove(GUIGroupElement& element, UINT32 renderElementIdx);

		/** Builds a structure with information required for rendering the provided mesh. */
		static GUIMeshRenderData getRenderData(const GUIMesh& guiMesh);

		/** Builds a structure with information required for rendering the provided draw group. */
		static GUIDrawGroupRenderData getRenderData(const GUIDrawGroup& drawGroup);
		
		/** Calculates the bounds of all visible elements in the draw group. */
		static Rect2I calculateBounds(GUIDrawGroup& group);

		Vector<GUIDrawGroup> mDrawGroups;
		UnorderedMap<GUIElement*, GUIGroupElement> mElements;
		UnorderedMap<GUIElement*, UINT32> mDirtyElements;
		bool mGroupsCoreDirty = true;
		
		SPtr<Mesh> mTriangleMesh;
		SPtr<Mesh> mLineMesh;
		mutable INT32 mNextDrawGroupId = 0;
	};
	
	/** @} */
	
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
		 * Rebuilds any dirty data required for GUI element rendering and returns the data required for updating the GUI
		 * renderer.
		 */
		GUIDrawGroupRenderDataUpdate rebuildDirtyRenderData();

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

		/**	Called when the parent window gained or lost focus. */
		virtual void ownerWindowFocusChanged();
	private:
		struct GUIGroupElement
		{
			GUIGroupElement() = default;
			GUIGroupElement(GUIElement* element, UINT32 renderElement)
				:element(element), renderElement(renderElement)
			{ }

			GUIElement* element = nullptr;
			UINT32 renderElement = 0;
		};

		/** Holds information about a set of GUI elements that can be drawn together. */
		struct GUIDrawGroup
		{
			UINT32 id = 0;
			UINT32 depthRange = 0;
			UINT32 minDepth = 0;
			Rect2I bounds;
			Vector<GUIGroupElement> cachedElements;
			Vector<GUIGroupElement> nonCachedElements;
		};
		
		/**	Calculates widget bounds using the bounds of all child elements. */
		void updateBounds() const;

		/**	Updates the size of the primary GUI panel based on the viewport. */
		void updateRootPanel();

		SPtr<Camera> mCamera;
		Vector<GUIElement*> mElements;
		GUIDrawGroups mDrawGroups;
		GUIPanel* mPanel = nullptr;
		UINT8 mDepth = 128;
		bool mIsActive = true;
		SPtr<GUINavGroup> mDefaultNavGroup;

		Vector3 mPosition = BsZero;
		Quaternion mRotation = BsIdentity;
		Vector3 mScale = Vector3::ONE;
		Matrix4 mTransform = BsIdentity;

		Set<GUIElement*> mDirtyContents;
		Set<GUIElement*> mDirtyContentsTemp;

		mutable UINT64 mCachedRTId = 0;
		mutable bool mWidgetIsDirty = false;
		mutable Rect2I mBounds;

		HGUISkin mSkin;
	};

	/** @} */
}
