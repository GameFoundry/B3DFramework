//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsBuiltinResourcesHelper.h"
#include "FileSystem/BsFileSystem.h"
#include "Importer/BsImporter.h"
#include "Resources/BsResources.h"
#include "Importer/BsShaderImportOptions.h"
#include "Importer/BsTextureImportOptions.h"
#include "Renderer/BsRendererMaterialManager.h"
#include "Renderer/BsRendererMaterial.h"
#include "Text/BsFontImportOptions.h"
#include "Image/BsSpriteTexture.h"
#include "Image/BsTexture.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsDataStream.h"
#include "Resources/BsResourceManifest.h"
#include "FileSystem/BsFileSystem.h"
#include "CoreThread/BsCoreThread.h"
#include "Utility/BsUUID.h"
#include "Material/BsShader.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsGpuProgram.h"

using json = nlohmann::json;

using namespace bs;

void BuiltinResourcesHelper::ImportAssets(const nlohmann::json& entries, const Vector<bool>& importFlags, const Path& inputFolder, const Path& outputFolder, const SPtr<ResourceManifest>& manifest, AssetType mode, nlohmann::json* dependencies, bool compress, bool mipmap)
{
	if(!FileSystem::Exists(inputFolder))
		return;

	bool outputExists = FileSystem::Exists(outputFolder);
	if(!outputExists)
		FileSystem::CreateDir(outputFolder);

	Path spriteOutputFolder = outputFolder + "/Sprites/";
	if(mode == AssetType::Sprite)
		FileSystem::CreateDir(spriteOutputFolder);

	struct QueuedImportOp
	{
		QueuedImportOp(const TAsyncOp<HResource>& op, const Path& outputPath, const nlohmann::json& jsonEntry)
			: Op(op), OutputPath(outputPath), JsonEntry(jsonEntry)
		{}

		TAsyncOp<HResource> Op;
		Path OutputPath;
		const nlohmann::json& JsonEntry;
	};

	List<QueuedImportOp> queuedOps;
	auto importResource = [&](const nlohmann::json& entry)
	{
		std::string name = entry["Path"];
		std::string uuidStr;

		if(mode == AssetType::Normal)
			uuidStr = entry["UUID"].get<std::string>();
		else if(mode == AssetType::Sprite)
			uuidStr = entry["TextureUUID"].get<std::string>();

		String fileName = name.c_str();
		UUID UUID(uuidStr.c_str());

		Path filePath = inputFolder + fileName;

		Path relativePath = fileName;
		Path relativeAssetPath = fileName;
		relativeAssetPath.SetFilename(relativeAssetPath.GetFilename() + u8".asset");

		SPtr<ImportOptions> importOptions = GetImporter().CreateImportOptions(filePath);
		if(importOptions != nullptr)
		{
			if(B3DRTTIIsOfType<TextureImportOptions>(importOptions))
			{
				SPtr<TextureImportOptions> texImportOptions =
					std::static_pointer_cast<TextureImportOptions>(importOptions);

				texImportOptions->GenerateMips = mipmap;
			}
			else if(B3DRTTIIsOfType<ShaderImportOptions>(importOptions))
			{
				ShaderDefines defines = RendererMaterialManager::GetDefinesInternal(relativePath);

				SPtr<ShaderImportOptions> shaderImportOptions =
					std::static_pointer_cast<ShaderImportOptions>(importOptions);

				UnorderedMap<String, String> allDefines = defines.GetAll();
				for(auto& define : allDefines)
					shaderImportOptions->SetDefine(define.first, define.second);
			}
		}

		Path outputPath = outputFolder + relativeAssetPath;

		TAsyncOp<HResource> op = GetImporter().ImportAsync(filePath, importOptions, UUID);
		queuedOps.emplace_back(op, outputPath, entry);
	};

	auto generateSprite = [&](const HTexture& texture, const String& fileName, const UUID& UUID)
	{
		Path relativePath = fileName;
		Path outputPath = spriteOutputFolder + relativePath;

		outputPath.SetFilename("sprite_" + fileName + ".asset");

		SPtr<SpriteTexture> spriteTexPtr = SpriteTexture::CreatePtrInternal(texture);
		HResource spriteTex = GetResources().CreateResourceHandleInternal(spriteTexPtr, UUID);

		Resources::Instance().Save(spriteTex, outputPath, true, compress);
		manifest->RegisterResource(spriteTex.GetId(), outputPath);
	};

	auto generateAnimatedSprite = [&](const HTexture& texture, const String& fileName, const UUID& UUID,
									  SpriteAnimationPlayback playback, const SpriteSheetGridAnimation& animation)
	{
		Path relativePath = fileName;
		Path outputPath = spriteOutputFolder + relativePath;

		outputPath.SetFilename("sprite_" + fileName + ".asset");

		SPtr<SpriteTexture> spriteTexPtr = SpriteTexture::CreatePtrInternal(texture);
		spriteTexPtr->SetAnimation(animation);
		spriteTexPtr->SetAnimationPlayback(playback);

		HResource spriteTex = GetResources().CreateResourceHandleInternal(spriteTexPtr, UUID);

		Resources::Instance().Save(spriteTex, outputPath, true, compress);
		manifest->RegisterResource(spriteTex.GetId(), outputPath);
	};

	// Start async import for all resources
	int idx = 0;
	for(auto& entry : entries)
	{
		if(!importFlags[idx])
		{
			idx++;
			continue;
		}

		importResource(entry);
		idx++;
	}

	struct IconData
	{
		String Name;
		HTexture Source;
		SPtr<PixelData> SrcData;
		std::string TextureUUIDs[3];
		std::string SpriteUUIDs[3];
	};

	Vector<IconData> iconsToGenerate;
	while(!queuedOps.empty())
	{
		for(auto iter = queuedOps.begin(); iter != queuedOps.end();)
		{
			QueuedImportOp& importOp = *iter;
			if(!importOp.Op.HasCompleted())
			{
				++iter;
				continue;
			}

			HResource outputRes = importOp.Op.GetReturnValue();
			if(outputRes != nullptr)
			{
				Resources::Instance().Save(outputRes, importOp.OutputPath, true, compress);
				manifest->RegisterResource(outputRes.GetId(), importOp.OutputPath);

				const nlohmann::json& entry = importOp.JsonEntry;

				std::string name = entry["Path"];

				bool isIcon = false;
				if(mode == AssetType::Normal)
					isIcon = entry.find("UUID16") != entry.end();
				else if(mode == AssetType::Sprite)
					isIcon = entry.find("TextureUUID16") != entry.end();

				if(B3DRTTIIsOfType<Shader>(outputRes.Get()))
				{
					HShader shader = B3DStaticResourceCast<Shader>(outputRes);
					if(!VerifyAndReportShader(shader))
					{
						iter = queuedOps.erase(iter);
						continue;
					}

					if(dependencies != nullptr)
					{
						SPtr<ShaderMetaData> shaderMetaData = std::static_pointer_cast<ShaderMetaData>(shader->GetMetaData());

						nlohmann::json dependencyEntries;
						if(shaderMetaData != nullptr && shaderMetaData->Includes.size() > 0)
						{
							for(auto& include : shaderMetaData->Includes)
							{
								Path includePath = include.c_str();
								if(include.substr(0, 8) == "$ENGINE$" || include.substr(0, 8) == "$EDITOR$")
								{
									if(include.size() > 8)
										includePath = include.substr(9, include.size() - 9);
								}

								nlohmann::json newDependencyEntry = {
									{ "Path", includePath.ToString().c_str() }
								};

								dependencyEntries.push_back(newDependencyEntry);
							}
						}

						(*dependencies)[name] = dependencyEntries;
					}
				}

				if(mode == AssetType::Sprite)
				{
					HTexture tex = B3DStaticResourceCast<Texture>(outputRes);
					std::string spriteUUID = entry["SpriteUUID"];

					bool isAnimated = entry.find("Animation") != entry.end();
					if(isAnimated)
					{
						auto& jsonAnimation = entry["Animation"];

						SpriteSheetGridAnimation animation;
						animation.NumRows = jsonAnimation["NumRows"].get<u32>();
						animation.NumColumns = jsonAnimation["NumColumns"].get<u32>();
						animation.Count = jsonAnimation["Count"].get<u32>();
						animation.Fps = jsonAnimation["FPS"].get<u32>();

						generateAnimatedSprite(tex, name.c_str(), UUID(spriteUUID.c_str()), SpriteAnimationPlayback::Loop, animation);
					}
					else
						generateSprite(tex, name.c_str(), UUID(spriteUUID.c_str()));
				}

				if(isIcon)
				{
					IconData iconData;
					iconData.Source = B3DStaticResourceCast<Texture>(outputRes);
					iconData.Name = name.c_str();

					if(mode == AssetType::Normal)
					{
						iconData.TextureUUIDs[0] = entry["UUID48"].get<std::string>();
						iconData.TextureUUIDs[1] = entry["UUID32"].get<std::string>();
						iconData.TextureUUIDs[2] = entry["UUID16"].get<std::string>();
					}
					else if(mode == AssetType::Sprite)
					{
						iconData.TextureUUIDs[0] = entry["TextureUUID48"].get<std::string>();
						iconData.TextureUUIDs[1] = entry["TextureUUID32"].get<std::string>();
						iconData.TextureUUIDs[2] = entry["TextureUUID16"].get<std::string>();

						iconData.SpriteUUIDs[0] = entry["SpriteUUID48"].get<std::string>();
						iconData.SpriteUUIDs[1] = entry["SpriteUUID32"].get<std::string>();
						iconData.SpriteUUIDs[2] = entry["SpriteUUID16"].get<std::string>();
					}

					iconsToGenerate.push_back(iconData);
				}
			}

			iter = queuedOps.erase(iter);
		}
	}

	for(u32 i = 0; i < (u32)iconsToGenerate.size(); i++)
	{
		IconData& data = iconsToGenerate[i];

		data.SrcData = data.Source->GetProperties().AllocBuffer(0, 0);
		data.Source->ReadData(data.SrcData);
	}

	GetCoreThread().Submit(true);

	auto saveTexture = [&](auto& pixelData, auto& path, std::string& uuid)
	{
		SPtr<Texture> texturePtr = Texture::CreateShared(pixelData);
		HResource texture = GetResources().CreateResourceHandleInternal(texturePtr, UUID(uuid.c_str()));

		Resources::Instance().Save(texture, path, true, compress);
		manifest->RegisterResource(texture.GetId(), path);

		return B3DStaticResourceCast<Texture>(texture);
	};

	for(u32 i = 0; i < (u32)iconsToGenerate.size(); i++)
	{
		SPtr<PixelData> src = iconsToGenerate[i].SrcData;

		SPtr<PixelData> scaled48 = PixelData::Create(48, 48, 1, src->GetFormat());
		PixelUtil::Scale(*src, *scaled48);

		SPtr<PixelData> scaled32 = PixelData::Create(32, 32, 1, src->GetFormat());
		PixelUtil::Scale(*scaled48, *scaled32);

		SPtr<PixelData> scaled16 = PixelData::Create(16, 16, 1, src->GetFormat());
		PixelUtil::Scale(*scaled32, *scaled16);

		Path outputPath48 = outputFolder + (iconsToGenerate[i].Name + "48.asset");
		Path outputPath32 = outputFolder + (iconsToGenerate[i].Name + "32.asset");
		Path outputPath16 = outputFolder + (iconsToGenerate[i].Name + "16.asset");

		HTexture tex48 = saveTexture(scaled48, outputPath48, iconsToGenerate[i].TextureUUIDs[0]);
		HTexture tex32 = saveTexture(scaled32, outputPath32, iconsToGenerate[i].TextureUUIDs[1]);
		HTexture tex16 = saveTexture(scaled16, outputPath16, iconsToGenerate[i].TextureUUIDs[2]);

		if(mode == AssetType::Sprite)
		{
			generateSprite(tex48, iconsToGenerate[i].Name + "48", UUID(iconsToGenerate[i].SpriteUUIDs[0].c_str()));
			generateSprite(tex32, iconsToGenerate[i].Name + "32", UUID(iconsToGenerate[i].SpriteUUIDs[1].c_str()));
			generateSprite(tex16, iconsToGenerate[i].Name + "16", UUID(iconsToGenerate[i].SpriteUUIDs[2].c_str()));
		}
	}
}

