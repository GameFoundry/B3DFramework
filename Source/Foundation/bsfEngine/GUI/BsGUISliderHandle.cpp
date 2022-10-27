//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISliderHandle.h"
#include "2D/BsImageSprite.h"
#include "GUI/BsGUISkin.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUIMouseEvent.h"

using namespace bs;

const u32 GUISliderHandle::kResizeHandleSize = 7;

const String& GUISliderHandle::GetGuiTypeName()
{
	static String name = "SliderHandle";
	return name;
}

GUISliderHandle::GUISliderHandle(GUISliderHandleFlags flags, const String& styleName, const GUIDimensions& dimensions)
	: GUIElement(styleName, dimensions), mFlags(flags)
{
	mImageSprite = bs_new<ImageSprite>();

	// Calling virtual method is okay in this case
	StyleUpdated();
}

GUISliderHandle::~GUISliderHandle()
{
	bs_delete(mImageSprite);
}

GUISliderHandle* GUISliderHandle::Create(GUISliderHandleFlags flags, const String& styleName)
{
	return new(bs_alloc<GUISliderHandle>()) GUISliderHandle(flags, GetStyleName<GUISliderHandle>(styleName), GUIDimensions::Create());
}

GUISliderHandle* GUISliderHandle::Create(GUISliderHandleFlags flags, const GUIOptions& options, const String& styleName)
{
	return new(bs_alloc<GUISliderHandle>()) GUISliderHandle(flags, GetStyleName<GUISliderHandle>(styleName), GUIDimensions::Create(options));
}

void GUISliderHandle::SetHandleSizeInternal(float pct)
{
	mPctHandleSize = Math::Clamp01(pct);
}

void GUISliderHandle::SetHandlePosInternal(float pct)
{
	float maxPct = 1.0f;
	if(mStep > 0.0f && pct < maxPct)
	{
		pct = (pct + mStep * 0.5f) - fmod(pct + mStep * 0.5f, mStep);
		maxPct = Math::Floor(1.0f / mStep) * mStep;
	}

	mPctHandlePos = Math::Clamp(pct, 0.0f, maxPct);
}

float GUISliderHandle::GetHandlePos() const
{
	return mPctHandlePos;
	;
}

float GUISliderHandle::GetStep() const
{
	return mStep;
}

void GUISliderHandle::SetStep(float step)
{
	mStep = Math::Clamp01(step);
}

u32 GUISliderHandle::GetScrollableSize() const
{
	return GetMaxSize() - GetHandleSize();
}

void GUISliderHandle::UpdateRenderElementsInternal()
{
	IMAGE_SPRITE_DESC desc;

	HSpriteTexture activeTex = GetActiveTexture();
	if(SpriteTexture::CheckIsLoaded(activeTex))
		desc.Texture = activeTex;

	u32 handleSize = GetHandleSize();
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		if(handleSize == 0 && desc.Texture != nullptr)
		{
			handleSize = desc.Texture->GetWidth();
			mPctHandleSize = handleSize / (float)GetMaxSize();
		}

		desc.Width = handleSize;
		desc.Height = mLayoutData.Area.Height;
	}
	else
	{
		if(handleSize == 0 && desc.Texture != nullptr)
		{
			handleSize = desc.Texture->GetHeight();
			mPctHandleSize = handleSize / (float)GetMaxSize();
		}

		desc.Width = mLayoutData.Area.Width;
		desc.Height = handleSize;
	}

	desc.BorderLeft = GetStyleInternal()->Border.Left;
	desc.BorderRight = GetStyleInternal()->Border.Right;
	desc.BorderTop = GetStyleInternal()->Border.Top;
	desc.BorderBottom = GetStyleInternal()->Border.Bottom;
	desc.Color = GetTint();
	mImageSprite->Update(desc, (u64)GetParentWidgetInternal());

	// Populate GUI render elements from the sprites
	{
		using T = impl::GUIRenderElementHelper;
		T::Populate({ T::SpriteInfo(mImageSprite) }, mRenderElements);
	}

	GUIElement::UpdateRenderElementsInternal();
}

