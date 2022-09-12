//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/Win32/BsWin32DropTarget.h"
#include "Private/Win32/BsWin32Platform.h"
#include "Platform/BsDropTarget.h"
#include "String/BsUnicode.h"

namespace bs
{
	DropTarget::DropTarget(const RenderWindow* ownerWindow, const Rect2I& area)
			:mArea(area), mActive(false), mOwnerWindow(ownerWindow), mDropType(DropTargetType::None)
	{
		Win32Platform::registerDropTarget(this);
	}

	DropTarget::~DropTarget()
	{
		Win32Platform::unregisterDropTarget(this);

		_clear();
	}

	void DropTarget::SetArea(const Rect2I& area)
	{
		mArea = area;
	}

	Win32DropTarget::Win32DropTarget(HWND hWnd)
		:mRefCount(1), mHWnd(hWnd), mAcceptDrag(false)
	{ }

	Win32DropTarget::~Win32DropTarget()
	{
		Lock Lock(mSync);

		for(auto& fileList : mFileLists)
			bs_delete(fileList);

		mFileLists.Clear();
		mQueuedDropOps.Clear();
	}

	void Win32DropTarget::RegisterWithOS()
	{
		CoLockObjectExternal(this, TRUE, FALSE);
		RegisterDragDrop(mHWnd, this);
	}

	void Win32DropTarget::UnregisterWithOS()
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
			bs_delete(this);
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

		mAcceptDrag = isDataValid(pDataObj);
		if(!mAcceptDrag)
			return S_OK;

		{
			Lock Lock(mSync);

			mFileLists.push_back(getFileListFromData(pDataObj));

			ScreenToClient(mHWnd, (POINT *)&pt);
			mQueuedDropOps.push_back(DropTargetOp(DropOpType::DragOver, Vector2I((int)pt.x, (int)pt.y)));

			DropTargetOp& op = mQueuedDropOps.Back();
			op.dataType = DropOpDataType::FileList;
			op.mFileList = mFileLists.Back();
		}

		return S_OK;
	}

	HRESULT __stdcall Win32DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;

		if(!mAcceptDrag)
			return S_OK;

		{
			Lock Lock(mSync);

			ScreenToClient(mHWnd, (POINT *)&pt);
			mQueuedDropOps.push_back(DropTargetOp(DropOpType::DragOver, Vector2I((int)pt.x, (int)pt.y)));

			DropTargetOp& op = mQueuedDropOps.Back();
			op.dataType = DropOpDataType::FileList;
			op.mFileList = mFileLists.Back();
		}

		return S_OK;
	}

	HRESULT __stdcall Win32DropTarget::DragLeave()
	{
		{
			Lock Lock(mSync);

			mQueuedDropOps.push_back(DropTargetOp(DropOpType::Leave, Vector2I()));

			DropTargetOp& op = mQueuedDropOps.Back();
			op.dataType = DropOpDataType::FileList;
			op.mFileList = mFileLists.Back();
		}

		return S_OK;
	}

	HRESULT __stdcall Win32DropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
	{
		*pdwEffect = DROPEFFECT_LINK;
		mAcceptDrag = false;

		if(!isDataValid(pDataObj))
			return S_OK;

		{
			Lock Lock(mSync);

			mFileLists.push_back(getFileListFromData(pDataObj));

			ScreenToClient(mHWnd, (POINT *)&pt);
			mQueuedDropOps.push_back(DropTargetOp(DropOpType::Drop, Vector2I((int)pt.x, (int)pt.y)));

			DropTargetOp& op = mQueuedDropOps.Back();
			op.dataType = DropOpDataType::FileList;
			op.mFileList = mFileLists.Back();
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
		if(findIter != mDropTargets.End())
			mDropTargets.Erase(findIter);
	}

	unsigned int Win32DropTarget::GetNumDropTargets() const
	{
		return (unsigned int)mDropTargets.Size();
	}

	void Win32DropTarget::Update()
	{
		Lock Lock(mSync);

		for(auto& op: mQueuedDropOps)
		{
			for(auto& target : mDropTargets)
			{
				if(op.type != DropOpType::Leave)
				{
					if(target->_isInside(op.position))
					{
						if(!target->_isActive())
						{
							target->_setFileList(*op.mFileList);
							target->_setActive(true);
							target->OnEnter(op.position.x, op.position.y);
						}

						if(op.type == DropOpType::DragOver)
							target->OnDragOver(op.position.x, op.position.y);
						else If(op.type == DropOpType::Drop)
						{
							target->_setFileList(*op.mFileList);
							target->OnDrop(op.position.x, op.position.y);
						}
					}
					else
					{
						if(target->_isActive())
						{
							target->OnLeave();
							target->_clear();
							target->_setActive(false);
						}
					}
				}
				else
				{
					if(target->_isActive())
					{
						target->OnLeave();
						target->_clear();
						target->_setActive(false);
					}
				}
			}

			if(op.type == DropOpType::Leave || op.type == DropOpType::Drop)
			{
				while (!mFileLists.Empty())
				{
					bool done = mFileLists[0] == op.mFileList;

					bs_delete(mFileLists[0]);
					mFileLists.Erase(mFileLists.begin());

					if (done)
						break;
				}
			}
		}

		mQueuedDropOps.Clear();
	}

	bool Win32DropTarget::IsDataValid(IDataObject* data)
	{
		// TODO - Currently only supports file drag and drop, so only CF_HDROP is used
		FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

		return data->QueryGetData(&fmtetc) == S_OK ? true : false;
	}

	/**	Gets a file list from data. Caller must ensure that the data actually contains a file list. */
	Vector<Path>* Win32DropTarget::getFileListFromData(IDataObject* data)
	{
		FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		STGMEDIUM stgmed;

		Vector<Path>* files = bs_new<Vector<Path>>();
		if(data->GetData(&fmtetc, &stgmed) == S_OK)
		{
			PVOID data = GlobalLock(stgmed.hGlobal);

			HDROP hDrop = (HDROP)data;
			UINT numFiles = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);

			files->Resize(numFiles);
			for(UINT i = 0; i < numFiles; i++)
			{
				UINT numChars = DragQueryFileW(hDrop, i, nullptr, 0) + 1;
				wchar_t* buffer = (wchar_t*)bs_alloc((UINT32)numChars * sizeof(wchar_t));

				DragQueryFileW(hDrop, i, buffer, numChars);

				(*files)[i] = UTF8::fromWide(WString(buffer));

				bs_free(buffer);
			}

			GlobalUnlock(stgmed.hGlobal);
			ReleaseStgMedium(&stgmed);
		}

		return files;
	}
}