void BuiltinResourcesHelper::ImportFont(const Path& inputFile, const String& outputName, const Path& outputFolder, const Vector<u32>& fontSizes, bool antialiasing, const UUID& UUID, const SPtr<ResourceManifest>& manifest)
{
	SPtr<ImportOptions> fontImportOptions = Importer::Instance().CreateImportOptions(inputFile);
	if(B3DRTTIIsOfType<FontImportOptions>(fontImportOptions))
	{
		FontImportOptions* importOptions = static_cast<FontImportOptions*>(fontImportOptions.get());

		importOptions->FontSizes = { fontSizes };
		importOptions->RenderMode = antialiasing ? FontRenderMode::HintedSmooth : FontRenderMode::HintedRaster;
	}
	else
		return;

	HFont font = Importer::Instance().Import<Font>(inputFile, fontImportOptions, UUID);

	String fontName = outputName;
	Path outputPath = outputFolder + fontName;
	outputPath.SetFilename(outputPath.GetFilename() + u8".asset");

	Resources::Instance().Save(font, outputPath, true);
	manifest->RegisterResource(font.GetId(), outputPath);

	// Save font texture pages as well. TODO - Later maybe figure out a more automatic way to do this
	for(auto& size : fontSizes)
	{
		SPtr<const FontBitmap> fontData = font->GetBitmap(size);

		Path texPageOutputPath = outputFolder;

		u32 pageIdx = 0;
		for(auto tex : fontData->TexturePages)
		{
			texPageOutputPath.SetFilename(fontName + u8"_" + ToString(size) + u8"_texpage_" + ToString(pageIdx) + u8".asset");

			Resources::Instance().Save(tex, texPageOutputPath, true);
			manifest->RegisterResource(tex.GetId(), texPageOutputPath);

			pageIdx++;
		}
	}
}

