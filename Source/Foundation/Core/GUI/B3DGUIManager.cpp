//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIManager.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUIInteractable.h"
#include "Image/B3DSpriteTexture.h"
#include "Utility/B3DTime.h"
#include "Scene/B3DSceneObject.h"
#include "Material/B3DMaterial.h"
#include "Mesh/B3DMeshData.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Mesh/B3DMesh.h"
#include "Managers/B3DRenderWindowManager.h"
#include "Platform/B3DPlatform.h"
#include "Math/B3DArea2.h"
#include "B3DGUIMeshBatches.h"
#include "Error/B3DException.h"
#include "Input/B3DInput.h"
#include "GUI/B3DGUIInputCaret.h"
#include "GUI/B3DGUIInputSelection.h"
#include "GUI/B3DGUIContextMenu.h"
#include "GUI/B3DDragAndDrop.h"
#include "GUI/B3DGUIDropDownBoxManager.h"
#include "GUI/B3DGUIPanel.h"
#include "GUI/B3DGUINavGroup.h"
#include "Profiling/B3DProfilerCPU.h"
#include "Input/B3DVirtualInput.h"
#include "Platform/B3DCursor.h"
#include "CoreObject/B3DRenderThread.h"
#include "Renderer/B3DRendererManager.h"
#include "Renderer/B3DRenderer.h"
#include "Components/B3DCamera.h"
#include "GUI/B3DGUITooltipManager.h"
#include "Renderer/B3DRendererUtility.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "RenderAPI/B3DSamplerState.h"
#include "Resources/B3DBuiltinResources.h"
#include "2D/B3DSpriteManager.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "VectorGraphics/B3DVectorSpriteAtlas.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "RenderAPI/B3DGpuBufferPool.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"


using namespace b3d;

const u32 GUIManager::kDragDistance = 3;
const float GUIManager::kTooltipHoverTime = 1.0f;

GUIManager::GUIManager()
{
	// Note: Hidden dependency. GUI must receive input events before other systems, in order so it can mark them as used
	// if required. e.g. clicking on a context menu should mark the event as used so that other non-GUI systems know
	// that they probably should not process such event themselves.
	mOnPointerMovedConn = GetInput().OnPointerMoved.Connect([this](const PointerEvent& event) { OnPointerMoved(event); });
	mOnPointerPressedConn = GetInput().OnPointerPressed.Connect([this](const PointerEvent& event) { OnPointerPressed(event); });
	mOnPointerReleasedConn = GetInput().OnPointerReleased.Connect([this](const PointerEvent& event) { OnPointerReleased(event); });
	mOnPointerDoubleClick = GetInput().OnPointerDoubleClick.Connect([this](const PointerEvent& event) { OnPointerDoubleClick(event); });
	mOnTextInputConn = GetInput().OnCharInput.Connect([this](const TextInputEvent& event) { OnTextInput(event); });
	mOnInputCommandConn = GetInput().OnInputCommand.Connect([this](InputCommandType commandType) { OnInputCommandEntered(commandType); });
	mOnVirtualButtonDown = VirtualInput::Instance().OnButtonDown.Connect([this](const VirtualButton& button, u32 deviceIndex) { OnVirtualButtonDown(button, deviceIndex); });

	mWindowGainedFocusConn = RenderWindowManager::Instance().OnFocusGained.Connect([this](RenderWindow& window) { OnWindowFocusGained(window); });
	mWindowLostFocusConn = RenderWindowManager::Instance().OnFocusLost.Connect([this](RenderWindow& window) { OnWindowFocusLost(window); });
	mMouseLeftWindowConn = RenderWindowManager::Instance().OnMouseLeftWindow.Connect([this](RenderWindow& window) { OnMouseLeftWindow(window); });

	mInputCaret = B3DNew<GUIInputCaret>();
	mInputSelection = B3DNew<GUIInputSelection>();

	DragAndDrop::StartUp();
	mDragEndedConn = DragAndDrop::Instance().OnDragEnded.Connect([this](const PointerEvent& event, DragCallbackInfo& dragInfo) { OnMouseDragEnded(event, dragInfo); });

	GUIDropDownBoxManager::StartUp();
	GUITooltipManager::StartUp();

	// Need to defer this call because I want to make sure all managers are initialized first
	DeferredCall([this]() { UpdateCaretTexture(); });
	DeferredCall([this]() { UpdateTextSelectionTexture(); });

	mVectorSpriteAtlas = B3DMakeUnique<GUIVectorSpriteAtlas>(GUIVectorSpriteAtlasSettings());
	mRenderer = RendererExtension::Create<render::GUIRenderer>(nullptr);
}

GUIManager::~GUIManager()
{
	GUITooltipManager::ShutDown();
	GUIDropDownBoxManager::ShutDown();
	DragAndDrop::ShutDown();

	// Make a copy of widgets, since destroying them will remove them from mWidgets and
	// we can't iterate over an array thats getting modified
	Vector<WidgetInfo> widgetCopy = mWidgets;
	for(auto& widget : widgetCopy)
		widget.Widget->Destroy(true);

	// Ensure everything queued get destroyed
	ProcessDestroyQueue();

	mOnPointerPressedConn.Disconnect();
	mOnPointerReleasedConn.Disconnect();
	mOnPointerMovedConn.Disconnect();
	mOnPointerDoubleClick.Disconnect();
	mOnTextInputConn.Disconnect();
	mOnInputCommandConn.Disconnect();
	mOnVirtualButtonDown.Disconnect();

	mDragEndedConn.Disconnect();

	mWindowGainedFocusConn.Disconnect();
	mWindowLostFocusConn.Disconnect();

	mMouseLeftWindowConn.Disconnect();

	B3DDelete(mInputCaret);
	B3DDelete(mInputSelection);
}

void GUIManager::DestroyRenderer(render::GUIRenderer* renderer)
{
	B3DDelete(renderer);
}

void GUIManager::RegisterWidget(GUIWidget* widget)
{
	const Viewport* renderTarget = widget->GetTarget();
	if(renderTarget == nullptr)
		return;

	mWidgets.push_back(WidgetInfo(widget));
}

void GUIManager::UnregisterWidget(GUIWidget* widget)
{
	{
		auto found = std::find_if(begin(mWidgets), end(mWidgets), [=](const WidgetInfo& x)
									 { return x.Widget == widget; });

		if(found != mWidgets.end())
			mWidgets.erase(found);
	}

	for(auto& entry : mElementsInFocus)
	{
		if(entry.Widget == widget)
			entry.Widget = nullptr;
	}

	for(auto& elementsPerWindow : mSavedFocusElements)
	{
		for(auto& entry : elementsPerWindow.second)
		{
			if(entry.Widget == widget)
				entry.Widget = nullptr;
		}
	}

	for(auto& entry : mElementsUnderPointer)
	{
		if(entry.Widget == widget)
			entry.Widget = nullptr;
	}

	for(auto& entry : mNewElementsUnderPointer)
	{
		if(entry.Widget == widget)
			entry.Widget = nullptr;
	}

	for(auto& entry : mActiveElements)
	{
		if(entry.Widget == widget)
			entry.Widget = nullptr;
	}

	auto widgetId = (u64)widget;
	GetRenderThread().PostCommand([renderer = mRenderer.get(),
								widgetId]()
							   { renderer->ClearDrawGroups(widgetId); }, "GUIRenderer::ClearDrawGroups");
}

