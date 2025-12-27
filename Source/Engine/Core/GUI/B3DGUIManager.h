//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIMeshBatches.h"
#include "B3DPrerequisites.h"
#include "Renderer/B3DRendererExtension.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "GUI/B3DGUITextInputEvent.h"
#include "GUI/B3DGUICommandEvent.h"
#include "GUI/B3DGUIVirtualButtonEvent.h"
#include "2D/B3DSprite.h"
#include "Utility/B3DModule.h"
#include "Image/B3DColor.h"
#include "Math/B3DMatrix4.h"
#include "Utility/B3DEvent.h"
#include "Material/B3DMaterialParam.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "RenderAPI/B3DGpuDevice.h"

namespace b3d
{
	class GUIVectorSpriteAtlas;
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	namespace render
	{
		class GUIRenderer;
	}

	/**
	 * Manages the rendering and input of all GUI widgets in the scene.
	 *
	 * @note
	 * If adding or modifying GUIManager functionality ensure that GUIManager data never gets modified outside of update()
	 * method or Input callbacks. If you need such functionality add temporary variables that store you changes and then
	 * execute them delayed in update().
	 * @par
	 * This ensures that GUIElements don't recursively modify GUIManager while GUIManager is still using that data.
	 * @par
	 * For example setFocus() usually gets called from within GUIElements, however we don't want elements in focus be
	 * modified immediately since that setFocus() call could have originated in sendCommandEvent and elements in focus array
	 * would be modified while still being iterated upon.
	 */
	class B3D_EXPORT GUIManager : public Module<GUIManager>
	{
		/**	Valid states of a drag and drop operation. */
		enum class DragState
		{
			NoDrag,
			HeldWithoutDrag,
			Dragging
		};

		/**	Container for a GUI widget. */
		struct WidgetInfo
		{
			WidgetInfo(GUIWidget* _widget)
				: Widget(_widget)
			{}

			GUIWidget* Widget;
		};

		/**	Container for data about a single GUI element and its widget. */
		struct ElementInfo
		{
			ElementInfo(GUIInteractable* element, GUIWidget* widget)
				: Element(element), Widget(widget)
			{}

			GUIInteractable* Element;
			GUIWidget* Widget;
		};

		/**	Container for data about a single GUI element and its widget currently under the pointer. */
		struct ElementInfoUnderPointer
		{
			ElementInfoUnderPointer(GUIInteractable* element, GUIWidget* widget)
				: Element(element), Widget(widget), UsesMouseOver(false), ReceivedMouseOver(false), IsHovering(false)
			{}

			GUIInteractable* Element;
			GUIWidget* Widget;
			bool UsesMouseOver;
			bool ReceivedMouseOver;
			bool IsHovering;
		};

		/**	Container for GUI element in focus. */
		struct ElementFocusInfo
		{
			ElementFocusInfo(GUIInteractable* element, GUIWidget* widget, bool usesFocus)
				: Element(element), Widget(widget), UsesFocus(usesFocus)
			{}

			GUIInteractable* Element;
			GUIWidget* Widget;
			bool UsesFocus;
		};

		/**	Container for GUI elements that need to have their focus state forcefully changed. */
		struct ElementForcedFocusInfo
		{
			GUIInteractable* Element;
			bool Focus;
		};

	public:
		GUIManager();
		~GUIManager();

		/** Registers a newly created widget with the GUI manager. This should be called by every GUI widget on creation. */
		void RegisterWidget(GUIWidget* widget);

		/**
		 * Unregisters a GUI widget from the GUI manager. This should be called by every GUI widget before getting deleted.
		 */
		void UnregisterWidget(GUIWidget* widget);

		/**	Called once per frame. */
		void Update();

		/** Queues the GUI element for destruction. Element will be destroyed during the next call to update(). */
		void QueueForDestroy(GUIElement* element);

		/** Forces all GUI elements that are queued for destruction to be destroyed immediately. */
		void ProcessDestroyQueue();

		/**
		 * Change the GUI element focus state.
		 *
		 * @param[in]	element		Element whose focus state to change
		 * @param[in]	focus		Give the element focus or take it away.
		 * @param[in]	clear		If true the focus will be cleared from any elements currently in focus. Otherwise
		 *							the element will just be appended to the in-focus list (if enabling focus).
		 */
		void SetFocus(GUIInteractable* element, bool focus, bool clear);

		/**	Changes the color of the input caret used in input boxes and similar controls. */
		void SetCaretColor(const Color& color)
		{
			mCaretColor = color;
			UpdateCaretTexture();
		}