Vector<bool> BuiltinResourcesHelper::GenerateImportFlags(const nlohmann::json& entries, const Path& inputFolder, time_t lastUpdateTime, bool forceImport, const nlohmann::json* dependencies, const Path& dependencyFolder)
{
	Vector<bool> output(entries.size());
	u32 idx = 0;
	for(auto& entry : entries)
	{
		std::string name = entry["Path"];

		if(forceImport)
			output[idx] = true;
		else
		{
			Path filePath = inputFolder + Path(name.c_str());

			// Check timestamp
			time_t lastModifiedSrc = FileSystem::GetLastModifiedTime(filePath);
			if(lastModifiedSrc > lastUpdateTime)
				output[idx] = true;
			else if(dependencies != nullptr) // Check dependencies
			{
				bool anyDepModified = false;
				auto iterFind = dependencies->find(name);
				if(iterFind != dependencies->end())
				{
					for(auto& dependency : *iterFind)
					{
						std::string dependencyName = dependency["Path"];
						Path dependencyPath = dependencyFolder + Path(dependencyName.c_str());

						time_t lastModifiedDep = FileSystem::GetLastModifiedTime(dependencyPath);
						if(lastModifiedDep > lastUpdateTime)
						{
							anyDepModified = true;
							break;
						}
					}
				}

				output[idx] = anyDepModified;
			}
			else
				output[idx] = false;
		}

		idx++;
	}

	return output;
}

