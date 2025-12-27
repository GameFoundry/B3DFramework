//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Win32/B3DWin32DropTarget.h"
#include "Private/Win32/B3DWin32Platform.h"
#include "Platform/B3DDropTarget.h"
#include "String/B3DUnicode.h"

using namespace b3d;

DropTarget::DropTarget(const RenderWindow* ownerWindow, const Area2I& area)
	: mArea(area), mActive(false), mOwnerWindow(ownerWindow), mDropType(DropTargetType::None)
{
	Win32Platform::RegisterDropTarget(this);
}

DropTarget::~DropTarget()
{
	Win32Platform::UnregisterDropTarget(this);

	ClearInternal();
}

void DropTarget::SetArea(const Area2I& area)
{
	mArea = area;
}

Win32DropTarget::Win32DropTarget(HWND hWnd)
	: mRefCount(1), mHWnd(hWnd), mAcceptDrag(false)
{}

Win32DropTarget::~Win32DropTarget()
{
	Lock lock(mSync);

	for(auto& fileList : mFileLists)
		B3DDelete(fileList);

	mFileLists.clear();
	mQueuedDropOps.clear();
}

void Win32DropTarget::RegisterWithOs()
{
	CoLockObjectExternal(this, TRUE, FALSE);
	RegisterDragDrop(mHWnd, this);
}

void Win32DropTarget::UnregisterWithOs()
{
	RevokeDragDrop(mHWnd);
	CoLockObjectExternal(this, FALSE, FALSE);
}

HRESULT __stdcall Win32DropTarget::QueryInterface(REFIID iid, void** ppvObject)
{
	if(iid == IID_IDropTarget || iid == IID_IUnknown)
	{
		AddRef();
		*ppvObject = this;
		return S_OK;
	}
	else
	{
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}
}

ULONG __stdcall Win32DropTarget::AddRef()
{
	return InterlockedIncrement(&mRefCount);
}

ULONG __stdcall Win32DropTarget::Release()
{
	LONG count = InterlockedDecrement(&mRefCount);

	if(count == 0)
	{
		B3DDelete(this);
		return 0;
	}
	else
	{
		return count;
	}
}

HRESULT __stdcall Win32DropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	*pdwEffect = DROPEFFECT_LINK;

	mAcceptDrag = IsDataValid(pDataObj);
	if(!mAcceptDrag)
		return S_OK;

	{
		Lock lock(mSync);

		mFileLists.push_back(GetFileListFromData(pDataObj));

		ScreenToClient(mHWnd, (POINT*)&pt);
		mQueuedDropOps.push_back(DropTargetOp(DropOpType::DragOver, Vector2I((int)pt.x, (int)pt.y)));

		DropTargetOp& op = mQueuedDropOps.back();
		op.DataType = DropOpDataType::FileList;
		op.MFileList = mFileLists.back();
	}

	return S_OK;
}

HRESULT __stdcall Win32DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	*pdwEffect = DROPEFFECT_LINK;

	if(!mAcceptDrag)
		return S_OK;

	{
		Lock lock(mSync);

		ScreenToClient(mHWnd, (POINT*)&pt);
		mQueuedDropOps.push_back(DropTargetOp(DropOpType::DragOver, Vector2I((int)pt.x, (int)pt.y)));

		DropTargetOp& op = mQueuedDropOps.back();
		op.DataType = DropOpDataType::FileList;
		op.MFileList = mFileLists.back();
	}

	return S_OK;
}

HRESULT __stdcall Win32DropTarget::DragLeave()
{
	{
		Lock lock(mSync);

		mQueuedDropOps.push_back(DropTargetOp(DropOpType::Leave, Vector2I(0, 0)));

		DropTargetOp& op = mQueuedDropOps.back();
		op.DataType = DropOpDataType::FileList;
		op.MFileList = mFileLists.back();
	}

	return S_OK;
}

