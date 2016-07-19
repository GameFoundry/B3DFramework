//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "math.h"
#include "BsGUISliderHandle.h"
#include "BsImageSprite.h"
#include "BsGUISkin.h"
#include "BsSpriteTexture.h"
#include "BsGUIDimensions.h"
#include "BsGUIMouseEvent.h"

namespace BansheeEngine
{
	const String& GUISliderHandle::getGUITypeName()
	{
		static String name = "SliderHandle";
		return name;
	}

	GUISliderHandle::GUISliderHandle(bool horizontal, bool jumpOnClick, const String& styleName, const GUIDimensions& dimensions)
		: GUIElement(styleName, dimensions), mHandleSize(0), mHorizontal(horizontal), mJumpOnClick(jumpOnClick)
		, mPctHandlePos(0.0f), mStep(0.0f), mDragStartPos(0), mMouseOverHandle(false), mHandleDragged(false)
		, mState(State::Normal)
	{
		mImageSprite = bs_new<ImageSprite>();
	}

	GUISliderHandle::~GUISliderHandle()
	{
		bs_delete(mImageSprite);
	}

	GUISliderHandle* GUISliderHandle::create(bool horizontal, bool jumpOnClick, const String& styleName)
	{
		return new (bs_alloc<GUISliderHandle>()) GUISliderHandle(horizontal, jumpOnClick, 
			getStyleName<GUISliderHandle>(styleName), GUIDimensions::create());
	}