bool BuiltinResourcesHelper::UpdateJson(const Path& folder, AssetType type, nlohmann::json& entries)
{
	UnorderedSet<Path> existingEntries;
	for(auto& entry : entries)
	{
		std::string strPath = entry["Path"];
		Path path = strPath.c_str();

		existingEntries.insert(path);
	}

	bool foundChanges = false;
	auto checkForChanges = [&](const Path& filePath)
	{
		Path relativePath = filePath.GetRelative(folder);

		auto iterFind = existingEntries.find(relativePath);
		if(iterFind == existingEntries.end())
		{
			if(type == AssetType::Normal)
			{
				String uuid = UUIDGenerator::GenerateRandom().ToString();
				nlohmann::json newEntry = {
					{ "Path", relativePath.ToString().c_str() },
					{ "UUID", uuid.c_str() }
				};

				entries.push_back(newEntry);
			}
			else // Sprite
			{
				String texUuid = UUIDGenerator::GenerateRandom().ToString();
				String spriteUuid = UUIDGenerator::GenerateRandom().ToString();
				nlohmann::json newEntry = {
					{ "Path", relativePath.ToString().c_str() },
					{ "SpriteUUID", spriteUuid.c_str() },
					{ "TextureUUID", texUuid.c_str() }
				};

				entries.push_back(newEntry);
			}

			foundChanges = true;
		}

		return true;
	};

	FileSystem::Iterate(folder, checkForChanges, nullptr, false);

	// Prune deleted entries
	auto iter = entries.begin();
	while(iter != entries.end())
	{
		std::string strPath = (*iter)["Path"];
		Path path = strPath.c_str();
		path = path.GetAbsolute(folder);

		if(!FileSystem::Exists(path))
		{
			iter = entries.erase(iter);
			foundChanges = true;
		}
		else
			++iter;
	}

	return foundChanges;
}