void GUIManager::Update()
{
	DragAndDrop::Instance().Update();

	// Show tooltip if needed
	if(mShowTooltip)
	{
		float diff = GetTime().GetRealTimeInSeconds() - mTooltipElementHoverStart;
		if(diff >= kTooltipHoverTime || GetInput().IsButtonHeld(ButtonCode::LeftControl) || GetInput().IsButtonHeld(ButtonCode::RightControl))
		{
			for(auto& entry : mElementsUnderPointer)
			{
				const String& tooltipText = entry.Element->GetTooltip();
				GUIWidget* parentWidget = entry.Element->GetParentWidget();

				if(!tooltipText.empty() && parentWidget != nullptr)
				{
					const RenderWindow* window = GetWidgetWindow(*parentWidget);
					if(window != nullptr)
					{
						GUIPhysicalPoint windowPos = window->ScreenToWindowPosition(GetInput().GetPointerPosition()).To<GUIPhysicalUnit>();

						GUITooltipManager::Instance().Show(*parentWidget, windowPos, tooltipText);
						break;
					}
				}
			}

			mShowTooltip = false;
		}
	}

	// Update layouts
	GetProfilerCPU().BeginSample("UpdateLayout");
	for(auto& widgetInfo : mWidgets)
	{
		widgetInfo.Widget->UpdateLayout();
	}
	GetProfilerCPU().EndSample("UpdateLayout");

	// Destroy all queued elements (and loop in case any new ones get queued during destruction)
	do
	{
		mNewElementsUnderPointer.clear();
		for(auto& elementInfo : mElementsUnderPointer)
		{
			if(!elementInfo.Element->IsPendingDestroy())
				mNewElementsUnderPointer.push_back(elementInfo);
		}

		mElementsUnderPointer.swap(mNewElementsUnderPointer);

		mNewActiveElements.clear();
		for(auto& elementInfo : mActiveElements)
		{
			if(!elementInfo.Element->IsPendingDestroy())
				mNewActiveElements.push_back(elementInfo);
		}

		mActiveElements.swap(mNewActiveElements);

		mNewElementsInFocus.clear();

		for(auto& elementInfo : mElementsInFocus)
		{
			if(!elementInfo.Element->IsPendingDestroy())
				mNewElementsInFocus.push_back(elementInfo);
		}

		mElementsInFocus.swap(mNewElementsInFocus);

		for(auto& elementsPerWindow : mSavedFocusElements)
		{
			mNewElementsInFocus.clear();
			for(auto& entry : elementsPerWindow.second)
			{
				if(!entry.Element->IsPendingDestroy())
					mNewElementsInFocus.push_back(entry);
			}

			elementsPerWindow.second.swap(mNewElementsInFocus);
		}

		if(mForcedClearFocus)
		{
			// Clear focus on all elements that aren't part of the forced focus list (in case they are already in focus)
			mCommandEvent.SetType(GUICommandEventType::FocusLost);

			for(auto iter = mElementsInFocus.begin(); iter != mElementsInFocus.end();)
			{
				const ElementFocusInfo& elementInfo = *iter;

				const auto found = std::find_if(begin(mForcedFocusElements), end(mForcedFocusElements), [&elementInfo](const ElementForcedFocusInfo& x)
												   { return x.Focus && x.Element == elementInfo.Element; });

				if(found == mForcedFocusElements.end())
				{
					SendCommandEvent(elementInfo.Element, mCommandEvent);
					iter = mElementsInFocus.erase(iter);
				}
				else
					++iter;
			}

			mForcedClearFocus = false;
		}

		for(auto& focusElementInfo : mForcedFocusElements)
		{
			if(focusElementInfo.Element->IsPendingDestroy())
				continue;

			const auto found = std::find_if(mElementsInFocus.begin(), mElementsInFocus.end(), [&](const ElementFocusInfo& x)
											   { return x.Element == focusElementInfo.Element; });

			if(focusElementInfo.Focus)
			{
				// Gain focus unless already in focus
				if(found == mElementsInFocus.end())
				{
					mElementsInFocus.push_back(ElementFocusInfo(focusElementInfo.Element, focusElementInfo.Element->GetParentWidget(), false));

					mCommandEvent = GUICommandEvent();
					mCommandEvent.SetType(GUICommandEventType::FocusGained);

					SendCommandEvent(focusElementInfo.Element, mCommandEvent);
				}
			}
			else
			{
				// Force clear focus
				if(found != mElementsInFocus.end())
				{
					mCommandEvent = GUICommandEvent();
					mCommandEvent.SetType(GUICommandEventType::FocusLost);

					SendCommandEvent(found->Element, mCommandEvent);
					B3DSwapAndErase(mElementsInFocus, found);
				}
			}
		}

		mForcedFocusElements.clear();
	}
	while(ProcessDestroyQueueIteration());

	// Blink caret
	float curTime = GetTime().GetRealTimeInSeconds();

	if((curTime - mCaretLastBlinkTime) >= mCaretBlinkInterval)
	{
		mCaretLastBlinkTime = curTime;
		mIsCaretOn = !mIsCaretOn;

		mCommandEvent = GUICommandEvent();
		mCommandEvent.SetType(GUICommandEventType::Redraw);

		for(auto& elementInfo : mElementsInFocus)
		{
			SendCommandEvent(elementInfo.Element, mCommandEvent);
		}
	}

	// Update dirty widget render data
	for(auto& entry : mWidgets)
	{
		GUIWidget* widget = entry.Widget;
		GUIDrawGroupRenderDataUpdate updateData = widget->RebuildDirtyRenderData();

		HCamera camera;
		camera = widget->GetCamera();
		if(camera == nullptr)
			continue;

		auto widgetId = (u64)widget;
		GetRenderThread().PostCommand([renderer = mRenderer.get(),
									updateData = std::move(updateData),
									camera = B3DGetRenderProxy(camera).get(),
									widgetId,
									widgetDepth = widget->GetDepth(),
									worldTransform = widget->SceneObject()->GetTransform().GetMatrix()]()
								   { renderer->UpdateDrawGroups(camera, widgetId, widgetDepth, worldTransform, updateData); }, "GUIRenderer::UpdateDrawGroups");
	}

	// Note: It's important to call this after GUI elements rebuild their render data, as this will request new vector paths
	mVectorSpriteAtlas->Update();

	GetRenderThread().PostCommand([renderer = mRenderer.get(), time = GetTime().GetRealTimeInSeconds()]()
							   { renderer->Update(time); }, "GUIRenderer::Update");
}

void GUIManager::ProcessDestroyQueue()
{
	// Loop until everything empties
	while(ProcessDestroyQueueIteration())
	{}
}

void GUIManager::UpdateCaretTexture()
{
	if(mCaretImage == nullptr)
	{
		TextureCreateInformation textureCreateInformation; // Default
		textureCreateInformation.Name = "Input Caret";

		HTexture newTex = Texture::Create(textureCreateInformation);
		mCaretImage = SpriteTexture::Create(newTex);
	}

	const HTexture& texture = mCaretImage->GetAtlasTexture();
	SPtr<PixelData> data = texture->GetProperties().AllocBuffer(0, 0);

	data->SetColorAt(mCaretColor, 0, 0);
	texture->WriteData(data);
}

void GUIManager::UpdateTextSelectionTexture()
{
	if(mTextSelectionImage == nullptr)
	{
		TextureCreateInformation textureCreateInformation; // Default
		textureCreateInformation.Name = "Input Caret";

		HTexture newTex = Texture::Create(textureCreateInformation);
		mTextSelectionImage = SpriteTexture::Create(newTex);
	}

	const HTexture& texture = mTextSelectionImage->GetAtlasTexture();
	SPtr<PixelData> data = texture->GetProperties().AllocBuffer(0, 0);

	data->SetColorAt(mTextSelectionColor, 0, 0);
	texture->WriteData(data);
}

void GUIManager::OnMouseDragEnded(const PointerEvent& event, DragCallbackInfo& dragInfo)
{
	GUIMouseButton guiButton = ButtonToGuiButton(event.Button);

	if(DragAndDrop::Instance().IsDragInProgress() && guiButton == GUIMouseButton::Left)
	{
		const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIPhysicalPoint localPos(BsZero);

			if(elementInfo.Widget != nullptr)
				localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);

			bool acceptDrop = true;
			if(DragAndDrop::Instance().NeedsValidDropTarget())
			{
				acceptDrop = elementInfo.Element->AcceptDragAndDrop(localPos, DragAndDrop::Instance().GetDragTypeId());
			}

			if(acceptDrop)
			{
				mMouseEvent.SetDragAndDropDroppedData(localPos, DragAndDrop::Instance().GetDragData());
				dragInfo.Processed = SendMouseEvent(elementInfo.Element, mMouseEvent);

				if(dragInfo.Processed)
					return;
			}
		}
	}

	dragInfo.Processed = false;
}

void GUIManager::OnPointerMoved(const PointerEvent& event)
{
	if(event.IsUsed)
		return;

	bool buttonStates[(int)GUIMouseButton::Count];
	buttonStates[0] = event.ButtonStates[0];
	buttonStates[1] = event.ButtonStates[1];
	buttonStates[2] = event.ButtonStates[2];

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	if(FindElementUnderPointer(screenPosition, buttonStates, event.Shift, event.Control, event.Alt))
		event.IsUsed = true;

	if(mDragState == DragState::HeldWithoutDrag)
	{
		u32 dist = (u32)mLastPointerClickPos.CalculateManhattanDistance(screenPosition);

		if(dist > kDragDistance)
		{
			for(auto& activeElement : mActiveElements)
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(activeElement.Widget, screenPosition);
				const GUIPhysicalPoint localDragStartPos = GetWidgetRelativePos(activeElement.Widget, mLastPointerClickPos);

				mMouseEvent.SetMouseDragStartData(localPos, localDragStartPos);
				if(SendMouseEvent(activeElement.Element, mMouseEvent))
					event.IsUsed = true;
			}

			mDragState = DragState::Dragging;
			mDragStartPos = screenPosition;
		}
	}

	// If mouse is being held down send MouseDrag events
	if(mDragState == DragState::Dragging)
	{
		for(auto& activeElement : mActiveElements)
		{
			if(mLastPointerScreenPos != event.ScreenPos)
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(activeElement.Widget, screenPosition);

				mMouseEvent.SetMouseDragData(localPos, screenPosition - mDragStartPos);
				if(SendMouseEvent(activeElement.Element, mMouseEvent))
					event.IsUsed = true;
			}
		}

		mLastPointerScreenPos = event.ScreenPos;

		// Also if drag is in progress send DragAndDrop events
		if(DragAndDrop::Instance().IsDragInProgress())
		{
			bool acceptDrop = true;
			for(auto& elementInfo : mElementsUnderPointer)
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);

				acceptDrop = true;
				if(DragAndDrop::Instance().NeedsValidDropTarget())
				{
					acceptDrop = elementInfo.Element->AcceptDragAndDrop(localPos, DragAndDrop::Instance().GetDragTypeId());
				}

				if(acceptDrop)
				{
					mMouseEvent.SetDragAndDropDraggedData(localPos, DragAndDrop::Instance().GetDragData());
					if(SendMouseEvent(elementInfo.Element, mMouseEvent))
					{
						event.IsUsed = true;
						break;
					}
				}
			}

			if(acceptDrop)
			{
				if(mActiveCursor != CursorType::ArrowDrag)
				{
					Cursor::Instance().SetCursor(CursorType::ArrowDrag);
					mActiveCursor = CursorType::ArrowDrag;
				}
			}
			else
			{
				if(mActiveCursor != CursorType::Deny)
				{
					Cursor::Instance().SetCursor(CursorType::Deny);
					mActiveCursor = CursorType::Deny;
				}
			}
		}
	}
	else // Otherwise, send MouseMove events if we are hovering over any element
	{
		if(mLastPointerScreenPos != event.ScreenPos)
		{
			bool moveProcessed = false;
			bool hasCustomCursor = false;
			for(auto& elementInfo : mElementsUnderPointer)
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);

				if(!moveProcessed)
				{
					// Send MouseMove event
					mMouseEvent.SetMouseMoveData(localPos);
					moveProcessed = SendMouseEvent(elementInfo.Element, mMouseEvent);

					if(moveProcessed)
						event.IsUsed = true;
				}

				if(mDragState == DragState::NoDrag)
				{
					CursorType newCursor = CursorType::Arrow;
					if(elementInfo.Element->HasCustomCursor(localPos, newCursor))
					{
						if(newCursor != mActiveCursor)
						{
							Cursor::Instance().SetCursor(newCursor);
							mActiveCursor = newCursor;
						}

						hasCustomCursor = true;
					}
				}

				if(moveProcessed)
					break;
			}

			// While dragging we don't want to modify the cursor
			if(mDragState == DragState::NoDrag)
			{
				if(!hasCustomCursor)
				{
					if(mActiveCursor != CursorType::Arrow)
					{
						Cursor::Instance().SetCursor(CursorType::Arrow);
						mActiveCursor = CursorType::Arrow;
					}
				}
			}
		}

		mLastPointerScreenPos = event.ScreenPos;

		if(Math::Abs(event.MouseWheelScrollAmount) > 0.00001f)
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				mMouseEvent.SetMouseWheelScrollData(event.MouseWheelScrollAmount);
				if(SendMouseEvent(elementInfo.Element, mMouseEvent))
				{
					event.IsUsed = true;
					break;
				}
			}
		}
	}
}

