//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DBuiltinResourcesHelper.h"

#include "B3DPackage.h"
#include "FileSystem/B3DFileSystem.h"
#include "Importer/B3DImporter.h"
#include "Resources/B3DResources.h"
#include "Importer/B3DShaderImportOptions.h"
#include "Importer/B3DTextureImportOptions.h"
#include "Renderer/B3DRendererMaterialManager.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Text/B3DFontImportOptions.h"
#include "Image/B3DSpriteTexture.h"
#include "Image/B3DTexture.h"
#include "Reflection/B3DRTTIType.h"
#include "FileSystem/B3DDataStream.h"
#include "FileSystem/B3DFileSystem.h"
#include "CoreObject/B3DRenderThread.h"
#include "Utility/B3DUUID.h"
#include "Material/B3DShader.h"
#include "Material/B3DPass.h"
#include "RenderAPI/B3DGpuProgram.h"

using json = nlohmann::json;

using namespace b3d;

void BuiltinResourcesHelper::ImportAssets(const nlohmann::json& entries, const Vector<bool>& importFlags, const Path& inputFolder, const Path& outputFolder, AssetType mode, bool compress, bool mipmap)
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
		QueuedImportOp(const TAsyncOp<HResource>& op, const Path& outputFolder, const String& outputName, const nlohmann::json& jsonEntry)
			: Op(op), OutputFolder(outputFolder), OutputName(outputName), JsonEntry(jsonEntry)
		{}

		TAsyncOp<HResource> Op;
		Path OutputFolder;
		String OutputName;
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

		TAsyncOp<HResource> op = GetImporter().ImportAsync(filePath, importOptions, UUID);
		queuedOps.emplace_back(op, outputFolder, fileName, entry);
	};

	auto fnGenerateAndSaveSprite = [&spriteOutputFolder, compress](const HTexture& texture, const String& fileName, const UUID& UUID)
	{
		const String spriteName = String("sprite_" + fileName);

		HResource spriteTexture = GetResources().CreateResourceHandle(SpriteTexture::CreateShared(texture), UUID);
		GetResources().SaveAsSinglePackage(spriteTexture, spriteOutputFolder, spriteName, ResourceSaveOptions(true, compress));
	};

	auto fnGenerateAndSaveAnimatedSprite = [&spriteOutputFolder, compress](const HTexture& texture, const String& fileName, const UUID& UUID,
									  SpriteAnimationPlayback playback, const SpriteSheetGridAnimation& animation)
	{
		const String spriteName = String("sprite_" + fileName);

		SPtr<SpriteTexture> spriteTextureShared = SpriteTexture::CreateShared(texture);
		spriteTextureShared->SetAnimation(animation);
		spriteTextureShared->SetAnimationPlayback(playback);

		HResource spriteTexture = GetResources().CreateResourceHandle(spriteTextureShared, UUID);
		GetResources().SaveAsSinglePackage(spriteTexture, spriteOutputFolder, spriteName, ResourceSaveOptions(true, compress));
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
				GetResources().SaveAsSinglePackage(outputRes, importOp.OutputFolder, importOp.OutputName, ResourceSaveOptions(true, compress));

				const nlohmann::json& entry = importOp.JsonEntry;

				std::string name = entry["Path"];

				bool isIcon = false;
				if(mode == AssetType::Normal)
					isIcon = entry.find("UUID16") != entry.end();
				else if(mode == AssetType::Sprite)
					isIcon = entry.find("TextureUUID16") != entry.end();

				if(mode == AssetType::Sprite)
				{
					HTexture tex = B3DStaticResourceCast<Texture>(outputRes);
					std::string spriteUUID = entry["SpriteUUID"];

					bool isAnimated = entry.find("Animation") != entry.end();
					if(isAnimated)
					{
						auto& jsonAnimation = entry["Animation"];

						SpriteSheetGridAnimation animation;
						animation.RowCount = jsonAnimation["NumRows"].get<u32>();
						animation.ColumnCount = jsonAnimation["NumColumns"].get<u32>();
						animation.FrameCount = jsonAnimation["Count"].get<u32>();
						animation.FramesPerSecond = jsonAnimation["FPS"].get<u32>();

						fnGenerateAndSaveAnimatedSprite(tex, name.c_str(), UUID(spriteUUID.c_str()), SpriteAnimationPlayback::Loop, animation);
					}
					else
						fnGenerateAndSaveSprite(tex, name.c_str(), UUID(spriteUUID.c_str()));
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

	GetRenderThread().PostCommand([] {}, "Reading back generated icon data", true);

	auto fnSaveTexture = [compress](const SPtr<PixelData>& pixelData, const Path& folder, const String& name, std::string& uuid)
	{
		SPtr<Texture> texturePtr = Texture::CreateShared(pixelData);
		HResource texture = GetResources().CreateResourceHandle(texturePtr, UUID(uuid.c_str()));

		GetResources().SaveAsSinglePackage(texture, folder, name, ResourceSaveOptions(true, compress));
		return B3DStaticResourceCast<Texture>(texture);
	};

	for(u32 i = 0; i < (u32)iconsToGenerate.size(); i++)
	{
		SPtr<PixelData> src = iconsToGenerate[i].SrcData;

		SPtr<PixelData> scaled48 = PixelData::Create(48, 48, 1, src->GetFormat());
		PixelUtility::Scale(*src, *scaled48);

		SPtr<PixelData> scaled32 = PixelData::Create(32, 32, 1, src->GetFormat());
		PixelUtility::Scale(*scaled48, *scaled32);

		SPtr<PixelData> scaled16 = PixelData::Create(16, 16, 1, src->GetFormat());
		PixelUtility::Scale(*scaled32, *scaled16);

		const String iconName48 = iconsToGenerate[i].Name + "48";
		const String iconName32 = iconsToGenerate[i].Name + "32";
		const String iconName16 = iconsToGenerate[i].Name + "16";

		HTexture tex48 = fnSaveTexture(scaled48, outputFolder, iconName48, iconsToGenerate[i].TextureUUIDs[0]);
		HTexture tex32 = fnSaveTexture(scaled32, outputFolder, iconName32, iconsToGenerate[i].TextureUUIDs[1]);
		HTexture tex16 = fnSaveTexture(scaled16, outputFolder, iconName16, iconsToGenerate[i].TextureUUIDs[2]);

		if(mode == AssetType::Sprite)
		{
			fnGenerateAndSaveSprite(tex48, iconName48, UUID(iconsToGenerate[i].SpriteUUIDs[0].c_str()));
			fnGenerateAndSaveSprite(tex32, iconName32, UUID(iconsToGenerate[i].SpriteUUIDs[1].c_str()));
			fnGenerateAndSaveSprite(tex16, iconName16, UUID(iconsToGenerate[i].SpriteUUIDs[2].c_str()));
		}
	}
}

void BuiltinResourcesHelper::ImportFont(const Path& inputFile, const String& outputName, const Path& outputFolder, const Vector<float>& fontSizes, bool antialiasing, const UUID& UUID)
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

	const SPtr<Package> package = Package::Create(outputName);
	package->AddResource(outputName, font);

	// Save font texture pages as well
	for(auto& size : fontSizes)
	{
		SPtr<const FontBitmapInformation> fontData = font->GetBitmap(size);

		UnorderedSet<HTexture> addedTextures;
		for(const auto& pair : fontData->Characters)
		{
			const FontBitmapPage& page = font->GetPage(pair.second.Page);
			if(page.Type == FontBitmapPageType::Runtime)
				continue;

			if(auto found = addedTextures.find(page.Texture); found == addedTextures.end())
			{
				const String& texturePageName = StringUtil::Format("{0}FontPage{1}", fontName, pair.second.Page);
				package->AddResource(texturePageName, page.Texture);
				addedTextures.insert(page.Texture);
			}
		}
	}

	const String& packageFilename = outputName + Package::kPackageExtension;
	const Path packagePath = Path::Combine(outputFolder, packageFilename);

	PackageManager& packageManager = GetPackageManager();

	PackageManagerSavePackageOptions savePackageOptions;
	savePackageOptions.Compress = false; // Note: Important that fonts are uncompressed, as the data will be read directly from the file when rendering the font glyphs

	packageManager.SavePackage(package, packagePath, savePackageOptions);
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
