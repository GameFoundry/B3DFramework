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
#include "2D/BsSpriteTexture.h"
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
		nlohmann::json* dependencies, bool compress)
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
			QueuedImportOp(const AsyncOp& op, const Path& outputPath, const nlohmann::json& jsonEntry)
				:op(op), outputPath(outputPath), jsonEntry(jsonEntry)
			{ }

			AsyncOp op;
			Path outputPath;
			const nlohmann::json& jsonEntry;
		};

		List<QueuedImportOp> queuedOps;
		auto importResource = [&](const nlohmann::json& entry)
		{
			std::string name = entry["Path"];
			std::string uuidStr;

			if (mode == AssetType::Normal)
				uuidStr = entry["UUID"];
			else if (mode == AssetType::Sprite)
				uuidStr = entry["TextureUUID"];

			String fileName = name.c_str();
			UUID UUID(uuidStr.c_str());

			Path filePath = inputFolder + fileName;

			Path relativePath = fileName;
			Path relativeAssetPath = fileName;
			relativeAssetPath.setFilename(relativeAssetPath.getFilename() + u8".asset");

			SPtr<ImportOptions> importOptions = gImporter().createImportOptions(filePath);
			if (importOptions != nullptr)
			{
				if (rtti_is_of_type<TextureImportOptions>(importOptions))
				{
					SPtr<TextureImportOptions> texImportOptions = 
						std::static_pointer_cast<TextureImportOptions>(importOptions);

					texImportOptions->setGenerateMipmaps(false);
				}
				else if (rtti_is_of_type<ShaderImportOptions>(importOptions))
				{
					ShaderDefines defines = RendererMaterialManager::_getDefines(relativePath);

					SPtr<ShaderImportOptions> shaderImportOptions = 
						std::static_pointer_cast<ShaderImportOptions>(importOptions);

					if (!defines.getAll().empty())
						shaderImportOptions->getDefines() = defines.getAll();
				}
			}

			Path outputPath = outputFolder + relativeAssetPath;

			AsyncOp op = gImporter().importAsync(filePath, importOptions, UUID);
			queuedOps.emplace_back(op, outputPath, entry);
		};

		auto generateSprite = [&](const HTexture& texture, const String& fileName, const UUID& UUID)
		{
			Path relativePath = fileName;
			Path outputPath = spriteOutputFolder + relativePath;

			outputPath.setFilename("sprite_" + fileName + ".asset");

			SPtr<SpriteTexture> spriteTexPtr = SpriteTexture::_createPtr(texture);
			HResource spriteTex = gResources()._createResourceHandle(spriteTexPtr, UUID);

			Resources::instance().save(spriteTex, outputPath, true, compress);
			manifest->registerResource(spriteTex.getUUID(), outputPath);
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
		while(!queuedOps.empty())
		{
			for(auto iter = queuedOps.begin(); iter != queuedOps.end();)
			{
				QueuedImportOp& importOp = *iter;
				if(!importOp.op.hasCompleted())
				{
					++iter;
					continue;
				}

				HResource outputRes = importOp.op.getReturnValue<HResource>();
				if (outputRes != nullptr)
				{
					Resources::instance().save(outputRes, importOp.outputPath, true, compress);
					manifest->registerResource(outputRes.getUUID(), importOp.outputPath);

					const nlohmann::json& entry = importOp.jsonEntry;

					std::string name = entry["Path"];

					bool isIcon = false;
					if (mode == AssetType::Normal)
						isIcon = entry.find("UUID16") != entry.end();
					else if (mode == AssetType::Sprite)
						isIcon = entry.find("TextureUUID16") != entry.end();

					if (rtti_is_of_type<Shader>(outputRes.get()))
					{
						HShader shader = static_resource_cast<Shader>(outputRes);
						if (!verifyAndReportShader(shader))
						{
							iter = queuedOps.erase(iter);
							continue;
						}

						if (dependencies != nullptr)
						{
							SPtr<ShaderMetaData> shaderMetaData = std::static_pointer_cast<ShaderMetaData>(shader->getMetaData());

							nlohmann::json dependencyEntries;
							if (shaderMetaData != nullptr && shaderMetaData->includes.size() > 0)
							{
								for (auto& include : shaderMetaData->includes)
								{
									Path includePath = include.c_str();
									if (include.substr(0, 8) == "$ENGINE$" || include.substr(0, 8) == "$EDITOR$")
									{
										if (include.size() > 8)
											includePath = include.substr(9, include.size() - 9);
									}

									nlohmann::json newDependencyEntry =
									{
										{ "Path", includePath.toString().c_str() }
									};

									dependencyEntries.push_back(newDependencyEntry);
								}
							}

							(*dependencies)[name] = dependencyEntries;
						}
					}

					if (mode == AssetType::Sprite)
					{
						std::string spriteUUID = entry["SpriteUUID"];

						HTexture tex = static_resource_cast<Texture>(outputRes);
						generateSprite(tex, name.c_str(), UUID(spriteUUID.c_str()));
					}

					if (isIcon)
					{
						IconData iconData;
						iconData.source = static_resource_cast<Texture>(outputRes);
						iconData.name = name.c_str();

						if (mode == AssetType::Normal)
						{
							iconData.TextureUUIDs[0] = entry["UUID48"];
							iconData.TextureUUIDs[1] = entry["UUID32"];
							iconData.TextureUUIDs[2] = entry["UUID16"];
						}
						else if (mode == AssetType::Sprite)
						{
							iconData.TextureUUIDs[0] = entry["TextureUUID48"];
							iconData.TextureUUIDs[1] = entry["TextureUUID32"];
							iconData.TextureUUIDs[2] = entry["TextureUUID16"];

							iconData.SpriteUUIDs[0] = entry["SpriteUUID48"];
							iconData.SpriteUUIDs[1] = entry["SpriteUUID32"];
							iconData.SpriteUUIDs[2] = entry["SpriteUUID16"];
						}

						iconsToGenerate.push_back(iconData);
					}
				}

				iter = queuedOps.erase(iter);
			}
		}

		for(UINT32 i = 0; i < (UINT32)iconsToGenerate.size(); i++)
		{
			IconData& data = iconsToGenerate[i];

			data.srcData = data.source->getProperties().allocBuffer(0, 0);
			data.source->readData(data.srcData);
		}

		gCoreThread().submit(true);

		auto saveTexture = [&](auto& pixelData, auto& path, std::string& uuid)
		{
			SPtr<Texture> texturePtr = Texture::_createPtr(pixelData);
			HResource texture = gResources()._createResourceHandle(texturePtr, UUID(uuid.c_str()));

			Resources::instance().save(texture, path, true, compress);
			manifest->registerResource(texture.getUUID(), path);

			return static_resource_cast<Texture>(texture);
		};

		for (UINT32 i = 0; i < (UINT32)iconsToGenerate.size(); i++)
		{
			SPtr<PixelData> src = iconsToGenerate[i].srcData;

			SPtr<PixelData> scaled48 = PixelData::create(48, 48, 1, src->getFormat());
			PixelUtil::scale(*src, *scaled48);

			SPtr<PixelData> scaled32 = PixelData::create(32, 32, 1, src->getFormat());
			PixelUtil::scale(*scaled48, *scaled32);

			SPtr<PixelData> scaled16 = PixelData::create(16, 16, 1, src->getFormat());
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
		SPtr<ImportOptions> fontImportOptions = Importer::instance().createImportOptions(inputFile);
		if (rtti_is_of_type<FontImportOptions>(fontImportOptions))
		{
			FontImportOptions* importOptions = static_cast<FontImportOptions*>(fontImportOptions.get());

			importOptions->setFontSizes(fontSizes);
			importOptions->setRenderMode(antialiasing ? FontRenderMode::HintedSmooth : FontRenderMode::HintedRaster);
		}
		else
			return;

		HFont font = Importer::instance().import<Font>(inputFile, fontImportOptions, UUID);

		String fontName = outputName;
		Path outputPath = outputFolder + fontName;
		outputPath.setFilename(outputPath.getFilename() + u8".asset");

		Resources::instance().save(font, outputPath, true);
		manifest->registerResource(font.getUUID(), outputPath);

		// Save font texture pages as well. TODO - Later maybe figure out a more automatic way to do this
		for (auto& size : fontSizes)
		{
			SPtr<const FontBitmap> fontData = font->getBitmap(size);

			Path texPageOutputPath = outputFolder;

			UINT32 pageIdx = 0;
			for (auto tex : fontData->texturePages)
			{
				texPageOutputPath.setFilename(fontName + u8"_" + toString(size) + u8"_texpage_" +
					toString(pageIdx) + u8".asset");

				Resources::instance().save(tex, texPageOutputPath, true);
				manifest->registerResource(tex.getUUID(), texPageOutputPath);

				pageIdx++;
			}
		}
	}

	Vector<bool> BuiltinResourcesHelper::generateImportFlags(const nlohmann::json& entries, const Path& inputFolder,
		time_t lastUpdateTime, bool forceImport, const nlohmann::json* dependencies, const Path& dependencyFolder)
	{
		Vector<bool> output(entries.size());
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
					auto iterFind = dependencies->find(name);
					if(iterFind != dependencies->end())
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

	bool BuiltinResourcesHelper::updateJSON(const Path& folder, AssetType type, nlohmann::json& entries)
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
			Path relativePath = filePath.getRelative(folder);

			auto iterFind = existingEntries.find(relativePath);
			if(iterFind == existingEntries.end())
			{
				if(type == AssetType::Normal)
				{
					String uuid = UUIDGenerator::generateRandom().toString();
					nlohmann::json newEntry =
					{ 
						{ "Path", relativePath.toString().c_str() },
						{ "UUID", uuid.c_str() } 
					};

					entries.push_back(newEntry);
				}
				else // Sprite
				{
					String texUuid = UUIDGenerator::generateRandom().toString();
					String spriteUuid = UUIDGenerator::generateRandom().toString();
					nlohmann::json newEntry = 
					{ 
						{ "Path", relativePath.toString().c_str() },
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
		auto iter = entries.begin();
		while(iter != entries.end())
		{
			std::string strPath = (*iter)["Path"];
			Path path = strPath.c_str();
			path = path.getAbsolute(folder);

			if (!FileSystem::exists(path))
			{
				iter = entries.erase(iter);
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
				uuid = entry["UUID"];
				isIcon = entry.find("UUID16") != entry.end();
			}
			else if (type == AssetType::Sprite)
			{
				uuid = entry["TextureUUID"];
				isIcon = entry.find("TextureUUID16") != entry.end();
			}

			Path path = folder + name.c_str();
			path.setFilename(path.getFilename() + u8".asset");

			manifest->registerResource(UUID(uuid.c_str()), path);
			
			if (type == AssetType::Sprite)
			{
				std::string spriteUUID = entry["SpriteUUID"];

				Path spritePath = folder + "/Sprites/";
				spritePath.setFilename(String("sprite_") + name.c_str() + ".asset");

				manifest->registerResource(UUID(spriteUUID.c_str()), spritePath);
			}

			if (isIcon)
			{
				std::string texUUIDs[3];

				if (type == AssetType::Normal)
				{
					texUUIDs[0] = entry["UUID48"];
					texUUIDs[1] = entry["UUID32"];
					texUUIDs[2] = entry["UUID16"];
				}
				else if (type == AssetType::Sprite)
				{
					texUUIDs[0] = entry["TextureUUID48"];
					texUUIDs[1] = entry["TextureUUID32"];
					texUUIDs[2] = entry["TextureUUID16"];
				}

				Path texPath = folder + name.c_str();

				texPath.setFilename(texPath.getFilename() + u8"48.asset");
				manifest->registerResource(UUID(texUUIDs[0].c_str()), texPath);

				texPath.setFilename(texPath.getFilename() + u8"32.asset");
				manifest->registerResource(UUID(texUUIDs[1].c_str()), texPath);

				texPath.setFilename(texPath.getFilename() + u8"16.asset");
				manifest->registerResource(UUID(texUUIDs[2].c_str()), texPath);

				if(type == AssetType::Sprite)
				{
					std::string spriteUUIDs[3];

					spriteUUIDs[0] = entry["SpriteUUID48"];
					spriteUUIDs[1] = entry["SpriteUUID32"];
					spriteUUIDs[2] = entry["SpriteUUID16"];
					
					Path spritePath = folder + "/Sprites/";

					spritePath.setFilename(String("sprite_") + name.c_str() + "48.asset");
					manifest->registerResource(UUID(spriteUUIDs[0].c_str()), spritePath);

					spritePath.setFilename(String("sprite_") + name.c_str() + "32.asset");
					manifest->registerResource(UUID(spriteUUIDs[1].c_str()), spritePath);

					spritePath.setFilename(String("sprite_") + name.c_str() + "16.asset");
					manifest->registerResource(UUID(spriteUUIDs[2].c_str()), spritePath);
				}
			}
		}
	}

	void BuiltinResourcesHelper::writeTimestamp(const Path& file)
	{
		SPtr<DataStream> fileStream = FileSystem::createAndOpenFile(file);

		time_t currentTime = std::time(nullptr);
		fileStream->write(&currentTime, sizeof(currentTime));
		fileStream->close();
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

	bool BuiltinResourcesHelper::verifyAndReportShader(const HShader& shader)
	{
		if(!shader.isLoaded(false) || shader->getNumTechniques() == 0)
		{
#if BS_DEBUG_MODE
			BS_EXCEPT(InvalidStateException, "Error occured while compiling a shader. Check earlier log messages for exact error.");
#else
			LOGERR("Error occured while compiling a shader. Check earlier log messages for exact error.")
#endif
			return false;
		}

		Vector<SPtr<Technique>> techniques = shader->getCompatibleTechniques();
		for(auto& technique : techniques)
		{
			technique->compile();

			UINT32 numPasses = technique->getNumPasses();
			for(UINT32 i = 0; i < numPasses; i++)
			{
				SPtr<Pass> pass = technique->getPass(i);

				std::array<SPtr<GpuProgram>, 6> gpuPrograms;

				const SPtr<GraphicsPipelineState>& graphicsPipeline = pass->getGraphicsPipelineState();
				if (graphicsPipeline)
				{
					gpuPrograms[0] = graphicsPipeline->getVertexProgram();
					gpuPrograms[1] = graphicsPipeline->getFragmentProgram();
					gpuPrograms[2] = graphicsPipeline->getGeometryProgram();
					gpuPrograms[3] = graphicsPipeline->getHullProgram();
					gpuPrograms[4] = graphicsPipeline->getDomainProgram();
				}

				const SPtr<ComputePipelineState>& computePipeline = pass->getComputePipelineState();
				if (computePipeline)
					gpuPrograms[5] = computePipeline->getProgram();

				for(auto& program : gpuPrograms)
				{
					if (program == nullptr)
						continue;

					program->blockUntilCoreInitialized();
					if(!program->isCompiled())
					{
						String errMsg = "Error occured while compiling a shader \"" + shader->getName() 
							+ "\". Error message: " + program->getCompileErrorMessage();

#if BS_DEBUG_MODE
						BS_EXCEPT(InvalidStateException, errMsg);
#else
						LOGERR(errMsg)
#endif
						return false;
					}
				}
			}
		}

		return true;
	}

	void BuiltinResourcesHelper::updateShaderBytecode(const Path& path)
	{
		HShader shader = gResources().load<Shader>(path, ResourceLoadFlag::KeepSourceData);
		if (!shader)
			return;

		Vector<SPtr<Technique>> techniques = shader->getCompatibleTechniques();
		bool hasBytecode = true;
		for (auto& technique : techniques)
		{
			UINT32 numPasses = technique->getNumPasses();
			for (UINT32 i = 0; i < numPasses; i++)
			{
				SPtr<Pass> pass = technique->getPass(i);

				for (UINT32 j = 0; j < GPT_COUNT; j++)
				{
					const GPU_PROGRAM_DESC& desc = pass->getProgramDesc((GpuProgramType)j);
					if (desc.source.empty())
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
			technique->compile();

		gResources().save(shader, path, true, true);
	}

	GUIElementStyle BuiltinResourcesHelper::loadGUIStyleFromJSON(const nlohmann::json& entry, 
		const GUIElementStyleLoader& loader)
	{
		GUIElementStyle style;

		if(entry.count("font") > 0)
		{
			std::string font = entry["font"];
			style.font = loader.loadFont(font.c_str());
		}

		if(entry.count("fontSize") > 0)
			style.fontSize = entry["fontSize"];

		if(entry.count("textHorzAlign") > 0)
			style.textHorzAlign = entry["textHorzAlign"];

		if(entry.count("textVertAlign") > 0)
			style.textVertAlign = entry["textVertAlign"];

		if(entry.count("imagePosition") > 0)
			style.imagePosition = entry["imagePosition"];

		if(entry.count("wordWrap") > 0)
			style.wordWrap = entry["wordWrap"];

		const auto loadState = [&loader, &entry](const char* name, GUIElementStyle::GUIElementStateStyle& state)
		{
			if (entry.count(name) == 0)
				return;

			nlohmann::json subEntry = entry[name];

			if(subEntry.count("texture") > 0)
			{
				std::string texture = subEntry["texture"];
				state.texture = loader.loadTexture(texture.c_str());
			}

			if(subEntry.count("textColor") > 0)
			{
				nlohmann::json colorEntry = subEntry["textColor"];

				state.textColor.r = colorEntry["r"];
				state.textColor.g = colorEntry["g"];
				state.textColor.b = colorEntry["b"];
				state.textColor.a = colorEntry["a"];
			}
		};

		loadState("normal", style.normal);
		loadState("hover", style.hover);
		loadState("active", style.active);
		loadState("focused", style.focused);

		loadState("normalOn", style.normalOn);
		loadState("hoverOn", style.hoverOn);
		loadState("activeOn", style.activeOn);
		loadState("focusedOn", style.focusedOn);

		const auto loadRectOffset = [entry](const char* name, RectOffset& state)
		{
			if (entry.count(name) == 0)
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

		if(entry.count("width") > 0)
			style.width = entry["width"];

		if(entry.count("height") > 0)
			style.height = entry["height"];

		if(entry.count("minWidth") > 0)
			style.minWidth = entry["minWidth"];

		if(entry.count("maxWidth") > 0)
			style.maxWidth = entry["maxWidth"];

		if(entry.count("minHeight") > 0)
			style.minHeight = entry["minHeight"];
		
		if(entry.count("maxHeight") > 0)
			style.maxHeight = entry["maxHeight"];

		if(entry.count("fixedWidth") > 0)
			style.fixedWidth = entry["fixedWidth"];

		if(entry.count("fixedHeight") > 0)
			style.fixedHeight = entry["fixedHeight"];

		if(entry.count("subStyles") > 0)
		{
			nlohmann::json subStyles = entry["subStyles"];
			for (auto& subStyle : subStyles)
			{
				std::string name = subStyle["name"];
				std::string styleName = subStyle["style"];

				style.subStyles.insert(std::make_pair(name.c_str(), styleName.c_str()));
			}
		}

		return style;
	}

	BuiltinResourceGUIElementStyleLoader::BuiltinResourceGUIElementStyleLoader(const Path& fontPath, const Path& texturePath)
		:mFontPath(fontPath), mTexturePath(texturePath)
	{ }


	HSpriteTexture BuiltinResourceGUIElementStyleLoader::loadTexture(const String& name) const
	{
		Path texturePath = mTexturePath;
		texturePath.append(u8"sprite_" + name + u8".asset");

		return gResources().load<SpriteTexture>(texturePath);
	}

	HFont BuiltinResourceGUIElementStyleLoader::loadFont(const String& name) const
	{
		Path fontPath = mFontPath;
		fontPath.append(name + u8".asset");

		return gResources().load<Font>(fontPath);
	}
}