void GUIManager::OnPointerReleased(const PointerEvent& event)
{
	if(event.IsUsed)
		return;

	bool buttonStates[(int)GUIMouseButton::Count];
	buttonStates[0] = event.ButtonStates[0];
	buttonStates[1] = event.ButtonStates[1];
	buttonStates[2] = event.ButtonStates[2];

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	if(FindElementUnderPointer(screenPosition, buttonStates, event.Shift, event.Control, event.Alt))
		event.IsUsed = true;

	mMouseEvent = GUIMouseEvent(buttonStates, event.Shift, event.Control, event.Alt);

	GUIMouseButton guiButton = ButtonToGuiButton(event.Button);

	// Send MouseUp event only if we are over the active element (we don't want to accidentally trigger other elements).
	// And only activate when a button that originally caused the active state is released, otherwise ignore it.
	if(mActiveMouseButton == guiButton)
	{
		for(auto& elementInfo : mElementsUnderPointer)
		{
			auto found = std::find_if(mActiveElements.begin(), mActiveElements.end(), [&](const ElementInfo& x)
										  { return x.Element == elementInfo.Element; });

			if(found != mActiveElements.end())
			{
				const GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);
				mMouseEvent.SetMouseUpData(localPos, guiButton);

				if(SendMouseEvent(elementInfo.Element, mMouseEvent))
				{
					event.IsUsed = true;
					break;
				}
			}
		}
	}

	// Send DragEnd event to whichever element is active
	bool acceptEndDrag = (mDragState == DragState::Dragging || mDragState == DragState::HeldWithoutDrag) && mActiveMouseButton == guiButton &&
		(guiButton == GUIMouseButton::Left);

	if(acceptEndDrag)
	{
		if(mDragState == DragState::Dragging)
		{
			for(auto& activeElement : mActiveElements)
			{
				GUIPhysicalPoint localPos = GetWidgetRelativePos(activeElement.Widget, screenPosition);

				mMouseEvent.SetMouseDragEndData(localPos);
				if(SendMouseEvent(activeElement.Element, mMouseEvent))
					event.IsUsed = true;
			}
		}

		mDragState = DragState::NoDrag;
	}

	if(mActiveMouseButton == guiButton)
	{
		mActiveElements.clear();
		mActiveMouseButton = GUIMouseButton::Left;
	}

	if(mActiveCursor != CursorType::Arrow)
	{
		Cursor::Instance().SetCursor(CursorType::Arrow);
		mActiveCursor = CursorType::Arrow;
	}
}

void GUIManager::OnPointerPressed(const PointerEvent& event)
{
	if(event.IsUsed)
		return;

	bool buttonStates[(int)GUIMouseButton::Count];
	buttonStates[0] = event.ButtonStates[0];
	buttonStates[1] = event.ButtonStates[1];
	buttonStates[2] = event.ButtonStates[2];

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	if(FindElementUnderPointer(screenPosition, buttonStates, event.Shift, event.Control, event.Alt))
		event.IsUsed = true;

	// Determine elements that gained focus
	mNewElementsInFocus.clear();

	mCommandEvent = GUICommandEvent();

	// Determine elements that gained focus
	for(auto& elementInfo : mElementsUnderPointer)
	{
		auto found = std::find_if(begin(mElementsInFocus), end(mElementsInFocus), [=](const ElementFocusInfo& x)
									 { return x.Element == elementInfo.Element; });

		if(found == mElementsInFocus.end())
		{
			bool processed = !elementInfo.Element->GetOptionFlags().IsSet(GUIElementOption::ClickThrough);
			mNewElementsInFocus.push_back(ElementFocusInfo(elementInfo.Element, elementInfo.Widget, processed));

			if(processed)
				break;
		}
		else
		{
			mNewElementsInFocus.push_back(*found);

			if(found->UsesFocus)
				break;
		}
	}

	// Send focus loss events
	// Note: Focus loss must trigger before mouse press because things like input boxes often only confirm changes
	// made to them when focus is lost. So if the user is confirming some input via a press of the button focus loss
	// must trigger on the input box first to make sure its contents get saved.
	mCommandEvent.SetType(GUICommandEventType::FocusLost);

	for(auto& elementInfo : mElementsInFocus)
	{
		auto found = std::find_if(begin(mNewElementsInFocus), end(mNewElementsInFocus), [=](const ElementFocusInfo& x)
									 { return x.Element == elementInfo.Element; });

		if(found == mNewElementsInFocus.end())
			SendCommandEvent(elementInfo.Element, mCommandEvent);
	}

	// Send focus gain events
	mCommandEvent.SetType(GUICommandEventType::FocusGained);

	for(auto& elementInfo : mNewElementsInFocus)
	{
		auto found = std::find_if(begin(mElementsInFocus), end(mElementsInFocus), [=](const ElementFocusInfo& x)
									 { return x.Element == elementInfo.Element; });

		if(found == mElementsInFocus.end())
			SendCommandEvent(elementInfo.Element, mCommandEvent);
	}

	mElementsInFocus.swap(mNewElementsInFocus);

	// Send mouse press event
	mMouseEvent = GUIMouseEvent(buttonStates, event.Shift, event.Control, event.Alt);
	GUIMouseButton guiButton = ButtonToGuiButton(event.Button);

	// We only check for mouse down if mouse isn't already being held down, and we are hovering over an element
	if(mActiveElements.size() == 0)
	{
		mNewActiveElements.clear();
		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);
			mMouseEvent.SetMouseDownData(localPos, guiButton);

			bool processed = SendMouseEvent(elementInfo.Element, mMouseEvent);

			if(guiButton == GUIMouseButton::Left)
			{
				mDragState = DragState::HeldWithoutDrag;
				mLastPointerClickPos = event.ScreenPos.To<GUIPhysicalUnit>();
			}

			mNewActiveElements.push_back(ElementInfo(elementInfo.Element, elementInfo.Widget));
			mActiveMouseButton = guiButton;

			if(processed)
			{
				event.IsUsed = true;
				break;
			}
		}

		mActiveElements.swap(mNewActiveElements);
	}

	// If right click try to open context menu
	if(buttonStates[2])
	{
		for(auto& elementInfo : mElementsUnderPointer)
		{
			SPtr<GUIContextMenu> menu = elementInfo.Element->GetContextMenu();

			if(menu != nullptr && elementInfo.Widget != nullptr)
			{
				const RenderWindow* window = GetWidgetWindow(*elementInfo.Widget);
				if(window != nullptr)
				{
					const GUIPhysicalPoint windowPos = window->ScreenToWindowPosition(event.ScreenPos).To<GUIPhysicalUnit>();

					menu->Open(windowPos, *elementInfo.Widget);
					event.IsUsed = true;
					break;
				}
			}
		}
	}
}

