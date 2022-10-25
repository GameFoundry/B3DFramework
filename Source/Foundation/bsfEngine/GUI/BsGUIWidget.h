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
		SubMesh SubMesh;
		SPtr<ct::Texture> Texture;
		SPtr<ct::SpriteTexture> SpriteTexture;
		SpriteMaterial* Material;
		Color Tint;
		float AnimationStartTime;
		SPtr<SpriteMaterialExtraInfo> AdditionalData;
		bool IsLine;
		u32 BufferIdx;
	};

	/** Information about a GUI element that is displaying a render target. */
	struct GUIRenderTargetRenderData
	{
		GUIRenderTargetRenderData() = default;

		GUIRenderTargetRenderData(SPtr<ct::RenderTarget> target)
			: Target(std::move(target))
		{}

		SPtr<ct::RenderTarget> Target;
		u64 LastUpdateCount = (u64)-1;
	};

	/** Data required for rendering all the batches in a single GUI draw group. */
	struct GUIDrawGroupRenderData
	{
		i32 Id = 0;
		SPtr<ct::RenderTexture> Destination;
		Rect2I Bounds;
		SubMesh SubMesh;
		bool RequiresRedraw = true;
		u32 BufferIdx = 0;

		Vector<GUIMeshRenderData> CachedElements;
		Vector<GUIMeshRenderData> NonCachedElements;
		Vector<GUIRenderTargetRenderData> RenderTargetElements;
	};

	/**
	 * Contains data about which draw group needs to be redrawn, as well as a set of new draw groups if
	 * draw groups were updated.
	 */
	struct GUIDrawGroupRenderDataUpdate
	{
		Vector<GUIDrawGroupRenderData> NewDrawGroups;
		Vector<bool> GroupDirtyState;
		SPtr<ct::Mesh> TriangleMesh;
		SPtr<ct::Mesh> LineMesh;
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
		GUIDrawGroups(GUIWidget* parentWidget);

		/** Iterates over all the render elements in the GUI elements and adds them to suitable draw groups. */
		void Add(GUIElement* element);

		/** Removes all render elements in the provided GUI element from their current set of draw groups. */
		void Remove(GUIElement* element);

		/** Rebuilds any dirty internal data and returns the data structure required for updating the GUI renderer. */
		GUIDrawGroupRenderDataUpdate RebuildDirty(bool forceRebuildMeshes);

		/** Notifies the system that element's contents were marked as dirty. */
		void NotifyContentDirty(GUIElement* element);

		/** Notifies the system that element's mesh was marked as dirty. */
		void NotifyMeshDirty(GUIElement* element);

	private:
		/** Single render element in a GUIDrawGroup */
		struct GUIGroupRenderElement
		{
			GUIGroupRenderElement() = default;

			GUIGroupRenderElement(GUIElement* element, u32 renderElementIdx)
				: Element(element), RenderElementIdx(renderElementIdx)
			{}

			GUIElement* Element = nullptr;
			u32 RenderElementIdx = 0;
		};

		/** Contains information about all draw groups a GUI element is part of. */
		struct GUIGroupElement
		{
			GUIElement* Element = nullptr;
			SmallVector<i32, 4> Groups;
			Rect2I Bounds;
		};

		/** Data required for rendering a single GUI mesh. */
		struct GUIMesh
		{
			u32 IndexOffset = 0;
			u32 IndexCount = 0;
			SpriteMaterial* Material;
			SpriteMaterialInfo MatInfo;
			bool IsLine;
		};

		/** Holds information about a set of GUI elements that can be drawn together. */
		struct GUIDrawGroup
		{
			i32 Id = 0;
			u32 DepthRange = 0;
			u32 MinDepth = 0;
			bool DirtyBounds = true;
			bool NeedsRedraw = true;
			bool DirtyTexture = true;
			Rect2I Bounds;
			Vector<GUIGroupRenderElement> CachedElements;
			Vector<GUIGroupRenderElement> NonCachedElements;
			Vector<GUIMesh> Meshes;
			SPtr<RenderTexture> OutputTexture;
		};

		/** Splits the provided draw group at the specified depth. Returns the second half of the group. */
		GUIDrawGroup& Split(u32 groupIdx, u32 depth);

		/** Rebuilds the GUI element meshes. */
		void RebuildMeshes();

		/**
		 * Adds a specific render element of a GUI element to the specified draw group. Caller is responsible for
		 * ensuring the element is a valid match for the group.
		 */
		void Add(GUIGroupElement& element, u32 renderElementIdx, u32 groupIdx);

		/** Adds a specific render element of a GUI element and adds it to a suitable draw group. */
		void Add(GUIGroupElement& element, u32 renderElementIdx);

		/**
		 * Removes a specific render element in the provided GUI element from the provided draw group. Caller is
		 * responsible for ensuring the provided draw group is the element's current draw group.
		 */
		void Remove(GUIGroupElement& element, u32 renderElementIdx, u32 groupIdx);

		/** Removes a specific render element in the provided GUI element from their current draw group. */
		void Remove(GUIGroupElement& element, u32 renderElementIdx);

		/** Builds a structure with information required for rendering the provided mesh. */
		static GUIMeshRenderData GetRenderData(const GUIMesh& guiMesh);

		/** Builds a structure with information required for rendering the provided draw group. */
		static GUIDrawGroupRenderData GetRenderData(const GUIDrawGroup& drawGroup);

		/** Calculates the bounds of all visible elements in the draw group. */
		static Rect2I CalculateBounds(GUIDrawGroup& group);

		Vector<GUIDrawGroup> mDrawGroups;
		UnorderedMap<GUIElement*, GUIGroupElement> mElements;
		UnorderedMap<GUIElement*, u32> mDirtyElements;
		bool mGroupsCoreDirty = true;
		GUIWidget* mWidget;

		SPtr<Mesh> mTriangleMesh;
		SPtr<Mesh> mLineMesh;
		mutable i32 mNextDrawGroupId = 0;
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
		void RegisterElementInternal(GUIElementBase* elem);

		/**
		 * Unregisters an element from the widget. Usually called when the element is destroyed, or reparented to another
		 * widget.
		 */
		void UnregisterElementInternal(GUIElementBase* elem);

		/**
		 * Returns the default navigation group assigned to all elements of this widget that don't have an explicit nav-
		 * group. See GUIElement::setNavGroup().
		 */
		SPtr<GUINavGroup> GetDefaultNavGroupInternal() const { return mDefaultNavGroup; }

		/**
		 * Marks the widget mesh dirty requiring a mesh rebuild. Provided element is the one that requested the mesh update.
		 */
		void MarkMeshDirtyInternal(GUIElementBase* elem);

		/**
		 * Marks the elements content as dirty, meaning its internal mesh will need to be rebuilt (this implies the entire
		 * widget mesh will be rebuilt as well).
		 */
		void MarkContentDirtyInternal(GUIElementBase* elem);

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
		GUIDrawGroups mDrawGroups;
		GUIPanel* mPanel = nullptr;
		u8 mDepth = 128;
		bool mIsActive = true;
		SPtr<GUINavGroup> mDefaultNavGroup;

		Vector3 mPosition = BsZero;
		Quaternion mRotation = BsIdentity;
		Vector3 mScale = Vector3::ONE;
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
