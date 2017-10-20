//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "CodeEditor/BsCodeEditor.h"
#include "BsEditorApplication.h"
#include "Library/BsProjectLibrary.h"
#include "Library/BsProjectResourceMeta.h"
#include "Resources/BsScriptCodeImportOptions.h"
#include "Build/BsBuildManager.h"
#include "CodeEditor/BsMDCodeEditor.h"

#if BS_PLATFORM == BS_PLATFORM_WIN32
#include "Win32/BsVSCodeEditor.h"
#else
// Add implementations for code editors on other platforms.
#endif

namespace bs
{
	CodeEditorManager::CodeEditorManager()
		:mActiveEditor(nullptr), mActiveEditorType(CodeEditorType::None)
	{
#if BS_PLATFORM == BS_PLATFORM_WIN32
		VSCodeEditorFactory* vsCodeEditorFactory = bs_new<VSCodeEditorFactory>();
		Vector<CodeEditorType> vsEditors = vsCodeEditorFactory->getAvailableEditors();
		for(auto& editor : vsEditors)
		{
			mFactoryPerEditor[editor] = vsCodeEditorFactory;
			mEditors.push_back(editor);
		}

		mFactories.push_back(vsCodeEditorFactory);
#else
		// Add implementations for code editors on other platforms.
#endif

		MDCodeEditorFactory* mdCodeEditorFactory = bs_new<MDCodeEditorFactory>();
		Vector<CodeEditorType> mdEditors = mdCodeEditorFactory->getAvailableEditors();

		for(auto& editor : mdEditors)
		{
			mFactoryPerEditor[editor] = mdCodeEditorFactory;
			mEditors.push_back(editor);
		}

		mFactories.push_back(mdCodeEditorFactory);
	}

	CodeEditorManager::~CodeEditorManager()
	{
		for (auto& factory : mFactories)
			bs_delete(factory);

		if (mActiveEditor != nullptr)
			bs_delete(mActiveEditor);
	}

	void CodeEditorManager::setActive(CodeEditorType editor)
	{
		if (mActiveEditor != nullptr)
		{
			bs_delete(mActiveEditor);
			mActiveEditor = nullptr;
		}

		auto findIter = mFactoryPerEditor.find(editor);
		if (findIter == mFactoryPerEditor.end())
			return;

		mActiveEditor = findIter->second->create(editor);
		mActiveEditorType = editor;
	}

	void CodeEditorManager::openFile(const Path& path, UINT32 lineNumber) const
	{
		if (mActiveEditor == nullptr)
			return;

		Path filePath = path;
		if (!path.isAbsolute())
			filePath.makeAbsolute(gProjectLibrary().getResourcesFolder());

		mActiveEditor->openFile(getSolutionPath(), filePath, lineNumber);
	}

	void CodeEditorManager::syncSolution() const
	{
		if (mActiveEditor == nullptr)
			return;

		CodeSolutionData slnData;
		slnData.name = gEditorApplication().getProjectName();

		Vector<UINT32> scriptTypeIds =
		{
			TID_ScriptCode, TID_PlainText, TID_Shader, TID_ShaderInclude
		};

		Vector<ProjectLibrary::LibraryEntry*> libraryEntries = gProjectLibrary().search(L"*", scriptTypeIds);
		
		PlatformType activePlatform = BuildManager::instance().getActivePlatform();
		Vector<WString> frameworkAssemblies = BuildManager::instance().getFrameworkAssemblies(activePlatform);

		slnData.projects.push_back(CodeProjectData());
		slnData.projects.push_back(CodeProjectData());

		// Game project
		CodeProjectData& gameProject = slnData.projects[0];
		gameProject.name = toWString(SCRIPT_GAME_ASSEMBLY);
		gameProject.defines = BuildManager::instance().getDefines(activePlatform);
		
		//// Add references
		gameProject.assemblyReferences.push_back(CodeProjectReference{ toWString(ENGINE_ASSEMBLY), gApplication().getEngineAssemblyPath() });
		for (auto& assemblyName : frameworkAssemblies)
			gameProject.assemblyReferences.push_back(CodeProjectReference{ assemblyName, Path::BLANK });

		// Editor project
		CodeProjectData& editorProject = slnData.projects[1];
		editorProject.name = toWString(SCRIPT_EDITOR_ASSEMBLY);

		//// Add references
		editorProject.assemblyReferences.push_back(CodeProjectReference{ toWString(ENGINE_ASSEMBLY), gApplication().getEngineAssemblyPath() });
		editorProject.assemblyReferences.push_back(CodeProjectReference{ toWString(EDITOR_ASSEMBLY), gEditorApplication().getEditorAssemblyPath() });
		for (auto& assemblyName : frameworkAssemblies)
			gameProject.assemblyReferences.push_back(CodeProjectReference{ assemblyName, Path::BLANK });

		editorProject.projectReferences.push_back(CodeProjectReference{ gameProject.name, Path::BLANK });

		//// Add files for both projects
		for (auto& entry : libraryEntries)
		{
			if (entry->type != ProjectLibrary::LibraryEntryType::File)
				continue;

			ProjectLibrary::FileEntry* resEntry = static_cast<ProjectLibrary::FileEntry*>(entry);
			if (resEntry->meta->hasTypeId(TID_ScriptCode))
			{
				SPtr<ScriptCodeImportOptions> scriptIO = std::static_pointer_cast<ScriptCodeImportOptions>(resEntry->meta->getImportOptions());

				bool isEditorScript = false;
				if (scriptIO != nullptr)
					isEditorScript = scriptIO->isEditorScript();

				if (isEditorScript)
					editorProject.codeFiles.push_back(resEntry->path);
				else
					gameProject.codeFiles.push_back(resEntry->path);
			}
			else
				gameProject.nonCodeFiles.push_back(resEntry->path);
		}

		mActiveEditor->syncSolution(slnData, gEditorApplication().getProjectPath());
	}