void GUIManager::OnPointerDoubleClick(const PointerEvent& event)
{
	if(event.IsUsed)
		return;

	bool buttonStates[(int)GUIMouseButton::Count];
	buttonStates[0] = event.ButtonStates[0];
	buttonStates[1] = event.ButtonStates[1];
	buttonStates[2] = event.ButtonStates[2];

	const GUIPhysicalPoint screenPosition = event.ScreenPos.To<GUIPhysicalUnit>();
	if(FindElementUnderPointer(screenPosition, buttonStates, event.Shift, event.Control, event.Alt))
		event.IsUsed = true;

	mMouseEvent = GUIMouseEvent(buttonStates, event.Shift, event.Control, event.Alt);

	GUIMouseButton guiButton = ButtonToGuiButton(event.Button);

	// We only check for mouse down if we are hovering over an element
	for(auto& elementInfo : mElementsUnderPointer)
	{
		GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, screenPosition);

		mMouseEvent.SetMouseDoubleClickData(localPos, guiButton);
		if(SendMouseEvent(elementInfo.Element, mMouseEvent))
		{
			event.IsUsed = true;
			break;
		}
	}
}

void GUIManager::OnInputCommandEntered(InputCommandType commandType)
{
	if(mElementsInFocus.empty())
		return;

	HideTooltip();

	// Tabs are handled by the GUI manager itself, while other events are passed to GUI elements
	if(commandType == InputCommandType::Tab)
	{
		TabFocusNext();
		return;
	}

	mCommandEvent = GUICommandEvent();

	switch(commandType)
	{
	case InputCommandType::Backspace:
		mCommandEvent.SetType(GUICommandEventType::Backspace);
		break;
	case InputCommandType::Delete:
		mCommandEvent.SetType(GUICommandEventType::Delete);
		break;
	case InputCommandType::Return:
		mCommandEvent.SetType(GUICommandEventType::Return);
		break;
	case InputCommandType::Confirm:
		mCommandEvent.SetType(GUICommandEventType::Confirm);
		break;
	case InputCommandType::Escape:
		mCommandEvent.SetType(GUICommandEventType::Escape);
		break;
	case InputCommandType::CursorMoveLeft:
		mCommandEvent.SetType(GUICommandEventType::MoveLeft);
		break;
	case InputCommandType::CursorMoveRight:
		mCommandEvent.SetType(GUICommandEventType::MoveRight);
		break;
	case InputCommandType::CursorMoveUp:
		mCommandEvent.SetType(GUICommandEventType::MoveUp);
		break;
	case InputCommandType::CursorMoveDown:
		mCommandEvent.SetType(GUICommandEventType::MoveDown);
		break;
	case InputCommandType::SelectLeft:
		mCommandEvent.SetType(GUICommandEventType::SelectLeft);
		break;
	case InputCommandType::SelectRight:
		mCommandEvent.SetType(GUICommandEventType::SelectRight);
		break;
	case InputCommandType::SelectUp:
		mCommandEvent.SetType(GUICommandEventType::SelectUp);
		break;
	case InputCommandType::SelectDown:
		mCommandEvent.SetType(GUICommandEventType::SelectDown);
		break;
	default:
		break;
	}

	for(auto& elementInfo : mElementsInFocus)
		SendCommandEvent(elementInfo.Element, mCommandEvent);
}

void GUIManager::OnVirtualButtonDown(const VirtualButton& button, u32 deviceIndex)
{
	HideTooltip();
	mVirtualButtonEvent.SetButton(button);

	for(auto& elementInFocus : mElementsInFocus)
	{
		bool processed = SendVirtualButtonEvent(elementInFocus.Element, mVirtualButtonEvent);

		if(processed)
			break;
	}
}

bool GUIManager::FindElementUnderPointer(const GUIPhysicalPoint& pointerScreenPos, bool buttonStates[3], bool shift, bool control, bool alt)
{
	Vector<const RenderWindow*> widgetWindows;
	for(auto& widgetInfo : mWidgets)
		widgetWindows.push_back(GetWidgetWindow(*widgetInfo.Widget));

#if B3D_DEBUG
	// Checks if all referenced windows actually exist
	Vector<RenderWindow*> activeWindows = RenderWindowManager::Instance().GetRenderWindows();
	for(auto& window : widgetWindows)
	{
		if(window == nullptr)
			continue;

		auto found = std::find(begin(activeWindows), end(activeWindows), window);

		if(found == activeWindows.end())
		{
			B3D_EXCEPT(InternalErrorException, "GUI manager has a reference to a window that doesn't exist. \
												  Please detach all GUIWidgets from windows before destroying a window.");
		}
	}
#endif

	mNewElementsUnderPointer.clear();

	const RenderWindow* windowUnderPointer = nullptr;
	UnorderedSet<const RenderWindow*> uniqueWindows;

	for(auto& window : widgetWindows)
	{
		if(window == nullptr)
			continue;

		uniqueWindows.insert(window);
	}

	RenderWindow* topMostModal = RenderWindowManager::Instance().GetTopMostModal();
	for(auto& window : uniqueWindows)
	{
		if(Platform::IsPointOverWindow(*window, pointerScreenPos.To<i32>()))
		{
			// If there's a top most modal window, it needs to be this one, otherwise we ignore input to that window
			if(topMostModal == nullptr || window == topMostModal)
				windowUnderPointer = window;

			break;
		}
	}

	if(windowUnderPointer != nullptr)
	{
		GUIPhysicalPoint windowPos = windowUnderPointer->ScreenToWindowPosition(pointerScreenPos.To<i32>()).To<GUIPhysicalUnit>();

		u32 widgetIndex = 0;
		for(auto& widgetInfo : mWidgets)
		{
			if(widgetWindows[widgetIndex] == nullptr)
			{
				widgetIndex++;
				continue;
			}

			GUIWidget* widget = widgetInfo.Widget;
			if(widgetWindows[widgetIndex] == windowUnderPointer && widget->InBounds(WindowToBridgedCoords(widget->GetTarget()->GetTarget(), windowPos)))
			{
				// Note: This should only be checking non-culled element (i.e. GUIElement::GetVisibleElements())
				const Vector<GUIRenderable*>& elements = widget->GetElements();
				GUIPhysicalPoint localPos = GetWidgetRelativePos(widget, pointerScreenPos);

				// Elements with lowest depth (most to the front) get handled first
				for(auto iter = elements.begin(); iter != elements.end(); ++iter)
				{
					GUIInteractable* const interactableElement = B3DRTTICast<GUIInteractable>(*iter);
					if(!interactableElement)
						continue;

					if(!interactableElement->IsHiddenOrCulled() && interactableElement->IsInInteractionBounds(localPos))
					{
						ElementInfoUnderPointer elementInfo(interactableElement, widget);

						auto found = std::find_if(mElementsUnderPointer.begin(), mElementsUnderPointer.end(), [=](const ElementInfoUnderPointer& x)
													 { return x.Element == interactableElement; });

						if(found != mElementsUnderPointer.end())
						{
							elementInfo.UsesMouseOver = found->UsesMouseOver;
							elementInfo.ReceivedMouseOver = found->ReceivedMouseOver;
						}

						mNewElementsUnderPointer.push_back(elementInfo);
					}
				}
			}

			widgetIndex++;
		}
	}

	std::sort(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(), [](const ElementInfoUnderPointer& a, const ElementInfoUnderPointer& b)
			  { return a.Element->GetDepth() < b.Element->GetDepth(); });

	// Send MouseOut and MouseOver events
	bool eventProcessed = false;

	for(auto& elementInfo : mNewElementsUnderPointer)
	{
		GUIInteractable* element = elementInfo.Element;
		GUIWidget* widget = elementInfo.Widget;

		if(elementInfo.ReceivedMouseOver)
		{
			elementInfo.IsHovering = true;
			if(elementInfo.UsesMouseOver)
				break;

			continue;
		}

		auto found = std::find_if(mActiveElements.begin(), mActiveElements.end(), [&](const ElementInfo& x)
									 { return x.Element == element; });

		// Send MouseOver event
		if(mActiveElements.size() == 0 || found != mActiveElements.end())
		{
			GUIPhysicalPoint localPos = GetWidgetRelativePos(widget, pointerScreenPos);

			mMouseEvent = GUIMouseEvent(buttonStates, shift, control, alt);

			mMouseEvent.SetMouseOverData(localPos);
			elementInfo.ReceivedMouseOver = true;
			elementInfo.IsHovering = true;
			if(SendMouseEvent(element, mMouseEvent))
			{
				eventProcessed = true;
				elementInfo.UsesMouseOver = true;
				break;
			}
		}
	}

	// Send DragAndDropLeft event - It is similar to MouseOut events but we send it to all
	// elements a user might hover over, while we send mouse over/out events only to active elements while dragging
	if(DragAndDrop::Instance().IsDragInProgress())
	{
		for(auto& elementInfo : mElementsUnderPointer)
		{
			auto found = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(), [=](const ElementInfoUnderPointer& x)
										 { return x.Element == elementInfo.Element; });

			if(found == mNewElementsUnderPointer.end())
			{
				GUIPhysicalPoint localPos = GetWidgetRelativePos(elementInfo.Widget, pointerScreenPos);

				mMouseEvent.SetDragAndDropLeftData(localPos, DragAndDrop::Instance().GetDragData());
				if(SendMouseEvent(elementInfo.Element, mMouseEvent))
				{
					eventProcessed = true;
					break;
				}
			}
		}
	}

	for(auto& elementInfo : mElementsUnderPointer)
	{
		GUIInteractable* element = elementInfo.Element;
		GUIWidget* widget = elementInfo.Widget;

		auto found = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(), [=](const ElementInfoUnderPointer& x)
									 { return x.Element == element; });

		if(!elementInfo.ReceivedMouseOver)
			continue;

		if(found == mNewElementsUnderPointer.end() || !found->IsHovering)
		{
			auto found2 = std::find_if(mActiveElements.begin(), mActiveElements.end(), [=](const ElementInfo& x)
										  { return x.Element == element; });

			// Send MouseOut event
			if(mActiveElements.size() == 0 || found2 != mActiveElements.end())
			{
				GUIPhysicalPoint localPos = GetWidgetRelativePos(widget, pointerScreenPos);

				mMouseEvent.SetMouseOutData(localPos);
				if(SendMouseEvent(element, mMouseEvent))
				{
					eventProcessed = true;
					break;
				}
			}
		}
	}

	mElementsUnderPointer.swap(mNewElementsUnderPointer);

	// Tooltip
	HideTooltip();
	if(mElementsUnderPointer.size() > 0)
		mShowTooltip = true;

	mTooltipElementHoverStart = GetTime().GetRealTimeInSeconds();

	return eventProcessed;
}

