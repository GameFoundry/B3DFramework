//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPlatform.h"

namespace bs
{
	OSDropTarget::OSDropTarget(const RenderWindow* ownerWindow, INT32 x, INT32 y, UINT32 width, UINT32 height)
		:mX(x), mY(y), mWidth(width), mHeight(height), mActive(false), mOwnerWindow(ownerWindow), 
		mDropType(OSDropType::None), mFileList(nullptr)
	{
		
	}

	OSDropTarget::~OSDropTarget()
	{
		_clear();
	}

	void OSDropTarget::setArea(INT32 x, INT32 y, UINT32 width, UINT32 height)
	{
		mX = x;
		mY = y;
		mWidth = width;
		mHeight = height;
	}

	void OSDropTarget::_clear()
	{
		if(mFileList != nullptr)
		{
			bs_delete(mFileList);
			mFileList = nullptr;
		}
	}

	bool OSDropTarget::_isInside(const Vector2I& pos) const
	{
		INT32 right = mX + mWidth;
		INT32 bottom = mY + mHeight;

		return (pos.x >= mX && pos.x < right && pos.y >= mY && pos.y < bottom);
	}

	void OSDropTarget::_setFileList(const Vector<WString>& fileList)
	{
		_clear();

		mDropType = OSDropType::FileList;
		mFileList = bs_new<Vector<WString>>();
		*mFileList = fileList;
	}
}