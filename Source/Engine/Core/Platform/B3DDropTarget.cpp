//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Platform/B3DDropTarget.h"

using namespace b3d;

void DropTarget::ClearInternal()
{
	mFileList.clear();
}

bool DropTarget::IsInside(const Vector2I& pos) const
{
	return mArea.Contains(pos);
}

void DropTarget::SetFileList(const Vector<Path>& fileList)
{
	ClearInternal();

	mDropType = DropTargetType::FileList;
	mFileList = fileList;
}

TShared<DropTarget> DropTarget::Create(const RenderWindow* window, const Area2I& area)
{
	DropTarget* target = new(B3DAllocate<DropTarget>()) DropTarget(window, area);
	return B3DMakeSharedFromExisting(target);
}
