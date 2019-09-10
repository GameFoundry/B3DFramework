//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISliderHandle.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIMouseEvent.h"

namespace bs
{
	const UINT32 GUISliderHandle::RESIZE_HANDLE_SIZE = 7;

	const String& GUISliderHandle::getGUITypeName()
	{
		static String name = "SliderHandle";
		return name;
	}

	GUISliderHandle::GUISliderHandle(GUISliderHandleFlags flags, const String& styleName, const GUIDimensions& dimensions)
		: GUIElement(styleName, dimensions), mFlags(flags)
	{
		mImageSprite = bs_new<ImageSprite>();

		// Calling virtual method is okay in this case
		styleUpdated();
	}

	GUISliderHandle::~GUISliderHandle()
	{
		bs_delete(mImageSprite);
	}

	GUISliderHandle* GUISliderHandle::create(GUISliderHandleFlags flags, const String& styleName)
	{
		return new (bs_alloc<GUISliderHandle>()) GUISliderHandle(flags,
			getStyleName<GUISliderHandle>(styleName), GUIDimensions::create());
	}

	GUISliderHandle* GUISliderHandle::create(GUISliderHandleFlags flags, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUISliderHandle>()) GUISliderHandle(flags,
			getStyleName<GUISliderHandle>(styleName), GUIDimensions::create(options));
	}

	void GUISliderHandle::_setHandleSize(float pct)
	{
		mPctHandleSize = Math::clamp01(pct);
	}

	void GUISliderHandle::_setHandlePos(float pct)
	{
		float maxPct = 1.0f;
		if (mStep > 0.0f && pct < maxPct)
		{
			pct = (pct + mStep * 0.5f) - fmod(pct + mStep * 0.5f, mStep);
			maxPct = Math::floor(1.0f / mStep) * mStep;
		}

		mPctHandlePos = Math::clamp(pct, 0.0f, maxPct);
	}

	float GUISliderHandle::getHandlePos() const
	{
		return mPctHandlePos;;
	}

	float GUISliderHandle::getStep() const
	{
		return mStep;
	}

	void GUISliderHandle::setStep(float step)
	{
		mStep = Math::clamp01(step);
	}

	UINT32 GUISliderHandle::getScrollableSize() const
	{
		return getMaxSize() - getHandleSize();
	}

	void GUISliderHandle::updateRenderElementsInternal()
	{		
		IMAGE_SPRITE_DESC desc;

		HSpriteTexture activeTex = getActiveTexture();
		if(SpriteTexture::checkIsLoaded(activeTex))
			desc.texture = activeTex;

		UINT32 handleSize = getHandleSize();
		if (mFlags.isSet(GUISliderHandleFlag::Horizontal))
		{
			if (handleSize == 0 && desc.texture != nullptr)
			{
				handleSize = desc.texture->getWidth();
				mPctHandleSize = handleSize / (float)getMaxSize();
			}

			desc.width = handleSize;
			desc.height = mLayoutData.area.height;
		}
		else
		{
			if (handleSize == 0 && desc.texture != nullptr)
			{
				handleSize = desc.texture->getHeight();
				mPctHandleSize = handleSize / (float)getMaxSize();
			}

			desc.width = mLayoutData.area.width;
			desc.height = handleSize;
		}

		desc.borderLeft = _getStyle()->border.left;
		desc.borderRight = _getStyle()->border.right;
		desc.borderTop = _getStyle()->border.top;
		desc.borderBottom = _getStyle()->border.bottom;
		desc.color = getTint();
		mImageSprite->update(desc, (UINT64)_getParentWidget());
		
		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::populate({ T::SpriteInfo(mImageSprite) }, mRenderElements);
		}

		GUIElement::updateRenderElementsInternal();
	}

	void GUISliderHandle::updateClippedBounds()
	{
		mClippedBounds = mLayoutData.area;
		mClippedBounds.clip(mLayoutData.clipRect);
	}

	Vector2I GUISliderHandle::_getOptimalSize() const
	{
		HSpriteTexture activeTex = getActiveTexture();

		if(SpriteTexture::checkIsLoaded(activeTex))
			return Vector2I(activeTex->getWidth(), activeTex->getHeight());

		return Vector2I();
	}

	void GUISliderHandle::_fillBuffer(
		UINT8* vertices,
		UINT32* indices,
		UINT32 vertexOffset,
		UINT32 indexOffset,
		const Vector2I& offset,
		UINT32 maxNumVerts,
		UINT32 maxNumIndices,
		UINT32 renderElementIdx) const
	{
		UINT8* uvs = vertices + sizeof(Vector2);
		UINT32 vertexStride = sizeof(Vector2) * 2;
		UINT32 indexStride = sizeof(UINT32);

		Vector2I layoutOffset = Vector2I(mLayoutData.area.x, mLayoutData.area.y) + offset;
		Rect2I clipRect = mLayoutData.getLocalClipRect();

		if (mFlags.isSet(GUISliderHandleFlag::Horizontal))
		{
			layoutOffset.x += getHandlePosPx();
			clipRect.x -= getHandlePosPx();
		}
		else
		{
			layoutOffset.y += getHandlePosPx();
			clipRect.y -= getHandlePosPx();
		}

		mImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
			vertexStride, indexStride, renderElementIdx, layoutOffset, clipRect);
	}

	bool GUISliderHandle::_mouseEvent(const GUIMouseEvent& ev)
	{
		UINT32 handleSize = getHandleSize();

		if(ev.getType() == GUIMouseEventType::MouseMove)
		{
			if (!_isDisabled())
			{
				if (mMouseOverHandle)
				{
					if (!isOnHandle(ev.getPosition()))
					{
						mMouseOverHandle = false;

						mState = State::Normal;
						_markLayoutAsDirty();

						return true;
					}
				}
				else
				{
					if (isOnHandle(ev.getPosition()))
					{
						mMouseOverHandle = true;

						mState = State::Hover;
						_markLayoutAsDirty();

						return true;
					}
				}
			}
		}

		bool jumpOnClick = mFlags.isSet(GUISliderHandleFlag::JumpOnClick);
		if(ev.getType() == GUIMouseEventType::MouseDown && (mMouseOverHandle || jumpOnClick))
		{
			if (!_isDisabled())
			{
				mState = State::Active;
				_markLayoutAsDirty();

				if (jumpOnClick)
				{
					float handlePosPx = 0.0f;

					if (mFlags.isSet(GUISliderHandleFlag::Horizontal))
						handlePosPx = (float)(ev.getPosition().x - (INT32)mLayoutData.area.x - handleSize * 0.5f);
					else
						handlePosPx = (float)(ev.getPosition().y - (INT32)mLayoutData.area.y - handleSize * 0.5f);

					setHandlePosPx((INT32)handlePosPx);
					onHandleMovedOrResized(mPctHandlePos, _getHandleSizePct());
				}

				bool isResizeable = mFlags.isSet(GUISliderHandleFlag::Resizeable);
				if (mFlags.isSet(GUISliderHandleFlag::Horizontal))
				{
					INT32 left = (INT32)mLayoutData.area.x + getHandlePosPx();

					if(isResizeable)
					{
						INT32 right = left + handleSize;

						INT32 clickPos = ev.getPosition().x;
						if (clickPos >= left && clickPos < (left + (INT32)RESIZE_HANDLE_SIZE))
							mDragState = DragState::LeftResize;
						else if (clickPos >= (right - (INT32)RESIZE_HANDLE_SIZE) && clickPos < right)
							mDragState = DragState::RightResize;
						else
							mDragState = DragState::Normal;
					}
					else
						mDragState = DragState::Normal;

					mDragStartPos = ev.getPosition().x - left;
				}
				else
				{
					INT32 top = (INT32)mLayoutData.area.y + getHandlePosPx();

					if(isResizeable)
					{
						INT32 bottom = top + handleSize;

						INT32 clickPos = ev.getPosition().y;
						if (clickPos >= top && clickPos < (top + (INT32)RESIZE_HANDLE_SIZE))
							mDragState = DragState::LeftResize;
						else if (clickPos >= (bottom - (INT32)RESIZE_HANDLE_SIZE) && clickPos < bottom)
							mDragState = DragState::RightResize;
						else
							mDragState = DragState::Normal;
					}
					else
						mDragState = DragState::Normal;

					mDragStartPos = ev.getPosition().y - top;
				}
				
				mHandleDragged = true;
			}

			return true;
		}

		if(ev.getType() == GUIMouseEventType::MouseDrag && mHandleDragged)
		{
			if (!_isDisabled())
			{
				if (mDragState == DragState::Normal)
				{
					INT32 handlePosPx;
					if (mFlags.isSet(GUISliderHandleFlag::Horizontal))
						handlePosPx = ev.getPosition().x - mDragStartPos - mLayoutData.area.x;
					else
						handlePosPx = ev.getPosition().y - mDragStartPos - mLayoutData.area.y;

					setHandlePosPx(handlePosPx);
					onHandleMovedOrResized(mPctHandlePos, _getHandleSizePct());
				}
				else // Resizing
				{
					INT32 clickPosPx;
					if (mFlags.isSet(GUISliderHandleFlag::Horizontal))
						clickPosPx = ev.getPosition().x - mLayoutData.area.x;
					else
						clickPosPx = ev.getPosition().y - mLayoutData.area.y;

					INT32 left = getHandlePosPx();
					UINT32 maxSize = getMaxSize();

					INT32 newHandleSize;
					float newHandlePos;
					if(mDragState == DragState::LeftResize)
					{
						INT32 newLeft = clickPosPx - mDragStartPos;
						INT32 right = left + handleSize;
						newLeft = Math::clamp(newLeft, 0, right);

						newHandleSize = std::max((INT32)mMinHandleSize, right - newLeft);
						newLeft = right - newHandleSize;

						float scrollableSize = (float)(maxSize - newHandleSize);
						if (scrollableSize > 0.0f)
							newHandlePos = newLeft / scrollableSize;
						else
							newHandlePos = 0.0f;
					}
					else // Right resize
					{
						INT32 newRight = clickPosPx;
						newHandleSize = std::max((INT32)mMinHandleSize, std::min(newRight, (INT32)maxSize) - left);

						float scrollableSize = (float)(maxSize - newHandleSize);
						if (scrollableSize > 0.0f)
							newHandlePos = left / scrollableSize;
						else
							newHandlePos = 0.0f;
					}

					_setHandleSize(newHandleSize / (float)maxSize);
					_setHandlePos(newHandlePos);

					onHandleMovedOrResized(mPctHandlePos, _getHandleSizePct());
				}

				_markLayoutAsDirty();
			}

			return true;
		}

		if(ev.getType() == GUIMouseEventType::MouseOut)
		{
			if (!_isDisabled())
			{
				mMouseOverHandle = false;

				if (!mHandleDragged)
				{
					mState = State::Normal;
					_markLayoutAsDirty();
				}
			}
			
			return true;
		}

		if(ev.getType() == GUIMouseEventType::MouseUp)
		{
			if (!_isDisabled())
			{
				if (mMouseOverHandle)
					mState = State::Hover;
				else
					mState = State::Normal;

				if (!mHandleDragged)
				{
					// If we clicked above or below the scroll handle, scroll by one page
					INT32 handlePosPx = getHandlePosPx();
					if (!mFlags.isSet(GUISliderHandleFlag::JumpOnClick))
					{
						if (mFlags.isSet(GUISliderHandleFlag::Horizontal))
						{
							INT32 handleLeft = (INT32)mLayoutData.area.x + handlePosPx;
							INT32 handleRight = handleLeft + handleSize;

							if (ev.getPosition().x < handleLeft)
								moveOneStep(false);
							else if (ev.getPosition().x > handleRight)
								moveOneStep(true);
						}
						else
						{
							INT32 handleTop = (INT32)mLayoutData.area.y + handlePosPx;
							INT32 handleBottom = handleTop + handleSize;

							if (ev.getPosition().y < handleTop)
								moveOneStep(false);
							else if (ev.getPosition().y > handleBottom)
								moveOneStep(true);
						}
					}
				}
				mHandleDragged = false;
				_markLayoutAsDirty();
			}

			return true;
		}

		if(ev.getType() == GUIMouseEventType::MouseDragEnd)
		{
			if (!_isDisabled())
			{
				mHandleDragged = false;
				if (mMouseOverHandle)
					mState = State::Hover;
				else
					mState = State::Normal;

				_markLayoutAsDirty();
			}

			return true;
		}
		
		return false;
	}

	void GUISliderHandle::moveOneStep(bool forward)
	{
		const UINT32 handleSize = getHandleSize();
		INT32 handlePosPx = getHandlePosPx();

		INT32 stepSizePx;
		if (mStep > 0.0f)
			stepSizePx = (INT32)(mStep * getMaxSize());
		else
			stepSizePx = (INT32)handleSize;

		handlePosPx += forward ? stepSizePx : -stepSizePx;

		setHandlePosPx(handlePosPx);
		onHandleMovedOrResized(mPctHandlePos, _getHandleSizePct());

		_markLayoutAsDirty();
	}

	bool GUISliderHandle::isOnHandle(const Vector2I& pos) const
	{
		UINT32 handleSize = getHandleSize();
		if(mFlags.isSet(GUISliderHandleFlag::Horizontal))
		{
			INT32 left = (INT32)mLayoutData.area.x + getHandlePosPx();
			INT32 right = left + handleSize;

			if(pos.x >= left && pos.x < right)
				return true;
		}
		else
		{
			INT32 top = (INT32)mLayoutData.area.y + getHandlePosPx();
			INT32 bottom = top + handleSize;

			if(pos.y >= top && pos.y < bottom)
				return true;
		}
		
		return false;
	}

	INT32 GUISliderHandle::getHandlePosPx() const
	{
		INT32 maxScrollAmount = std::max(0, (INT32)getMaxSize() - (INT32)getHandleSize());
		return Math::floorToInt(mPctHandlePos * maxScrollAmount);
	}

	UINT32 GUISliderHandle::getHandleSize() const
	{
		return std::max(mMinHandleSize, (UINT32)(getMaxSize() * mPctHandleSize));
	}

	float GUISliderHandle::_getHandleSizePct() const
	{
		return mPctHandleSize;
	}

	void GUISliderHandle::styleUpdated()
	{
		const GUIElementStyle* style = _getStyle();
		if (style != nullptr)
		{
			if (mFlags.isSet(GUISliderHandleFlag::Horizontal))
				mMinHandleSize = style->fixedWidth ? style->width : style->minWidth;
			else
				mMinHandleSize = style->fixedHeight ? style->height : style->minHeight;
		}
	}

	void GUISliderHandle::setHandlePosPx(INT32 pos)
	{
		float scrollableSize = (float)getMaxSize() - getHandleSize();

		if (scrollableSize > 0.0f)
			_setHandlePos(pos / scrollableSize);
		else
			_setHandlePos(0.0f);
	}

	UINT32 GUISliderHandle::getMaxSize() const
	{
		UINT32 maxSize = mLayoutData.area.height;
		if(mFlags.isSet(GUISliderHandleFlag::Horizontal))
			maxSize = mLayoutData.area.width;

		return maxSize;
	}

	const HSpriteTexture& GUISliderHandle::getActiveTexture() const
	{
		switch(mState)
		{
		case State::Active:
			return _getStyle()->active.texture;
		case State::Hover:
			return _getStyle()->hover.texture;
		case State::Normal:
			return _getStyle()->normal.texture;
		}

		return _getStyle()->normal.texture;
	}
}