HRESULT __stdcall Win32DropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
	*pdwEffect = DROPEFFECT_LINK;
	mAcceptDrag = false;

	if(!IsDataValid(pDataObj))
		return S_OK;

	{
		Lock lock(mSync);

		mFileLists.push_back(GetFileListFromData(pDataObj));

		ScreenToClient(mHWnd, (POINT*)&pt);
		mQueuedDropOps.push_back(DropTargetOp(DropOpType::Drop, Vector2I((int)pt.x, (int)pt.y)));

		DropTargetOp& op = mQueuedDropOps.back();
		op.DataType = DropOpDataType::FileList;
		op.MFileList = mFileLists.back();
	}

	return S_OK;
}

void Win32DropTarget::RegisterDropTarget(DropTarget* dropTarget)
{
	mDropTargets.push_back(dropTarget);
}

void Win32DropTarget::UnregisterDropTarget(DropTarget* dropTarget)
{
	auto findIter = std::find(begin(mDropTargets), end(mDropTargets), dropTarget);
	if(findIter != mDropTargets.end())
		mDropTargets.erase(findIter);
}

unsigned int Win32DropTarget::GetNumDropTargets() const
{
	return (unsigned int)mDropTargets.size();
}

void Win32DropTarget::Update()
{
	Lock lock(mSync);

	for(auto& op : mQueuedDropOps)
	{
		for(auto& target : mDropTargets)
		{
			if(op.Type != DropOpType::Leave)
			{
				if(target->IsInsideInternal(op.Position))
				{
					if(!target->IsActiveInternal())
					{
						target->SetFileListInternal(*op.MFileList);
						target->SetActiveInternal(true);
						target->OnEnter(op.Position.X, op.Position.Y);
					}

					if(op.Type == DropOpType::DragOver)
						target->OnDragOver(op.Position.X, op.Position.Y);
					else if(op.Type == DropOpType::Drop)
					{
						target->SetFileListInternal(*op.MFileList);
						target->OnDrop(op.Position.X, op.Position.Y);
					}
				}
				else
				{
					if(target->IsActiveInternal())
					{
						target->OnLeave();
						target->ClearInternal();
						target->SetActiveInternal(false);
					}
				}
			}
			else
			{
				if(target->IsActiveInternal())
				{
					target->OnLeave();
					target->ClearInternal();
					target->SetActiveInternal(false);
				}
			}
		}

		if(op.Type == DropOpType::Leave || op.Type == DropOpType::Drop)
		{
			while(!mFileLists.empty())
			{
				bool done = mFileLists[0] == op.MFileList;

				B3DDelete(mFileLists[0]);
				mFileLists.erase(mFileLists.begin());

				if(done)
					break;
			}
		}
	}

	mQueuedDropOps.clear();
}

bool Win32DropTarget::IsDataValid(IDataObject* data)
{
	// TODO - Currently only supports file drag and drop, so only CF_HDROP is used
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	return data->QueryGetData(&fmtetc) == S_OK ? true : false;
}

/**	Gets a file list from data. Caller must ensure that the data actually contains a file list. */
Vector<Path>* Win32DropTarget::GetFileListFromData(IDataObject* data)
{
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stgmed;

	Vector<Path>* files = B3DNew<Vector<Path>>();
	if(data->GetData(&fmtetc, &stgmed) == S_OK)
	{
		PVOID data = GlobalLock(stgmed.hGlobal);

		HDROP hDrop = (HDROP)data;
		UINT numFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

		files->resize(numFiles);
		for(UINT i = 0; i < numFiles; i++)
		{
			UINT numChars = DragQueryFileW(hDrop, i, nullptr, 0) + 1;
			wchar_t* buffer = (wchar_t*)B3DAllocate((u32)numChars * sizeof(wchar_t));

			DragQueryFileW(hDrop, i, buffer, numChars);

			(*files)[i] = UTF8::FromWide(WString(buffer));

			B3DFree(buffer);
		}

		GlobalUnlock(stgmed.hGlobal);
		ReleaseStgMedium(&stgmed);
	}

	return files;
}
