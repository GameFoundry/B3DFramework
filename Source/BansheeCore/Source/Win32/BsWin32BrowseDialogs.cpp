//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCorePrerequisites.h"
#include "BsPlatform.h"
#include "BsAsyncOp.h"
#include "BsCoreThread.h"
#include "Win32/BsWin32Window.h"
#include <ShObjIdl.h>

using namespace std::placeholders;

namespace bs
{
	void addFiltersToDialog(IFileDialog* fileDialog, const WString& filterList)
	{
		const wchar_t EMPTY_WSTR[] = L"";

		if (filterList.empty())
			return;

		Vector<WString> filters = StringUtil::split(filterList, L";");
		UINT32 numFilters = (UINT32)filters.size();
		if (numFilters == 0)
			return;

		COMDLG_FILTERSPEC* specList = bs_newN<COMDLG_FILTERSPEC>(numFilters);
		for (size_t i = 0; i < numFilters; ++i)
		{
			specList[i].pszName = EMPTY_WSTR;
			specList[i].pszSpec = filters[i].c_str();
		}

		fileDialog->SetFileTypes(numFilters, specList);

		bs_deleteN(specList, numFilters);
	}

	void setDefaultPath(IFileDialog* dialog, const Path& defaultPath)
	{
		WString pathStr = defaultPath.toWString();
		const wchar_t* defaultPathW = pathStr.c_str();

		IShellItem* folder;
		HRESULT result = SHCreateItemFromParsingName(defaultPathW, NULL, IID_PPV_ARGS(&folder));

		// Valid non results.
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) || result == HRESULT_FROM_WIN32(ERROR_INVALID_DRIVE))
			return;

		if (!SUCCEEDED(result))
			return;

		dialog->SetFolder(folder);
		folder->Release();
	}

	void getPaths(IShellItemArray* shellItems, Vector<Path>& paths)
	{
		DWORD numShellItems;
		shellItems->GetCount(&numShellItems);

		for (DWORD i = 0; i < numShellItems; ++i)
		{
			IShellItem* shellItem = nullptr;
			shellItems->GetItemAt(i, &shellItem);

			SFGAOF attribs;
			shellItem->GetAttributes(SFGAO_FILESYSTEM, &attribs);

			if (!(attribs & SFGAO_FILESYSTEM))
				continue;

			LPWSTR name;
			shellItem->GetDisplayName(SIGDN_FILESYSPATH, &name);
			paths.push_back((Path)name);
			CoTaskMemFree(name);
		}
	}

	void openBrowseDialogCore(FileDialogType type, const Path& defaultPath, const WString& filterList,
		Vector<Path>& paths, AsyncOp& returnValue)
	{
		// Init COM library.
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		IFileDialog* fileDialog = nullptr;

		UINT32 dialogType = ((UINT32)type & (UINT32)FileDialogType::TypeMask);
		bool isOpenDialog = dialogType == (UINT32)FileDialogType::OpenFile || dialogType == (UINT32)FileDialogType::OpenFolder;

		// Create dialog
		IID classId = isOpenDialog ? CLSID_FileOpenDialog : CLSID_FileSaveDialog;
		CoCreateInstance(classId, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&fileDialog));

		addFiltersToDialog(fileDialog, filterList);
		setDefaultPath(fileDialog, defaultPath);

		// Apply multiselect flags
		bool isMultiselect = false;
		if (isOpenDialog)
		{
			if (dialogType == (UINT32)FileDialogType::OpenFile)
			{
				if (((UINT32)type & (UINT32)FileDialogType::Multiselect) != 0)
				{
					DWORD dwFlags;
					fileDialog->GetOptions(&dwFlags);
					fileDialog->SetOptions(dwFlags | FOS_ALLOWMULTISELECT);

					isMultiselect = true;
				}
			}
			else
			{
				DWORD dwFlags;
				fileDialog->GetOptions(&dwFlags);
				fileDialog->SetOptions(dwFlags | FOS_PICKFOLDERS);
			}
		}

		// Show the dialog
		bool finalResult = false;

		// Need to enable all windows, otherwise when the browse dialog closes the active window will become some 
		// background window
		Win32Window::_enableAllWindows();

		if (SUCCEEDED(fileDialog->Show(nullptr)))
		{
			if (isMultiselect)
			{
				// Get file names
				IFileOpenDialog* fileOpenDialog;
				fileDialog->QueryInterface(IID_IFileOpenDialog, (void**)&fileOpenDialog);
				
				IShellItemArray* shellItems = nullptr;
				fileOpenDialog->GetResults(&shellItems);

				getPaths(shellItems, paths);
				shellItems->Release();
				fileOpenDialog->Release();
			}
			else
			{
				// Get the file name
				IShellItem* shellItem = nullptr;
				fileDialog->GetResult(&shellItem);

				LPWSTR filePath = nullptr;
				shellItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);

				paths.push_back((Path)filePath);
				CoTaskMemFree(filePath);

				shellItem->Release();
			}

			finalResult = true;
		}

		// Restore modal window state (before we enabled all windows)
		Win32Window::_restoreModalWindows();

		CoUninitialize();

		returnValue._completeOperation(finalResult);
	}

	bool Platform::openBrowseDialog(FileDialogType type, const Path& defaultPath, const WString& filterList,
		Vector<Path>& paths)
	{
		AsyncOp returnValue = gCoreThread().queueReturnCommand(std::bind(&openBrowseDialogCore, type, 
			std::cref(defaultPath), std::cref(filterList), std::ref(paths), _1), 
			CTQF_InternalQueue | CTQF_BlockUntilComplete);

		return returnValue.getReturnValue<bool>();
	}
}