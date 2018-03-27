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

		auto importResource = [&](const String& fileName, const UUID& UUID)
		{
			Path filePath = inputFolder + fileName;
			Vector<std::pair<Path, SPtr<ImportOptions>>> resourcesToSave;

			{
				Path relativePath = fileName;
				Path relativeAssetPath = fileName;
				relativeAssetPath.setFilename(relativeAssetPath.getWFilename() + L".asset");

				SPtr<ImportOptions> importOptions = gImporter().createImportOptions(filePath);
				if (importOptions != nullptr)
				{
					if (rtti_is_of_type<TextureImportOptions>(importOptions))
					{
						SPtr<TextureImportOptions> texImportOptions = std::static_pointer_cast<TextureImportOptions>(importOptions);
						texImportOptions->setGenerateMipmaps(false);

						resourcesToSave.push_back(std::make_pair(relativeAssetPath, texImportOptions));
					}
					else if (rtti_is_of_type<ShaderImportOptions>(importOptions))
					{
						ShaderDefines defines = RendererMaterialManager::_getDefines(relativePath);

						SPtr<ShaderImportOptions> shaderImportOptions;
						if (!defines.getAll().empty())
						{
							shaderImportOptions =
								std::static_pointer_cast<ShaderImportOptions>(gImporter().createImportOptions(filePath));

							shaderImportOptions->getDefines() = defines.getAll();
						}

						resourcesToSave.push_back(std::make_pair(relativeAssetPath, shaderImportOptions));
					}
					else
						resourcesToSave.push_back(std::make_pair(relativeAssetPath, nullptr));
				}
				else
					resourcesToSave.push_back(std::make_pair(relativeAssetPath, nullptr));
			}

			// Use the provided UUID if just one resource, otherwise we ignore the UUID. The current assumption is that
			// such resources don't require persistent UUIDs. If that changes then this method needs to be updated.
			Vector<HResource> savedResources(resourcesToSave.size());
			if (resourcesToSave.size() == 1)
			{
				Path outputPath = outputFolder + resourcesToSave[0].first;

				HResource resource = Importer::instance().import(filePath, resourcesToSave[0].second, UUID);
				if (resource != nullptr)
				{
					Resources::instance().save(resource, outputPath, true, compress);
					manifest->registerResource(resource.getUUID(), outputPath);
				}

				savedResources[0] = resource;
			}
			else
			{
				UINT32 idx = 0;
				for (auto& entry : resourcesToSave)
				{
					Path outputPath = outputFolder + entry.first;;

					HResource resource = Importer::instance().import(filePath, entry.second);
					if (resource != nullptr)
					{
						Resources::instance().save(resource, outputPath, true, compress);
						manifest->registerResource(resource.getUUID(), outputPath);
					}

					savedResources[idx] = resource;
					idx++;
				}
			}

			return savedResources;
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

		struct IconData
		{
			String name;
			HTexture source;
			SPtr<PixelData> srcData;
			std::string TextureUUIDs[3];
			std::string SpriteUUIDs[3];
		};

		Vector<IconData> iconsToGenerate;

		int idx = 0;
		for(auto& entry : entries)
		{
			if(!importFlags[idx])
			{
				idx++;
				continue;
			}

			std::string name = entry["Path"];
			std::string uuid;

			bool isIcon = false;
			if (mode == AssetType::Normal)
			{
				uuid = entry["UUID"];
				isIcon = entry.find("UUID16") != entry.end();
			}
			else if (mode == AssetType::Sprite)
			{
				uuid = entry["TextureUUID"];
				isIcon = entry.find("TextureUUID16") != entry.end();
			}

			Vector<HResource> outputResources = importResource(name.c_str(), UUID(uuid.c_str()));
			bool foundDependencies = false;
			for (auto& outputRes : outputResources)
			{
				if (outputRes == nullptr)
					continue;

				if (rtti_is_of_type<Shader>(outputRes.get()))
				{
					HShader shader = static_resource_cast<Shader>(outputRes);
					if (!verifyAndReportShader(shader))
						return;

					if (!foundDependencies && dependencies != nullptr)
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
						foundDependencies = true;
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

			idx++;
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

	void BuiltinResourcesHelper::importFont(const Path& inputFile, const WString& outputName, const Path& outputFolder,
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

		WString fontName = outputName;
		Path outputPath = outputFolder + fontName;
		outputPath.setFilename(outputPath.getWFilename() + L".asset");

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
				texPageOutputPath.setFilename(fontName + L"_" + toWString(size) + L"_texpage_" +
					toWString(pageIdx) + L".asset");

				Resources::instance().save(tex, texPageOutputPath, true);
				manifest->registerResource(tex.getUUID(), texPageOutputPath);
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
			path.setFilename(path.getWFilename() + L".asset");

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

				texPath.setFilename(texPath.getWFilename() + L"48.asset");
				manifest->registerResource(UUID(texUUIDs[0].c_str()), texPath);

				texPath.setFilename(texPath.getWFilename() + L"32.asset");
				manifest->registerResource(UUID(texUUIDs[1].c_str()), texPath);

				texPath.setFilename(texPath.getWFilename() + L"16.asset");
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
						String errMsg = "Error occured while compiling a shader \"" + toString(shader->getName()) 
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
		HShader shader = gResources().load<Shader>(path);
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
}