		/**	Changes the text selection highlight color used in input boxes and similar controls. */
		void SetTextSelectionColor(const Color& color)
		{
			mTextSelectionColor = color;
			UpdateTextSelectionTexture();
		}

		/**	Returns the default caret texture used for rendering the input caret sprite. */
		const HSpriteTexture& GetCaretTexture() const { return mCaretImage; }

		/**	Returns the default selection highlight texture used for rendering the selection highlight sprites. */
		const HSpriteTexture& GetTextSelectionTexture() const { return mTextSelectionImage; }

		/**	Checks is the input caret visible this frame. */
		bool GetCaretBlinkState() const { return mIsCaretOn; }

		/**
		 * Returns input caret helper tool that allows you to easily position and show an input caret in your GUI controls.
		 */
		GUIInputCaret* GetInputCaretTool() const { return mInputCaret; }

		/**
		 * Returns input selection helper tool that allows you to easily position and show an input selection highlight in
		 * your GUI controls.
		 */
		GUIInputSelection* GetInputSelectionTool() const { return mInputSelection; }

		/**
		 * Returns an atlas that vector paths are rasterized into. Any GUI element using vector paths will register the path in this atlas.
		 * GUI manager will then rasterize the shapes before they are needed in GUI rendering.
		 */
		GUIVectorSpriteAtlas& GetVectorSpriteAtlas() const { return *mVectorSpriteAtlas; }

		/**
		 * Allows you to bridge GUI input from a GUI element into another render target.
		 *
		 * @param[in]	renderTex 	The render target to which to bridge the input.
		 * @param[in]	element		The element from which to bridge input. Input will be transformed according to this
		 *							elements position and size. Provide nullptr if you want to remove a bridge for the
		 *							specified widget.
		 *
		 * @note
		 * This is useful if you use render textures, where your GUI is rendered off-screen. In such case you need to
		 * display the render texture within another GUIElement in a GUIWidget, but have no way of sending input to the
		 * render texture (normally input is only sent to render windows). This allows you to change that - any GUIWidget
		 * using the bridged render texture as a render target will then receive input when mouse is over the specified
		 * element.
		 * @note
		 * Bridged element needs to remove itself as the bridge when it is destroyed.
		 */
		void SetInputBridge(const SPtr<RenderTexture>& renderTex, const GUIInteractable* element);

		/**
		 * Converts window coordinates to coordinates relative to the specified bridged render target (target displayed
		 * with a GUI element). Returned coordinates will be relative to the bridge element.
		 *
		 * @return	If provided widget has no bridge, coordinates are returned as is.
		 */
		GUIPhysicalPoint WindowToBridgedCoords(const SPtr<RenderTarget>& target, const GUIPhysicalPoint& windowPos) const;

		/**
		 * Returns the render window that holds the GUI element that displays the provided render texture.
		 *
		 * @param[in]	target	Render texture to find the bridged window for.
		 * @return				Window that displays the GUI element with the render texture, or null if the render texture
		 *						is not bridged.
		 */
		SPtr<RenderWindow> GetBridgeWindow(const SPtr<RenderTexture>& target) const;

		/** Returns all GUI elements that have input bridging set up and belong to the provided GUI widget. */
		void GetBridgedElements(const GUIWidget* widget, TInlineArray<std::pair<const GUIInteractable*, SPtr<const RenderTarget>>, 4>& elements);

		/**	Returns the parent render window of the specified widget. */
		const RenderWindow* GetWidgetWindow(const GUIWidget& widget) const;

	private:
		friend class render::GUIRenderer;

		/**	Recreates the input caret texture. */
		void UpdateCaretTexture();

		/**	Recreates the input text selection highlight texture. */
		void UpdateTextSelectionTexture();

		/**
		 * Destroys the render thread counterpart of the GUI manager.
		 *
		 * @param[in]	renderer	Previously constructed render thread GUI manager instance.
		 */
		void DestroyRenderer(render::GUIRenderer* renderer);

		/**
		 * Destroys any elements or widgets queued for destruction.
		 *
		 * @note
		 * Returns true if more elements have been added for destruction (will happen when destruction of one element
		 * queues up destruction of another). Usually needs to be run in a loop with multiple iterations.
		 */
		bool ProcessDestroyQueueIteration();

