//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Win32/BsVSCodeEditor.h"
#include <windows.h>
#include <atlbase.h>
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"

// Import EnvDTE
// Uncomment this code to generate the dte80a.tlh file below. We're not using #import directly because it is not
// compatible with multi-processor compilation of the build
//#pragma warning(disable: 4278)
//#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("8.0") lcid("0") raw_interfaces_only named_guids
//#pragma warning(default: 4278)

#include "Win32/dte80a.tlh"

#include "Win32/Setup.Configuration.h"
_COM_SMARTPTR_TYPEDEF(ISetupInstance, __uuidof(ISetupInstance));
_COM_SMARTPTR_TYPEDEF(ISetupInstance2, __uuidof(ISetupInstance2));
_COM_SMARTPTR_TYPEDEF(IEnumSetupInstances, __uuidof(IEnumSetupInstances));
_COM_SMARTPTR_TYPEDEF(ISetupConfiguration, __uuidof(ISetupConfiguration));
_COM_SMARTPTR_TYPEDEF(ISetupConfiguration2, __uuidof(ISetupConfiguration2));
_COM_SMARTPTR_TYPEDEF(ISetupHelper, __uuidof(ISetupHelper));
_COM_SMARTPTR_TYPEDEF(ISetupPackageReference, __uuidof(ISetupPackageReference));
_COM_SMARTPTR_TYPEDEF(ISetupPropertyStore, __uuidof(ISetupPropertyStore));
_COM_SMARTPTR_TYPEDEF(ISetupInstanceCatalog, __uuidof(ISetupInstanceCatalog));

namespace bs
{
	/**
	 * Reads a string value from the specified key in the registry.
	 * 
	 * @param[in]	key				Registry key to read from.
	 * @param[in]	name			Identifier of the value to read from.
	 * @param[in]	value			Output value read from the key.
	 * @param[in]	defaultValue	Default value to return if the key or identifier doesn't exist.
	 */
	static LONG getRegistryStringValue(HKEY hKey, const WString& name, WString& value, const WString& defaultValue)
	{
		value = defaultValue;

		wchar_t strBuffer[512];
		DWORD strBufferSize = sizeof(strBuffer);
		ULONG result = RegQueryValueExW(hKey, name.c_str(), 0, nullptr, (LPBYTE)strBuffer, &strBufferSize);
		if (result == ERROR_SUCCESS)
			value = strBuffer;

		return result;
	}

	/** Contains data about a Visual Studio project. */
	struct VSProjectInfo
	{
		WString GUID;
		WString name;
		Path path;
	};

	/**
	 * Handles retrying of calls that fail to access Visual Studio. This is due to the weird nature of VS when calling its
	 * methods from external code. If this message filter isn't registered some calls will just fail silently.
	 */
	class VSMessageFilter : public IMessageFilter
	{
		DWORD __stdcall HandleInComingCall(DWORD dwCallType, HTASK htaskCaller, DWORD dwTickCount, LPINTERFACEINFO lpInterfaceInfo) override
		{
			return SERVERCALL_ISHANDLED;
		}

		DWORD __stdcall RetryRejectedCall(HTASK htaskCallee, DWORD dwTickCount, DWORD dwRejectType) override
		{
			if (dwRejectType == SERVERCALL_RETRYLATER)
			{
				// Retry immediatey
				return 99;
			}
			// Cancel the call
			return -1;
		}

		DWORD __stdcall MessagePending(HTASK htaskCallee, DWORD dwTickCount, DWORD dwPendingType) override
		{
			return PENDINGMSG_WAITDEFPROCESS;
		}

		/**	COM requirement. Returns instance of an interface of provided type. */
		HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override
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

		/** COM requirement. Increments objects reference count. */
		ULONG __stdcall AddRef() override
		{
			return InterlockedIncrement(&mRefCount);
		} 

		/**	COM requirement. Decreases the objects reference count and deletes the object if its zero. */
		ULONG __stdcall Release() override
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

	private:
		LONG mRefCount;
	};