void GUISliderHandle::UpdateClippedBounds()
{
	mClippedBounds = mLayoutData.Area;
	mClippedBounds.Clip(mLayoutData.ClipRect);
}

Vector2I GUISliderHandle::GetOptimalSizeInternal() const
{
	HSpriteTexture activeTex = GetActiveTexture();

	if(SpriteTexture::CheckIsLoaded(activeTex))
		return Vector2I(activeTex->GetWidth(), activeTex->GetHeight());

	return Vector2I();
}

void GUISliderHandle::FillBuffer(
	u8* vertices,
	u32* indices,
	u32 vertexOffset,
	u32 indexOffset,
	const Vector2I& offset,
	u32 maxNumVerts,
	u32 maxNumIndices,
	u32 renderElementIdx) const
{
	u8* uvs = vertices + sizeof(Vector2);
	u32 vertexStride = sizeof(Vector2) * 2;
	u32 indexStride = sizeof(u32);

	Vector2I layoutOffset = Vector2I(mLayoutData.Area.X, mLayoutData.Area.Y) + offset;
	Rect2I clipRect = mLayoutData.GetLocalClipRect();

	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		layoutOffset.X += GetHandlePosPx();
		clipRect.X -= GetHandlePosPx();
	}
	else
	{
		layoutOffset.Y += GetHandlePosPx();
		clipRect.Y -= GetHandlePosPx();
	}

	mImageSprite->FillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices, vertexStride, indexStride, renderElementIdx, layoutOffset, clipRect);
}