	GUISliderHandle* GUISliderHandle::create(bool horizontal, bool jumpOnClick, const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUISliderHandle>()) GUISliderHandle(horizontal, jumpOnClick, 
			getStyleName<GUISliderHandle>(styleName), GUIDimensions::create(options));
	}

	void GUISliderHandle::_setHandleSize(UINT32 size)
	{
		mHandleSize = std::min(getMaxSize(), size);
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
		return getMaxSize() - mHandleSize;
	}

	UINT32 GUISliderHandle::_getNumRenderElements() const
	{
		return mImageSprite->getNumRenderElements();
	}

	const SpriteMaterialInfo& GUISliderHandle::_getMaterial(UINT32 renderElementIdx, SpriteMaterial** material) const
	{
		*material = mImageSprite->getMaterial(renderElementIdx);
		return mImageSprite->getMaterialInfo(renderElementIdx);
	}

	void GUISliderHandle::_getMeshInfo(UINT32 renderElementIdx, UINT32& numVertices, UINT32& numIndices, GUIMeshType& type) const
	{
		UINT32 numQuads = mImageSprite->getNumQuads(renderElementIdx);
		numVertices = numQuads * 4;
		numIndices = numQuads * 6;
		type = GUIMeshType::Triangle;
	}

	void GUISliderHandle::updateRenderElementsInternal()
	{		
		IMAGE_SPRITE_DESC desc;

		HSpriteTexture activeTex = getActiveTexture();
		if(SpriteTexture::checkIsLoaded(activeTex))
			desc.texture = activeTex.getInternalPtr();

		if (mHorizontal)
		{
			if (mHandleSize == 0 && desc.texture != nullptr)
				mHandleSize = desc.texture->getWidth();

			desc.width = mHandleSize;
			desc.height = mLayoutData.area.height;
		}
		else
		{
			if (mHandleSize == 0 && desc.texture != nullptr)
				mHandleSize = desc.texture->getHeight();

			desc.width = mLayoutData.area.width;
			desc.height = mHandleSize;
		}

		desc.borderLeft = _getStyle()->border.left;
		desc.borderRight = _getStyle()->border.right;
		desc.borderTop = _getStyle()->border.top;
		desc.borderBottom = _getStyle()->border.bottom;
		desc.color = getTint();
		mImageSprite->update(desc, (UINT64)_getParentWidget());
		
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

	void GUISliderHandle::_fillBuffer(UINT8* vertices, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
		UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 renderElementIdx) const
	{
		UINT8* uvs = vertices + sizeof(Vector2);
		UINT32 vertexStride = sizeof(Vector2) * 2;
		UINT32 indexStride = sizeof(UINT32);

		Vector2I offset(mLayoutData.area.x, mLayoutData.area.y);
		if(mHorizontal)
			offset.x += getHandlePosPx();
		else
			offset.y += getHandlePosPx();

		Rect2I clipRect = mLayoutData.getLocalClipRect();
		if(mHorizontal)
			clipRect.x -= getHandlePosPx();
		else
			clipRect.y -= getHandlePosPx();

		mImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
			vertexStride, indexStride, renderElementIdx, offset, clipRect);
	}

	bool GUISliderHandle::_mouseEvent(const GUIMouseEvent& ev)
	{
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

		if(ev.getType() == GUIMouseEventType::MouseDown && (mMouseOverHandle || mJumpOnClick))
		{
			if (!_isDisabled())
			{
				mState = State::Active;
				_markLayoutAsDirty();

				if (mJumpOnClick)
				{
					float handlePosPx = 0.0f;

					if (mHorizontal)
						handlePosPx = (float)(ev.getPosition().x - (INT32)mLayoutData.area.x - mHandleSize * 0.5f);
					else
						handlePosPx = (float)(ev.getPosition().y - (INT32)mLayoutData.area.y - mHandleSize * 0.5f);

					setHandlePosPx((INT32)handlePosPx);
					onHandleMoved(mPctHandlePos);
				}

				if (mHorizontal)
				{
					INT32 left = (INT32)mLayoutData.area.x + getHandlePosPx();
					mDragStartPos = ev.getPosition().x - left;
				}
				else
				{
					INT32 top = (INT32)mLayoutData.area.y + getHandlePosPx();
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
				float handlePosPx = 0.0f;
				if (mHorizontal)
				{
					handlePosPx = (float)(ev.getPosition().x - mDragStartPos - mLayoutData.area.x);
				}
				else
				{
					handlePosPx = (float)(ev.getPosition().y - mDragStartPos - mLayoutData.area.y);
				}

				setHandlePosPx((INT32)handlePosPx);
				onHandleMoved(mPctHandlePos);

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
					if (!mJumpOnClick)
					{
						UINT32 stepSizePx = 0;
						if (mStep > 0.0f)
							stepSizePx = (UINT32)(mStep * getMaxSize());
						else
							stepSizePx = mHandleSize;

						INT32 handleOffset = 0;
						if (mHorizontal)
						{
							INT32 handleLeft = (INT32)mLayoutData.area.x + handlePosPx;
							INT32 handleRight = handleLeft + mHandleSize;

							if (ev.getPosition().x < handleLeft)
								handleOffset -= stepSizePx;
							else if (ev.getPosition().x > handleRight)
								handleOffset += stepSizePx;
						}
						else
						{
							INT32 handleTop = (INT32)mLayoutData.area.y + handlePosPx;
							INT32 handleBottom = handleTop + mHandleSize;

							if (ev.getPosition().y < handleTop)
								handleOffset -= stepSizePx;
							else if (ev.getPosition().y > handleBottom)
								handleOffset += stepSizePx;
						}

						handlePosPx += handleOffset;
					}

					setHandlePosPx(handlePosPx);
					onHandleMoved(mPctHandlePos);
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

	bool GUISliderHandle::isOnHandle(const Vector2I& pos) const
	{
		if(mHorizontal)
		{
			INT32 left = (INT32)mLayoutData.area.x + getHandlePosPx();
			INT32 right = left + mHandleSize;

			if(pos.x >= left && pos.x < right)
				return true;
		}
		else
		{
			INT32 top = (INT32)mLayoutData.area.y + getHandlePosPx();
			INT32 bottom = top + mHandleSize;

			if(pos.y >= top && pos.y < bottom)
				return true;
		}
		
		return false;
	}

	INT32 GUISliderHandle::getHandlePosPx() const
	{
		UINT32 maxScrollAmount = getMaxSize() - mHandleSize;
		return Math::floorToInt(mPctHandlePos * maxScrollAmount);
	}

	void GUISliderHandle::setHandlePosPx(INT32 pos)
	{
		float maxScrollAmount = (float)getMaxSize() - mHandleSize;
		_setHandlePos(pos / maxScrollAmount);
	}

	UINT32 GUISliderHandle::getMaxSize() const
	{
		UINT32 maxSize = mLayoutData.area.height;
		if(mHorizontal)
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