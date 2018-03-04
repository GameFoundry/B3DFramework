//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "GUI/BsGUIManager.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUIElement.h"
#include "2D/BsSpriteTexture.h"
#include "Utility/BsTime.h"
#include "Scene/BsSceneObject.h"
#include "Material/BsMaterial.h"
#include "Mesh/BsMeshData.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Mesh/BsMesh.h"
#include "Managers/BsRenderWindowManager.h"
#include "Platform/BsPlatform.h"
#include "Math/BsRect2I.h"
#include "BsCoreApplication.h"
#include "Error/BsException.h"
#include "Input/BsInput.h"
#include "GUI/BsGUIInputCaret.h"
#include "GUI/BsGUIInputSelection.h"
#include "GUI/BsGUIContextMenu.h"
#include "GUI/BsDragAndDropManager.h"
#include "GUI/BsGUIDropDownBoxManager.h"
#include "Profiling/BsProfilerCPU.h"
#include "Input/BsVirtualInput.h"
#include "Platform/BsCursor.h"
#include "CoreThread/BsCoreThread.h"
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsCamera.h"
#include "GUI/BsGUITooltipManager.h"
#include "Renderer/BsRendererUtility.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsRenderTexture.h"
#include "RenderAPI/BsSamplerState.h"
#include "Managers/BsRenderStateManager.h"
#include "Resources/BsBuiltinResources.h"

using namespace std::placeholders;

namespace bs
{
	struct GUIGroupElement
	{
		GUIGroupElement()
		{ }

		GUIGroupElement(GUIElement* _element, UINT32 _renderElement)
			:element(_element), renderElement(_renderElement)
		{ }

		GUIElement* element;
		UINT32 renderElement;
	};

	struct GUIMaterialGroup
	{
		SpriteMaterial* material;
		SpriteMaterialInfo matInfo;
		GUIMeshType meshType;
		UINT32 numVertices;
		UINT32 numIndices;
		UINT32 depth;
		UINT32 minDepth;
		Rect2I bounds;
		Vector<GUIGroupElement> elements;
	};

	const UINT32 GUIManager::DRAG_DISTANCE = 3;
	const float GUIManager::TOOLTIP_HOVER_TIME = 1.0f;

	GUIManager::GUIManager()
		: mCoreDirty(false), mActiveMouseButton(GUIMouseButton::Left), mShowTooltip(false), mTooltipElementHoverStart(0.0f)
		, mInputCaret(nullptr), mInputSelection(nullptr), mSeparateMeshesByWidget(true), mDragState(DragState::NoDrag)
		, mCaretColor(1.0f, 0.6588f, 0.0f), mCaretBlinkInterval(0.5f), mCaretLastBlinkTime(0.0f), mIsCaretOn(false)
		, mActiveCursor(CursorType::Arrow), mTextSelectionColor(0.0f, 114/255.0f, 188/255.0f)
	{
		// Note: Hidden dependency. GUI must receive input events before other systems, in order so it can mark them as used
		// if required. e.g. clicking on a context menu should mark the event as used so that other non-GUI systems know
		// that they probably should not process such event themselves.
		mOnPointerMovedConn = gInput().onPointerMoved.connect(std::bind(&GUIManager::onPointerMoved, this, _1));
		mOnPointerPressedConn = gInput().onPointerPressed.connect(std::bind(&GUIManager::onPointerPressed, this, _1));
		mOnPointerReleasedConn = gInput().onPointerReleased.connect(std::bind(&GUIManager::onPointerReleased, this, _1));
		mOnPointerDoubleClick = gInput().onPointerDoubleClick.connect(std::bind(&GUIManager::onPointerDoubleClick, this, _1));
		mOnTextInputConn = gInput().onCharInput.connect(std::bind(&GUIManager::onTextInput, this, _1)); 
		mOnInputCommandConn = gInput().onInputCommand.connect(std::bind(&GUIManager::onInputCommandEntered, this, _1)); 
		mOnVirtualButtonDown = VirtualInput::instance().onButtonDown.connect(std::bind(&GUIManager::onVirtualButtonDown, this, _1, _2));

		mWindowGainedFocusConn = RenderWindowManager::instance().onFocusGained.connect(std::bind(&GUIManager::onWindowFocusGained, this, _1));
		mWindowLostFocusConn = RenderWindowManager::instance().onFocusLost.connect(std::bind(&GUIManager::onWindowFocusLost, this, _1));
		mMouseLeftWindowConn = RenderWindowManager::instance().onMouseLeftWindow.connect(std::bind(&GUIManager::onMouseLeftWindow, this, _1));

		mInputCaret = bs_new<GUIInputCaret>();
		mInputSelection = bs_new<GUIInputSelection>();

		DragAndDropManager::startUp();
		mDragEndedConn = DragAndDropManager::instance().onDragEnded.connect(std::bind(&GUIManager::onMouseDragEnded, this, _1, _2));

		GUIDropDownBoxManager::startUp();
		GUITooltipManager::startUp();

		mTriangleVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mTriangleVertexDesc->addVertElem(VET_FLOAT2, VES_POSITION);
		mTriangleVertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		mLineVertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		mLineVertexDesc->addVertElem(VET_FLOAT2, VES_POSITION);

		// Need to defer this call because I want to make sure all managers are initialized first
		deferredCall(std::bind(&GUIManager::updateCaretTexture, this));
		deferredCall(std::bind(&GUIManager::updateTextSelectionTexture, this));

		mRenderer = RendererExtension::create<ct::GUIRenderer>(nullptr);
	}

	GUIManager::~GUIManager()
	{
		GUITooltipManager::shutDown();
		GUIDropDownBoxManager::shutDown();
		DragAndDropManager::shutDown();

		// Make a copy of widgets, since destroying them will remove them from mWidgets and
		// we can't iterate over an array thats getting modified
		Vector<WidgetInfo> widgetCopy = mWidgets;
		for(auto& widget : widgetCopy)
			widget.widget->_destroy();

		// Ensure everything queued get destroyed
		processDestroyQueue();

		mOnPointerPressedConn.disconnect();
		mOnPointerReleasedConn.disconnect();
		mOnPointerMovedConn.disconnect();
		mOnPointerDoubleClick.disconnect();
		mOnTextInputConn.disconnect();
		mOnInputCommandConn.disconnect();
		mOnVirtualButtonDown.disconnect();

		mDragEndedConn.disconnect();

		mWindowGainedFocusConn.disconnect();
		mWindowLostFocusConn.disconnect();

		mMouseLeftWindowConn.disconnect();

		bs_delete(mInputCaret);
		bs_delete(mInputSelection);

		assert(mCachedGUIData.size() == 0);
	}

	void GUIManager::destroyCore(ct::GUIRenderer* core)
	{
		bs_delete(core);
	}

	void GUIManager::registerWidget(GUIWidget* widget)
	{
		const Viewport* renderTarget = widget->getTarget();
		if (renderTarget == nullptr)
			return;

		mWidgets.push_back(WidgetInfo(widget));
		auto findIter = mCachedGUIData.find(renderTarget);

		if(findIter == end(mCachedGUIData))
			mCachedGUIData[renderTarget] = GUIRenderData();

		GUIRenderData& windowData = mCachedGUIData[renderTarget];
		windowData.widgets.push_back(widget);
		windowData.isDirty = true;
	}