bool GUISliderHandle::MouseEventInternal(const GUIMouseEvent& ev)
{
	u32 handleSize = GetHandleSize();

	if(ev.GetType() == GUIMouseEventType::MouseMove)
	{
		if(!IsDisabledInternal())
		{
			if(mMouseOverHandle)
			{
				if(!IsOnHandle(ev.GetPosition()))
				{
					mMouseOverHandle = false;

					mState = State::Normal;
					MarkLayoutAsDirtyInternal();

					return true;
				}
			}
			else
			{
				if(IsOnHandle(ev.GetPosition()))
				{
					mMouseOverHandle = true;

					mState = State::Hover;
					MarkLayoutAsDirtyInternal();

					return true;
				}
			}
		}
	}

	bool jumpOnClick = mFlags.IsSet(GUISliderHandleFlag::JumpOnClick);
	if(ev.GetType() == GUIMouseEventType::MouseDown && (mMouseOverHandle || jumpOnClick))
	{
		if(!IsDisabledInternal())
		{
			mState = State::Active;
			MarkLayoutAsDirtyInternal();

			if(jumpOnClick)
			{
				float handlePosPx = 0.0f;

				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					handlePosPx = (float)(ev.GetPosition().X - (i32)mLayoutData.Area.X - handleSize * 0.5f);
				else
					handlePosPx = (float)(ev.GetPosition().Y - (i32)mLayoutData.Area.Y - handleSize * 0.5f);

				SetHandlePosPx((i32)handlePosPx);
				OnHandleMovedOrResized(mPctHandlePos, GetHandleSizePctInternal());
			}

			bool isResizeable = mFlags.IsSet(GUISliderHandleFlag::Resizeable);
			if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
			{
				i32 left = (i32)mLayoutData.Area.X + GetHandlePosPx();

				if(isResizeable)
				{
					i32 right = left + handleSize;

					i32 clickPos = ev.GetPosition().X;
					if(clickPos >= left && clickPos < (left + (i32)kResizeHandleSize))
						mDragState = DragState::LeftResize;
					else if(clickPos >= (right - (i32)kResizeHandleSize) && clickPos < right)
						mDragState = DragState::RightResize;
					else
						mDragState = DragState::Normal;
				}
				else
					mDragState = DragState::Normal;

				mDragStartPos = ev.GetPosition().X - left;
			}
			else
			{
				i32 top = (i32)mLayoutData.Area.Y + GetHandlePosPx();

				if(isResizeable)
				{
					i32 bottom = top + handleSize;

					i32 clickPos = ev.GetPosition().Y;
					if(clickPos >= top && clickPos < (top + (i32)kResizeHandleSize))
						mDragState = DragState::LeftResize;
					else if(clickPos >= (bottom - (i32)kResizeHandleSize) && clickPos < bottom)
						mDragState = DragState::RightResize;
					else
						mDragState = DragState::Normal;
				}
				else
					mDragState = DragState::Normal;

				mDragStartPos = ev.GetPosition().Y - top;
			}

			mHandleDragged = true;
		}

		return true;
	}

	if(ev.GetType() == GUIMouseEventType::MouseDrag && mHandleDragged)
	{
		if(!IsDisabledInternal())
		{
			if(mDragState == DragState::Normal)
			{
				i32 handlePosPx;
				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					handlePosPx = ev.GetPosition().X - mDragStartPos - mLayoutData.Area.X;
				else
					handlePosPx = ev.GetPosition().Y - mDragStartPos - mLayoutData.Area.Y;

				SetHandlePosPx(handlePosPx);
				OnHandleMovedOrResized(mPctHandlePos, GetHandleSizePctInternal());
			}
			else // Resizing
			{
				i32 clickPosPx;
				if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					clickPosPx = ev.GetPosition().X - mLayoutData.Area.X;
				else
					clickPosPx = ev.GetPosition().Y - mLayoutData.Area.Y;

				i32 left = GetHandlePosPx();
				u32 maxSize = GetMaxSize();

				i32 newHandleSize;
				float newHandlePos;
				if(mDragState == DragState::LeftResize)
				{
					i32 newLeft = clickPosPx - mDragStartPos;
					i32 right = left + handleSize;
					newLeft = Math::Clamp(newLeft, 0, right);

					newHandleSize = std::max((i32)mMinHandleSize, right - newLeft);
					newLeft = right - newHandleSize;

					float scrollableSize = (float)(maxSize - newHandleSize);
					if(scrollableSize > 0.0f)
						newHandlePos = newLeft / scrollableSize;
					else
						newHandlePos = 0.0f;
				}
				else // Right resize
				{
					i32 newRight = clickPosPx;
					newHandleSize = std::max((i32)mMinHandleSize, std::min(newRight, (i32)maxSize) - left);

					float scrollableSize = (float)(maxSize - newHandleSize);
					if(scrollableSize > 0.0f)
						newHandlePos = left / scrollableSize;
					else
						newHandlePos = 0.0f;
				}

				SetHandleSizeInternal(newHandleSize / (float)maxSize);
				SetHandlePosInternal(newHandlePos);

				OnHandleMovedOrResized(mPctHandlePos, GetHandleSizePctInternal());
			}

			MarkLayoutAsDirtyInternal();
		}

		return true;
	}

	if(ev.GetType() == GUIMouseEventType::MouseOut)
	{
		if(!IsDisabledInternal())
		{
			mMouseOverHandle = false;

			if(!mHandleDragged)
			{
				mState = State::Normal;
				MarkLayoutAsDirtyInternal();
			}
		}

		return true;
	}

	if(ev.GetType() == GUIMouseEventType::MouseUp)
	{
		if(!IsDisabledInternal())
		{
			if(mMouseOverHandle)
				mState = State::Hover;
			else
				mState = State::Normal;

			if(!mHandleDragged)
			{
				// If we clicked above or below the scroll handle, scroll by one page
				i32 handlePosPx = GetHandlePosPx();
				if(!mFlags.IsSet(GUISliderHandleFlag::JumpOnClick))
				{
					if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
					{
						i32 handleLeft = (i32)mLayoutData.Area.X + handlePosPx;
						i32 handleRight = handleLeft + handleSize;

						if(ev.GetPosition().X < handleLeft)
							MoveOneStep(false);
						else if(ev.GetPosition().X > handleRight)
							MoveOneStep(true);
					}
					else
					{
						i32 handleTop = (i32)mLayoutData.Area.Y + handlePosPx;
						i32 handleBottom = handleTop + handleSize;

						if(ev.GetPosition().Y < handleTop)
							MoveOneStep(false);
						else if(ev.GetPosition().Y > handleBottom)
							MoveOneStep(true);
					}
				}
			}
			mHandleDragged = false;
			MarkLayoutAsDirtyInternal();
		}

		return true;
	}

	if(ev.GetType() == GUIMouseEventType::MouseDragEnd)
	{
		if(!IsDisabledInternal())
		{
			mHandleDragged = false;
			if(mMouseOverHandle)
				mState = State::Hover;
			else
				mState = State::Normal;

			MarkLayoutAsDirtyInternal();
		}

		return true;
	}

	return false;
}