	/** Contains various helper functionality for interacting with a Visual Studio instance running on this machine. */
	class VisualStudio
	{
	public:
		/**
		 * Scans the running processes to find a running Visual Studio instance with the specified version and open solution.
		 *
		 * @param[in]	clsID			Class ID of the specific Visual Studio version we are looking for.
		 * @param[in]	solutionPath	Path to the solution the instance needs to have open.
		 * @return						DTE object that may be used to interact with the Visual Studio instance, or null if
		 *								not found.
		 */
		static CComPtr<EnvDTE::_DTE> findRunningInstance(const CLSID& clsID, const Path& solutionPath)
		{
			CComPtr<IRunningObjectTable> runningObjectTable = nullptr;
			if (FAILED(GetRunningObjectTable(0, &runningObjectTable)))
				return nullptr;

			CComPtr<IEnumMoniker> enumMoniker = nullptr;
			if (FAILED(runningObjectTable->EnumRunning(&enumMoniker)))
				return nullptr;

			CComPtr<IMoniker> dteMoniker = nullptr;
			if (FAILED(CreateClassMoniker(clsID, &dteMoniker)))
				return nullptr;

			CComBSTR bstrSolution(solutionPath.toWString(Path::PathType::Windows).c_str());
			CComPtr<IMoniker> moniker;
			ULONG count = 0;
			while (enumMoniker->Next(1, &moniker, &count) == S_OK)
			{
				if (moniker->IsEqual(dteMoniker))
				{
					CComPtr<IUnknown> curObject = nullptr;
					HRESULT result = runningObjectTable->GetObject(moniker, &curObject);
					moniker = nullptr;

					if (result != S_OK)
						continue;

					CComPtr<EnvDTE::_DTE> dte;
					curObject->QueryInterface(__uuidof(EnvDTE::_DTE), (void**)&dte);

					if (dte == nullptr)
						continue;

					CComPtr<EnvDTE::_Solution> solution;
					if (FAILED(dte->get_Solution(&solution)))
						continue;

					CComBSTR fullName;
					if (FAILED(solution->get_FullName(&fullName)))
						continue;

					if (fullName == bstrSolution)
						return dte;
				}
			}

			return nullptr;
		}

		/**
		 * Opens a new Visual Studio instance of the specified version with the provided solution.
		 *
		 * @param[in]	clsID			Class ID of the specific Visual Studio version to start.
		 * @param[in]	solutionPath	Path to the solution the instance needs to open.
		 */
		static CComPtr<EnvDTE::_DTE> openInstance(const CLSID& clsid, const Path& solutionPath)
		{
			CComPtr<IUnknown> newInstance = nullptr;
			if (FAILED(::CoCreateInstance(clsid, nullptr, CLSCTX_LOCAL_SERVER, EnvDTE::IID__DTE, (LPVOID*)&newInstance)))
				return nullptr;

			CComPtr<EnvDTE::_DTE> dte;
			newInstance->QueryInterface(__uuidof(EnvDTE::_DTE), (void**)&dte);

			if (dte == nullptr)
				return nullptr;

			dte->put_UserControl(TRUE);

			CComPtr<EnvDTE::_Solution> solution;
			if (FAILED(dte->get_Solution(&solution)))
				return nullptr;

			CComBSTR bstrSolution(solutionPath.toWString(Path::PathType::Windows).c_str());
			if (FAILED(solution->Open(bstrSolution)))
				return nullptr;

			// Wait until VS opens
			UINT32 elapsed = 0;
			while (elapsed < 10000)
			{
				EnvDTE::Window* window = nullptr;
				if (SUCCEEDED(dte->get_MainWindow(&window)))
					return dte;

				Sleep(100);
				elapsed += 100;
			}

			return nullptr;
		}

		/**
		 * Opens a file on a specific line in a running Visual Studio instance.
		 *
		 * @param[in]	dte			DTE object retrieved from findRunningInstance() or openInstance().
		 * @param[in]	filePath	Path of the file to open. File should be a part of the VS solution.
		 * @param[in]	line		Line on which to focus Visual Studio after the file is open.
		 */
		static bool openFile(CComPtr<EnvDTE::_DTE> dte, const Path& filePath, UINT32 line)
		{
			// Open file
			CComPtr<EnvDTE::ItemOperations> itemOperations;
			if (FAILED(dte->get_ItemOperations(&itemOperations)))
				return false;

			CComBSTR bstrFilePath(filePath.toWString(Path::PathType::Windows).c_str());
			CComBSTR bstrKind(EnvDTE::vsViewKindPrimary);
			CComPtr<EnvDTE::Window> window = nullptr;
			if (FAILED(itemOperations->OpenFile(bstrFilePath, bstrKind, &window)))
				return false;

			// Scroll to line
			CComPtr<EnvDTE::Document> activeDocument;
			if (SUCCEEDED(dte->get_ActiveDocument(&activeDocument)))
			{
				CComPtr<IDispatch> selection;
				if (SUCCEEDED(activeDocument->get_Selection(&selection)))
				{
					CComPtr<EnvDTE::TextSelection> textSelection;
					if (SUCCEEDED(selection->QueryInterface(&textSelection)))
					{
						textSelection->GotoLine(line, TRUE);
					}
				}
			}

			// Bring the window in focus
			window = nullptr;
			if (SUCCEEDED(dte->get_MainWindow(&window)))
			{
				window->Activate();

				HWND hWnd;
				window->get_HWnd((LONG*)&hWnd);
				SetForegroundWindow(hWnd);
			}

			return true;
		}
	};

