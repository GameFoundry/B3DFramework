//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollBar.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUISliderHandle.h"
#include "GUI/BsGUISpace.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 GUIScrollBar::ButtonScrollAmount = 10;

	const String& GUIScrollBar::getHScrollHandleType()
	{
		static String typeName = "UIScrollBarHHandle";
		return typeName;
	}

	const String& GUIScrollBar::getVScrollHandleType()
	{
		static String typeName = "UIScrollBarVHandle";
		return typeName;
	}

	GUIScrollBar::GUIScrollBar(bool horizontal, bool resizable, const String& styleName, const GUIDimensions& dimensions)
		:GUIElement(styleName, dimensions), mHorizontal(horizontal)
	{
		mImageSprite = bs_new<ImageSprite>();

		GUISliderHandleFlags flags;
		if (resizable)
			flags |= GUISliderHandleFlag::Resizeable;

		if(mHorizontal)
		{
			mLayout = GUILayoutX::create();
			_registerChildElement(mLayout);

			mUpBtn = GUIButton::create(HString(""), "ScrollLeftBtn");
			mDownBtn = GUIButton::create(HString(""), "ScrollRightBtn");

			mHandleBtn = GUISliderHandle::create(flags | GUISliderHandleFlag::Horizontal, getSubStyleName(getHScrollHandleType()));
		}
		else
		{
			mLayout = GUILayoutY::create();
			_registerChildElement(mLayout);

			mUpBtn = GUIButton::create(HString(""), "ScrollUpBtn");
			mDownBtn = GUIButton::create(HString(""), "ScrollDownBtn");

			mHandleBtn = GUISliderHandle::create(flags | GUISliderHandleFlag::Vertical, getSubStyleName(getVScrollHandleType()));
		}

		GUIElementOptions scrollUpBtnOptions = mUpBtn->getOptionFlags();
		scrollUpBtnOptions.unset(GUIElementOption::AcceptsKeyFocus);

		mUpBtn->setOptionFlags(scrollUpBtnOptions);

		GUIElementOptions scrollDownBtnOptions = mDownBtn->getOptionFlags();
		scrollDownBtnOptions.unset(GUIElementOption::AcceptsKeyFocus);

		mDownBtn->setOptionFlags(scrollDownBtnOptions);

		mLayout->addNewElement<GUIFixedSpace>(2);
		mLayout->addElement(mUpBtn);
		mLayout->addElement(mHandleBtn);
		mLayout->addElement(mDownBtn);
		mLayout->addNewElement<GUIFixedSpace>(2);

		mHandleBtn->onHandleMovedOrResized.connect(std::bind(&GUIScrollBar::handleMoved, this, _1, _2));

		mUpBtn->onClick.connect(std::bind(&GUIScrollBar::upButtonClicked, this));
		mDownBtn->onClick.connect(std::bind(&GUIScrollBar::downButtonClicked, this));
	}

	GUIScrollBar::~GUIScrollBar()
	{
		bs_delete(mImageSprite);

		GUIElement::destroy(mUpBtn);
		GUIElement::destroy(mDownBtn);
		GUIElement::destroy(mHandleBtn);
	}

	void GUIScrollBar::updateRenderElementsInternal()
	{
		IMAGE_SPRITE_DESC desc;

		if(_getStyle()->normal.texture != nullptr && _getStyle()->normal.texture.isLoaded())
			desc.texture = _getStyle()->normal.texture;

		desc.width = mLayoutData.area.width;
		desc.height = mLayoutData.area.height;
		desc.color = getTint();

		mImageSprite->update(desc, (UINT64)_getParentWidget());

		// Populate GUI render elements from the sprites
		{
			using T = impl::GUIRenderElementHelper;
			T::populate({ T::SpriteInfo(mImageSprite, 2) }, mRenderElements); // +2 depth because child buttons use +1
		}

		GUIElement::updateRenderElementsInternal();
	}

	void GUIScrollBar::updateClippedBounds()
	{
		mClippedBounds = Rect2I(0, 0, 0, 0); // We don't want any mouse input for this element. This is just a container.
	}

	Vector2I GUIScrollBar::_getOptimalSize() const
	{
		return mLayout->_getOptimalSize();
	}

	UINT32 GUIScrollBar::_getRenderElementDepthRange() const
	{
		return 3;
	}

	void GUIScrollBar::_fillBuffer(
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
		mImageSprite->fillBuffer(vertices, uvs, indices, vertexOffset, indexOffset, maxNumVerts, maxNumIndices,
			vertexStride, indexStride, renderElementIdx, layoutOffset, mLayoutData.getLocalClipRect());
	}

	void GUIScrollBar::styleUpdated()
	{
		if (mHorizontal)
			mHandleBtn->setStyle(getSubStyleName(getHScrollHandleType()));
		else
			mHandleBtn->setStyle(getSubStyleName(getVScrollHandleType()));
	}

	void GUIScrollBar::handleMoved(float handlePct, float sizePct)
	{
		if(!onScrollOrResize.empty())
			onScrollOrResize(handlePct, sizePct);
	}

	void GUIScrollBar::upButtonClicked()
	{
		float handleOffset = 0.0f;
		float scrollableSize = (float)mHandleBtn->getScrollableSize();
		
		if(scrollableSize > 0.0f)
			handleOffset = ButtonScrollAmount / scrollableSize;

		scroll(handleOffset);
	}

	void GUIScrollBar::downButtonClicked()
	{
		float handleOffset = 0.0f;
		float scrollableSize = (float)mHandleBtn->getScrollableSize();

		if(scrollableSize > 0.0f)
			handleOffset = ButtonScrollAmount / scrollableSize;

		scroll(-handleOffset);
	}

	void GUIScrollBar::scroll(float amount)
	{
		float newHandlePos = Math::clamp01(mHandleBtn->getHandlePos() - amount);

		float oldHandlePos = mHandleBtn->getHandlePos();
		mHandleBtn->_setHandlePos(newHandlePos);

		if (oldHandlePos != mHandleBtn->getHandlePos())
		{
			mHandleBtn->_markLayoutAsDirty();

			if (!onScrollOrResize.empty())
				onScrollOrResize(newHandlePos, mHandleBtn->_getHandleSizePct());
		}
	}

	void GUIScrollBar::_setHandleSize(float pct)
	{
		mHandleBtn->_setHandleSize(pct);
	}

	void GUIScrollBar::_setScrollPos(float pct)
	{
		mHandleBtn->_setHandlePos(pct);
	}

	float GUIScrollBar::getScrollPos() const
	{
		return mHandleBtn->getHandlePos();
	}

	void GUIScrollBar::setScrollPos(float pct)
	{
		float oldHandlePos = mHandleBtn->getHandlePos();
		mHandleBtn->_setHandlePos(pct);

		if (oldHandlePos != mHandleBtn->getHandlePos())
			mHandleBtn->_markLayoutAsDirty();
	}

	float GUIScrollBar::getHandleSize() const
	{
		return mHandleBtn->_getHandleSizePct();
	}

	void GUIScrollBar::setHandleSize(float pct)
	{
		mHandleBtn->_setHandleSize(pct);
		mHandleBtn->_markLayoutAsDirty();
	}

	UINT32 GUIScrollBar::getScrollableSize() const
	{
		return mHandleBtn->getScrollableSize();
	}

	void GUIScrollBar::setTint(const Color& color)
	{
		mUpBtn->setTint(color);
		mDownBtn->setTint(color);
		mHandleBtn->setTint(color);

		GUIElement::setTint(color);
	}
}