void GUIManager::OnTextInput(const TextInputEvent& event)
{
	mTextInputEvent = GUITextInputEvent();
	mTextInputEvent.SetData(event.TextChar);

	for(auto& elementInFocus : mElementsInFocus)
	{
		if(SendTextInputEvent(elementInFocus.Element, mTextInputEvent))
			event.IsUsed = true;
	}
}

void GUIManager::OnWindowFocusGained(RenderWindow& win)
{
	for(auto& widgetInfo : mWidgets)
	{
		GUIWidget* widget = widgetInfo.Widget;
		if(GetWidgetWindow(*widget) == &win)
			widget->OwnerWindowFocusChanged();
	}

	auto found = mSavedFocusElements.find(&win);
	if(found != mSavedFocusElements.end())
	{
		Vector<ElementFocusInfo>& savedFocusedElements = found->second;

		mNewElementsInFocus.clear();
		for(auto& focusedElement : mElementsInFocus)
		{
			if(focusedElement.Element->IsPendingDestroy())
				continue;

			auto found2 = std::find_if(savedFocusedElements.begin(), savedFocusedElements.end(), [&focusedElement](const ElementFocusInfo& x)
										  { return x.Element == focusedElement.Element; });

			if(found2 == savedFocusedElements.end())
			{
				mCommandEvent = GUICommandEvent();
				mCommandEvent.SetType(GUICommandEventType::FocusLost);

				SendCommandEvent(focusedElement.Element, mCommandEvent);
				savedFocusedElements.push_back(focusedElement);
			}
			else
				mNewElementsInFocus.push_back(focusedElement);
		}

		mElementsInFocus.swap(mNewElementsInFocus);

		for(auto& entry : savedFocusedElements)
		{
			if(entry.Element->IsPendingDestroy())
				continue;

			auto found2 = std::find_if(mElementsInFocus.begin(), mElementsInFocus.end(), [&entry](const ElementFocusInfo& x)
										  { return x.Element == entry.Element; });

			if(found2 == mElementsInFocus.end())
			{
				mCommandEvent = GUICommandEvent();
				mCommandEvent.SetType(GUICommandEventType::FocusGained);

				SendCommandEvent(entry.Element, mCommandEvent);
				mElementsInFocus.push_back(entry);
			}
		}

		mSavedFocusElements.erase(found);
	}
}

void GUIManager::OnWindowFocusLost(RenderWindow& win)
{
	for(auto& widgetInfo : mWidgets)
	{
		GUIWidget* widget = widgetInfo.Widget;
		if(GetWidgetWindow(*widget) == &win)
			widget->OwnerWindowFocusChanged();
	}

	Vector<ElementFocusInfo>& savedFocusedElements = mSavedFocusElements[&win];
	savedFocusedElements.clear();

	mNewElementsInFocus.clear();
	for(auto& focusedElement : mElementsInFocus)
	{
		if(focusedElement.Element->IsPendingDestroy())
			continue;

		if(focusedElement.Widget != nullptr && GetWidgetWindow(*focusedElement.Widget) == &win)
		{
			mCommandEvent = GUICommandEvent();
			mCommandEvent.SetType(GUICommandEventType::FocusLost);

			SendCommandEvent(focusedElement.Element, mCommandEvent);
			savedFocusedElements.push_back(focusedElement);
		}
		else
			mNewElementsInFocus.push_back(focusedElement);
	}

	mElementsInFocus.swap(mNewElementsInFocus);
}

// We stop getting mouse move events once it leaves the window, so make sure
// nothing stays in hover state
void GUIManager::OnMouseLeftWindow(RenderWindow& win)
{
	mNewElementsUnderPointer.clear();

	for(auto& elementInfo : mElementsUnderPointer)
	{
		GUIInteractable* element = elementInfo.Element;
		GUIWidget* widget = elementInfo.Widget;

		if(widget != nullptr && widget->GetTarget()->GetTarget().get() != &win)
		{
			mNewElementsUnderPointer.push_back(elementInfo);
			continue;
		}

		auto found = std::find_if(mActiveElements.begin(), mActiveElements.end(), [&](const ElementInfo& x)
									 { return x.Element == element; });

		// Send MouseOut event
		if(mActiveElements.size() == 0 || found != mActiveElements.end())
		{
			GUIPhysicalPoint localPos = GetWidgetRelativePos(widget, GUIPhysicalPoint(BsZero));

			mMouseEvent.SetMouseOutData(localPos);
			SendMouseEvent(element, mMouseEvent);
		}
	}

	mElementsUnderPointer.swap(mNewElementsUnderPointer);

	HideTooltip();
	if(mDragState != DragState::Dragging)
	{
		if(mActiveCursor != CursorType::Arrow)
		{
			Cursor::Instance().SetCursor(CursorType::Arrow);
			mActiveCursor = CursorType::Arrow;
		}
	}
}

void GUIManager::HideTooltip()
{
	GUITooltipManager::Instance().Hide();
	mShowTooltip = false;
}

void GUIManager::QueueForDestroy(GUIElement* element)
{
	mScheduledForDestruction.push(element);
}

void GUIManager::SetFocus(GUIInteractable* element, bool focus, bool clear)
{
	ElementForcedFocusInfo efi;
	efi.Element = element;
	efi.Focus = focus;

	if(clear)
	{
		mForcedClearFocus = true;
		mForcedFocusElements.clear();
	}

	mForcedFocusElements.push_back(efi);
}

bool GUIManager::ProcessDestroyQueueIteration()
{
	Stack<GUIElement*> toDestroy = mScheduledForDestruction;
	mScheduledForDestruction = Stack<GUIElement*>();

	while(!toDestroy.empty())
	{
		B3DDelete(toDestroy.top());
		toDestroy.pop();
	}

	return !mScheduledForDestruction.empty();
}

void GUIManager::SetInputBridge(const SPtr<RenderTexture>& renderTex, const GUIInteractable* element)
{
	if(element == nullptr)
		mInputBridge.erase(renderTex);
	else
		mInputBridge[renderTex] = element;
}

GUIMouseButton GUIManager::ButtonToGuiButton(PointerEventButton pointerButton) const
{
	if(pointerButton == PointerEventButton::Left)
		return GUIMouseButton::Left;
	else if(pointerButton == PointerEventButton::Middle)
		return GUIMouseButton::Middle;
	else if(pointerButton == PointerEventButton::Right)
		return GUIMouseButton::Right;

	B3D_EXCEPT(InvalidParametersException, "Provided button is not a GUI supported mouse button.");
	return GUIMouseButton::Left;
}

GUIPhysicalPoint GUIManager::GetWidgetRelativePos(const GUIWidget* widget, const GUIPhysicalPoint& screenPos) const
{
	if(widget == nullptr)
		return screenPos;

	const RenderWindow* window = GetWidgetWindow(*widget);
	if(window == nullptr)
		return GUIPhysicalPoint(BsZero);

	GUIPhysicalPoint windowPos = window->ScreenToWindowPosition(screenPos.To<i32>()).To<GUIPhysicalUnit>();
	windowPos = WindowToBridgedCoords(widget->GetTarget()->GetTarget(), windowPos);

	const Matrix4& worldTfrm = widget->SceneObject()->GetTransform().GetMatrix();

	Vector4 vecLocalPos = worldTfrm.Inverse().MultiplyAffine(Vector4((float)windowPos.X, (float)windowPos.Y, 0.0f, 1.0f));
	GUIPhysicalPoint curLocalPos(Math::RoundToI32(vecLocalPos.X), Math::RoundToI32(vecLocalPos.Y));

	return curLocalPos;
}

GUIPhysicalPoint GUIManager::WindowToBridgedCoords(const SPtr<RenderTarget>& target, const GUIPhysicalPoint& windowPos) const
{
	// This cast might not be valid (the render target could be a window), but we only really need to cast
	// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
	// (in which case we know it is a RenderTexture)
	SPtr<const RenderTexture> renderTexture = std::static_pointer_cast<const RenderTexture>(target);
	const RenderTargetProperties& rtProps = renderTexture->GetProperties();

	auto found = mInputBridge.find(renderTexture);
	if(found != mInputBridge.end()) // Widget input is bridged, which means we need to transform the coordinates
	{
		const GUIInteractable* bridgeElement = found->second;
		const GUIWidget* parentWidget = bridgeElement->GetParentWidget();
		if(parentWidget == nullptr)
			return windowPos;

		const Matrix4& worldTfrm = parentWidget->SceneObject()->GetTransform().GetMatrix();

		Vector4 vecLocalPos = worldTfrm.Inverse().MultiplyAffine(Vector4((float)windowPos.X, (float)windowPos.Y, 0.0f, 1.0f));
		GUIPhysicalArea bridgeBounds = bridgeElement->GetAbsoluteBounds();

		// Find coordinates relative to the bridge element
		float x = vecLocalPos.X - (float)bridgeBounds.X;
		float y = vecLocalPos.Y - (float)bridgeBounds.Y;

		float scaleX = bridgeBounds.Width > 0 ? rtProps.Width / (float)bridgeBounds.Width : 0.0f;
		float scaleY = bridgeBounds.Height > 0 ? rtProps.Height / (float)bridgeBounds.Height : 0.0f;

		return GUIPhysicalPoint(Math::RoundToI32(x * scaleX), Math::RoundToI32(y * scaleY));
	}

	return windowPos;
}

