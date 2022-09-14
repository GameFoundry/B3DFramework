//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/BsDropTarget.h"

namespace bs
{
	void DropTarget::ClearInternal()
	{
		mFileList.clear();
	}

	bool DropTarget::IsInsideInternal(const Vector2I& pos) const
	{
		return mArea.contains(pos);
	}

	void DropTarget::SetFileListInternal(const Vector<Path>& fileList)
	{
		ClearInternal();

		mDropType = DropTargetType::FileList;
		mFileList = fileList;
	}

	SPtr<DropTarget> DropTarget::create(const RenderWindow* window, const Rect2I& area)
	{
		DropTarget* target = new (bs_alloc<DropTarget>()) DropTarget(window, area);
		return bs_shared_ptr(target);
	}
}