	void GUIManager::unregisterWidget(GUIWidget* widget)
	{
		{
			auto findIter = std::find_if(begin(mWidgets), end(mWidgets), [=] (const WidgetInfo& x) { return x.widget == widget; } );

			if(findIter != mWidgets.end())
				mWidgets.erase(findIter);
		}

		for(auto& entry : mElementsInFocus)
		{
			if (entry.widget == widget)
				entry.widget = nullptr;
		}

		for (auto& entry : mElementsUnderPointer)
		{
			if (entry.widget == widget)
				entry.widget = nullptr;
		}

		for (auto& entry : mActiveElements)
		{
			if (entry.widget == widget)
				entry.widget = nullptr;
		}

		const Viewport* renderTarget = widget->getTarget();
		GUIRenderData& renderData = mCachedGUIData[renderTarget];

		{
			auto findIter = std::find(begin(renderData.widgets), end(renderData.widgets), widget);
			
			if(findIter != end(renderData.widgets))
				renderData.widgets.erase(findIter);
		}

		if(renderData.widgets.size() == 0)
		{
			mCachedGUIData.erase(renderTarget);
			mCoreDirty = true;
		}
		else
			renderData.isDirty = true;
	}

	void GUIManager::update()
	{
		DragAndDropManager::instance()._update();

		// Show tooltip if needed
		if (mShowTooltip)
		{
			float diff = gTime().getTime() - mTooltipElementHoverStart;
			if (diff >= TOOLTIP_HOVER_TIME || gInput().isButtonHeld(BC_LCONTROL) || gInput().isButtonHeld(BC_RCONTROL))
			{
				for(auto& entry : mElementsUnderPointer)
				{
					const WString& tooltipText = entry.element->_getTooltip();
					GUIWidget* parentWidget = entry.element->_getParentWidget();

					if (!tooltipText.empty() && parentWidget != nullptr)
					{
						const RenderWindow* window = getWidgetWindow(*parentWidget);
						if (window != nullptr)
						{
							Vector2I windowPos = window->screenToWindowPos(gInput().getPointerPosition());

							GUITooltipManager::instance().show(*parentWidget, windowPos, tooltipText);
							break;
						}
					}
				}

				mShowTooltip = false;
			}
		}

		// Update layouts
		gProfilerCPU().beginSample("UpdateLayout");
		for(auto& widgetInfo : mWidgets)
		{
			widgetInfo.widget->_updateLayout();
		}
		gProfilerCPU().endSample("UpdateLayout");

		// Destroy all queued elements (and loop in case any new ones get queued during destruction)
		do
		{
			mNewElementsUnderPointer.clear();
			for (auto& elementInfo : mElementsUnderPointer)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewElementsUnderPointer.push_back(elementInfo);
			}

			mElementsUnderPointer.swap(mNewElementsUnderPointer);

			mNewActiveElements.clear();
			for (auto& elementInfo : mActiveElements)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewActiveElements.push_back(elementInfo);
			}

			mActiveElements.swap(mNewActiveElements);

			mNewElementsInFocus.clear();
			for (auto& elementInfo : mElementsInFocus)
			{
				if (!elementInfo.element->_isDestroyed())
					mNewElementsInFocus.push_back(elementInfo);
			}

			mElementsInFocus.swap(mNewElementsInFocus);

			for (auto& focusElementInfo : mForcedFocusElements)
			{
				if (focusElementInfo.element->_isDestroyed())
					continue;

				if (focusElementInfo.focus)
				{
					auto iterFind = std::find_if(mElementsInFocus.begin(), mElementsInFocus.end(),
						[&](const ElementFocusInfo& x) { return x.element == focusElementInfo.element; });

					if (iterFind == mElementsInFocus.end())
					{
						mElementsInFocus.push_back(ElementFocusInfo(focusElementInfo.element, 
							focusElementInfo.element->_getParentWidget(), false));

						mCommandEvent = GUICommandEvent();
						mCommandEvent.setType(GUICommandEventType::FocusGained);

						sendCommandEvent(focusElementInfo.element, mCommandEvent);
					}
				}
				else
				{
					mNewElementsInFocus.clear();
					for (auto& elementInfo : mElementsInFocus)
					{
						if (elementInfo.element == focusElementInfo.element)
						{
							mCommandEvent = GUICommandEvent();
							mCommandEvent.setType(GUICommandEventType::FocusLost);

							sendCommandEvent(elementInfo.element, mCommandEvent);
						}
						else
							mNewElementsInFocus.push_back(elementInfo);
					}

					mElementsInFocus.swap(mNewElementsInFocus);
				}
			}