const RenderWindow* GUIManager::GetWidgetWindow(const GUIWidget& widget) const
{
	const Viewport* viewport = widget.GetTarget();
	if(viewport == nullptr)
		return nullptr;

	SPtr<RenderTarget> target = viewport->GetTarget();
	if(target == nullptr)
		return nullptr;

	// This cast might not be valid (the render target could be a window), but we only really need to cast
	// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
	// (in which case we know it is a RenderTexture)
	SPtr<const RenderTexture> renderTexture = std::static_pointer_cast<const RenderTexture>(target);

	auto found = mInputBridge.find(renderTexture);
	if(found != mInputBridge.end())
	{
		GUIWidget* parentWidget = found->second->GetParentWidget();
		if(parentWidget == nullptr)
			return nullptr;

		if(parentWidget != &widget)
			return GetWidgetWindow(*parentWidget);
	}

	Vector<RenderWindow*> renderWindows = RenderWindowManager::Instance().GetRenderWindows();

	auto foundWin = std::find(renderWindows.begin(), renderWindows.end(), target.get());
	if(foundWin != renderWindows.end())
		return static_cast<RenderWindow*>(target.get());

	return nullptr;
}

SPtr<RenderWindow> GUIManager::GetBridgeWindow(const SPtr<RenderTexture>& target) const
{
	if(target == nullptr)
		return nullptr;

	while(true)
	{
		auto found = mInputBridge.find(target);
		if(found == mInputBridge.end())
			return nullptr;

		GUIWidget* parentWidget = found->second->GetParentWidget();
		if(parentWidget == nullptr)
			return nullptr;

		SPtr<RenderTarget> curTarget = parentWidget->GetTarget()->GetTarget();
		if(curTarget == nullptr)
			return nullptr;

		if(curTarget == target)
			return nullptr;

		if(curTarget->GetProperties().IsWindow)
			return std::static_pointer_cast<RenderWindow>(curTarget);
	}

	return nullptr;
}

void GUIManager::GetBridgedElements(const GUIWidget* widget, TInlineArray<std::pair<const GUIInteractable*, SPtr<const RenderTarget>>, 4>& elements)
{
	if(widget == nullptr)
		return;

	for(auto& entry : mInputBridge)
	{
		const GUIInteractable* element = entry.second;
		GUIWidget* parentWidget = element->GetParentWidget();
		if(parentWidget == widget)
			elements.Add(std::make_pair(element, entry.first));
	}
}

void GUIManager::TabFocusFirst()
{
	u32 nearestDist = std::numeric_limits<u32>::max();
	GUIInteractable* closestElement = nullptr;

	// Find to top-left most element
	for(auto& widgetInfo : mWidgets)
	{
		const RenderWindow* window = GetWidgetWindow(*widgetInfo.Widget);

		const Vector<GUIRenderable*>& elements = widgetInfo.Widget->GetElements();
		for(auto& element : elements)
		{
			GUIInteractable* const interactableElement = B3DRTTICast<GUIInteractable>(element);
			if(!interactableElement)
				continue;

			const bool acceptsKeyFocus = interactableElement->GetOptionFlags().IsSet(GUIElementOption::AcceptsKeyFocus);
			if(!acceptsKeyFocus || element->IsDisabled() || element->IsHidden())
				continue;

			const GUIPhysicalArea elemBounds = element->GetAbsoluteClippedArea();
			const bool isFullyClipped = elemBounds.Width == 0 || elemBounds.Height == 0;

			if(isFullyClipped)
				continue;

			GUIPhysicalPoint elementPos = elemBounds.GetPosition();
			Vector2I screenPos = window->WindowToScreenPosition(elementPos.To<i32>());

			const u32 dist = screenPos.SquaredLength();
			if(dist < nearestDist)
			{
				nearestDist = dist;
				closestElement = interactableElement;
			}
		}
	}

	if(closestElement == nullptr)
		return;

	// Don't use the element directly though, since its tab group could have explicit ordering
	const SPtr<GUINavGroup>& navGroup = closestElement->GetNavigationGroup();
	navGroup->FocusFirst();
}

void GUIManager::TabFocusNext()
{
	for(auto& entry : mElementsInFocus)
	{
		const SPtr<GUINavGroup>& navGroup = entry.Element->GetNavigationGroup();
		GUIElementOptions elementOptions = entry.Element->GetOptionFlags();
		if(elementOptions.IsSet(GUIElementOption::AcceptsKeyFocus) && navGroup != nullptr)
		{
			navGroup->FocusNext(entry.Element);
			return;
		}
	}

	// Nothing currently in focus
	TabFocusFirst();
}

bool GUIManager::SendMouseEvent(GUIInteractable* element, const GUIMouseEvent& event)
{
	if(element->IsPendingDestroy())
		return false;

	return element->DoOnMouseEvent(event);
}

bool GUIManager::SendTextInputEvent(GUIInteractable* element, const GUITextInputEvent& event)
{
	if(element->IsPendingDestroy())
		return false;

	return element->DoOnTextInputEvent(event);
}

bool GUIManager::SendCommandEvent(GUIInteractable* element, const GUICommandEvent& event)
{
	if(element->IsPendingDestroy())
		return false;

	return element->DoOnCommandEvent(event);
}

bool GUIManager::SendVirtualButtonEvent(GUIInteractable* element, const GUIVirtualButtonEvent& event)
{
	if(element->IsPendingDestroy())
		return false;

	return element->DoOnVirtualButtonEvent(event);
}

namespace b3d
{
	struct GpuBackendConventions;

	GUIManager& GetGUIManager()
{
	return GUIManager::Instance();
}
} // namespace b3d

namespace b3d { namespace render
{
	/** If enabled, regions of the GUI that are being redrawn will be drawn in a special debug material so they are easily noticeable. */
	static constexpr bool kEnableGUIRegionDebugDrawing = false;

	/** If true, all draw regions will be redrawn every frame, regardless if dirty or not. */
	static constexpr bool kRedrawAllRegions = false;

	/** Maximum number of clip regions per draw call. */
	static constexpr u32 kMaxClipRegionsPerDraw = 64;

	/** Clip region data stored in GPU buffer. */
	struct ClipRegionArea
	{
		Vector2I TopLeft;
		Vector2I BottomRight;
	};

