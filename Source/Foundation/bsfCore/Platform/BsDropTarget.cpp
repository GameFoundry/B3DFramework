//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/BsDropTarget.h"

namespace bs
{
	void DropTarget::_clear()
	{
		mFileList.Clear();
	}

	bool DropTarget::_isInside(const Vector2I& pos) const
	{
		return mArea.Contains(pos);
	}

	void DropTarget::_setFileList(const Vector<Path>& fileList)
	{
		_clear();

		mDropType = DropTargetType::FileList;
		mFileList = fileList;
	}

	SPtr<DropTarget> DropTarget::Create(const RenderWindow* window, const Rect2I& area)
	{
		DropTarget* target = new (bs_alloc<DropTarget>()) DropTarget(window, area);
		return bs_shared_ptr(target);
	}
}

