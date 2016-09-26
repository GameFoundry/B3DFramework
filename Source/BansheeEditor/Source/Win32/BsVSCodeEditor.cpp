//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Win32/BsVSCodeEditor.h"
#include <windows.h>
#include <atlbase.h>
#include "BsFileSystem.h"
#include "BsDataStream.h"

// Import EnvDTE
#pragma warning(disable: 4278)
#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("8.0") lcid("0") raw_interfaces_only named_guids
#pragma warning(default: 4278)

namespace BansheeEngine
{
	/**
	 * Reads a string value from the specified key in the registry.
	 * 
	 * @param[in]	key				Registry key to read from.
	 * @param[in]	name			Identifier of the value to read from.
	 * @param[in]	value			Output value read from the key.
	 * @param[in]	defaultValue	Default value to return if the key or identifier doesn't exist.
	 */
	LONG getRegistryStringValue(HKEY hKey, const WString& name, WString& value, const WString& defaultValue)
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

	/** Contains various helper classes for interacting with a Visual Studio instance running on this machine. */
	class VisualStudio
	{
	private:
		static const String SLN_TEMPLATE; /**< Template text used for a solution file. */
		static const String PROJ_ENTRY_TEMPLATE; /**< Template text used for a project entry in a solution file. */
		static const String PROJ_PLATFORM_TEMPLATE; /**< Template text used for platform specific information for a project entry in a solution file. */

		static const String PROJ_TEMPLATE; /**< Template XML used for a project file. */
		static const String REFERENCE_ENTRY_TEMPLATE; /**< Template XML used for a reference to another assembly entry by name. */
		static const String REFERENCE_PROJECT_ENTRY_TEMPLATE; /**< Template XML used for a reference to another project entry. */
		static const String REFERENCE_PATH_ENTRY_TEMPLATE; /**< Template XML used for a reference to another assembly entry by name and path. */
		static const String CODE_ENTRY_TEMPLATE; /**< Template XML used for a single code file entry in a project. */
		static const String NON_CODE_ENTRY_TEMPLATE; /**< Template XML used for a single non-code file entry in a project. */

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

		/**	Generates a Visual Studio project GUID from the project name. */
		static String getProjectGUID(const WString& projectName)
		{
			static const String guidTemplate = "{0}-{1}-{2}-{3}-{4}";
			String hash = md5(projectName);

			String output = StringUtil::format(guidTemplate, hash.substr(0, 8),
				hash.substr(8, 4), hash.substr(12, 4), hash.substr(16, 4), hash.substr(20, 12));
			StringUtil::toUpperCase(output);

			return output;
		}

		/**
		 * Builds the Visual Studio solution text (.sln) for the provided version, using the provided solution data.
		 *
		 * @param[in]	version	Visual Studio version for which we're generating the solution file.
		 * @param[in]	data	Data containing a list of projects and other information required to build the solution text.
		 * @return				Generated text of the solution file.
		 */
		static String writeSolution(VisualStudioVersion version, const CodeSolutionData& data)
		{
			struct VersionData
			{
				String formatVersion;
			};

			Map<VisualStudioVersion, VersionData> versionData =
			{
				{ VisualStudioVersion::VS2008, { "10.00" } },
				{ VisualStudioVersion::VS2010, { "11.00" } },
				{ VisualStudioVersion::VS2012, { "12.00" } },
				{ VisualStudioVersion::VS2013, { "12.00" } },
				{ VisualStudioVersion::VS2015, { "12.00" } }
			};

			StringStream projectEntriesStream;
			StringStream projectPlatformsStream;
			for (auto& project : data.projects)
			{
				String guid = getProjectGUID(project.name);
				String projectName = toString(project.name);

				projectEntriesStream << StringUtil::format(PROJ_ENTRY_TEMPLATE, projectName, projectName + ".csproj", guid);
				projectPlatformsStream << StringUtil::format(PROJ_PLATFORM_TEMPLATE, guid);
			}

			String projectEntries = projectEntriesStream.str();
			String projectPlatforms = projectPlatformsStream.str();

			return StringUtil::format(SLN_TEMPLATE, versionData[version].formatVersion, projectEntries, projectPlatforms);
		}