	GUIRenderer::GUIWidgetRenderData::GUIWidgetRenderData(u64 widgetId, GpuDevice& device)
		: WidgetId(widgetId)
		, UniformBufferPool(device, GpuBufferCreateInformation::CreateUniform(gGUISpriteUniformBufferDefinition.GetSize()), 256)
		, ClipRegionBufferPool(device, GpuBufferCreateInformation::CreateStructuredStorage(sizeof(ClipRegionArea), kMaxClipRegionsPerDraw, GpuBufferFlag::StoreOnCPUWithGPUAccess), 1, 4)
	{ }

GUIRenderer::GUIRenderer()
	: RendererExtension(RenderLocation::Overlay, 10)
{}

void GUIRenderer::Initialize(const Any& data)
{
	const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();

	SamplerStateInformation ssDesc;
	ssDesc.MagFilter = FO_POINT;
	ssDesc.MinFilter = FO_POINT;
	ssDesc.MipFilter = FO_POINT;

	mSamplerState = gpuDevice->FindOrCreateSamplerState(ssDesc);
}

RendererExtensionRequest GUIRenderer::Check(const Camera& camera)
{
	auto found = mPerCameraData.find(&camera);
	if(found == mPerCameraData.end())
		return RendererExtensionRequest::DontRender;

	GUICameraRenderData& cameraRenderData = found->second;
	Vector<GUIWidgetRenderData>& widgetRenderData = cameraRenderData.WidgetRenderData;
	bool needsRedraw = !cameraRenderData.DirtyRegions.empty() || !cameraRenderData.LastFrameDirtyDebugDrawRegions.empty();
	if(!needsRedraw)
	{
		for(auto& widget : widgetRenderData)
		{
			for(auto& drawGroup : widget.Batches)
			{
				for(auto& renderTargetElem : drawGroup.RenderTargetElements)
				{
					if(renderTargetElem.LastUpdateCount != renderTargetElem.Target->GetUpdateCount())
					{
						needsRedraw = true;
						break;
					}
				}
			}
		}
	}

	return needsRedraw ? RendererExtensionRequest::ForceRender : RendererExtensionRequest::RenderIfTargetDirty;
}

void GUIRenderer::Render(const Camera& camera, const RendererViewContext& viewContext)
{
	// TODO - Sprite animation might be broken. I need to continually mark the animated region as dirty.

	FrameAllocatorScope frameScope;
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuBackendConventions& gpuBackendConventions = gpuDevice->GetCapabilities().Conventions;

	GUICameraRenderData& cameraRenderData = mPerCameraData[&camera];
	Vector<GUIWidgetRenderData>& widgetRenderData = cameraRenderData.WidgetRenderData;

	const bool viewflipYFlip = gpuBackendConventions.NdcYAxis == GpuBackendConventions::Axis::Down;

	GpuCommandBuffer& commandBuffer = *viewContext.CommandBuffer;
	commandBuffer.BeginLabel("GUIRenderer::Render");

	// Re-create cached render texture if needed
	const SPtr<RenderTarget> renderTarget = viewContext.CurrentTarget;
	const u32 renderTargetWidth = renderTarget->GetProperties().Width;
	const u32 renderTargetHeight = renderTarget->GetProperties().Height;

	const bool rebuildCachedRenderTexture = cameraRenderData.CachedRenderTexture == nullptr ||
		cameraRenderData.CachedRenderTexture->GetProperties().Width != renderTargetWidth ||
		cameraRenderData.CachedRenderTexture->GetProperties().Height != renderTargetHeight;

	if(rebuildCachedRenderTexture)
	{
		TextureCreateInformation cachedColorTextureCreateInformation;
		cachedColorTextureCreateInformation.Width = renderTargetWidth;
		cachedColorTextureCreateInformation.Height = renderTargetHeight;
		cachedColorTextureCreateInformation.Format = PF_RGBA8;
		cachedColorTextureCreateInformation.Usage = TU_RENDERTARGET;

		const SPtr<Texture> cachedColorTexture = gpuDevice->CreateTexture(cachedColorTextureCreateInformation);

		RenderTextureCreateInformation cachedRenderTextureCreateInformation;
		cachedRenderTextureCreateInformation.ColorSurfaces[0].Texture = cachedColorTexture;

		cameraRenderData.CachedRenderTexture = RenderTexture::Create(cachedRenderTextureCreateInformation);
	}

	for(auto& widget : widgetRenderData)
	{
		for(auto& drawGroup : widget.Batches)
		{
			for(auto& renderTargetElem : drawGroup.RenderTargetElements)
			{
				if(renderTargetElem.LastUpdateCount != renderTargetElem.Target->GetUpdateCount())
				{
					renderTargetElem.LastUpdateCount = renderTargetElem.Target->GetUpdateCount();
					Area2I::AddUnique(renderTargetElem.Area, cameraRenderData.DirtyRegions);
				}
			}
		}
	}

	auto fnCreateClipRegionBuffer = [](GUIWidgetRenderData& widgetRenderData, const FrameVector<Area2I>& clipRegions) -> SPtr<GpuBuffer>
	{
		const u32 clipRegionCount = (u32)clipRegions.size();
		B3D_ASSERT(clipRegionCount <= kMaxClipRegionsPerDraw);

		GpuBufferSuballocation suballocation = widgetRenderData.ClipRegionBufferPool.Allocate();
		const SPtr<GpuBuffer>& clipRegionBuffer = suballocation.GetBuffer();
		const u32 writeSize = sizeof(ClipRegionArea) * clipRegionCount;

		ClipRegionArea* destination = reinterpret_cast<ClipRegionArea*>(clipRegionBuffer->Lock(0, writeSize, GBL_WRITE_ONLY));

		for (u32 dirtyRegionIndex = 0; dirtyRegionIndex < clipRegionCount; ++dirtyRegionIndex)
		{
			const Area2I& dirtyRegion = clipRegions[dirtyRegionIndex];

			destination[dirtyRegionIndex].TopLeft = dirtyRegion.GetPosition();
			destination[dirtyRegionIndex].BottomRight = dirtyRegion.GetPosition() + Vector2I(dirtyRegion.GetSize().Width, dirtyRegion.GetSize().Height);
		}

		clipRegionBuffer->Unlock();

		return clipRegionBuffer;
	};

	// Structure to hold prepared rendering information
	struct PreparedMeshData
	{
		const GUIMeshRenderData* RenderData = nullptr;
		FrameVector<Area2I> OverlappingRegions;
		SPtr<render::GpuParameterSet> GpuParameters;
		SPtr<GpuBuffer> ClipRegionBuffer;
		u32 ClipRegionCount = 0;
	};

	// Accumulate all GPU parameters before starting render pass
	RenderPassCreateInformation mainRenderPassCreateInformation(cameraRenderData.CachedRenderTexture, RT_NONE, RT_ALL);

	auto fnPrepareRegions = [this, &fnCreateClipRegionBuffer, &widgetRenderData, renderTargetWidth, renderTargetHeight, viewflipYFlip, &mainRenderPassCreateInformation](const Vector<Area2I>& regions, bool useDebugMaterial, FrameVector<PreparedMeshData>& outPreparedMeshData)
	{
		const Area2I clipRectangle(0, 0, renderTargetWidth, renderTargetHeight);

		FrameVector<Area2I> clippedRegions;
		clippedRegions.reserve(regions.size());

		for(Area2I region : regions)
		{
			region.Clip(clipRectangle);
			clippedRegions.push_back(region);
		}

		const Vector2I viewportOffset(0, 0);
		const float inverseRegionWidth = 1.0f / (renderTargetWidth* 0.5f);
		const float inverseRegionHeight = 1.0f / (renderTargetHeight * 0.5f);

		struct MeshToDraw
		{
			const GUIMeshRenderData* RenderData = nullptr;
			const GpuBufferSuballocation UniformBuffer;
			FrameVector<Area2I> OverlappingRegions;
		};

		FrameVector<MeshToDraw> meshesToRedraw;
		for(GUIWidgetRenderData& widget : widgetRenderData)
		{
			meshesToRedraw.clear();

			for(auto batchIterator = widget.Batches.rbegin(); batchIterator != widget.Batches.rend(); ++batchIterator)
			{
				const GUIBatchRenderData& batch = *batchIterator;

				// Check if the draw group overlaps any dirty regions, if not we can skip it
				bool isDrawGroupOverlappingAny = false;
				for (const Area2I& region : regions)
				{
					if (batch.Bounds.Overlaps(region))
					{
						isDrawGroupOverlappingAny = true;
						break;
					}
				}

				if (!isDrawGroupOverlappingAny)
					continue;

				// Find exact elements of the draw group which overlap dirty regions and record those elements and their overlapping regions
				for(const GUIMeshRenderData& meshRenderData : batch.Elements)
				{
					FrameVector<Area2I> overlappingDirtyRegions;
					for (const Area2I& region : clippedRegions)
					{
						if (meshRenderData.Bounds.Overlaps(region))
						{
							overlappingDirtyRegions.push_back(region);

							// When we hit the max, create an entry and continue collecting remaining regions
							if (overlappingDirtyRegions.size() == kMaxClipRegionsPerDraw)
							{
								GpuBufferSuballocation uniformBuffer = widget.UniformBufferPool.Allocate();
								SpriteMaterial::PopulateUniformBuffer(uniformBuffer, viewportOffset, inverseRegionWidth, inverseRegionHeight, viewflipYFlip, mTime, (u32)overlappingDirtyRegions.size(), widget.WorldTransform, meshRenderData.MaterialInformation);

								meshesToRedraw.push_back({ &meshRenderData, std::move(uniformBuffer), std::move(overlappingDirtyRegions) });
								overlappingDirtyRegions = FrameVector<Area2I>();
							}
						}
					}

					if(overlappingDirtyRegions.empty())
						continue;

					// Note: We will unnecessarily do this update multiple times if the same mesh overlaps multiple dirty regions
					GpuBufferSuballocation uniformBuffer = widget.UniformBufferPool.Allocate();
					SpriteMaterial::PopulateUniformBuffer(uniformBuffer, viewportOffset, inverseRegionWidth, inverseRegionHeight, viewflipYFlip, mTime, (u32)overlappingDirtyRegions.size(), widget.WorldTransform, meshRenderData.MaterialInformation);

					meshesToRedraw.push_back({ &meshRenderData, std::move(uniformBuffer), std::move(overlappingDirtyRegions) });
				}
			}

			for(const MeshToDraw& meshToDraw : meshesToRedraw)
			{
				const GUIMeshRenderData* const meshRenderData = meshToDraw.RenderData;
				SpriteMaterial* const material = meshRenderData->Material;
				const GUIBatchGpuParameterInfo& parameterInfo = widget.GpuParameterInfos[meshRenderData->GpuParametersIndex];

				const GpuBufferSuballocation& uniformBuffer = meshToDraw.UniformBuffer;
				const SPtr<GpuBuffer>& clipRegionBuffer = fnCreateClipRegionBuffer(widget, meshToDraw.OverlappingRegions);
				const SPtr<MaterialParameterAdapter>& materialParameterAdapter = widget.MaterialParameterAdapters[parameterInfo.MaterialParameterIndex];

				// Prepare material and get GPU parameters
				if(!kEnableGUIRegionDebugDrawing || !useDebugMaterial)
					material->Prepare(materialParameterAdapter, meshRenderData->Mesh, meshRenderData->MaterialInformation.Texture, mSamplerState, uniformBuffer, clipRegionBuffer);
				else
					material->Prepare(materialParameterAdapter, meshRenderData->Mesh, Texture::kPink, mSamplerState, uniformBuffer, clipRegionBuffer);

				PreparedMeshData preparedData;
				preparedData.RenderData = meshRenderData;
				preparedData.OverlappingRegions = meshToDraw.OverlappingRegions;
				preparedData.GpuParameters = materialParameterAdapter->GetGpuParameterSet();
				preparedData.ClipRegionBuffer = clipRegionBuffer;
				preparedData.ClipRegionCount = (u32)meshToDraw.OverlappingRegions.size();

				mainRenderPassCreateInformation.Parameters.Add(preparedData.GpuParameters);
				outPreparedMeshData.push_back(std::move(preparedData));
			}
		}
	};

	// Determine which regions to prepare
	FrameVector<PreparedMeshData> debugDrawPreparedMeshes;
	FrameVector<PreparedMeshData> preparedMeshes;

	if(!rebuildCachedRenderTexture && !kRedrawAllRegions)
	{
		fnPrepareRegions(cameraRenderData.LastFrameDirtyDebugDrawRegions, false, debugDrawPreparedMeshes);
		fnPrepareRegions(cameraRenderData.DirtyRegions, true, preparedMeshes);
	}
	else
	{
		fnPrepareRegions({ Area2I(0, 0, renderTargetWidth, renderTargetHeight) }, true, preparedMeshes);
	}

	if(rebuildCachedRenderTexture)
	{
		mainRenderPassCreateInformation.ClearMask = RT_COLOR_ALL;
		mainRenderPassCreateInformation.ClearColor = Color::kZero;;
	}

	// Begin render pass with all accumulated GPU parameters
	commandBuffer.BeginRenderPass(mainRenderPassCreateInformation);

	// Now execute the actual rendering using the prepared data
	auto fnDrawPreparedMeshes = [&commandBuffer, renderTargetWidth, renderTargetHeight, rebuildCachedRenderTexture](const FrameVector<PreparedMeshData>& preparedMeshes, const Vector<Area2I>& regions)
	{
		const Area2I clipRectangle(0, 0, renderTargetWidth, renderTargetHeight);

		// Clear all regions
		for(Area2I region : regions)
		{
			region.Clip(clipRectangle);

			const Area2 normalizedRegionArea =
				Area2(
					region.X / (float)renderTargetWidth,
					region.Y / (float)renderTargetHeight,
					region.Width / (float)renderTargetWidth,
					region.Height / (float)renderTargetHeight);

			// TODO - This could probably be more efficient by clearing N regions in a single draw call
			commandBuffer.SetViewport(normalizedRegionArea);

			if(!rebuildCachedRenderTexture)
				commandBuffer.ClearViewport(RT_COLOR_ALL, Color::kZero);
		}

		commandBuffer.SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));