			mForcedFocusElements.clear();

		} while (processDestroyQueueIteration());

		// Blink caret
		float curTime = gTime().getTime();

		if ((curTime - mCaretLastBlinkTime) >= mCaretBlinkInterval)
		{
			mCaretLastBlinkTime = curTime;
			mIsCaretOn = !mIsCaretOn;

			mCommandEvent = GUICommandEvent();
			mCommandEvent.setType(GUICommandEventType::Redraw);

			for (auto& elementInfo : mElementsInFocus)
			{
				sendCommandEvent(elementInfo.element, mCommandEvent);
			}
		}

		PROFILE_CALL(updateMeshes(), "UpdateMeshes");

		// Send potentially updated meshes to core for rendering
		if (mCoreDirty)
		{
			UnorderedMap<SPtr<ct::Camera>, Vector<GUICoreRenderData>> corePerCameraData;

			for (auto& viewportData : mCachedGUIData)
			{
				const GUIRenderData& renderData = viewportData.second;

				SPtr<Camera> camera;
				for (auto& widget : viewportData.second.widgets)
				{
					camera = widget->getCamera();
					if (camera != nullptr)
						break;
				}

				if (camera == nullptr)
					continue;

				auto insertedData = corePerCameraData.insert(std::make_pair(camera->getCore(), Vector<GUICoreRenderData>()));
				Vector<GUICoreRenderData>& cameraData = insertedData.first->second;

				for (auto& entry : renderData.cachedMeshes)
				{
					const SPtr<Mesh>& mesh = entry.isLine ? renderData.lineMesh : renderData.triangleMesh;
					if(!mesh)
						continue;

					cameraData.push_back(GUICoreRenderData());
					GUICoreRenderData& newEntry = cameraData.back();

					SPtr<ct::Texture> textureCore;
					if (entry.matInfo.texture.isLoaded())
						textureCore = entry.matInfo.texture->getCore();
					else
						textureCore = nullptr;

					newEntry.material = entry.material;
					newEntry.mesh = mesh->getCore();
					newEntry.texture = textureCore;
					newEntry.tint = entry.matInfo.tint;
					newEntry.worldTransform = entry.widget->getWorldTfrm();
					newEntry.additionalData = entry.matInfo.additionalData;

					newEntry.subMesh.indexOffset = entry.indexOffset;
					newEntry.subMesh.indexCount = entry.indexCount;
					newEntry.subMesh.drawOp = entry.isLine ? DOT_LINE_LIST : DOT_TRIANGLE_LIST;
				}
			}

			gCoreThread().queueCommand(std::bind(&ct::GUIRenderer::updateData, mRenderer.get(), corePerCameraData));

			mCoreDirty = false;
		}
	}

	void GUIManager::processDestroyQueue()
	{
		// Loop until everything empties
		while (processDestroyQueueIteration())
		{ }
	}

	void GUIManager::updateMeshes()
	{
		for(auto& cachedMeshData : mCachedGUIData)
		{
			GUIRenderData& renderData = cachedMeshData.second;

			// Check if anything is dirty. If nothing is we can skip the update
			bool isDirty = renderData.isDirty;
			renderData.isDirty = false;

			for(auto& widget : renderData.widgets)
			{
				if (widget->isDirty(true))
				{
					isDirty = true;
				}
			}

			if(!isDirty)
				continue;

			mCoreDirty = true;

			bs_frame_mark();
			{
				// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
				auto elemComp = [](const GUIGroupElement& a, const GUIGroupElement& b)
				{
					UINT32 aDepth = a.element->_getRenderElementDepth(a.renderElement);
					UINT32 bDepth = b.element->_getRenderElementDepth(b.renderElement);

					// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set
					// requires all elements to be unique
					return (aDepth > bDepth) || 
						(aDepth == bDepth && a.element > b.element) || 
						(aDepth == bDepth && a.element == b.element && a.renderElement > b.renderElement); 
				};

				FrameSet<GUIGroupElement, std::function<bool(const GUIGroupElement&, const GUIGroupElement&)>> allElements(elemComp);

				for (auto& widget : renderData.widgets)
				{
					const Vector<GUIElement*>& elements = widget->getElements();

					for (auto& element : elements)
					{
						if (!element->_isVisible())
							continue;

						UINT32 numRenderElems = element->_getNumRenderElements();
						for (UINT32 i = 0; i < numRenderElems; i++)
						{
							allElements.insert(GUIGroupElement(element, i));
						}
					}
				}

				// Group the elements in such a way so that we end up with a smallest amount of
				// meshes, without breaking back to front rendering order
				FrameUnorderedMap<UINT64, FrameVector<GUIMaterialGroup>> materialGroups;
				for (auto& elem : allElements)
				{
					GUIElement* guiElem = elem.element;
					UINT32 renderElemIdx = elem.renderElement;
					UINT32 elemDepth = guiElem->_getRenderElementDepth(renderElemIdx);

					Rect2I tfrmedBounds = guiElem->_getClippedBounds();
					tfrmedBounds.transform(guiElem->_getParentWidget()->getWorldTfrm());

					SpriteMaterial* spriteMaterial = nullptr;
					const SpriteMaterialInfo& matInfo = guiElem->_getMaterial(renderElemIdx, &spriteMaterial);
					assert(spriteMaterial != nullptr);

					UINT64 hash = spriteMaterial->getMergeHash(matInfo);
					FrameVector<GUIMaterialGroup>& groupsPerMaterial = materialGroups[hash];
					
					// Try to find a group this material will fit in:
					//  - Group that has a depth value same or one below elements depth will always be a match
					//  - Otherwise, we search higher depth values as well, but we only use them if no elements in between those depth values
					//    overlap the current elements bounds.
					GUIMaterialGroup* foundGroup = nullptr;

					for (auto groupIter = groupsPerMaterial.rbegin(); groupIter != groupsPerMaterial.rend(); ++groupIter)
					{
						// If we separate meshes by widget, ignore any groups with widget parents other than mine
						if (mSeparateMeshesByWidget)
						{
							if (groupIter->elements.size() > 0)
							{
								GUIElement* otherElem = groupIter->elements.begin()->element; // We only need to check the first element
								if (otherElem->_getParentWidget() != guiElem->_getParentWidget())
									continue;
							}
						}

						GUIMaterialGroup& group = *groupIter;

						if (group.depth == elemDepth)
						{
							foundGroup = &group;
							break;
						}
						else
						{
							UINT32 startDepth = elemDepth;
							UINT32 endDepth = group.depth;

							Rect2I potentialGroupBounds = group.bounds;
							potentialGroupBounds.encapsulate(tfrmedBounds);

							bool foundOverlap = false;
							for (auto& material : materialGroups)
							{
								for (auto& matGroup : material.second)
								{
									if (&matGroup == &group)
										continue;

									if ((matGroup.minDepth >= startDepth && matGroup.minDepth <= endDepth)
										|| (matGroup.depth >= startDepth && matGroup.depth <= endDepth))
									{
										if (matGroup.bounds.overlaps(potentialGroupBounds))
										{
											foundOverlap = true;
											break;
										}
									}
								}
							}

							if (!foundOverlap)
							{
								foundGroup = &group;
								break;
							}
						}
					}

					if (foundGroup == nullptr)
					{
						groupsPerMaterial.push_back(GUIMaterialGroup());
						foundGroup = &groupsPerMaterial[groupsPerMaterial.size() - 1];

						foundGroup->depth = elemDepth;
						foundGroup->minDepth = elemDepth;
						foundGroup->bounds = tfrmedBounds;
						foundGroup->elements.push_back(GUIGroupElement(guiElem, renderElemIdx));
						foundGroup->matInfo = matInfo.clone();
						foundGroup->material = spriteMaterial;

						guiElem->_getMeshInfo(renderElemIdx, foundGroup->numVertices, foundGroup->numIndices, foundGroup->meshType);
					}
					else
					{
						foundGroup->bounds.encapsulate(tfrmedBounds);
						foundGroup->elements.push_back(GUIGroupElement(guiElem, renderElemIdx));
						foundGroup->minDepth = std::min(foundGroup->minDepth, elemDepth);
						
						UINT32 numVertices;
						UINT32 numIndices;
						GUIMeshType meshType;
						guiElem->_getMeshInfo(renderElemIdx, numVertices, numIndices, meshType);
						assert(meshType == foundGroup->meshType); // It's expected that GUI element doesn't use same material for different mesh types so this should always be true

						foundGroup->numVertices += numVertices;
						foundGroup->numIndices += numIndices;

						spriteMaterial->merge(foundGroup->matInfo, matInfo);
					}
				}

				// Make a list of all GUI elements, sorted from farthest to nearest (highest depth to lowest)
				auto groupComp = [](GUIMaterialGroup* a, GUIMaterialGroup* b)
				{
					return (a->depth > b->depth) || (a->depth == b->depth && a > b);
					// Compare pointers just to differentiate between two elements with the same depth, their order doesn't really matter, but std::set
					// requires all elements to be unique
				};

				UINT32 numMeshes = 0;
				UINT32 numIndices[2] = { 0, 0 };
				UINT32 numVertices[2] = { 0, 0 };

				FrameSet<GUIMaterialGroup*, std::function<bool(GUIMaterialGroup*, GUIMaterialGroup*)>> sortedGroups(groupComp);
				for(auto& material : materialGroups)
				{
					for(auto& group : material.second)
					{
						sortedGroups.insert(&group);

						UINT32 typeIdx = (UINT32)group.meshType;
						numIndices[typeIdx] += group.numIndices;
						numVertices[typeIdx] += group.numVertices;

						numMeshes++;
					}
				}

				renderData.triangleMesh = nullptr;
				renderData.lineMesh = nullptr;

				renderData.cachedMeshes.resize(numMeshes);

				SPtr<MeshData> meshData[2];
				SPtr<VertexDataDesc> vertexDesc[2] = { mTriangleVertexDesc, mLineVertexDesc };

				UINT8* vertices[2] = { nullptr, nullptr };
				UINT32* indices[2] = { nullptr, nullptr };

				for(UINT32 i = 0; i < 2; i++)
				{
					if(numVertices[i] > 0 && numIndices[i] > 0)
					{
						meshData[i] = MeshData::create(numVertices[i], numIndices[i], vertexDesc[i]);

						vertices[i] = meshData[i]->getElementData(VES_POSITION);
						indices[i] = meshData[i]->getIndices32();
					}
				}

				// Fill buffers for each group and update their meshes
				UINT32 meshIdx = 0;
				UINT32 vertexOffset[2] = { 0, 0 };
				UINT32 indexOffset[2] = { 0, 0 };

				for(auto& group : sortedGroups)
				{
					GUIWidget* widget;

					if (group->elements.size() == 0)
						widget = nullptr;
					else
					{
						GUIElement* elem = group->elements.begin()->element;
						widget = elem->_getParentWidget();
					}

					GUIMeshData& guiMeshData = renderData.cachedMeshes[meshIdx];
					guiMeshData.matInfo = group->matInfo;
					guiMeshData.material = group->material;
					guiMeshData.widget = widget;
					guiMeshData.isLine = group->meshType == GUIMeshType::Line;

					UINT32 typeIdx = (UINT32)group->meshType;
					guiMeshData.indexOffset = indexOffset[typeIdx];

					UINT32 groupNumIndices = 0;
					for(auto& matElement : group->elements)
					{
						matElement.element->_fillBuffer(
							vertices[typeIdx], indices[typeIdx], 
							vertexOffset[typeIdx], indexOffset[typeIdx], 
							numVertices[typeIdx], numIndices[typeIdx], matElement.renderElement);

						UINT32 elemNumVertices;
						UINT32 elemNumIndices;
						GUIMeshType meshType;
						matElement.element->_getMeshInfo(matElement.renderElement, elemNumVertices, elemNumIndices, meshType);

						UINT32 indexStart = indexOffset[typeIdx];
						UINT32 indexEnd = indexStart + elemNumIndices;

						for(UINT32 i = indexStart; i < indexEnd; i++)
							indices[typeIdx][i] += vertexOffset[typeIdx];

						indexOffset[typeIdx] += elemNumIndices;
						vertexOffset[typeIdx] += elemNumVertices;

						groupNumIndices += elemNumIndices;
					}

					guiMeshData.indexCount = groupNumIndices;

					meshIdx++;
				}

				if(meshData[0])
					renderData.triangleMesh = Mesh::_createPtr(meshData[0], MU_STATIC, DOT_TRIANGLE_LIST);

				if(meshData[1])
					renderData.lineMesh = Mesh::_createPtr(meshData[1], MU_STATIC, DOT_LINE_LIST);
			}

			bs_frame_clear();			
		}
	}

	void GUIManager::updateCaretTexture()
	{
		if(mCaretTexture == nullptr)
		{
			TEXTURE_DESC texDesc; // Default

			HTexture newTex = Texture::create(texDesc);
			mCaretTexture = SpriteTexture::create(newTex);
		}

		const HTexture& tex = mCaretTexture->getTexture();
		SPtr<PixelData> data = tex->getProperties().allocBuffer(0, 0);

		data->setColorAt(mCaretColor, 0, 0);
		tex->writeData(data);
	}

	void GUIManager::updateTextSelectionTexture()
	{
		if(mTextSelectionTexture == nullptr)
		{
			TEXTURE_DESC texDesc; // Default

			HTexture newTex = Texture::create(texDesc);
			mTextSelectionTexture = SpriteTexture::create(newTex);
		}

		const HTexture& tex = mTextSelectionTexture->getTexture();
		SPtr<PixelData> data = tex->getProperties().allocBuffer(0, 0);

		data->setColorAt(mTextSelectionColor, 0, 0);
		tex->writeData(data);
	}

	void GUIManager::onMouseDragEnded(const PointerEvent& event, DragCallbackInfo& dragInfo)
	{
		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		if(DragAndDropManager::instance().isDragInProgress() && guiButton == GUIMouseButton::Left)
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				Vector2I localPos;

				if(elementInfo.widget != nullptr)
					localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

				bool acceptDrop = true;
				if(DragAndDropManager::instance().needsValidDropTarget())
				{
					acceptDrop = elementInfo.element->_acceptDragAndDrop(localPos, DragAndDropManager::instance().getDragTypeId());
				}

				if(acceptDrop)
				{
					mMouseEvent.setDragAndDropDroppedData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
					dragInfo.processed = sendMouseEvent(elementInfo.element, mMouseEvent);

					if(dragInfo.processed)
						return;
				}
			}
		}

		dragInfo.processed = false;
	}

	void GUIManager::onPointerMoved(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		if(mDragState == DragState::HeldWithoutDrag)
		{
			UINT32 dist = mLastPointerClickPos.manhattanDist(event.screenPos);

			if(dist > DRAG_DISTANCE)
			{
				for(auto& activeElement : mActiveElements)
				{
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);
					Vector2I localDragStartPos = getWidgetRelativePos(activeElement.widget, mLastPointerClickPos);

					mMouseEvent.setMouseDragStartData(localPos, localDragStartPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
				}

				mDragState = DragState::Dragging;
				mDragStartPos = event.screenPos;
			}
		}

		// If mouse is being held down send MouseDrag events
		if(mDragState == DragState::Dragging)
		{
			for(auto& activeElement : mActiveElements)
			{
				if(mLastPointerScreenPos != event.screenPos)
				{
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);

					mMouseEvent.setMouseDragData(localPos, event.screenPos - mDragStartPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
				}
			}

			mLastPointerScreenPos = event.screenPos;

			// Also if drag is in progress send DragAndDrop events
			if(DragAndDropManager::instance().isDragInProgress())
			{
				bool acceptDrop = true;
				for(auto& elementInfo : mElementsUnderPointer)
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

					acceptDrop = true;
					if(DragAndDropManager::instance().needsValidDropTarget())
					{
						acceptDrop = elementInfo.element->_acceptDragAndDrop(localPos, DragAndDropManager::instance().getDragTypeId());
					}

					if(acceptDrop)
					{
						mMouseEvent.setDragAndDropDraggedData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
						if(sendMouseEvent(elementInfo.element, mMouseEvent))
						{
							event.markAsUsed();
							break;
						}
					}
				}

				if(acceptDrop)
				{
					if(mActiveCursor != CursorType::ArrowDrag)
					{
						Cursor::instance().setCursor(CursorType::ArrowDrag);
						mActiveCursor = CursorType::ArrowDrag;
					}
				}
				else
				{
					if(mActiveCursor != CursorType::Deny)
					{
						Cursor::instance().setCursor(CursorType::Deny);
						mActiveCursor = CursorType::Deny;
					}
				}				
			}
		}
		else // Otherwise, send MouseMove events if we are hovering over any element
		{
			if(mLastPointerScreenPos != event.screenPos)
			{
				bool moveProcessed = false;
				bool hasCustomCursor = false;
				for(auto& elementInfo : mElementsUnderPointer)
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

					if(!moveProcessed)
					{
						// Send MouseMove event
						mMouseEvent.setMouseMoveData(localPos);
						moveProcessed = sendMouseEvent(elementInfo.element, mMouseEvent);

						if(moveProcessed)
							event.markAsUsed();
					}

					if (mDragState == DragState::NoDrag)
					{
						CursorType newCursor = CursorType::Arrow;
						if(elementInfo.element->_hasCustomCursor(localPos, newCursor))
						{
							if(newCursor != mActiveCursor)
							{
								Cursor::instance().setCursor(newCursor);
								mActiveCursor = newCursor;
							}

							hasCustomCursor = true;
						}
					}

					if(moveProcessed)
						break;
				}

				// While dragging we don't want to modify the cursor
				if (mDragState == DragState::NoDrag)
				{
					if (!hasCustomCursor)
					{
						if (mActiveCursor != CursorType::Arrow)
						{
							Cursor::instance().setCursor(CursorType::Arrow);
							mActiveCursor = CursorType::Arrow;
						}
					}
				}
			}

			mLastPointerScreenPos = event.screenPos;

			if(Math::abs(event.mouseWheelScrollAmount) > 0.00001f)
			{
				for(auto& elementInfo : mElementsUnderPointer)
				{
					mMouseEvent.setMouseWheelScrollData(event.mouseWheelScrollAmount);
					if(sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						event.markAsUsed();
						break;
					}
				}
			}
		}
	}

	void GUIManager::onPointerReleased(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);

		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// Send MouseUp event only if we are over the active element (we don't want to accidentally trigger other elements).
		// And only activate when a button that originally caused the active state is released, otherwise ignore it.
		if(mActiveMouseButton == guiButton)
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				auto iterFind2 = std::find_if(mActiveElements.begin(), mActiveElements.end(), 
					[&](const ElementInfo& x) { return x.element == elementInfo.element; });

				if(iterFind2 != mActiveElements.end())
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);
					mMouseEvent.setMouseUpData(localPos, guiButton);

					if(sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						event.markAsUsed();
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
					Vector2I localPos = getWidgetRelativePos(activeElement.widget, event.screenPos);

					mMouseEvent.setMouseDragEndData(localPos);
					if(sendMouseEvent(activeElement.element, mMouseEvent))
						event.markAsUsed();
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
			Cursor::instance().setCursor(CursorType::Arrow);
			mActiveCursor = CursorType::Arrow;
		}
	}

	void GUIManager::onPointerPressed(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		// Determine elements that gained focus
		mNewElementsInFocus.clear();

		mCommandEvent = GUICommandEvent();
		mCommandEvent.setType(GUICommandEventType::FocusGained);

		for (auto& elementInfo : mElementsUnderPointer)
		{
			auto iterFind = std::find_if(begin(mElementsInFocus), end(mElementsInFocus),
				[=](const ElementFocusInfo& x) { return x.element == elementInfo.element; });

			if (iterFind == mElementsInFocus.end())
			{
				bool processed = sendCommandEvent(elementInfo.element, mCommandEvent);
				mNewElementsInFocus.push_back(ElementFocusInfo(elementInfo.element, elementInfo.widget, processed));

				if (processed)
					break;
			}
			else
			{
				mNewElementsInFocus.push_back(*iterFind);

				if (iterFind->usesFocus)
					break;
			}
		}

		// Determine elements that lost focus
		// Note: Focus loss must trigger before mouse press because things like input boxes often only confirm changes
		// made to them when focus is lost. So if the user is confirming some input via a press of the button focus loss
		// must trigger on the input box first to make sure its contents get saved.
		mCommandEvent.setType(GUICommandEventType::FocusLost);

		for (auto& elementInfo : mElementsInFocus)
		{
			auto iterFind = std::find_if(begin(mNewElementsInFocus), end(mNewElementsInFocus),
				[=](const ElementFocusInfo& x) { return x.element == elementInfo.element; });

			if (iterFind == mNewElementsInFocus.end())
			{
				sendCommandEvent(elementInfo.element, mCommandEvent);
			}
		}

		mElementsInFocus.swap(mNewElementsInFocus);

		// Send mouse press event
		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);
		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// We only check for mouse down if mouse isn't already being held down, and we are hovering over an element
		if(mActiveElements.size() == 0)
		{
			mNewActiveElements.clear();
			for(auto& elementInfo : mElementsUnderPointer)
			{
				Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);
				mMouseEvent.setMouseDownData(localPos, guiButton);

				bool processed = sendMouseEvent(elementInfo.element, mMouseEvent);

				if(guiButton == GUIMouseButton::Left)
				{
					mDragState = DragState::HeldWithoutDrag;
					mLastPointerClickPos = event.screenPos;
				}

				mNewActiveElements.push_back(ElementInfo(elementInfo.element, elementInfo.widget));
				mActiveMouseButton = guiButton;

				if(processed)
				{
					event.markAsUsed();
					break;
				}
			}

			mActiveElements.swap(mNewActiveElements);
		}

		// If right click try to open context menu
		if(buttonStates[2] == true) 
		{
			for(auto& elementInfo : mElementsUnderPointer)
			{
				SPtr<GUIContextMenu> menu = elementInfo.element->_getContextMenu();

				if(menu != nullptr && elementInfo.widget != nullptr)
				{
					const RenderWindow* window = getWidgetWindow(*elementInfo.widget);
					if (window != nullptr)
					{
						Vector2I windowPos = window->screenToWindowPos(event.screenPos);

						menu->open(windowPos, *elementInfo.widget);
						event.markAsUsed();
						break;
					}
				}
			}
		}
	}

	void GUIManager::onPointerDoubleClick(const PointerEvent& event)
	{
		if(event.isUsed())
			return;

		bool buttonStates[(int)GUIMouseButton::Count];
		buttonStates[0] = event.buttonStates[0];
		buttonStates[1] = event.buttonStates[1];
		buttonStates[2] = event.buttonStates[2];

		if(findElementUnderPointer(event.screenPos, buttonStates, event.shift, event.control, event.alt))
			event.markAsUsed();

		mMouseEvent = GUIMouseEvent(buttonStates, event.shift, event.control, event.alt);

		GUIMouseButton guiButton = buttonToGUIButton(event.button);

		// We only check for mouse down if we are hovering over an element
		for(auto& elementInfo : mElementsUnderPointer)
		{
			Vector2I localPos = getWidgetRelativePos(elementInfo.widget, event.screenPos);

			mMouseEvent.setMouseDoubleClickData(localPos, guiButton);
			if(sendMouseEvent(elementInfo.element, mMouseEvent))
			{
				event.markAsUsed();
				break;
			}
		}
	}

	void GUIManager::onInputCommandEntered(InputCommandType commandType)
	{
		if(mElementsInFocus.size() == 0)
			return;

		hideTooltip();
		mCommandEvent = GUICommandEvent();

		switch(commandType)
		{
		case InputCommandType::Backspace:
			mCommandEvent.setType(GUICommandEventType::Backspace);
			break;
		case InputCommandType::Delete:
			mCommandEvent.setType(GUICommandEventType::Delete);
			break;
		case InputCommandType::Return:
			mCommandEvent.setType(GUICommandEventType::Return);
			break;
		case InputCommandType::Confirm:
			mCommandEvent.setType(GUICommandEventType::Confirm);
			break;
		case InputCommandType::Escape:
			mCommandEvent.setType(GUICommandEventType::Escape);
			break;
		case InputCommandType::CursorMoveLeft:
			mCommandEvent.setType(GUICommandEventType::MoveLeft);
			break;
		case InputCommandType::CursorMoveRight:
			mCommandEvent.setType(GUICommandEventType::MoveRight);
			break;
		case InputCommandType::CursorMoveUp:
			mCommandEvent.setType(GUICommandEventType::MoveUp);
			break;
		case InputCommandType::CursorMoveDown:
			mCommandEvent.setType(GUICommandEventType::MoveDown);
			break;
		case InputCommandType::SelectLeft:
			mCommandEvent.setType(GUICommandEventType::SelectLeft);
			break;
		case InputCommandType::SelectRight:
			mCommandEvent.setType(GUICommandEventType::SelectRight);
			break;
		case InputCommandType::SelectUp:
			mCommandEvent.setType(GUICommandEventType::SelectUp);
			break;
		case InputCommandType::SelectDown:
			mCommandEvent.setType(GUICommandEventType::SelectDown);
			break;
		}

		for(auto& elementInfo : mElementsInFocus)
		{
			sendCommandEvent(elementInfo.element, mCommandEvent);
		}		
	}

	void GUIManager::onVirtualButtonDown(const VirtualButton& button, UINT32 deviceIdx)
	{
		hideTooltip();
		mVirtualButtonEvent.setButton(button);
		
		for(auto& elementInFocus : mElementsInFocus)
		{
			bool processed = sendVirtualButtonEvent(elementInFocus.element, mVirtualButtonEvent);

			if(processed)
				break;
		}
	}

	bool GUIManager::findElementUnderPointer(const Vector2I& pointerScreenPos, bool buttonStates[3], bool shift, bool control, bool alt)
	{
		Vector<const RenderWindow*> widgetWindows;
		for(auto& widgetInfo : mWidgets)
			widgetWindows.push_back(getWidgetWindow(*widgetInfo.widget));

#if BS_DEBUG_MODE
		// Checks if all referenced windows actually exist
		Vector<RenderWindow*> activeWindows = RenderWindowManager::instance().getRenderWindows();
		for(auto& window : widgetWindows)
		{
			if(window == nullptr)
				continue;

			auto iterFind = std::find(begin(activeWindows), end(activeWindows), window);

			if(iterFind == activeWindows.end())
			{
				BS_EXCEPT(InternalErrorException, "GUI manager has a reference to a window that doesn't exist. \
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

		RenderWindow* topMostModal = RenderWindowManager::instance().getTopMostModal();
		for(auto& window : uniqueWindows)
		{
			if(Platform::isPointOverWindow(*window, pointerScreenPos))
			{
				// If there's a top most modal window, it needs to be this one, otherwise we ignore input to that window
				if(topMostModal == nullptr || window == topMostModal)
					windowUnderPointer = window;

				break;
			}
		}

		if(windowUnderPointer != nullptr)
		{
			Vector2I windowPos = windowUnderPointer->screenToWindowPos(pointerScreenPos);

			UINT32 widgetIdx = 0;
			for(auto& widgetInfo : mWidgets)
			{
				if(widgetWindows[widgetIdx] == nullptr)
				{
					widgetIdx++;
					continue;
				}

				GUIWidget* widget = widgetInfo.widget;
				if(widgetWindows[widgetIdx] == windowUnderPointer 
					&& widget->inBounds(windowToBridgedCoords(widget->getTarget()->getTarget(), windowPos)))
				{
					const Vector<GUIElement*>& elements = widget->getElements();
					Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

					// Elements with lowest depth (most to the front) get handled first
					for(auto iter = elements.begin(); iter != elements.end(); ++iter)
					{
						GUIElement* element = *iter;

						if(element->_isVisible() && element->_isInBounds(localPos))
						{
							ElementInfoUnderPointer elementInfo(element, widget);

							auto iterFind = std::find_if(mElementsUnderPointer.begin(), mElementsUnderPointer.end(),
								[=](const ElementInfoUnderPointer& x) { return x.element == element; });

							if (iterFind != mElementsUnderPointer.end())
							{
								elementInfo.usesMouseOver = iterFind->usesMouseOver;
								elementInfo.receivedMouseOver = iterFind->receivedMouseOver;
							}

							mNewElementsUnderPointer.push_back(elementInfo);
						}
					}
				}

				widgetIdx++;
			}
		}

		std::sort(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(), 
			[](const ElementInfoUnderPointer& a, const ElementInfoUnderPointer& b)
		{
			return a.element->_getDepth() < b.element->_getDepth();
		});

		// Send MouseOut and MouseOver events
		bool eventProcessed = false;

		for (auto& elementInfo : mNewElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			GUIWidget* widget = elementInfo.widget;

			if (elementInfo.receivedMouseOver)
			{
				elementInfo.isHovering = true;
				if (elementInfo.usesMouseOver)
					break;

				continue;
			}

			auto iterFind = std::find_if(mActiveElements.begin(), mActiveElements.end(),
				[&](const ElementInfo& x) { return x.element == element; });

			// Send MouseOver event
			if (mActiveElements.size() == 0 || iterFind != mActiveElements.end())
			{
				Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

				mMouseEvent = GUIMouseEvent(buttonStates, shift, control, alt);

				mMouseEvent.setMouseOverData(localPos);
				elementInfo.receivedMouseOver = true;
				elementInfo.isHovering = true;
				if (sendMouseEvent(element, mMouseEvent))
				{
					eventProcessed = true;
					elementInfo.usesMouseOver = true;
					break;
				}
			}
		}

		// Send DragAndDropLeft event - It is similar to MouseOut events but we send it to all
		// elements a user might hover over, while we send mouse over/out events only to active elements while dragging
		if (DragAndDropManager::instance().isDragInProgress())
		{
			for (auto& elementInfo : mElementsUnderPointer)
			{
				auto iterFind = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(),
					[=](const ElementInfoUnderPointer& x) { return x.element == elementInfo.element; });

				if (iterFind == mNewElementsUnderPointer.end())
				{
					Vector2I localPos = getWidgetRelativePos(elementInfo.widget, pointerScreenPos);

					mMouseEvent.setDragAndDropLeftData(localPos, DragAndDropManager::instance().getDragTypeId(), DragAndDropManager::instance().getDragData());
					if (sendMouseEvent(elementInfo.element, mMouseEvent))
					{
						eventProcessed = true;
						break;
					}
				}
			}
		}

		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			GUIWidget* widget = elementInfo.widget;

			auto iterFind = std::find_if(mNewElementsUnderPointer.begin(), mNewElementsUnderPointer.end(),
				[=](const ElementInfoUnderPointer& x) { return x.element == element; });

			if (!elementInfo.receivedMouseOver)
				continue;

			if (iterFind == mNewElementsUnderPointer.end() || !iterFind->isHovering)
			{
				auto iterFind2 = std::find_if(mActiveElements.begin(), mActiveElements.end(), 
					[=](const ElementInfo& x) { return x.element == element; });

				// Send MouseOut event
				if(mActiveElements.size() == 0 || iterFind2 != mActiveElements.end())
				{
					Vector2I localPos = getWidgetRelativePos(widget, pointerScreenPos);

					mMouseEvent.setMouseOutData(localPos);
					if (sendMouseEvent(element, mMouseEvent))
					{
						eventProcessed = true;
						break;
					}
				}
			}
		}

		mElementsUnderPointer.swap(mNewElementsUnderPointer);

		// Tooltip
		hideTooltip();
		if (mElementsUnderPointer.size() > 0)
			mShowTooltip = true;

		mTooltipElementHoverStart = gTime().getTime();

		return eventProcessed;
	}

	void GUIManager::onTextInput(const TextInputEvent& event)
	{
		mTextInputEvent = GUITextInputEvent();
		mTextInputEvent.setData(event.textChar);

		for(auto& elementInFocus : mElementsInFocus)
		{
			if(sendTextInputEvent(elementInFocus.element, mTextInputEvent))
				event.markAsUsed();
		}
	}

	void GUIManager::onWindowFocusGained(RenderWindow& win)
	{
		for(auto& widgetInfo : mWidgets)
		{
			GUIWidget* widget = widgetInfo.widget;
			if(getWidgetWindow(*widget) == &win)
				widget->ownerWindowFocusChanged();
		}
	}

	void GUIManager::onWindowFocusLost(RenderWindow& win)
	{
		for(auto& widgetInfo : mWidgets)
		{
			GUIWidget* widget = widgetInfo.widget;
			if(getWidgetWindow(*widget) == &win)
				widget->ownerWindowFocusChanged();
		}

		mNewElementsInFocus.clear();
		for(auto& focusedElement : mElementsInFocus)
		{
			if (focusedElement.element->_isDestroyed())
				continue;

			if (focusedElement.widget != nullptr && getWidgetWindow(*focusedElement.widget) == &win)
			{
				mCommandEvent = GUICommandEvent();
				mCommandEvent.setType(GUICommandEventType::FocusLost);

				sendCommandEvent(focusedElement.element, mCommandEvent);
			}
			else
				mNewElementsInFocus.push_back(focusedElement);
		}

		mElementsInFocus.swap(mNewElementsInFocus);
	}

	// We stop getting mouse move events once it leaves the window, so make sure
	// nothing stays in hover state
	void GUIManager::onMouseLeftWindow(RenderWindow& win)
	{
		mNewElementsUnderPointer.clear();

		for(auto& elementInfo : mElementsUnderPointer)
		{
			GUIElement* element = elementInfo.element;
			GUIWidget* widget = elementInfo.widget;

			if (widget != nullptr && widget->getTarget()->getTarget().get() != &win)
			{
				mNewElementsUnderPointer.push_back(elementInfo);
				continue;
			}

			auto iterFind = std::find_if(mActiveElements.begin(), mActiveElements.end(), 
				[&](const ElementInfo& x) { return x.element == element; });

			// Send MouseOut event
			if(mActiveElements.size() == 0 || iterFind != mActiveElements.end())
			{
				Vector2I localPos = getWidgetRelativePos(widget, Vector2I());

				mMouseEvent.setMouseOutData(localPos);
				sendMouseEvent(element, mMouseEvent);
			}
		}

		mElementsUnderPointer.swap(mNewElementsUnderPointer);

		hideTooltip();
		if(mDragState != DragState::Dragging)
		{
			if(mActiveCursor != CursorType::Arrow)
			{
				Cursor::instance().setCursor(CursorType::Arrow);
				mActiveCursor = CursorType::Arrow;
			}
		}
	}
	
	void GUIManager::hideTooltip()
	{
		GUITooltipManager::instance().hide();
		mShowTooltip = false;
	}

	void GUIManager::queueForDestroy(GUIElement* element)
	{
		mScheduledForDestruction.push(element);
	}

	void GUIManager::setFocus(GUIElement* element, bool focus)
	{
		ElementForcedFocusInfo efi;
		efi.element = element;
		efi.focus = focus;

		mForcedFocusElements.push_back(efi);
	}

	bool GUIManager::processDestroyQueueIteration()
	{
		Stack<GUIElement*> toDestroy = mScheduledForDestruction;
		mScheduledForDestruction = Stack<GUIElement*>();

		while (!toDestroy.empty())
		{
			bs_delete(toDestroy.top());
			toDestroy.pop();
		}

		return !mScheduledForDestruction.empty();
	}

	void GUIManager::setInputBridge(const RenderTexture* renderTex, const GUIElement* element)
	{
		if(element == nullptr)
			mInputBridge.erase(renderTex);
		else
			mInputBridge[renderTex] = element;
	}

	GUIMouseButton GUIManager::buttonToGUIButton(PointerEventButton pointerButton) const
	{
		if(pointerButton == PointerEventButton::Left)
			return GUIMouseButton::Left;
		else if(pointerButton == PointerEventButton::Middle)
			return GUIMouseButton::Middle;
		else if(pointerButton == PointerEventButton::Right)
			return GUIMouseButton::Right;

		BS_EXCEPT(InvalidParametersException, "Provided button is not a GUI supported mouse button.");
		return GUIMouseButton::Left;
	}

	Vector2I GUIManager::getWidgetRelativePos(const GUIWidget* widget, const Vector2I& screenPos) const
	{
		if (widget == nullptr)
			return screenPos;

		const RenderWindow* window = getWidgetWindow(*widget);
		if(window == nullptr)
			return Vector2I();

		Vector2I windowPos = window->screenToWindowPos(screenPos);
		windowPos = windowToBridgedCoords(widget->getTarget()->getTarget(), windowPos);

		const Matrix4& worldTfrm = widget->getWorldTfrm();

		Vector4 vecLocalPos = worldTfrm.inverse().multiplyAffine(Vector4((float)windowPos.x, (float)windowPos.y, 0.0f, 1.0f));
		Vector2I curLocalPos(Math::roundToInt(vecLocalPos.x), Math::roundToInt(vecLocalPos.y));

		return curLocalPos;
	}

	Vector2I GUIManager::windowToBridgedCoords(const SPtr<RenderTarget>& target, const Vector2I& windowPos) const
	{
		// This cast might not be valid (the render target could be a window), but we only really need to cast
		// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
		// (in which case we know it is a RenderTexture)
		const RenderTexture* renderTexture = static_cast<const RenderTexture*>(target.get());
		const RenderTargetProperties& rtProps = renderTexture->getProperties();

		auto iterFind = mInputBridge.find(renderTexture);
		if(iterFind != mInputBridge.end()) // Widget input is bridged, which means we need to transform the coordinates
		{
			const GUIElement* bridgeElement = iterFind->second;
			const GUIWidget* parentWidget = bridgeElement->_getParentWidget();
			if (parentWidget == nullptr)
				return windowPos;

			const Matrix4& worldTfrm = parentWidget->getWorldTfrm();

			Vector4 vecLocalPos = worldTfrm.inverse().multiplyAffine(Vector4((float)windowPos.x, (float)windowPos.y, 0.0f, 1.0f));
			Rect2I bridgeBounds = bridgeElement->_getLayoutData().area;

			// Find coordinates relative to the bridge element
			float x = vecLocalPos.x - (float)bridgeBounds.x;
			float y = vecLocalPos.y - (float)bridgeBounds.y;

			float scaleX = rtProps.width / (float)bridgeBounds.width;
			float scaleY = rtProps.height / (float)bridgeBounds.height;

			return Vector2I(Math::roundToInt(x * scaleX), Math::roundToInt(y * scaleY));
		}

		return windowPos;
	}

	const RenderWindow* GUIManager::getWidgetWindow(const GUIWidget& widget) const
	{
		// This cast might not be valid (the render target could be a window), but we only really need to cast
		// so that mInputBridge map allows us to search through it - we don't access anything unless the target is bridged
		// (in which case we know it is a RenderTexture)

		const Viewport* viewport = widget.getTarget();
		if (viewport == nullptr)
			return nullptr;

		SPtr<RenderTarget> target = viewport->getTarget();
		if (target == nullptr)
			return nullptr;

		const RenderTexture* renderTexture = static_cast<const RenderTexture*>(target.get());

		auto iterFind = mInputBridge.find(renderTexture);
		if(iterFind != mInputBridge.end())
		{
			GUIWidget* parentWidget = iterFind->second->_getParentWidget();
			if (parentWidget == nullptr)
				return nullptr;

			if(parentWidget != &widget)
				return getWidgetWindow(*parentWidget);
		}

		Vector<RenderWindow*> renderWindows = RenderWindowManager::instance().getRenderWindows();

		auto iterFindWin = std::find(renderWindows.begin(), renderWindows.end(), target.get());
		if(iterFindWin != renderWindows.end())
			return static_cast<RenderWindow*>(target.get());

		return nullptr;
	}

	SPtr<RenderWindow> GUIManager::getBridgeWindow(const SPtr<RenderTexture>& target) const
	{
		if (target == nullptr)
			return nullptr;

		while (true)
		{
			auto iterFind = mInputBridge.find(target.get());
			if (iterFind == mInputBridge.end())
				return nullptr;

			GUIWidget* parentWidget = iterFind->second->_getParentWidget();
			if (parentWidget == nullptr)
				return nullptr;

			SPtr<RenderTarget> curTarget = parentWidget->getTarget()->getTarget();
			if (curTarget == nullptr)
				return nullptr;

			if (curTarget == target)
				return nullptr;

			if (curTarget->getProperties().isWindow)
				return std::static_pointer_cast<RenderWindow>(curTarget);
		}

		return nullptr;
	}

	bool GUIManager::sendMouseEvent(GUIElement* element, const GUIMouseEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_mouseEvent(event);
	}

	bool GUIManager::sendTextInputEvent(GUIElement* element, const GUITextInputEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_textInputEvent(event);
	}

	bool GUIManager::sendCommandEvent(GUIElement* element, const GUICommandEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_commandEvent(event);
	}

	bool GUIManager::sendVirtualButtonEvent(GUIElement* element, const GUIVirtualButtonEvent& event)
	{
		if (element->_isDestroyed())
			return false;

		return element->_virtualButtonEvent(event);
	}

	GUIManager& gGUIManager()
	{
		return GUIManager::instance();
	}

	namespace ct
	{
	GUISpriteParamBlockDef gGUISpriteParamBlockDef;

	GUIRenderer::GUIRenderer()
		:RendererExtension(RenderLocation::Overlay, 10)
	{ }

	void GUIRenderer::initialize(const Any& data)
	{
		SAMPLER_STATE_DESC ssDesc;
		ssDesc.magFilter = FO_POINT;
		ssDesc.minFilter = FO_POINT;
		ssDesc.mipFilter = FO_POINT;

		mSamplerState = RenderStateManager::instance().createSamplerState(ssDesc);
	}

	bool GUIRenderer::check(const Camera& camera)
	{
		auto iterFind = mPerCameraData.find(&camera);
		return iterFind != mPerCameraData.end();
	}

	void GUIRenderer::render(const Camera& camera)
	{
		Vector<GUIManager::GUICoreRenderData>& renderData = mPerCameraData[&camera];

		float invViewportWidth = 1.0f / (camera.getViewport()->getPixelArea().width * 0.5f);
		float invViewportHeight = 1.0f / (camera.getViewport()->getPixelArea().height * 0.5f);
		float viewflipYFlip = bs::RenderAPI::getAPIInfo().isFlagSet(RenderAPIFeatureFlag::NDCYAxisDown) ? -1.0f : 1.0f;

		for (auto& entry : renderData)
		{
			SPtr<GpuParamBlockBuffer> buffer = mParamBlocks[entry.bufferIdx];

			gGUISpriteParamBlockDef.gInvViewportWidth.set(buffer, invViewportWidth);
			gGUISpriteParamBlockDef.gInvViewportHeight.set(buffer, invViewportHeight);
			gGUISpriteParamBlockDef.gViewportYFlip.set(buffer, viewflipYFlip);

			buffer->flushToGPU();
		}

		for (auto& entry : renderData)
		{
			// TODO - I shouldn't be re-applying the entire material for each entry, instead just check which programs
			// changed, and apply only those + the modified constant buffers and/or texture.

			SPtr<GpuParamBlockBuffer> buffer = mParamBlocks[entry.bufferIdx];

			entry.material->render(entry.mesh, entry.subMesh, entry.texture, mSamplerState, buffer, entry.additionalData);
		}
	}

	void GUIRenderer::updateData(const UnorderedMap<SPtr<Camera>, Vector<GUIManager::GUICoreRenderData>>& newPerCameraData)
	{
		bs_frame_mark();

		{
			mPerCameraData.clear();
			mReferencedCameras.clear();

			for (auto& newCameraData : newPerCameraData)
			{
				SPtr<Camera> camera = newCameraData.first;

				mPerCameraData.insert(std::make_pair(camera.get(), newCameraData.second));
				mReferencedCameras.insert(camera);
			}

			// Allocate GPU buffers containing the material parameters
			UINT32 numBuffers = 0;
			for (auto& cameraData : mPerCameraData)
				numBuffers += (UINT32)cameraData.second.size();

			UINT32 numAllocatedBuffers = (UINT32)mParamBlocks.size();
			if (numBuffers > numAllocatedBuffers)
			{
				mParamBlocks.resize(numBuffers);

				for (UINT32 i = numAllocatedBuffers; i < numBuffers; i++)
					mParamBlocks[i] = gGUISpriteParamBlockDef.createBuffer();
			}

			UINT32 curBufferIdx = 0;
			for (auto& cameraData : mPerCameraData)
			{
				for(auto& entry : cameraData.second)
				{
					SPtr<GpuParamBlockBuffer> buffer = mParamBlocks[curBufferIdx];

					gGUISpriteParamBlockDef.gTint.set(buffer, entry.tint);
					gGUISpriteParamBlockDef.gWorldTransform.set(buffer, entry.worldTransform);

					entry.bufferIdx = curBufferIdx;
					curBufferIdx++;
				}
			}
		}

		bs_frame_clear();
	}
	}
}