void BuiltinResourcesHelper::UpdateManifest(const Path& folder, const nlohmann::json& entries, const SPtr<ResourceManifest>& manifest, AssetType type)
{
	for(auto& entry : entries)
	{
		std::string name = entry["Path"];
		std::string uuid;

		bool isIcon = false;
		if(type == AssetType::Normal)
		{
			uuid = entry["UUID"].get<std::string>();
			isIcon = entry.find("UUID16") != entry.end();
		}
		else if(type == AssetType::Sprite)
		{
			uuid = entry["TextureUUID"].get<std::string>();
			isIcon = entry.find("TextureUUID16") != entry.end();
		}

		Path path = folder + name.c_str();
		path.SetFilename(path.GetFilename() + u8".asset");

		manifest->RegisterResource(UUID(uuid.c_str()), path);

		if(type == AssetType::Sprite)
		{
			std::string spriteUUID = entry["SpriteUUID"];

			Path spritePath = folder + "/Sprites/";
			spritePath.SetFilename(String("sprite_") + name.c_str() + ".asset");

			manifest->RegisterResource(UUID(spriteUUID.c_str()), spritePath);
		}

		if(isIcon)
		{
			std::string texUUIDs[3];

			if(type == AssetType::Normal)
			{
				texUUIDs[0] = entry["UUID48"].get<std::string>();
				texUUIDs[1] = entry["UUID32"].get<std::string>();
				texUUIDs[2] = entry["UUID16"].get<std::string>();
			}
			else if(type == AssetType::Sprite)
			{
				texUUIDs[0] = entry["TextureUUID48"].get<std::string>();
				texUUIDs[1] = entry["TextureUUID32"].get<std::string>();
				texUUIDs[2] = entry["TextureUUID16"].get<std::string>();
			}

			Path texPath = folder + name.c_str();

			texPath.SetFilename(texPath.GetFilename() + u8"48.asset");
			manifest->RegisterResource(UUID(texUUIDs[0].c_str()), texPath);

			texPath.SetFilename(texPath.GetFilename() + u8"32.asset");
			manifest->RegisterResource(UUID(texUUIDs[1].c_str()), texPath);

			texPath.SetFilename(texPath.GetFilename() + u8"16.asset");
			manifest->RegisterResource(UUID(texUUIDs[2].c_str()), texPath);

			if(type == AssetType::Sprite)
			{
				std::string spriteUUIDs[3];

				spriteUUIDs[0] = entry["SpriteUUID48"].get<std::string>();
				spriteUUIDs[1] = entry["SpriteUUID32"].get<std::string>();
				spriteUUIDs[2] = entry["SpriteUUID16"].get<std::string>();

				Path spritePath = folder + "/Sprites/";

				spritePath.SetFilename(String("sprite_") + name.c_str() + "48.asset");
				manifest->RegisterResource(UUID(spriteUUIDs[0].c_str()), spritePath);

				spritePath.SetFilename(String("sprite_") + name.c_str() + "32.asset");
				manifest->RegisterResource(UUID(spriteUUIDs[1].c_str()), spritePath);

				spritePath.SetFilename(String("sprite_") + name.c_str() + "16.asset");
				manifest->RegisterResource(UUID(spriteUUIDs[2].c_str()), spritePath);
			}
		}
	}
}

void BuiltinResourcesHelper::WriteTimestamp(const Path& file)
{
	SPtr<DataStream> fileStream = FileSystem::CreateAndOpenFile(file);

	time_t currentTime = std::time(nullptr);
	fileStream->Write(&currentTime, sizeof(currentTime));
	fileStream->Close();
}

u32 BuiltinResourcesHelper::CheckForModifications(const Path& folder, const Path& timeStampFile, time_t& lastUpdateTime)
{
	lastUpdateTime = 0;

	if(!FileSystem::Exists(timeStampFile))
		return 2;

	lastUpdateTime = FileSystem::GetLastModifiedTime(timeStampFile);

	bool upToDate = true;
	auto checkUpToDate = [&](const Path& filePath)
	{
		time_t fileLastModified = FileSystem::GetLastModifiedTime(filePath);

		if(fileLastModified > lastUpdateTime)
		{
			upToDate = false;
			return false;
		}

		return true;
	};

	FileSystem::Iterate(folder, checkUpToDate, nullptr);

	if(!upToDate)
		return 1;

	return 0;
}