void GUISliderHandle::MoveOneStep(bool forward)
{
	const u32 handleSize = GetHandleSize();
	i32 handlePosPx = GetHandlePosPx();

	i32 stepSizePx;
	if(mStep > 0.0f)
		stepSizePx = (i32)(mStep * GetMaxSize());
	else
		stepSizePx = (i32)handleSize;

	handlePosPx += forward ? stepSizePx : -stepSizePx;

	SetHandlePosPx(handlePosPx);
	OnHandleMovedOrResized(mPctHandlePos, GetHandleSizePctInternal());

	MarkLayoutAsDirtyInternal();
}

bool GUISliderHandle::IsOnHandle(const Vector2I& pos) const
{
	u32 handleSize = GetHandleSize();
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
	{
		i32 left = (i32)mLayoutData.Area.X + GetHandlePosPx();
		i32 right = left + handleSize;

		if(pos.X >= left && pos.X < right)
			return true;
	}
	else
	{
		i32 top = (i32)mLayoutData.Area.Y + GetHandlePosPx();
		i32 bottom = top + handleSize;

		if(pos.Y >= top && pos.Y < bottom)
			return true;
	}

	return false;
}

i32 GUISliderHandle::GetHandlePosPx() const
{
	i32 maxScrollAmount = std::max(0, (i32)GetMaxSize() - (i32)GetHandleSize());
	return Math::FloorToInt(mPctHandlePos * maxScrollAmount);
}

u32 GUISliderHandle::GetHandleSize() const
{
	return std::max(mMinHandleSize, (u32)(GetMaxSize() * mPctHandleSize));
}

float GUISliderHandle::GetHandleSizePctInternal() const
{
	return mPctHandleSize;
}

void GUISliderHandle::StyleUpdated()
{
	const GUIElementStyle* style = GetStyleInternal();
	if(style != nullptr)
	{
		if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
			mMinHandleSize = style->FixedWidth ? style->Width : style->MinWidth;
		else
			mMinHandleSize = style->FixedHeight ? style->Height : style->MinHeight;
	}
}

void GUISliderHandle::SetHandlePosPx(i32 pos)
{
	float scrollableSize = (float)GetMaxSize() - GetHandleSize();

	if(scrollableSize > 0.0f)
		SetHandlePosInternal(pos / scrollableSize);
	else
		SetHandlePosInternal(0.0f);
}

u32 GUISliderHandle::GetMaxSize() const
{
	u32 maxSize = mLayoutData.Area.Height;
	if(mFlags.IsSet(GUISliderHandleFlag::Horizontal))
		maxSize = mLayoutData.Area.Width;

	return maxSize;
}

const HSpriteTexture& GUISliderHandle::GetActiveTexture() const
{
	switch(mState)
	{
	case State::Active:
		return GetStyleInternal()->Active.Texture;
	case State::Hover:
		return GetStyleInternal()->Hover.Texture;
	case State::Normal:
		return GetStyleInternal()->Normal.Texture;
	}

	return GetStyleInternal()->Normal.Texture;
}