		/**
		 * Builds the Visual Studio project text (.csproj) for the provided version, using the provided project data.
		 *
		 * @param[in]	version		Visual Studio version for which we're generating the project file.
		 * @param[in]	projectData	Data containing a list of files, references and other information required to 
		 *							build the project text.
		 * @return					Generated text of the project file.
		 */
		static String writeProject(VisualStudioVersion version, const CodeProjectData& projectData)
		{
			struct VersionData
			{
				String toolsVersion;
			};

			Map<VisualStudioVersion, VersionData> versionData =
			{
				{ VisualStudioVersion::VS2008, { "3.5" } },
				{ VisualStudioVersion::VS2010, { "4.0" } },
				{ VisualStudioVersion::VS2012, { "4.0" } },
				{ VisualStudioVersion::VS2013, { "12.0" } },
				{ VisualStudioVersion::VS2015, { "13.0" } }
			};

			StringStream tempStream;
			for (auto& codeEntry : projectData.codeFiles)
				tempStream << StringUtil::format(CODE_ENTRY_TEMPLATE, codeEntry.toString());

			String codeEntries = tempStream.str();
			tempStream.str("");
			tempStream.clear();

			for (auto& nonCodeEntry : projectData.nonCodeFiles)
				tempStream << StringUtil::format(NON_CODE_ENTRY_TEMPLATE, nonCodeEntry.toString());

			String nonCodeEntries = tempStream.str();
			tempStream.str("");
			tempStream.clear();

			for (auto& referenceEntry : projectData.assemblyReferences)
			{
				String referenceName = toString(referenceEntry.name);

				if (referenceEntry.path.isEmpty())
					tempStream << StringUtil::format(REFERENCE_ENTRY_TEMPLATE, referenceName);
				else
					tempStream << StringUtil::format(REFERENCE_PATH_ENTRY_TEMPLATE, referenceName, referenceEntry.path.toString());
			}

			String referenceEntries = tempStream.str();
			tempStream.str("");
			tempStream.clear();

			for (auto& referenceEntry : projectData.projectReferences)
			{
				String referenceName = toString(referenceEntry.name);
				String projectGUID = getProjectGUID(referenceEntry.name);

				tempStream << StringUtil::format(REFERENCE_PROJECT_ENTRY_TEMPLATE, referenceName, projectGUID);
			}

			String projectReferenceEntries = tempStream.str();
			tempStream.str("");
			tempStream.clear();

			tempStream << toString(projectData.defines);

			String defines = tempStream.str();
			String projectGUID = getProjectGUID(projectData.name);

			return StringUtil::format(PROJ_TEMPLATE, versionData[version].toolsVersion, projectGUID, 
				toString(projectData.name), defines, referenceEntries, projectReferenceEntries, codeEntries, nonCodeEntries);
		}
	};

	const String VisualStudio::SLN_TEMPLATE =
		R"(Microsoft Visual Studio Solution File, Format Version {0}
# Visual Studio 2013
VisualStudioVersion = 12.0.30723.0
MinimumVisualStudioVersion = 10.0.40219.1{1}
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Release|Any CPU = Release|Any CPU
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution{2}
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
EndGlobal
)";

	const String VisualStudio::PROJ_ENTRY_TEMPLATE =
		R"(