	VSCodeEditor::VSCodeEditor(VisualStudioVersion version, const Path& execPath, const WString& CLSID)
		:mVersion(version), mExecPath(execPath), mCLSID(CLSID)
	{
		
	}

	void VSCodeEditor::openFile(const Path& solutionPath, const Path& filePath, UINT32 lineNumber) const
	{
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		CLSID clsID;
		if (FAILED(CLSIDFromString(mCLSID.c_str(), &clsID)))
		{
			CoUninitialize();
			return;
		}

		CComPtr<EnvDTE::_DTE> dte = VisualStudio::findRunningInstance(clsID, solutionPath);
		if (dte == nullptr)
			dte = VisualStudio::openInstance(clsID, solutionPath);

		if (dte == nullptr)
		{
			CoUninitialize();
			return;
		}

		VSMessageFilter* newFilter = new VSMessageFilter();
		IMessageFilter* oldFilter;

		CoRegisterMessageFilter(newFilter, &oldFilter);
		EnvDTE::Window* window = nullptr;
		if (SUCCEEDED(dte->get_MainWindow(&window)))
			window->Activate();

		VisualStudio::openFile(dte, filePath, lineNumber);
		CoRegisterMessageFilter(oldFilter, nullptr);

		CoUninitialize();
	}

	void VSCodeEditor::syncSolution(const CodeSolutionData& data, const Path& outputPath) const
	{
		CSProjectVersion csProjVer;
		switch(mVersion)
		{
		case VisualStudioVersion::VS2008: csProjVer = CSProjectVersion::VS2008; break;
		case VisualStudioVersion::VS2010: csProjVer = CSProjectVersion::VS2010; break;
		case VisualStudioVersion::VS2012: csProjVer = CSProjectVersion::VS2012; break;
		case VisualStudioVersion::VS2013: csProjVer = CSProjectVersion::VS2013; break;
		case VisualStudioVersion::VS2015: csProjVer = CSProjectVersion::VS2015; break;
		default:
		case VisualStudioVersion::VS2017: csProjVer = CSProjectVersion::VS2017;  break;
		}

		String solutionString = CSProject::writeSolution(csProjVer, data);
		solutionString = StringUtil::replaceAll(solutionString, "\n", "\r\n");
		Path solutionPath = outputPath;
		solutionPath.append(data.name + L".sln");

		for (auto& project : data.projects)
		{
			String projectString = CSProject::writeProject(csProjVer, project);
			projectString = StringUtil::replaceAll(projectString, "\n", "\r\n");

			Path projectPath = outputPath;
			projectPath.append(project.name + L".csproj");

			SPtr<DataStream> projectStream = FileSystem::createAndOpenFile(projectPath);
			projectStream->write(projectString.c_str(), projectString.size() * sizeof(String::value_type));
			projectStream->close();
		}

		SPtr<DataStream> solutionStream = FileSystem::createAndOpenFile(solutionPath);
		solutionStream->write(solutionString.c_str(), solutionString.size() * sizeof(String::value_type));
		solutionStream->close();
	}

	VSCodeEditorFactory::VSCodeEditorFactory()
		:mAvailableVersions(getAvailableVersions())
	{ 
		for (auto& version : mAvailableVersions)
			mAvailableEditors.push_back(version.first);
	}

