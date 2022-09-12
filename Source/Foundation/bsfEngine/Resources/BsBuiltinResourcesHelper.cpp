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

namespace bs
{
	void BuiltinResourcesHelper::importAssets(const nlohmann::json& entries, const Vector<bool>& importFlags,
		const Path& inputFolder, const Path& outputFolder, const SPtr<ResourceManifest>& manifest, AssetType mode,
		nlohmann::json* dependencies, bool compress, bool mipmap)
	{
		if (!FileSystem::exists(inputFolder))
			return;

		bool outputExists = FileSystem::exists(outputFolder);
		if(!outputExists)
			FileSystem::createDir(outputFolder);

		Path spriteOutputFolder = outputFolder + "/Sprites/";
		if(mode == AssetType::Sprite)
			FileSystem::createDir(spriteOutputFolder);

		struct QueuedImportOp
		{
			QueuedImportOp(const TAsyncOp<HResource>& op, const Path& outputPath, const nlohmann::json& jsonEntry)
				:op(op), outputPath(outputPath), jsonEntry(jsonEntry)
			{ }

			TAsyncOp<HResource> op;
			Path outputPath;
			const nlohmann::json& jsonEntry;
		};

		List<QueuedImportOp> queuedOps;
		auto importResource = [&](const nlohmann::json& entry)
		{
			std::string name = entry["Path"];
			std::string uuidStr;

			if (mode == AssetType::Normal)
				uuidStr = entry["UUID"].get<std::string>();
			else if (mode == AssetType::Sprite)
				uuidStr = entry["TextureUUID"].get<std::string>();

			String fileName = name.c_str();
			UUID UUID(uuidStr.c_str());

			Path filePath = inputFolder + fileName;

			Path relativePath = fileName;
			Path relativeAssetPath = fileName;
			relativeAssetPath.SetFilename(relativeAssetPath.getFilename() + u8".asset");

			SPtr<ImportOptions> importOptions = gImporter().CreateImportOptions(filePath);
			if (importOptions != nullptr)
			{
				if (rtti_is_of_type<TextureImportOptions>(importOptions))
				{
					SPtr<TextureImportOptions> texImportOptions =
						std::static_pointer_cast<TextureImportOptions>(importOptions);

					texImportOptions->generateMips = mipmap;
				}
				else if (rtti_is_of_type<ShaderImportOptions>(importOptions))
				{
					ShaderDefines defines = RendererMaterialManager::_getDefines(relativePath);

					SPtr<ShaderImportOptions> shaderImportOptions =
						std::static_pointer_cast<ShaderImportOptions>(importOptions);

					UnorderedMap<String, String> allDefines = defines.GetAll();
					for(auto& define : allDefines)
						shaderImportOptions->SetDefine(define.first, define.second);
				}
			}

			Path outputPath = outputFolder + relativeAssetPath;

			TAsyncOp<HResource> op = gImporter().ImportAsync(filePath, importOptions, UUID);
			queuedOps.emplace_back(op, outputPath, entry);
		};

		auto generateSprite = [&](const HTexture& texture, const String& fileName, const UUID& UUID)
		{
			Path relativePath = fileName;
			Path outputPath = spriteOutputFolder + relativePath;

			outputPath.SetFilename("sprite_" + fileName + ".asset");

			SPtr<SpriteTexture> spriteTexPtr = SpriteTexture::_createPtr(texture);
			HResource spriteTex = gResources()._createResourceHandle(spriteTexPtr, UUID);

			Resources::instance().Save(spriteTex, outputPath, true, compress);
			manifest->RegisterResource(spriteTex.GetUUID(), outputPath);
		};

		auto generateAnimatedSprite = [&](const HTexture& texture, const String& fileName, const UUID& UUID,
			SpriteAnimationPlayback playback, const SpriteSheetGridAnimation& animation)
		{
			Path relativePath = fileName;
			Path outputPath = spriteOutputFolder + relativePath;

			outputPath.SetFilename("sprite_" + fileName + ".asset");

			SPtr<SpriteTexture> spriteTexPtr = SpriteTexture::_createPtr(texture);
			spriteTexPtr->SetAnimation(animation);
			spriteTexPtr->SetAnimationPlayback(playback);

			HResource spriteTex = gResources()._createResourceHandle(spriteTexPtr, UUID);

			Resources::instance().Save(spriteTex, outputPath, true, compress);
			manifest->RegisterResource(spriteTex.GetUUID(), outputPath);
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
			String name;
			HTexture source;
			SPtr<PixelData> srcData;
			std::string TextureUUIDs[3];
			std::string SpriteUUIDs[3];
		};

		Vector<IconData> iconsToGenerate;
		while(!queuedOps.Empty())
		{
			for(auto iter = queuedOps.Begin(); iter != queuedOps.end();)
			{
				QueuedImportOp& importOp = *iter;
				if(!importOp.op.HasCompleted())
				{
					++iter;
					continue;
				}

				HResource outputRes = importOp.op.GetReturnValue();
				if (outputRes != nullptr)
				{
					Resources::instance().Save(outputRes, importOp.outputPath, true, compress);
					manifest->RegisterResource(outputRes.GetUUID(), importOp.outputPath);

					const nlohmann::json& entry = importOp.jsonEntry;

					std::string name = entry["Path"];

					bool isIcon = false;
					if (mode == AssetType::Normal)
						isIcon = entry.Find("UUID16") != entry.end();
					else if (mode == AssetType::Sprite)
						isIcon = entry.Find("TextureUUID16") != entry.end();

					if (rtti_is_of_type<Shader>(outputRes.Get()))
					{
						HShader shader = static_resource_cast<Shader>(outputRes);
						if (!verifyAndReportShader(shader))
						{
							iter = queuedOps.Erase(iter);
							continue;
						}

						if (dependencies != nullptr)
						{
							SPtr<ShaderMetaData> shaderMetaData = std::static_pointer_cast<ShaderMetaData>(shader->GetMetaData());

							nlohmann::json dependencyEntries;
							if (shaderMetaData != nullptr && shaderMetaData->includes.Size() > 0)
							{
								for (auto& include : shaderMetaData->includes)
								{
									Path includePath = include.c_str();
									if (include.Substr(0, 8) == "$ENGINE$" || include.substr(0, 8) == "$EDITOR$")
									{
										if (include.Size() > 8)
											includePath = include.Substr(9, include.size() - 9);
									}

									nlohmann::json newDependencyEntry =
									{
										{ "Path", includePath.ToString().c_str() }
									};

									dependencyEntries.push_back(newDependencyEntry);
								}
							}

							(*dependencies)[name] = dependencyEntries;
						}
					}

					if (mode == AssetType::Sprite)
					{
						HTexture tex = static_resource_cast<Texture>(outputRes);
						std::string spriteUUID = entry["SpriteUUID"];

						bool isAnimated = entry.Find("Animation") != entry.end();
						if(isAnimated)
						{
							auto& jsonAnimation = entry["Animation"];

							SpriteSheetGridAnimation animation;
							animation.numRows = jsonAnimation["NumRows"].get<UINT32>();
							animation.numColumns = jsonAnimation["NumColumns"].get<UINT32>();
							animation.count = jsonAnimation["Count"].get<UINT32>();
							animation.fps = jsonAnimation["FPS"].get<UINT32>();

							generateAnimatedSprite(tex, name.c_str(), UUID(spriteUUID.c_str()),
								SpriteAnimationPlayback::Loop, animation);
						}
						else
							generateSprite(tex, name.c_str(), UUID(spriteUUID.c_str()));

					}

					if (isIcon)
					{
						IconData iconData;
						iconData.source = static_resource_cast<Texture>(outputRes);
						iconData.name = name.c_str();

						if (mode == AssetType::Normal)
						{
							iconData.TextureUUIDs[0] = entry["UUID48"].get<std::string>();
							iconData.TextureUUIDs[1] = entry["UUID32"].get<std::string>();
							iconData.TextureUUIDs[2] = entry["UUID16"].get<std::string>();
						}
						else if (mode == AssetType::Sprite)
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

				iter = queuedOps.Erase(iter);
			}
		}

		for(UINT32 i = 0; i < (UINT32)iconsToGenerate.Size(); i++)
		{
			IconData& data = iconsToGenerate[i];

			data.srcData = data.source->GetProperties().AllocBuffer(0, 0);
			data.source->ReadData(data.srcData);
		}

		gCoreThread().Submit(true);

		auto saveTexture = [&](auto& pixelData, auto& path, std::string& uuid)
		{
			SPtr<Texture> texturePtr = Texture::_createPtr(pixelData);
			HResource texture = gResources()._createResourceHandle(texturePtr, UUID(uuid.c_str()));

			Resources::instance().Save(texture, path, true, compress);
			manifest->RegisterResource(texture.GetUUID(), path);

			return static_resource_cast<Texture>(texture);
		};

		for (UINT32 i = 0; i < (UINT32)iconsToGenerate.Size(); i++)
		{
			SPtr<PixelData> src = iconsToGenerate[i].srcData;

			SPtr<PixelData> scaled48 = PixelData::create(48, 48, 1, src->GetFormat());
			PixelUtil::scale(*src, *scaled48);

			SPtr<PixelData> scaled32 = PixelData::create(32, 32, 1, src->GetFormat());
			PixelUtil::scale(*scaled48, *scaled32);

			SPtr<PixelData> scaled16 = PixelData::create(16, 16, 1, src->GetFormat());
			PixelUtil::scale(*scaled32, *scaled16);

			Path outputPath48 = outputFolder + (iconsToGenerate[i].name + "48.asset");
			Path outputPath32 = outputFolder + (iconsToGenerate[i].name + "32.asset");
			Path outputPath16 = outputFolder + (iconsToGenerate[i].name + "16.asset");

			HTexture tex48 = saveTexture(scaled48, outputPath48, iconsToGenerate[i].TextureUUIDs[0]);
			HTexture tex32 = saveTexture(scaled32, outputPath32, iconsToGenerate[i].TextureUUIDs[1]);
			HTexture tex16 = saveTexture(scaled16, outputPath16, iconsToGenerate[i].TextureUUIDs[2]);

			if (mode == AssetType::Sprite)
			{
				generateSprite(tex48, iconsToGenerate[i].name + "48", UUID(iconsToGenerate[i].SpriteUUIDs[0].c_str()));
				generateSprite(tex32, iconsToGenerate[i].name + "32", UUID(iconsToGenerate[i].SpriteUUIDs[1].c_str()));
				generateSprite(tex16, iconsToGenerate[i].name + "16", UUID(iconsToGenerate[i].SpriteUUIDs[2].c_str()));
			}
		}
	}

	void BuiltinResourcesHelper::importFont(const Path& inputFile, const String& outputName, const Path& outputFolder,
		const Vector<UINT32>& fontSizes, bool antialiasing, const UUID& UUID, const SPtr<ResourceManifest>& manifest)
	{
		SPtr<ImportOptions> fontImportOptions = Importer::instance().CreateImportOptions(inputFile);
		if (rtti_is_of_type<FontImportOptions>(fontImportOptions))
		{
			FontImportOptions* importOptions = static_cast<FontImportOptions*>(fontImportOptions.Get());

			importOptions->fontSizes = { fontSizes };
			importOptions->renderMode = antialiasing ? FontRenderMode::HintedSmooth : FontRenderMode::HintedRaster;
		}
		else
			return;

		HFont font = Importer::instance().import<Font>(inputFile, fontImportOptions, UUID);

		String fontName = outputName;
		Path outputPath = outputFolder + fontName;
		outputPath.SetFilename(outputPath.getFilename() + u8".asset");

		Resources::instance().Save(font, outputPath, true);
		manifest->RegisterResource(font.GetUUID(), outputPath);

		// Save font texture pages as well. TODO - Later maybe figure out a more automatic way to do this
		for (auto& size : fontSizes)
		{
			SPtr<const FontBitmap> fontData = font->GetBitmap(size);

			Path texPageOutputPath = outputFolder;

			UINT32 pageIdx = 0;
			for (auto tex : fontData->texturePages)
			{
				texPageOutputPath.SetFilename(fontName + u8"_" + toString(size) + u8"_texpage_" +
					toString(pageIdx) + u8".asset");

				Resources::instance().Save(tex, texPageOutputPath, true);
				manifest->RegisterResource(tex.GetUUID(), texPageOutputPath);

				pageIdx++;
			}
		}
	}

	Vector<bool> BuiltinResourcesHelper::generateImportFlags(const nlohmann::json& entries, const Path& inputFolder,
		time_t lastUpdateTime, bool forceImport, const nlohmann::json* dependencies, const Path& dependencyFolder)
	{
		Vector<bool> Output(entries.Size());
		UINT32 idx = 0;
		for (auto& entry : entries)
		{
			std::string name = entry["Path"];

			if (forceImport)
				output[idx] = true;
			else
			{
				Path filePath = inputFolder + Path(name.c_str());

				// Check timestamp
				time_t lastModifiedSrc = FileSystem::getLastModifiedTime(filePath);
				if (lastModifiedSrc > lastUpdateTime)
					output[idx] = true;
				else if (dependencies != nullptr) // Check dependencies
				{
					bool anyDepModified = false;
					auto iterFind = dependencies->Find(name);
					if(iterFind != dependencies->End())
					{
						for(auto& dependency : *iterFind)
						{
							std::string dependencyName = dependency["Path"];
							Path dependencyPath = dependencyFolder + Path(dependencyName.c_str());

							time_t lastModifiedDep = FileSystem::getLastModifiedTime(dependencyPath);
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

	bool BuiltinResourcesHelper::UpdateJSON(const Path& folder, AssetType type, nlohmann::json& entries)
	{
		UnorderedSet<Path> existingEntries;
		for(auto& entry : entries)
		{
			std::string strPath = entry["Path"];
			Path path = strPath.c_str();

			existingEntries.Insert(path);
		}

		bool foundChanges = false;
		auto checkForChanges = [&](const Path& filePath)
		{
			Path relativePath = filePath.GetRelative(folder);

			auto iterFind = existingEntries.Find(relativePath);
			if(iterFind == existingEntries.End())
			{
				if(type == AssetType::Normal)
				{
					String uuid = UUIDGenerator::generateRandom().ToString();
					nlohmann::json newEntry =
					{
						{ "Path", relativePath.ToString().c_str() },
						{ "UUID", uuid.c_str() }
					};

					entries.push_back(newEntry);
				}
				else // Sprite
				{
					String texUuid = UUIDGenerator::generateRandom().ToString();
					String spriteUuid = UUIDGenerator::generateRandom().ToString();
					nlohmann::json newEntry =
					{
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

		FileSystem::iterate(folder, checkForChanges, nullptr, false);

		// Prune deleted entries
		auto iter = entries.Begin();
		while(iter != entries.End())
		{
			std::string strPath = (*iter)["Path"];
			Path path = strPath.c_str();
			path = path.GetAbsolute(folder);

			if (!FileSystem::exists(path))
			{
				iter = entries.Erase(iter);
				foundChanges = true;
			}
			else
				++iter;
		}

		return foundChanges;
	}

	void BuiltinResourcesHelper::updateManifest(const Path& folder, const nlohmann::json& entries,
		const SPtr<ResourceManifest>& manifest, AssetType type)
	{
		for (auto& entry : entries)
		{
			std::string name = entry["Path"];
			std::string uuid;

			bool isIcon = false;
			if (type == AssetType::Normal)
			{
				uuid = entry["UUID"].get<std::string>();
				isIcon = entry.Find("UUID16") != entry.end();
			}
			else if (type == AssetType::Sprite)
			{
				uuid = entry["TextureUUID"].get<std::string>();
				isIcon = entry.Find("TextureUUID16") != entry.end();
			}

			Path path = folder + name.c_str();
			path.SetFilename(path.getFilename() + u8".asset");

			manifest->RegisterResource(UUID(uuid.c_str()), path);
			
			if (type == AssetType::Sprite)
			{
				std::string spriteUUID = entry["SpriteUUID"];

				Path spritePath = folder + "/Sprites/";
				spritePath.SetFilename(String("sprite_") + name.c_str() + ".asset");

				manifest->RegisterResource(UUID(spriteUUID.c_str()), spritePath);
			}

			if (isIcon)
			{
				std::string texUUIDs[3];

				if (type == AssetType::Normal)
				{
					texUUIDs[0] = entry["UUID48"].get<std::string>();
					texUUIDs[1] = entry["UUID32"].get<std::string>();
					texUUIDs[2] = entry["UUID16"].get<std::string>();
				}
				else if (type == AssetType::Sprite)
				{
					texUUIDs[0] = entry["TextureUUID48"].get<std::string>();
					texUUIDs[1] = entry["TextureUUID32"].get<std::string>();
					texUUIDs[2] = entry["TextureUUID16"].get<std::string>();
				}

				Path texPath = folder + name.c_str();

				texPath.SetFilename(texPath.getFilename() + u8"48.asset");
				manifest->RegisterResource(UUID(texUUIDs[0].c_str()), texPath);

				texPath.SetFilename(texPath.getFilename() + u8"32.asset");
				manifest->RegisterResource(UUID(texUUIDs[1].c_str()), texPath);

				texPath.SetFilename(texPath.getFilename() + u8"16.asset");
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
		SPtr<DataStream> fileStream = FileSystem::createAndOpenFile(file);

		time_t currentTime = std::time(nullptr);
		fileStream->Write(&currentTime, sizeof(currentTime));
		fileStream->Close();
	}

	UINT32 BuiltinResourcesHelper::checkForModifications(const Path& folder, const Path& timeStampFile,
		time_t& lastUpdateTime)
	{
		lastUpdateTime = 0;

		if (!FileSystem::exists(timeStampFile))
			return 2;

		lastUpdateTime = FileSystem::getLastModifiedTime(timeStampFile);

		bool upToDate = true;
		auto checkUpToDate = [&](const Path& filePath)
		{
			time_t fileLastModified = FileSystem::getLastModifiedTime(filePath);

			if (fileLastModified > lastUpdateTime)
			{
				upToDate = false;
				return false;
			}

			return true;
		};

		FileSystem::iterate(folder, checkUpToDate, nullptr);
		
		if (!upToDate)
			return 1;

		return 0;
	}

	bool BuiltinResourcesHelper::VerifyAndReportShader(const HShader& shader)
	{
		if(!shader.IsLoaded(false) || shader->GetNumTechniques() == 0)
		{
#if BS_DEBUG_MODE
			BS_EXCEPT(InvalidStateException, "Error occured while compiling a shader. Check earlier log messages for exact error.");
#else
			BS_LOG(Error, Importer, "Error occured while compiling a shader. Check earlier log messages for exact error.");
#endif
			return false;
		}

		Vector<SPtr<Technique>> techniques = shader->GetCompatibleTechniques();
		for(auto& technique : techniques)
		{
			technique->Compile();

			UINT32 numPasses = technique->GetNumPasses();
			for(UINT32 i = 0; i < numPasses; i++)
			{
				SPtr<Pass> pass = technique->GetPass(i);

				std::array<SPtr<GpuProgram>, 6> gpuPrograms;

				const SPtr<GraphicsPipelineState>& graphicsPipeline = pass->GetGraphicsPipelineState();
				if (graphicsPipeline)
				{
					gpuPrograms[0] = graphicsPipeline->GetVertexProgram();
					gpuPrograms[1] = graphicsPipeline->GetFragmentProgram();
					gpuPrograms[2] = graphicsPipeline->GetGeometryProgram();
					gpuPrograms[3] = graphicsPipeline->GetHullProgram();
					gpuPrograms[4] = graphicsPipeline->GetDomainProgram();
				}

				const SPtr<ComputePipelineState>& computePipeline = pass->GetComputePipelineState();
				if (computePipeline)
					gpuPrograms[5] = computePipeline->GetProgram();

				for(auto& program : gpuPrograms)
				{
					if (program == nullptr)
						continue;

					program->BlockUntilCoreInitialized();
					if(!program->IsCompiled())
					{
						String errMsg = "Error occured while compiling a shader \"" + shader->GetName()
							+ "\". Error message: " + program->GetCompileErrorMessage();

#if BS_DEBUG_MODE
						BS_EXCEPT(InvalidStateException, errMsg);
#else
						BS_LOG(Error, Importer, errMsg);
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
		HShader shader = gResources().load<Shader>(path, ResourceLoadFlag::KeepSourceData);
		if (!shader)
			return;

		Vector<SPtr<Technique>> techniques = shader->GetCompatibleTechniques();
		bool hasBytecode = true;
		for (auto& technique : techniques)
		{
			UINT32 numPasses = technique->GetNumPasses();
			for (UINT32 i = 0; i < numPasses; i++)
			{
				SPtr<Pass> pass = technique->GetPass(i);

				for (UINT32 j = 0; j < GPT_COUNT; j++)
				{
					const GPU_PROGRAM_DESC& desc = pass->GetProgramDesc((GpuProgramType)j);
					if (desc.source.Empty())
						continue;

					if (!desc.bytecode)
					{
						hasBytecode = false;
						break;
					}
				}

				if (!hasBytecode)
					break;
			}

			if (!hasBytecode)
				break;
		}

		if (hasBytecode)
			return;

		for (auto& technique : techniques)
			technique->Compile();

		gResources().Save(shader, path, true, true);
	}

	GUIElementStyle BuiltinResourcesHelper::loadGUIStyleFromJSON(const nlohmann::json& entry,
		const GUIElementStyleLoader& loader)
	{
		GUIElementStyle style;

		if(entry.Count("font") > 0)
		{
			std::string font = entry["font"];
			style.font = loader.LoadFont(font.c_str());
		}

		if(entry.Count("fontSize") > 0)
			style.fontSize = entry["fontSize"];

		if(entry.Count("textHorzAlign") > 0)
			style.textHorzAlign = entry["textHorzAlign"];

		if(entry.Count("textVertAlign") > 0)
			style.textVertAlign = entry["textVertAlign"];

		if(entry.Count("imagePosition") > 0)
			style.imagePosition = entry["imagePosition"];

		if(entry.Count("wordWrap") > 0)
			style.wordWrap = entry["wordWrap"];

		const auto loadState = [&loader, &entry](const char* name, GUIElementStateStyle& state)
		{
			if (entry.Count(name) == 0)
				return false;

			nlohmann::json subEntry = entry[name];

			if(subEntry.Count("texture") > 0)
			{
				std::string texture = subEntry["texture"];
				state.texture = loader.LoadTexture(texture.c_str());
			}

			if(subEntry.Count("textColor") > 0)
			{
				nlohmann::json colorEntry = subEntry["textColor"];

				state.textColor.r = colorEntry["r"];
				state.textColor.g = colorEntry["g"];
				state.textColor.b = colorEntry["b"];
				state.textColor.a = colorEntry["a"];
			}

			return true;
		};

		loadState("normal", style.normal);

		const bool hasHover = loadState("hover", style.hover);
		if(!hasHover)
			style.hover = style.normal;

		if(!loadState("active", style.active))
			style.active = style.normal;

		if(!loadState("focused", style.focused))
			style.focused = style.normal;

		if(!loadState("focusedHover", style.focusedHover))
		{
			if(hasHover)
				style.focusedHover = style.hover;
			else
				style.focusedHover = style.normal;
		}

		loadState("normalOn", style.normalOn);

		const bool hasHoverOn = loadState("hoverOn", style.hoverOn);
		if(!hasHoverOn)
			style.hoverOn = style.normalOn;

		if(!loadState("activeOn", style.activeOn))
			style.activeOn = style.normalOn;

		if(!loadState("focusedOn", style.focusedOn))
			style.focusedOn = style.normalOn;

		if(!loadState("focusedHoverOn", style.focusedHoverOn))
		{
			if(hasHoverOn)
				style.focusedHoverOn = style.hoverOn;
			else
				style.focusedHoverOn = style.normalOn;
		}

		const auto loadRectOffset = [entry](const char* name, RectOffset& state)
		{
			if (entry.Count(name) == 0)
				return;

			nlohmann::json subEntry = entry[name];
			state.left = subEntry["left"];
			state.right = subEntry["right"];
			state.top = subEntry["top"];
			state.bottom = subEntry["bottom"];
		};

		loadRectOffset("border", style.border);
		loadRectOffset("margins", style.margins);
		loadRectOffset("contentOffset", style.contentOffset);
		loadRectOffset("padding", style.padding);

		if(entry.Count("width") > 0)
			style.width = entry["width"];

		if(entry.Count("height") > 0)
			style.height = entry["height"];

		if(entry.Count("minWidth") > 0)
			style.minWidth = entry["minWidth"];

		if(entry.Count("maxWidth") > 0)
			style.maxWidth = entry["maxWidth"];

		if(entry.Count("minHeight") > 0)
			style.minHeight = entry["minHeight"];
		
		if(entry.Count("maxHeight") > 0)
			style.maxHeight = entry["maxHeight"];

		if(entry.Count("fixedWidth") > 0)
			style.fixedWidth = entry["fixedWidth"];

		if(entry.Count("fixedHeight") > 0)
			style.fixedHeight = entry["fixedHeight"];

		if(entry.Count("subStyles") > 0)
		{
			nlohmann::json subStyles = entry["subStyles"];
			for (auto& subStyle : subStyles)
			{
				std::string name = subStyle["name"];
				std::string styleName = subStyle["style"];

				style.subStyles.Insert(std::make_pair(name.c_str(), styleName.c_str()));
			}
		}

		return style;
	}

	BuiltinResourceGUIElementStyleLoader::BuiltinResourceGUIElementStyleLoader(const Path& fontPath, const Path& texturePath)
		:mFontPath(fontPath), mTexturePath(texturePath)
	{ }


	HSpriteTexture BuiltinResourceGUIElementStyleLoader::LoadTexture(const String& name) const
	{
		Path texturePath = mTexturePath;
		texturePath.Append(u8"sprite_" + name + u8".asset");

		return GResources().load<SpriteTexture>(texturePath);
	}

	HFont BuiltinResourceGUIElementStyleLoader::LoadFont(const String& name) const
	{
		Path fontPath = mFontPath;
		fontPath.Append(name + u8".asset");

		return GResources().load<Font>(fontPath);
	}
}