bool BuiltinResourcesHelper::VerifyAndReportShader(const HShader& shader)
{
	if(!shader.IsLoaded(false) || shader->GetTechniqueCount() == 0)
	{
#if B3D_DEBUG
		B3D_EXCEPT(InvalidStateException, "Error occured while compiling a shader. Check earlier log messages for exact error.");
#else
		B3D_LOG(Error, Importer, "Error occured while compiling a shader. Check earlier log messages for exact error.");
#endif
		return false;
	}

	Vector<SPtr<Technique>> techniques = shader->GetCompatibleTechniques();
	for(auto& technique : techniques)
	{
		technique->Compile();

		u32 numPasses = technique->GetPassCount();
		for(u32 i = 0; i < numPasses; i++)
		{
			SPtr<Pass> pass = technique->GetPass(i);

			std::array<SPtr<ct::GpuProgram>, 6> gpuPrograms;

			const SPtr<GpuGraphicsPipelineState>& graphicsPipeline = pass->GetGraphicsPipelineState();
			if(graphicsPipeline)
			{
				gpuPrograms[0] = graphicsPipeline->GetVertexProgram();
				gpuPrograms[1] = graphicsPipeline->GetFragmentProgram();
				gpuPrograms[2] = graphicsPipeline->GetGeometryProgram();
				gpuPrograms[3] = graphicsPipeline->GetHullProgram();
				gpuPrograms[4] = graphicsPipeline->GetDomainProgram();
			}

			const SPtr<GpuComputePipelineState>& computePipeline = pass->GetComputePipelineState();
			if(computePipeline)
				gpuPrograms[5] = computePipeline->GetProgram();

			for(auto& program : gpuPrograms)
			{
				if(program == nullptr)
					continue;

				if(!program->IsCompiled())
				{
					String errMsg = "Error occured while compiling a shader \"" + shader->GetName() + "\". Error message: " + program->GetCompileErrorMessage();

#if B3D_DEBUG
					B3D_EXCEPT(InvalidStateException, errMsg);
#else
					B3D_LOG(Error, Importer, errMsg);
#endif
					return false;
				}
			}
		}
	}

	return true;
}

void BuiltinResourcesHelper::UpdateShaderBytecode(const Path& path)
{
	HShader shader = GetResources().Load<Shader>(path, ResourceLoadFlag::KeepSourceData);
	if(!shader)
		return;

	Vector<SPtr<Technique>> techniques = shader->GetCompatibleTechniques();
	bool hasBytecode = true;
	for(auto& technique : techniques)
	{
		u32 numPasses = technique->GetPassCount();
		for(u32 i = 0; i < numPasses; i++)
		{
			SPtr<Pass> pass = technique->GetPass(i);

			for(u32 j = 0; j < GPT_COUNT; j++)
			{
				const GpuProgramCreateInformation& desc = pass->GetGpuProgramCreateInformation((GpuProgramType)j);
				if(desc.Source.empty())
					continue;

				if(!desc.Bytecode)
				{
					hasBytecode = false;
					break;
				}
			}

			if(!hasBytecode)
				break;
		}

		if(!hasBytecode)
			break;
	}

	if(hasBytecode)
		return;

	for(auto& technique : techniques)
		technique->Compile();

	GetResources().Save(shader, path, true, true);
}