		/**
		 * Finds a GUI element under the pointer at the specified screen position. This method will also trigger pointer
		 * move/hover/leave events.
		 *
		 * @param[in]	screenMousePos	Position of the pointer in screen coordinates.
		 * @param[in]	buttonStates	States of the three mouse buttons (left, right, middle).
		 * @param[in]	shift			Is shift key held.
		 * @param[in]	control			Is control key held.
		 * @param[in]	alt				Is alt key held.
		 */
		bool FindElementUnderPointer(const GUIPhysicalPoint& screenMousePos, bool buttonStates[3], bool shift, bool control, bool alt);

		/**	Called whenever a pointer (for example mouse cursor) is moved. */
		void OnPointerMoved(const PointerEvent& event);

		/**	Called whenever a pointer button (for example mouse button) is released. */
		void OnPointerReleased(const PointerEvent& event);

		/**	Called whenever a pointer button (for example mouse button) is pressed. */
		void OnPointerPressed(const PointerEvent& event);

		/**	Called whenever a pointer button (for example mouse button) is double clicked. */
		void OnPointerDoubleClick(const PointerEvent& event);

		/**	Called whenever a text is input. */
		void OnTextInput(const TextInputEvent& event);

		/**	Called whenever an input command is input. */
		void OnInputCommandEntered(InputCommandType commandType);

		/**	Called whenever a virtual button is pressed. */
		void OnVirtualButtonDown(const VirtualButton& button, u32 deviceIdx);

		/**	Called by the drag and drop managed to notify us the drag ended. */
		void OnMouseDragEnded(const PointerEvent& event, DragCallbackInfo& dragInfo);

		/**	Called when the specified window gains focus. */
		void OnWindowFocusGained(RenderWindow& win);

		/**	Called when the specified window loses focus. */
		void OnWindowFocusLost(RenderWindow& win);

		/**	Called when the mouse leaves the specified window. */
		void OnMouseLeftWindow(RenderWindow& win);

		/**	Converts pointer buttons to mouse buttons. */
		GUIMouseButton ButtonToGuiButton(PointerEventButton pointerButton) const;

		/**	Converts screen coordinates to coordinates relative to the specified widget. */
		GUIPhysicalPoint GetWidgetRelativePos(const GUIWidget* widget, const GUIPhysicalPoint& screenPos) const;

		/**	Hides the tooltip if any is shown. */
		void HideTooltip();

		/** Switches the focus to the first element in the tab group. */
		void TabFocusFirst();

		/** Switches the focus to the next element in the tab group. Usually triggered when the user hits Tab key. */
		void TabFocusNext();

		/**
		 * Sends a mouse event to the specified GUI element.
		 *
		 * @param[in]	element	Element to send the event to.
		 * @param[in]	event	Event data.
		 */
		bool SendMouseEvent(GUIInteractable* element, const GUIMouseEvent& event);

		/**
		 * Sends a text input event to the specified GUI element.
		 *
		 * @param[in]	element	Element to send the event to.
		 * @param[in]	event	Event data.
		 */
		bool SendTextInputEvent(GUIInteractable* element, const GUITextInputEvent& event);

		/**
		 * Sends a command event to the specified GUI element.
		 *
		 * @param[in]	element	Element to send the event to.
		 * @param[in]	event	Event data.
		 */
		bool SendCommandEvent(GUIInteractable* element, const GUICommandEvent& event);

		/**
		 * Sends a virtual button event to the specified GUI element.
		 *
		 * @param[in]	element	Element to send the event to.
		 * @param[in]	event	Event data.
		 */
		bool SendVirtualButtonEvent(GUIInteractable* element, const GUIVirtualButtonEvent& event);

		static const u32 kDragDistance;
		static const float kTooltipHoverTime;

		static const u32 kMeshHeapInitialNumVerts;
		static const u32 kMeshHeapInitialNumIndices;

		Vector<WidgetInfo> mWidgets;
		SPtr<render::GUIRenderer> mRenderer;

		Stack<GUIElement*> mScheduledForDestruction;

		// Element and widget pointer is currently over
		Vector<ElementInfoUnderPointer> mElementsUnderPointer;
		Vector<ElementInfoUnderPointer> mNewElementsUnderPointer;

		// Element and widget that's being clicked on
		GUIMouseButton mActiveMouseButton = GUIMouseButton::Left;
		Vector<ElementInfo> mActiveElements;
		Vector<ElementInfo> mNewActiveElements;

		// Element and widget that currently have the keyboard focus
		Vector<ElementFocusInfo> mElementsInFocus;
		Vector<ElementFocusInfo> mNewElementsInFocus;
		UnorderedMap<RenderWindow*, Vector<ElementFocusInfo>> mSavedFocusElements;

