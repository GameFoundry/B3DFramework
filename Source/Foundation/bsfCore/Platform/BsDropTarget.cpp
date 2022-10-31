//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/BsDropTarget.h"

using namespace bs;

void DropTarget::ClearInternal()
{
	mFileList.clear();
}

bool DropTarget::IsInsideInternal(const Vector2I& pos) const
{
	return mArea.Contains(pos);
}

void DropTarget::SetFileListInternal(const Vector<Path>& fileList)
{
	ClearInternal();

	mDropType = DropTargetType::FileList;
	mFileList = fileList;
}

SPtr<DropTarget> DropTarget::Create(const RenderWindow* window, const Rect2I& area)
{
	DropTarget* target = new(B3DAllocate<DropTarget>()) DropTarget(window, area);
	return B3DMakeSharedFromExisting(target);
}