	Path CodeEditorManager::getSolutionPath() const
	{
		Path path = gEditorApplication().getProjectPath();
		path.append(gEditorApplication().getProjectName() + L".sln");

		return path;
	}

	const String CSProject::SLN_TEMPLATE =
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

	const String CSProject::PROJ_ENTRY_TEMPLATE =
		R"(
Project("\{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC\}") = "{0}", "{1}", "\{{2}\}"
EndProject)";

	const String CSProject::PROJ_PLATFORM_TEMPLATE =
		R"(
		\{{0}\}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		\{{0}\}.Debug|Any CPU.Build.0 = Debug|Any CPU
		\{{0}\}.Release|Any CPU.ActiveCfg = Release|Any CPU
		\{{0}\}.Release|Any CPU.Build.0 = Release|Any CPU)";

	const String CSProject::PROJ_TEMPLATE =
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
	<TargetFrameworkVersion>v4.5</TargetFrameworkVersion>
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

	const String CSProject::REFERENCE_ENTRY_TEMPLATE =
		R"(
	<Reference Include="{0}"/>)";

	const String CSProject::REFERENCE_PATH_ENTRY_TEMPLATE =
		R"(
	<Reference Include="{0}">
	  <HintPath>{1}</HintPath>
	</Reference>)";

	const String CSProject::REFERENCE_PROJECT_ENTRY_TEMPLATE =
		R"(
	<ProjectReference Include="{0}.csproj">
	  <Project>\{{1}\}</Project>
	  <Name>{0}</Name>
	</ProjectReference>)";

	const String CSProject::CODE_ENTRY_TEMPLATE =
		R"(
	<Compile Include="{0}"/>)";

	const String CSProject::NON_CODE_ENTRY_TEMPLATE =
		R"(
	<None Include="{0}"/>)";

	/**	Generates a C# project GUID from the project name. */
	String getProjectGUID(const WString& projectName)
	{
		static const String guidTemplate = "{0}-{1}-{2}-{3}-{4}";
		String hash = md5(projectName);

		String output = StringUtil::format(guidTemplate, hash.substr(0, 8),
			hash.substr(8, 4), hash.substr(12, 4), hash.substr(16, 4), hash.substr(20, 12));
		StringUtil::toUpperCase(output);

		return output;
	}

	String CSProject::writeSolution(CSProjectVersion version, const CodeSolutionData& data)
	{
		struct VersionData
		{
			String formatVersion;
		};

		Map<CSProjectVersion, VersionData> versionData =
		{
			{ CSProjectVersion::VS2008, { "10.00" } },
			{ CSProjectVersion::VS2010, { "11.00" } },
			{ CSProjectVersion::VS2012, { "12.00" } },
			{ CSProjectVersion::VS2013, { "12.00" } },
			{ CSProjectVersion::VS2015, { "12.00" } },
			{ CSProjectVersion::VS2017, { "12.00" } },
			{ CSProjectVersion::MonoDevelop, { "12.00" } }
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

	String CSProject::writeProject(CSProjectVersion version, const CodeProjectData& projectData)
	{
		struct VersionData
		{
			String toolsVersion;
		};

		Map<CSProjectVersion, VersionData> versionData =
		{
			{ CSProjectVersion::VS2008, { "3.5" } },
			{ CSProjectVersion::VS2010, { "4.0" } },
			{ CSProjectVersion::VS2012, { "4.0" } },
			{ CSProjectVersion::VS2013, { "12.0" } },
			{ CSProjectVersion::VS2015, { "13.0" } },
			{ CSProjectVersion::VS2017, { "15.0" } },
			{ CSProjectVersion::MonoDevelop, { "14.0" } }
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
}