		bool mForcedClearFocus = false;
		Vector<ElementForcedFocusInfo> mForcedFocusElements;

		// Tooltip
		bool mShowTooltip = false;
		float mTooltipElementHoverStart = 0.0f;

		GUIInputCaret* mInputCaret = nullptr;
		GUIInputSelection* mInputSelection = nullptr;

		Vector2I mLastPointerScreenPos{BsZero};

		DragState mDragState = DragState::NoDrag;
		GUIPhysicalPoint mLastPointerClickPos{BsZero};
		GUIPhysicalPoint mDragStartPos{BsZero};

		GUIMouseEvent mMouseEvent;
		GUITextInputEvent mTextInputEvent;
		GUICommandEvent mCommandEvent;
		GUIVirtualButtonEvent mVirtualButtonEvent;

		HSpriteTexture mCaretImage;
		Color mCaretColor{ 1.0f, 0.6588f, 0.0f };
		float mCaretBlinkInterval = 0.5f;
		float mCaretLastBlinkTime = 0.0f;
		bool mIsCaretOn = false;
		CursorType mActiveCursor = CursorType::Arrow;

		HSpriteTexture mTextSelectionImage;
		Color mTextSelectionColor{ 0.0f, 114 / 255.0f, 188 / 255.0f };

		Map<SPtr<const RenderTexture>, const GUIInteractable*> mInputBridge;
		UPtr<GUIVectorSpriteAtlas> mVectorSpriteAtlas;

		HEvent mOnPointerMovedConn;
		HEvent mOnPointerPressedConn;
		HEvent mOnPointerReleasedConn;
		HEvent mOnPointerDoubleClick;
		HEvent mOnTextInputConn;
		HEvent mOnInputCommandConn;
		HEvent mOnVirtualButtonDown;

		HEvent mDragEndedConn;

		HEvent mWindowGainedFocusConn;
		HEvent mWindowLostFocusConn;

		HEvent mMouseLeftWindowConn;
	};

	namespace render
	{
		/**	Handles GUI rendering on the render thread. */
		class B3D_EXPORT GUIRenderer : public RendererExtension
		{
			friend class b3d::GUIManager;

		public:
			GUIRenderer();

			void Initialize(const Any& data) override;
			RendererExtensionRequest Check(const Camera& camera) override;
			void Render(const Camera& camera, const RendererViewContext& viewContext) override;

		private:
			/** Called every frame from the main thread with the time of the current frame. */
			void Update(float time);

			/** Updates the data required for rendering draw groups on the specified widget. */
			void UpdateDrawGroups(const Camera* camera, u64 widgetId, u32 widgetDepth, const Matrix4& worldTransform, const GUIDrawGroupRenderDataUpdate& data);

			/** Clears all draw groups from the specified widget. */
			void ClearDrawGroups(u64 widgetId);

			struct GUIBatchGpuParameterInfo
			{
				u32 MaterialParameterIndex = ~0u;
			};

			struct GUIWidgetRenderData
			{
				GUIWidgetRenderData() = default;
				GUIWidgetRenderData(u64 widgetId, GpuDevice& device);

				u64 WidgetId;
				u32 WidgetDepth = 0;
				Vector<GUIBatchRenderData> Batches;
				TArray<GUIBatchGpuParameterInfo> GpuParameterInfos;
				TArray<SPtr<MaterialParameterAdapter>> MaterialParameterAdapters;
				TransientGpuBufferPool UniformBufferPool;
				TransientGpuBufferPool ClipRegionBufferPool;

				Matrix4 WorldTransform = Matrix4::kIdentity;
			};

			struct GUICameraRenderData
			{
				Vector<GUIWidgetRenderData> WidgetRenderData;
				SPtr<RenderTexture> CachedRenderTexture;
				Vector<Area2I> DirtyRegions;
				Vector<Area2I> LastFrameDirtyDebugDrawRegions;
			};

			UnorderedMap<const Camera*, GUICameraRenderData> mPerCameraData;
			UnorderedMap<u64, const Camera*> mWidgetToCameraMap;
			UnorderedMap<SpriteMaterial*, TArray<SPtr<MaterialParameterAdapter>>> mMaterialParameterAdapterPool;
			SPtr<SamplerState> mSamplerState;
			float mTime = 0.0f;
		};
	} // namespace render

	/** Provides easier access to GUIManager. */
	B3D_EXPORT GUIManager& GetGUIManager();

	/** @} */
} // namespace b3d