GUIElementStyle BuiltinResourcesHelper::LoadGuiStyleFromJson(const nlohmann::json& entry, const GUIElementStyleLoader& loader)
{
	GUIElementStyle style;

	if(entry.count("font") > 0)
	{
		std::string font = entry["font"];
		style.Font = loader.LoadFont(font.c_str());
	}

	if(entry.count("fontSize") > 0)
		style.FontSize = entry["fontSize"];

	if(entry.count("textHorzAlign") > 0)
		style.TextHorzAlign = entry["textHorzAlign"];

	if(entry.count("textVertAlign") > 0)
		style.TextVertAlign = entry["textVertAlign"];

	if(entry.count("imagePosition") > 0)
		style.ImagePosition = entry["imagePosition"];

	if(entry.count("wordWrap") > 0)
		style.WordWrap = entry["wordWrap"];

	const auto loadState = [&loader, &entry](const char* name, GUIElementStateStyle& state)
	{
		if(entry.count(name) == 0)
			return false;

		nlohmann::json subEntry = entry[name];

		if(subEntry.count("texture") > 0)
		{
			std::string texture = subEntry["texture"];
			state.Texture = loader.LoadTexture(texture.c_str());
		}

		if(subEntry.count("textColor") > 0)
		{
			nlohmann::json colorEntry = subEntry["textColor"];

			state.TextColor.R = colorEntry["r"];
			state.TextColor.G = colorEntry["g"];
			state.TextColor.B = colorEntry["b"];
			state.TextColor.A = colorEntry["a"];
		}

		return true;
	};

	loadState("normal", style.Normal);

	const bool hasHover = loadState("hover", style.Hover);
	if(!hasHover)
		style.Hover = style.Normal;

	if(!loadState("active", style.Active))
		style.Active = style.Normal;

	if(!loadState("focused", style.Focused))
		style.Focused = style.Normal;

	if(!loadState("focusedHover", style.FocusedHover))
	{
		if(hasHover)
			style.FocusedHover = style.Hover;
		else
			style.FocusedHover = style.Normal;
	}

	loadState("normalOn", style.NormalOn);

	const bool hasHoverOn = loadState("hoverOn", style.HoverOn);
	if(!hasHoverOn)
		style.HoverOn = style.NormalOn;

	if(!loadState("activeOn", style.ActiveOn))
		style.ActiveOn = style.NormalOn;

	if(!loadState("focusedOn", style.FocusedOn))
		style.FocusedOn = style.NormalOn;

	if(!loadState("focusedHoverOn", style.FocusedHoverOn))
	{
		if(hasHoverOn)
			style.FocusedHoverOn = style.HoverOn;
		else
			style.FocusedHoverOn = style.NormalOn;
	}

	const auto loadRectOffset = [entry](const char* name, RectOffset& state)
	{
		if(entry.count(name) == 0)
			return;

		nlohmann::json subEntry = entry[name];
		state.Left = subEntry["left"];
		state.Right = subEntry["right"];
		state.Top = subEntry["top"];
		state.Bottom = subEntry["bottom"];
	};

	loadRectOffset("border", style.Border);
	loadRectOffset("margins", style.Margins);
	loadRectOffset("contentOffset", style.ContentOffset);
	loadRectOffset("padding", style.Padding);

	if(entry.count("width") > 0)
		style.Width = entry["width"];

	if(entry.count("height") > 0)
		style.Height = entry["height"];

	if(entry.count("minWidth") > 0)
		style.MinWidth = entry["minWidth"];

	if(entry.count("maxWidth") > 0)
		style.MaxWidth = entry["maxWidth"];

	if(entry.count("minHeight") > 0)
		style.MinHeight = entry["minHeight"];

	if(entry.count("maxHeight") > 0)
		style.MaxHeight = entry["maxHeight"];

	if(entry.count("fixedWidth") > 0)
		style.FixedWidth = entry["fixedWidth"];

	if(entry.count("fixedHeight") > 0)
		style.FixedHeight = entry["fixedHeight"];

	if(entry.count("subStyles") > 0)
	{
		nlohmann::json subStyles = entry["subStyles"];
		for(auto& subStyle : subStyles)
		{
			std::string name = subStyle["name"];
			std::string styleName = subStyle["style"];

			style.SubStyles.insert(std::make_pair(name.c_str(), styleName.c_str()));
		}
	}

	return style;
}

BuiltinResourceGUIElementStyleLoader::BuiltinResourceGUIElementStyleLoader(const Path& fontPath, const Path& texturePath)
	: mFontPath(fontPath), mTexturePath(texturePath)
{}

HSpriteTexture BuiltinResourceGUIElementStyleLoader::LoadTexture(const String& name) const
{
	Path texturePath = mTexturePath;
	texturePath.Append(u8"sprite_" + name + u8".asset");

	return GetResources().Load<SpriteTexture>(texturePath);
}

HFont BuiltinResourceGUIElementStyleLoader::LoadFont(const String& name) const
{
	Path fontPath = mFontPath;
	fontPath.Append(name + u8".asset");

	return GetResources().Load<Font>(fontPath);
}