		for(const PreparedMeshData& preparedData : preparedMeshes)
		{
			const GUIMeshRenderData* const meshRenderData = preparedData.RenderData;
			SpriteMaterial* const material = meshRenderData->Material;

			// Note: Sprite material is being re-bound for each draw. We should ensure the material is bound only when it actually changes.
			// Note: Ideally all these buffers are using dynamic buffer offsets, and textures are atlased so we can avoid rebinding parameters each time
			material->Render(commandBuffer, preparedData.GpuParameters, meshRenderData->Mesh, meshRenderData->SubMesh, preparedData.ClipRegionBuffer, preparedData.ClipRegionCount, meshRenderData->MaterialInformation.AdditionalData);
		}
	};

	if(!rebuildCachedRenderTexture && !kRedrawAllRegions)
	{
		if(!cameraRenderData.LastFrameDirtyDebugDrawRegions.empty())
			fnDrawPreparedMeshes(debugDrawPreparedMeshes, cameraRenderData.LastFrameDirtyDebugDrawRegions);

		if(!cameraRenderData.DirtyRegions.empty())
			fnDrawPreparedMeshes(preparedMeshes, cameraRenderData.DirtyRegions);
	}
	else
	{
		fnDrawPreparedMeshes(preparedMeshes, { Area2I(0, 0, renderTargetWidth, renderTargetHeight) });
	}

	cameraRenderData.LastFrameDirtyDebugDrawRegions.clear();
	if(kEnableGUIRegionDebugDrawing)
		std::swap(cameraRenderData.DirtyRegions, cameraRenderData.LastFrameDirtyDebugDrawRegions);

	cameraRenderData.DirtyRegions.clear();

	// Blit cached texture into main output
	// Note: This could be optimized by blitting only the modified regions
	commandBuffer.EndRenderPass();

	BlitInformation blitInformation = BlitInformation::Blend(cameraRenderData.CachedRenderTexture->GetColorTexture(0), renderTarget, Area2I::kEmpty, RT_NONE, RT_ALL);
	blitInformation.OutputArea = Area2(0.0f, 0.0f, 1.0f, 1.0f);
	blitInformation.WriteAlpha = true;

	GetRendererUtility().Blit(commandBuffer, blitInformation);

	commandBuffer.EndLabel();

	// Restore original viewport
	commandBuffer.SetViewport(camera.GetViewport()->GetArea());
}

void GUIRenderer::Update(float time)
{
	mTime = time;
}

void GUIRenderer::UpdateDrawGroups(const Camera* camera, u64 widgetId, u32 widgetDepth, const Matrix4& worldTransform, const GUIDrawGroupRenderDataUpdate& data)
{
	mWidgetToCameraMap[widgetId] = camera;

	const SPtr<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
	GUICameraRenderData& cameraRenderData = mPerCameraData[camera];
	Vector<GUIWidgetRenderData>& widgets = cameraRenderData.WidgetRenderData;
	GUIWidgetRenderData* widget;

	auto found = std::find_if(widgets.begin(), widgets.end(), [widgetId](auto& x) { return x.WidgetId == widgetId; });
	if(found == widgets.end())
	{
		widgets.push_back(GUIWidgetRenderData(widgetId, *gpuDevice));
		widget = &widgets.back();
	}
	else
		widget = &(*found);

	if(!data.Batches.empty())
	{
		// Move old parameter adapters back to the pool so they can be re-used
		u32 removedAdapterCount = 0;
		for(auto& batch : widget->Batches)
		{
			for(auto& entry : batch.Elements)
			{
				const GUIBatchGpuParameterInfo& gpuParameterInfo = widget->GpuParameterInfos[entry.GpuParametersIndex];

				auto foundPool = mMaterialParameterAdapterPool.find(entry.Material);
				if(B3D_ENSURE(foundPool != mMaterialParameterAdapterPool.end()))
				{
					foundPool->second.Add(widget->MaterialParameterAdapters[gpuParameterInfo.MaterialParameterIndex]);
					removedAdapterCount++;
				}
			}
		}

		B3D_ENSURE(removedAdapterCount == widget->MaterialParameterAdapters.Size());
		widget->MaterialParameterAdapters.Clear();

		widget->Batches = data.Batches;

		// Allocate GPU buffers containing the material parameters
		u32 elementCount = 0;
		for(auto& batch : widget->Batches)
			elementCount += (u32)batch.Elements.size();

		const u32 allocatedParameterInfoCount = (u32)widget->GpuParameterInfos.size();
		if(elementCount > allocatedParameterInfoCount)
			widget->GpuParameterInfos.resize(elementCount);

		u32 currentBufferIndex = 0;
		for(auto& batch : widget->Batches)
		{
			for(auto& entry : batch.Elements)
			{
				entry.GpuParametersIndex = currentBufferIndex++;

				TArray<SPtr<MaterialParameterAdapter>>& materialParameterAdapterPool = mMaterialParameterAdapterPool[entry.Material];
				SPtr<MaterialParameterAdapter> materialParameterAdapter;
				if(materialParameterAdapterPool.Empty())
					materialParameterAdapter = entry.Material->CreateParameterAdapter(true);
				else
				{
					materialParameterAdapter = materialParameterAdapterPool.Back();
					materialParameterAdapterPool.Pop();
				}

				widget->GpuParameterInfos[entry.GpuParametersIndex].MaterialParameterIndex = (u32)widget->MaterialParameterAdapters.Size();
				widget->MaterialParameterAdapters.Add(materialParameterAdapter);
			}
		}
	}

	widget->WorldTransform = worldTransform;

	for(const Area2I& dirtyRegion : data.DirtyRegions)
		Area2I::AddUnique(dirtyRegion, cameraRenderData.DirtyRegions);

	if(widget->WidgetDepth != widgetDepth)
	{
		widget->WidgetDepth = widgetDepth;

		std::sort(widgets.begin(), widgets.end(), [](auto& x, auto& y)
				  { return x.WidgetDepth > y.WidgetDepth; });
	}
}

void GUIRenderer::ClearDrawGroups(u64 widgetId)
{
	const Camera* camera = nullptr;
	if(auto found = mWidgetToCameraMap.find(widgetId); found != mWidgetToCameraMap.end())
		camera = found->second;

	auto found = mPerCameraData.find(camera);
	if(found == mPerCameraData.end())
		return;

	Vector<GUIWidgetRenderData>& widgetData = found->second.WidgetRenderData;
	auto found2 = std::find_if(widgetData.begin(), widgetData.end(), [widgetId](auto& x)
								  { return x.WidgetId == widgetId; });
	if(found2 == widgetData.end())
		return;

	for(const auto& drawGroup : found2->Batches)
		Area2I::AddUnique(drawGroup.Bounds, found->second.DirtyRegions);

	widgetData.erase(found2);

	if(widgetData.empty())
		mPerCameraData.erase(found);

	mWidgetToCameraMap.erase(widgetId);
}

}}
