//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Platform/BsDropTarget.h"

namespace bs
{
	void DropTarget::_clear()
	{
		mFileList.clear();
	}

	bool DropTarget::_isInside(const Vector2I& pos) const
	{
		return mArea.contains(pos);
	}

	void DropTarget::_setFileList(const Vector<Path>& fileList)
	{
		_clear();

		mDropType = DropTargetType::FileList;
		mFileList = fileList;
	}

	SPtr<DropTarget> DropTarget::create(const RenderWindow* window, const Rect2I& area)
	{
		DropTarget* target = new (bs_alloc<DropTarget>()) DropTarget(window, area);
		return bs_shared_ptr(target);
	}
}