	Map<CodeEditorType, VSCodeEditorFactory::VSVersionInfo> VSCodeEditorFactory::getAvailableVersions() const
	{
#if BS_ARCH_TYPE == BS_ARCHITECTURE_x86_64
		BOOL is64bit = 1;
#else
		BOOL is64bit = 0;
		
		HANDLE process = GetCurrentProcess();
		IsWow64Process(process, (PBOOL)&is64bit);
#endif
		
		WString registryKeyRoot;
		if (is64bit)
			registryKeyRoot = L"SOFTWARE\\Wow6432Node\\Microsoft"; 
		else
			registryKeyRoot = L"SOFTWARE\\Microsoft";

		struct VersionData
		{
			CodeEditorType type;
			WString registryKey;
			WString name;
			WString executable;
		};

		// Pre-2017 versions all have registry entries we can search
		Map<VisualStudioVersion, VersionData> versionToVersionNumber =
		{ 
			{ VisualStudioVersion::VS2008, { CodeEditorType::VS2008, L"VisualStudio\\9.0", L"Visual Studio 2008", L"devenv.exe" } },
			{ VisualStudioVersion::VS2010, { CodeEditorType::VS2010, L"VisualStudio\\10.0", L"Visual Studio 2010", L"devenv.exe" } },
			{ VisualStudioVersion::VS2012, { CodeEditorType::VS2012, L"VisualStudio\\11.0", L"Visual Studio 2012", L"devenv.exe" } },
			{ VisualStudioVersion::VS2013, { CodeEditorType::VS2013, L"VisualStudio\\12.0", L"Visual Studio 2013", L"devenv.exe" } },
			{ VisualStudioVersion::VS2015, { CodeEditorType::VS2015, L"VisualStudio\\14.0", L"Visual Studio 2015", L"devenv.exe" } }
		};

		Map<CodeEditorType, VSVersionInfo> versionInfo;
		for(auto version : versionToVersionNumber)
		{
			WString registryKey = registryKeyRoot + L"\\" + version.second.registryKey;

			HKEY regKey;
			LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, registryKey.c_str(), 0, KEY_READ, &regKey);
			if (result != ERROR_SUCCESS)
				continue;

			WString installPath;
			getRegistryStringValue(regKey, L"InstallDir", installPath, StringUtil::WBLANK);
			if (installPath.empty())
				continue;

			WString clsID;
			getRegistryStringValue(regKey, L"ThisVersionDTECLSID", clsID, StringUtil::WBLANK);

			VSVersionInfo info;
			info.name = version.second.name;
			info.execPath = installPath.append(version.second.executable);
			info.CLSID = clsID;
			info.version = version.first;

			versionInfo[version.second.type] = info;
		}

		// 2017 and later need to be queried through Setup Config
		CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		ISetupConfigurationPtr query;
		auto hr = query.CreateInstance(__uuidof(SetupConfiguration));
		if (hr == REGDB_E_CLASSNOTREG || FAILED(hr))
		{
			CoUninitialize();
			return versionInfo;
		}

		ISetupConfiguration2Ptr query2(query);
		IEnumSetupInstancesPtr e;

		hr = query2->EnumAllInstances(&e);
		if (FAILED(hr))
		{
			CoUninitialize();
			return versionInfo;
		}

		ISetupHelperPtr helper(query);

		ISetupInstance* pInstances[1] = {};
		hr = e->Next(1, pInstances, nullptr);
		while (hr == S_OK)
		{
			ISetupInstancePtr instance(pInstances[0], false);

			bstr_t bstrName;
			hr = instance->GetDisplayName(0, bstrName.GetAddress());

			if (FAILED(hr))
			{
				hr = e->Next(1, pInstances, nullptr);
				continue;
			}

			bstr_t bstrVersion;
			hr = instance->GetInstallationVersion(bstrVersion.GetAddress());

			if (FAILED(hr))
			{
				hr = e->Next(1, pInstances, nullptr);
				continue;
			}

			bstr_t bstrInstallationPath;
			hr = instance->GetInstallationPath(bstrInstallationPath.GetAddress());
			if (FAILED(hr))
			{
				hr = e->Next(1, pInstances, nullptr);
				continue;
			}

			Vector<WString> versionBits = StringUtil::split(WString(bstrVersion), L".");
			if (versionBits.size() < 1)
			{
				hr = e->Next(1, pInstances, nullptr);
				continue;
			}

			// VS2017
			if (versionBits[0] == L"15")
			{
				VSVersionInfo info;
				info.name = WString(bstrName);
				info.execPath = Path(WString(bstrInstallationPath)) + Path(L"Common7/IDE/devenv.exe");
				info.version = VisualStudioVersion::VS2017;
				info.CLSID = L"{3829D1F4-A427-4C75-B63C-7ABB7521B225}";

				versionInfo[CodeEditorType::VS2017] = info; 
			}
			
			hr = e->Next(1, pInstances, nullptr);
		}

		CoUninitialize();
		return versionInfo;
	}

	CodeEditor* VSCodeEditorFactory::create(CodeEditorType type) const
	{
		auto findIter = mAvailableVersions.find(type);
		if (findIter == mAvailableVersions.end())
			return nullptr;

		return bs_new<VSCodeEditor>(findIter->second.version, findIter->second.execPath, findIter->second.CLSID);
	}
}