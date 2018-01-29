//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "CodeEditor/BsMDCodeEditor.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"
#include "String/BsUnicode.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include <Windows.h>
#elif BS_PLATFORM == BS_PLATFORM_LINUX
#include "Private/Linux/BsLinuxPlatform.h"
#endif

namespace bs
{
#if BS_PLATFORM == BS_PLATFORM_WIN32
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
#else


#endif

	MDCodeEditor::MDCodeEditor(const Path& execPath)
		:mExecPath(execPath)
	{
	}

	void MDCodeEditor::openFile(const Path& solutionPath, const Path& filePath, UINT32 lineNumber) const
	{
		WString args = L"--no-splash \"" + solutionPath.toWString() + L"\" \"" + filePath.toWString() + L";" + toWString(lineNumber) + L"\"";

#if BS_PLATFORM == BS_PLATFORM_WIN32
		WString pathStr = mExecPath.toWString();
		ShellExecuteW(0, L"open", pathStr.c_str(), args.c_str(), NULL, SW_HIDE);
#elif BS_PLATFORM == BS_PLATFORM_LINUX
		String narrowArgs = UTF8::fromWide(args);
		pid_t pid = fork();

		if(pid == 0)
		{
			const char* commandPattern = "flatpak run com.xamarin.MonoDevelop %s";

			char* commandStr = (char*) malloc((UINT32) narrowArgs.size() + (UINT32) strlen(commandPattern) + 1);
			sprintf(commandStr, commandPattern, narrowArgs.c_str());

			system(commandStr);
			free(commandStr);

			exit(1);
		}
#endif
	}

	void MDCodeEditor::syncSolution(const CodeSolutionData& data, const Path& outputPath) const
	{
		String solutionString = CSProject::writeSolution(CSProjectVersion::MonoDevelop, data);
		solutionString = StringUtil::replaceAll(solutionString, "\n", "\r\n");
		Path solutionPath = outputPath;
		solutionPath.append(data.name + L".sln");

		for (auto& project : data.projects)
		{
			String projectString = CSProject::writeProject(CSProjectVersion::MonoDevelop, project);
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

	MDCodeEditorFactory::MDCodeEditorFactory()
	{ 
#if BS_PLATFORM == BS_PLATFORM_WIN32

#if BS_ARCH_TYPE == BS_ARCHITECTURE_x86_64
		BOOL is64bit = 1;
#else
		BOOL is64bit = 0;
		
		HANDLE process = GetCurrentProcess();
		IsWow64Process(process, (PBOOL)&is64bit);
#endif
		WString registryKeyRoot;
		if (is64bit)
			registryKeyRoot = L"SOFTWARE\\Wow6432Node\\"; 
		else
			registryKeyRoot = L"SOFTWARE\\";

		WString registryKey = registryKeyRoot + L"\\Xamarin\\XamarinStudio";

		HKEY regKey;
		LONG result = RegOpenKeyExW(HKEY_LOCAL_MACHINE, registryKey.c_str(), 0, KEY_READ, &regKey);
		if (result != ERROR_SUCCESS)
			return;

		WString installPath;
		getRegistryStringValue(regKey, L"Path", installPath, StringUtil::WBLANK);
		if (installPath.empty())
			return;

		mInstallPath = Path(installPath) + Path("XamarinStudio.exe");
#elif BS_PLATFORM == BS_PLATFORM_LINUX
		Path monoDevelopDir = LinuxPlatform::getHomeDir();
		monoDevelopDir.append(".local/share/flatpak/app/com.xamarin.MonoDevelop");

		if(FileSystem::exists(monoDevelopDir))
			mInstallPath = monoDevelopDir;
#endif

		if (!mInstallPath.isEmpty())
			mSupportedEditors.push_back(CodeEditorType::MonoDevelop);
	}

	CodeEditor* MDCodeEditorFactory::create(CodeEditorType type) const
	{
		if (mInstallPath.isEmpty())
			return nullptr;

		if (type != CodeEditorType::MonoDevelop)
			return nullptr;

		return bs_new<MDCodeEditor>(mInstallPath);
	}
}