Project("\{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC\}") = "{0}", "{1}", "\{{2}\}"
EndProject)";

	const String VisualStudio::PROJ_PLATFORM_TEMPLATE =
		R"(
		\{{0}\}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		\{{0}\}.Debug|Any CPU.Build.0 = Debug|Any CPU
		\{{0}\}.Release|Any CPU.ActiveCfg = Release|Any CPU
		\{{0}\}.Release|Any CPU.Build.0 = Release|Any CPU)";

	const String VisualStudio::PROJ_TEMPLATE =
		R"literal(<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="{0}" DefaultTargets="Build" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <Import Project="$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props" Condition="Exists('$(MSBuildExtensionsPath)\\$(MSBuildToolsVersion)\\Microsoft.Common.props')" />
  <PropertyGroup>
    <Configuration Condition = " '$(Configuration)' == '' ">Debug</Configuration>
    <Platform Condition = " '$(Platform)' == '' ">AnyCPU</Platform>
    <ProjectGuid>\{{1}\}</ProjectGuid>
    <OutputType>Library</OutputType>
    <AppDesignerFolder>Properties</AppDesignerFolder>
    <RootNamespace></RootNamespace>
    <AssemblyName>{2}</AssemblyName>
    <TargetFrameworkVersion>v4.0</TargetFrameworkVersion>
    <FileAlignment>512</FileAlignment>
    <BaseDirectory>Resources</BaseDirectory>
    <SchemaVersion>2.0</SchemaVersion>
  </PropertyGroup>
  <PropertyGroup Condition = " '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' ">
    <DebugSymbols>true</DebugSymbols>
    <DebugType>full</DebugType>
    <Optimize>false</Optimize>
    <OutputPath>Internal\\Temp\\Assemblies\\Debug\\</OutputPath>
    <BaseIntermediateOutputPath>Internal\\Temp\\Assemblies\\</BaseIntermediateOutputPath>
    <DefineConstants>DEBUG;TRACE;{3}</DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel >
  </PropertyGroup>
  <PropertyGroup Condition = " '$(Configuration)|$(Platform)' == 'Release|AnyCPU' ">
    <DebugType>pdbonly</DebugType>
    <Optimize>true</Optimize>
    <OutputPath>Internal\\Temp\\Assemblies\\Release\\</OutputPath>
	<BaseIntermediateOutputPath>Internal\\Temp\\Assemblies\\</BaseIntermediateOutputPath>
    <DefineConstants>TRACE;{3}</DefineConstants>
    <ErrorReport>prompt</ErrorReport>
    <WarningLevel>4</WarningLevel>
  </PropertyGroup>
  <ItemGroup>{4}
  </ItemGroup>
  <ItemGroup>{5}
  </ItemGroup>
  <ItemGroup>{6}
  </ItemGroup>
  <ItemGroup>{7}
  </ItemGroup>
  <Import Project = "$(MSBuildToolsPath)\\Microsoft.CSharp.targets"/>
</Project>)literal";

	const String VisualStudio::REFERENCE_ENTRY_TEMPLATE =
		R"(
    <Reference Include="{0}"/>)";

	const String VisualStudio::REFERENCE_PATH_ENTRY_TEMPLATE =
		R"(
    <Reference Include="{0}">
      <HintPath>{1}</HintPath>
    </Reference>)";

	const String VisualStudio::REFERENCE_PROJECT_ENTRY_TEMPLATE =
		R"(
    <ProjectReference Include="{0}.csproj">
      <Project>\{{1}\}</Project>
      <Name>{0}</Name>
    </ProjectReference>)";

	const String VisualStudio::CODE_ENTRY_TEMPLATE =
		R"(
    <Compile Include="{0}"/>)";

	const String VisualStudio::NON_CODE_ENTRY_TEMPLATE =
		R"(
    <None Include="{0}"/>)";

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
		String solutionString = VisualStudio::writeSolution(mVersion, data);
		solutionString = StringUtil::replaceAll(solutionString, "\n", "\r\n");
		Path solutionPath = outputPath;
		solutionPath.append(data.name + L".sln");

		for (auto& project : data.projects)
		{
			String projectString = VisualStudio::writeProject(mVersion, project);
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

		return versionInfo;
	}

	CodeEditor* VSCodeEditorFactory::create(CodeEditorType type) const
	{
		auto findIter = mAvailableVersions.find(type);
		if (findIter == mAvailableVersions.end())
			return nullptr;

		// TODO - Also create VSExpress and VSCommunity editors

		return bs_new<VSCodeEditor>(findIter->second.version, findIter->second.execPath, findIter->second.CLSID);
	}
}