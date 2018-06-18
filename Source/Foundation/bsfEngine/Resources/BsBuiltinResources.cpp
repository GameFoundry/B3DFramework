//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsBuiltinResources.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUIInputBox.h"
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUIDropDownContent.h"
#include "GUI/BsGUITooltip.h"
#include "GUI/BsGUISlider.h"
#include "GUI/BsGUIScrollBar.h"
#include "2D/BsTextSprite.h"
#include "Image/BsSpriteTexture.h"
#include "Text/BsFont.h"
#include "Image/BsTexture.h"
#include "Importer/BsImporter.h"
#include "Importer/BsTextureImportOptions.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResourcesHelper.h"
#include "Resources/BsResourceManifest.h"
#include "Material/BsShader.h"
#include "Material/BsMaterial.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsDataStream.h"
#include "CoreThread/BsCoreThread.h"
#include "Utility/BsShapeMeshes3D.h"
#include "Mesh/BsMesh.h"
#include "Serialization/BsFileSerializer.h"
#include "Renderer/BsRendererMeshData.h"
#include "RenderAPI/BsRenderAPI.h"
#include "RenderAPI/BsVertexDataDesc.h"

using json = nlohmann::json;

namespace bs
{
	constexpr const char* BuiltinResources::IconTextureName;
	constexpr const char* BuiltinResources::MultiLineLabelStyle;

	const String BuiltinResources::DefaultFontFilename = u8"arial.ttf";
	const UINT32 BuiltinResources::DefaultFontSize = 8;

	const String BuiltinResources::GUISkinFile = u8"GUISkin";

	/************************************************************************/
	/* 								GUI TEXTURES                      		*/
	/************************************************************************/

	const String BuiltinResources::WhiteTex = u8"White.psd";

	/************************************************************************/
	/* 							CURSOR TEXTURES                      		*/
	/************************************************************************/

	const String BuiltinResources::CursorArrowTex = u8"Arrow.psd";
	const String BuiltinResources::CursorArrowDragTex = u8"ArrowDrag.psd";
	const String BuiltinResources::CursorArrowLeftRightTex = u8"ArrowLeftRight.psd";
	const String BuiltinResources::CursorIBeamTex = u8"IBeam.psd";
	const String BuiltinResources::CursorDenyTex = u8"Deny.psd";
	const String BuiltinResources::CursorWaitTex = u8"Wait.psd";
	const String BuiltinResources::CursorSizeNESWTex = u8"SizeNESW.psd";
	const String BuiltinResources::CursorSizeNSTex = u8"SizeNS.psd";
	const String BuiltinResources::CursorSizeNWSETex = u8"SizeNWSE.psd";
	const String BuiltinResources::CursorSizeWETex = u8"SizeWE.psd";

	const Vector2I BuiltinResources::CursorArrowHotspot = Vector2I(10, 8);
	const Vector2I BuiltinResources::CursorArrowDragHotspot = Vector2I(8, 4);
	const Vector2I BuiltinResources::CursorArrowLeftRightHotspot = Vector2I(13, 9);
	const Vector2I BuiltinResources::CursorIBeamHotspot = Vector2I(16, 15);
	const Vector2I BuiltinResources::CursorDenyHotspot = Vector2I(15, 15);
	const Vector2I BuiltinResources::CursorWaitHotspot = Vector2I(15, 15);
	const Vector2I BuiltinResources::CursorSizeNESWHotspot = Vector2I(16, 15);
	const Vector2I BuiltinResources::CursorSizeNSHotspot = Vector2I(16, 15);
	const Vector2I BuiltinResources::CursorSizeNWSEHotspot = Vector2I(16, 15);
	const Vector2I BuiltinResources::CursorSizeWEHotspot = Vector2I(16, 15);

	/************************************************************************/
	/* 									SHADERS                      		*/
	/************************************************************************/

	const String BuiltinResources::ShaderSpriteTextFile = u8"SpriteText.bsl";
	const String BuiltinResources::ShaderSpriteImageAlphaFile = u8"SpriteImageAlpha.bsl";
	const String BuiltinResources::ShaderSpriteImageNoAlphaFile = u8"SpriteImageNoAlpha.bsl";
	const String BuiltinResources::ShaderSpriteLineFile = u8"SpriteLine.bsl";
	const String BuiltinResources::ShaderDiffuseFile = u8"Diffuse.bsl";
	const String BuiltinResources::ShaderTransparentFile = u8"Transparent.bsl";
	const String BuiltinResources::ShaderParticlesUnlitFile = u8"ParticlesUnlit.bsl";

	/************************************************************************/
	/* 								MESHES							  		*/
	/************************************************************************/

	const String BuiltinResources::MeshSphereFile = u8"Sphere.asset";
	const String BuiltinResources::MeshBoxFile = u8"Box.asset";
	const String BuiltinResources::MeshConeFile = u8"Cone.asset";
	const String BuiltinResources::MeshCylinderFile = u8"Cylinder.asset";
	const String BuiltinResources::MeshQuadFile = u8"Quad.asset";
	const String BuiltinResources::MeshDiscFile = u8"Disc.asset";

	/************************************************************************/
	/* 								TEXTURES							  	*/
	/************************************************************************/

	const String BuiltinResources::TextureWhiteFile = u8"White.asset";
	const String BuiltinResources::TextureBlackFile = u8"Black.asset";
	const String BuiltinResources::TextureNormalFile = u8"Normal.asset";

	BuiltinResources::~BuiltinResources()
	{
		mCursorArrow = nullptr;
		mCursorArrowDrag = nullptr;
		mCursorArrowLeftRight = nullptr;
		mCursorIBeam = nullptr;
		mCursorDeny = nullptr;
		mCursorWait = nullptr;
		mCursorSizeNESW = nullptr;
		mCursorSizeNS = nullptr;
		mCursorSizeNWSE = nullptr;
		mCursorSizeWE = nullptr;
		mFrameworkIcon = nullptr;
	}

	BuiltinResources::BuiltinResources()
	{
		// Set up paths
		mBuiltinRawDataFolder = Paths::getDataPath() + u8"Raw/";

		mBuiltinDataFolder = Paths::getDataPath();
		mEngineSkinSpritesFolder = mBuiltinDataFolder + SKIN_FOLDER + SPRITE_FOLDER;
		mEngineShaderFolder = mBuiltinDataFolder + SHADER_FOLDER;
		mEngineMeshFolder = mBuiltinDataFolder + MESH_FOLDER;
		mEngineCursorFolder = mBuiltinDataFolder + CURSOR_FOLDER;

		ResourceManifestPath = mBuiltinDataFolder + "ResourceManifest.asset";

		// Update from raw assets if needed
		if (FileSystem::exists(mBuiltinRawDataFolder))
		{
			time_t lastUpdateTime;
			UINT32 modifications = BuiltinResourcesHelper::checkForModifications(
				mBuiltinRawDataFolder,
				mBuiltinDataFolder + u8"Timestamp.asset",
				lastUpdateTime);

			// Check if manifest needs to be rebuilt
			if (modifications == 0 && !FileSystem::exists(ResourceManifestPath))
				modifications = 1;

			if (modifications > 0)
			{
				bool fullReimport = modifications == 2;

				SPtr<ResourceManifest> oldResourceManifest;
				if (!fullReimport && FileSystem::exists(ResourceManifestPath))
				{
					oldResourceManifest = ResourceManifest::load(ResourceManifestPath, mBuiltinDataFolder);
					if (oldResourceManifest != nullptr)
						gResources().registerResourceManifest(oldResourceManifest);
				}

				if (oldResourceManifest)
					mResourceManifest = oldResourceManifest;
				else
				{
					mResourceManifest = ResourceManifest::create("BuiltinResources");
					gResources().registerResourceManifest(mResourceManifest);
				}

				preprocess(fullReimport, lastUpdateTime);
				BuiltinResourcesHelper::writeTimestamp(mBuiltinDataFolder + u8"Timestamp.asset");

				ResourceManifest::save(mResourceManifest, ResourceManifestPath, mBuiltinDataFolder);
			}
		}

		// Load manifest
		if (mResourceManifest == nullptr)
		{
			if (FileSystem::exists(ResourceManifestPath))
				mResourceManifest = ResourceManifest::load(ResourceManifestPath, mBuiltinDataFolder);

			if (mResourceManifest == nullptr)
				mResourceManifest = ResourceManifest::create("BuiltinResources");

			gResources().registerResourceManifest(mResourceManifest);
		}

		if (FileSystem::exists(mBuiltinRawDataFolder))
		{
			// Update shader bytecode for the current render backend, if needed
			const RenderAPIInfo& apiInfo = ct::RenderAPI::instance().getAPIInfo();
			bool supportsBytecodeCaching = apiInfo.isFlagSet(RenderAPIFeatureFlag::ByteCodeCaching);

			if (supportsBytecodeCaching)
			{
				Path dataListsFilePath = mBuiltinRawDataFolder + DATA_LIST_JSON;
				SPtr<DataStream> dataListStream = FileSystem::openFile(dataListsFilePath);
				if (dataListStream)
				{
					json dataListJSON = json::parse(dataListStream->getAsString().c_str());

					json shadersJSON = dataListJSON["Shaders"];
					for (auto& entry : shadersJSON)
					{
						std::string uuidStr = entry["UUID"];
						UUID uuid(uuidStr.c_str());

						Path filePath;
						if (!mResourceManifest->uuidToFilePath(uuid, filePath))
							continue;

						BuiltinResourcesHelper::updateShaderBytecode(filePath);
					}
				}
			}
		}

		// Load basic resources
		mShaderSpriteText = getShader(ShaderSpriteTextFile);
		mShaderSpriteImage = getShader(ShaderSpriteImageAlphaFile);
		mShaderSpriteNonAlphaImage = getShader(ShaderSpriteImageNoAlphaFile);
		mShaderSpriteLine = getShader(ShaderSpriteLineFile);
		mShaderDiffuse = getShader(ShaderDiffuseFile);
		mShaderTransparent = getShader(ShaderTransparentFile);
		mShaderParticlesUnlit = getShader(ShaderParticlesUnlitFile);

		SPtr<PixelData> dummyPixelData = PixelData::create(2, 2, 1, PF_RGBA8);

		dummyPixelData->setColorAt(Color::Red, 0, 0);
		dummyPixelData->setColorAt(Color::Red, 0, 1);
		dummyPixelData->setColorAt(Color::Red, 1, 0);
		dummyPixelData->setColorAt(Color::Red, 1, 1);

		mDummyTexture = Texture::create(dummyPixelData);

		mWhiteSpriteTexture = getSkinTexture(WhiteTex);
		mDummySpriteTexture = SpriteTexture::create(mDummyTexture);

		mFont = gResources().load<Font>(mBuiltinDataFolder + (DefaultFontFilename + u8".asset"));
		mSkin = gResources().load<GUISkin>(mBuiltinDataFolder + (GUISkinFile + u8".asset"));
		mEmptySkin = GUISkin::create();

		/************************************************************************/
		/* 								CURSOR		                     		*/
		/************************************************************************/

		HTexture cursorArrowTex = getCursorTexture(CursorArrowTex);
		HTexture cursorArrowDragTex = getCursorTexture(CursorArrowDragTex);
		HTexture cursorArrowLeftRightTex = getCursorTexture(CursorArrowLeftRightTex);
		HTexture cursorIBeamTex = getCursorTexture(CursorIBeamTex);
		HTexture cursorDenyTex = getCursorTexture(CursorDenyTex);
		HTexture cursorWaitTex = getCursorTexture(CursorWaitTex);
		HTexture cursorSizeNESWTex = getCursorTexture(CursorSizeNESWTex);
		HTexture cursorSizeNSTex = getCursorTexture(CursorSizeNSTex);
		HTexture cursorSizeNWSETex = getCursorTexture(CursorSizeNWSETex);
		HTexture cursorSizeWETex = getCursorTexture(CursorSizeWETex);

		mCursorArrow = cursorArrowTex->getProperties().allocBuffer(0, 0);
		cursorArrowTex->readData(mCursorArrow);

		mCursorArrowDrag = cursorArrowDragTex->getProperties().allocBuffer(0, 0);
		cursorArrowDragTex->readData(mCursorArrowDrag);

		mCursorArrowLeftRight = cursorArrowLeftRightTex->getProperties().allocBuffer(0, 0);
		cursorArrowLeftRightTex->readData(mCursorArrowLeftRight);

		mCursorIBeam = cursorIBeamTex->getProperties().allocBuffer(0, 0);
		cursorIBeamTex->readData(mCursorIBeam);

		mCursorDeny = cursorDenyTex->getProperties().allocBuffer(0, 0);
		cursorDenyTex->readData(mCursorDeny);

		mCursorWait = cursorWaitTex->getProperties().allocBuffer(0, 0);
		cursorWaitTex->readData(mCursorWait);

		mCursorSizeNESW = cursorSizeNESWTex->getProperties().allocBuffer(0, 0);
		cursorSizeNESWTex->readData(mCursorSizeNESW);

		mCursorSizeNS = cursorSizeNSTex->getProperties().allocBuffer(0, 0);
		cursorSizeNSTex->readData(mCursorSizeNS);

		mCursorSizeNWSE = cursorSizeNWSETex->getProperties().allocBuffer(0, 0);
		cursorSizeNWSETex->readData(mCursorSizeNWSE);

		mCursorSizeWE = cursorSizeWETex->getProperties().allocBuffer(0, 0);
		cursorSizeWETex->readData(mCursorSizeWE);

		/************************************************************************/
		/* 								ICON		                     		*/
		/************************************************************************/

		Path iconPath = mBuiltinDataFolder + ICON_FOLDER;
		iconPath.append(String(IconTextureName) + u8".asset");

		HTexture iconTex = gResources().load<Texture>(iconPath);

		mFrameworkIcon = iconTex->getProperties().allocBuffer(0, 0);
		iconTex->readData(mFrameworkIcon);

		gCoreThread().submit(true);
	}

	void BuiltinResources::preprocess(bool forceImport, time_t lastUpdateTime)
	{
		// Hidden dependency: Textures need to be generated before shaders as they may use the default textures
		generateTextures();

		Path dataListsFilePath = mBuiltinRawDataFolder + DATA_LIST_JSON;
		SPtr<DataStream> dataListStream = FileSystem::openFile(dataListsFilePath);
		json dataListJSON = json::parse(dataListStream->getAsString().c_str());

		json skinJSON = dataListJSON["Skin"];
		json cursorsJSON = dataListJSON["Cursors"];
		json iconsJSON = dataListJSON["Icons"];
		json includesJSON = dataListJSON["Includes"];
		json shadersJSON = dataListJSON["Shaders"];

		Path rawSkinFolder = mBuiltinRawDataFolder + SKIN_FOLDER;
		Path rawCursorFolder = mBuiltinRawDataFolder + CURSOR_FOLDER;
		Path rawIconFolder = mBuiltinRawDataFolder + ICON_FOLDER;
		Path rawShaderFolder = mBuiltinRawDataFolder + SHADER_FOLDER;
		Path rawShaderIncludeFolder = mBuiltinRawDataFolder + SHADER_INCLUDE_FOLDER;

		// Update DataList.json if needed
		bool updatedDataLists = false;
		updatedDataLists |= BuiltinResourcesHelper::updateJSON(
			rawCursorFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			cursorsJSON);

		updatedDataLists |= BuiltinResourcesHelper::updateJSON(
			rawIconFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			iconsJSON);

		updatedDataLists |= BuiltinResourcesHelper::updateJSON(
			rawShaderIncludeFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			includesJSON);

		updatedDataLists |= BuiltinResourcesHelper::updateJSON(
			rawShaderFolder,
			BuiltinResourcesHelper::AssetType::Normal,
			shadersJSON);

		updatedDataLists |= BuiltinResourcesHelper::updateJSON(
			rawSkinFolder,
			BuiltinResourcesHelper::AssetType::Sprite,
			skinJSON);

		dataListStream->close();

		if(updatedDataLists)
		{
			FileSystem::remove(dataListsFilePath);

			dataListJSON["Skin"] = skinJSON;
			dataListJSON["Cursors"] = cursorsJSON;
			dataListJSON["Icons"] = iconsJSON;
			dataListJSON["Includes"] = includesJSON;
			dataListJSON["Shaders"] = shadersJSON;

			String jsonString = dataListJSON.dump(4).c_str();
			dataListStream = FileSystem::createAndOpenFile(dataListsFilePath);
			dataListStream->writeString(jsonString);
			dataListStream->close();
		}

		Path skinFolder = mBuiltinDataFolder + SKIN_FOLDER;
		Path iconFolder = mBuiltinDataFolder + ICON_FOLDER;
		Path shaderIncludeFolder = mBuiltinDataFolder + SHADER_INCLUDE_FOLDER;
		Path shaderDependenciesFile = mBuiltinDataFolder + "ShaderDependencies.json";

		// If forcing import, clear all data folders since everything will be recreated anyway
		if(forceImport)
		{
			FileSystem::remove(mEngineCursorFolder);
			FileSystem::remove(iconFolder);
			FileSystem::remove(shaderIncludeFolder);
			FileSystem::remove(mEngineShaderFolder);
			FileSystem::remove(skinFolder);
			
			FileSystem::remove(shaderDependenciesFile);
		}

		// Read shader dependencies JSON
		json shaderDependenciesJSON;
		if(FileSystem::exists(shaderDependenciesFile))
		{
			SPtr<DataStream> stream = FileSystem::openFile(shaderDependenciesFile);
			shaderDependenciesJSON = json::parse(stream->getAsString().c_str());
			stream->close();
		}

		// Import cursors
		{
			BuiltinResourcesHelper::updateManifest(
				mEngineCursorFolder,
				cursorsJSON,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Normal);

			Vector<bool> importFlags = BuiltinResourcesHelper::generateImportFlags(
				cursorsJSON,
				rawCursorFolder,
				lastUpdateTime,
				forceImport);

			BuiltinResourcesHelper::importAssets(
				cursorsJSON,
				importFlags,
				rawCursorFolder,
				mEngineCursorFolder,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Normal);
		}

		// Import icons
		{
			BuiltinResourcesHelper::updateManifest(
				iconFolder,
				iconsJSON,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Normal);

			Vector<bool> importFlags = BuiltinResourcesHelper::generateImportFlags(
				iconsJSON,
				rawIconFolder,
				lastUpdateTime,
				forceImport);

			BuiltinResourcesHelper::importAssets(
				iconsJSON,
				importFlags,
				rawIconFolder,
				iconFolder,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Normal);
		}

		// Import shaders
		{
			BuiltinResourcesHelper::updateManifest(
				shaderIncludeFolder,
				includesJSON,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Normal);

			BuiltinResourcesHelper::updateManifest(
				mEngineShaderFolder,
				shadersJSON,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Normal);

			Vector<bool> includeImportFlags = BuiltinResourcesHelper::generateImportFlags(
				includesJSON,
				rawShaderIncludeFolder,
				lastUpdateTime,
				forceImport);

			Vector<bool> shaderImportFlags = BuiltinResourcesHelper::generateImportFlags(
				shadersJSON,
				rawShaderFolder,
				lastUpdateTime,
				forceImport,
				&shaderDependenciesJSON,
				rawShaderIncludeFolder);

			// Hidden dependency: Includes must be imported before shaders, but import flags for shaders must be generated
			// before includes are imported, since the process checks if imports changed
			BuiltinResourcesHelper::importAssets(
				includesJSON,
				includeImportFlags,
				rawShaderIncludeFolder,
				shaderIncludeFolder,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Normal,
				nullptr,
				true);


			BuiltinResourcesHelper::importAssets(
				shadersJSON,
				shaderImportFlags,
				rawShaderFolder,
				mEngineShaderFolder,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Normal,
				&shaderDependenciesJSON,
				true);
		}

		// Import GUI sprites
		{
			BuiltinResourcesHelper::updateManifest(
				skinFolder,
				skinJSON,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Sprite);

			Vector<bool> skinImportFlags = BuiltinResourcesHelper::generateImportFlags(
				skinJSON,
				rawSkinFolder,
				lastUpdateTime,
				forceImport);

			BuiltinResourcesHelper::importAssets(
				skinJSON,
				skinImportFlags,
				rawSkinFolder,
				skinFolder,
				mResourceManifest,
				BuiltinResourcesHelper::AssetType::Sprite);
		}

		// Update shader dependencies JSON
		{
			String jsonString = shaderDependenciesJSON.dump(4).c_str();

			dataListStream = FileSystem::createAndOpenFile(shaderDependenciesFile);
			dataListStream->writeString(jsonString);
			dataListStream->close();
		}

		// Import font
		BuiltinResourcesHelper::importFont(
			mBuiltinRawDataFolder + DefaultFontFilename,
			DefaultFontFilename,
			mBuiltinDataFolder,
			{ DefaultFontSize },
			false,
			UUID("c9f08cab-f9c9-47c4-96e0-1066a8d4455b"),
			mResourceManifest);

		// Generate & save GUI skin
		{
			SPtr<GUISkin> skin = generateGUISkin();
			Path outputPath = mBuiltinDataFolder + (GUISkinFile + u8".asset");

			HResource skinResource = gResources()._createResourceHandle(skin, UUID("c1bf9a9d-4355-4841-a538-25e67730ec4b"));

			gResources().save(skinResource, outputPath, true);
			mResourceManifest->registerResource(skinResource.getUUID(), outputPath);
		}

		// Generate & save meshes
		generateMeshes();
		
		Resources::instance().unloadAllUnused();
	}

	SPtr<GUISkin> BuiltinResources::generateGUISkin()
	{
		BuiltinResourceGUIElementStyleLoader loader(mBuiltinDataFolder, mEngineSkinSpritesFolder);

		Path guiSkinPath = mBuiltinRawDataFolder + GUI_SKIN_JSON;
		SPtr<DataStream> guiSkinStream = FileSystem::openFile(guiSkinPath);
		json guiSkinJSON = json::parse(guiSkinStream->getAsString().c_str());

		SPtr<GUISkin> skin = GUISkin::_createPtr();

		for(auto& entry : guiSkinJSON)
		{
			std::string name = entry["name"];

			GUIElementStyle style = BuiltinResourcesHelper::loadGUIStyleFromJSON(entry, loader);
			skin->setStyle(name.c_str(), style);
		}

		return skin;
	}

	void BuiltinResources::generateTextures()
	{
		StringStream ss;

		SPtr<PixelData> blackPixelData = PixelData::create(2, 2, 1, PF_RGBA8);
		blackPixelData->setColorAt(Color::Black, 0, 0);
		blackPixelData->setColorAt(Color::Black, 0, 1);
		blackPixelData->setColorAt(Color::Black, 1, 0);
		blackPixelData->setColorAt(Color::Black, 1, 1);

		SPtr<Texture> blackTexture = Texture::_createPtr(blackPixelData);

		SPtr<PixelData> whitePixelData = PixelData::create(2, 2, 1, PF_RGBA8);
		whitePixelData->setColorAt(Color::White, 0, 0);
		whitePixelData->setColorAt(Color::White, 0, 1);
		whitePixelData->setColorAt(Color::White, 1, 0);
		whitePixelData->setColorAt(Color::White, 1, 1);

		SPtr<Texture> whiteTexture = Texture::_createPtr(whitePixelData);

		SPtr<PixelData> normalPixelData = PixelData::create(2, 2, 1, PF_RGBA8);

		Color encodedNormal(0.5f, 0.5f, 1.0f);
		normalPixelData->setColorAt(encodedNormal, 0, 0);
		normalPixelData->setColorAt(encodedNormal, 0, 1);
		normalPixelData->setColorAt(encodedNormal, 1, 0);
		normalPixelData->setColorAt(encodedNormal, 1, 1);

		SPtr<Texture> normalTexture = Texture::_createPtr(normalPixelData);

		// Save all textures
		Path outputDir = mBuiltinDataFolder + TEXTURE_FOLDER;

		auto saveTexture = [&](const Path& path, const SPtr<Texture>& texture, const String& uuid)
		{
			HResource textureResource = gResources()._createResourceHandle(texture, UUID(uuid));

			gResources().save(textureResource, path, true);
			mResourceManifest->registerResource(textureResource.getUUID(), path);
		};

		Path whitePath = outputDir + TextureWhiteFile;
		saveTexture(whitePath, whiteTexture, "1f7d0e3f-d81b-42ee-9d31-cb6c6fc55824");

		Path blackPath = outputDir + TextureBlackFile;
		saveTexture(blackPath, blackTexture, "149a5c05-9570-4915-9dbd-69acf88b865b");

		Path normalPath = outputDir + TextureNormalFile;
		saveTexture(normalPath, normalTexture, "afb29163-1ef0-4440-9cfb-c1ebb3b3d452");
	}

	void BuiltinResources::generateMeshes()
	{
		SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);
		vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
		vertexDesc->addVertElem(VET_FLOAT4, VES_TANGENT);
		vertexDesc->addVertElem(VET_COLOR, VES_COLOR);

		UINT32 boxNumVertices = 0;
		UINT32 boxNumIndices = 0;
		ShapeMeshes3D::getNumElementsAABox(boxNumVertices, boxNumIndices);
		SPtr<MeshData> boxMeshData = MeshData::create(boxNumVertices, boxNumIndices, vertexDesc);
		AABox box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		ShapeMeshes3D::solidAABox(box, boxMeshData, 0, 0);
		SPtr<Mesh> boxMesh = Mesh::_createPtr(RendererMeshData::convert(boxMeshData));

		UINT32 sphereNumVertices = 0;
		UINT32 sphereNumIndices = 0;
		ShapeMeshes3D::getNumElementsSphere(3, sphereNumVertices, sphereNumIndices);
		SPtr<MeshData> sphereMeshData = bs_shared_ptr_new<MeshData>(sphereNumVertices, sphereNumIndices, vertexDesc);

		ShapeMeshes3D::solidSphere(Sphere(Vector3::ZERO, 1.0f), sphereMeshData, 0, 0, 3);
		SPtr<Mesh> sphereMesh = Mesh::_createPtr(RendererMeshData::convert(sphereMeshData));

		UINT32 coneNumVertices = 0;
		UINT32 coneNumIndices = 0;
		ShapeMeshes3D::getNumElementsCone(10, coneNumVertices, coneNumIndices);
		SPtr<MeshData> coneMeshData = bs_shared_ptr_new<MeshData>(coneNumVertices, coneNumIndices, vertexDesc);

		ShapeMeshes3D::solidCone(Vector3::ZERO, Vector3::UNIT_Y, 1.0f, 1.0f, Vector2::ONE, coneMeshData, 0, 0);
		SPtr<Mesh> coneMesh = Mesh::_createPtr(RendererMeshData::convert(coneMeshData));

		UINT32 cylinderNumVertices = 0;
		UINT32 cylinderNumIndices = 0;
		ShapeMeshes3D::getNumElementsCylinder(10, cylinderNumVertices, cylinderNumIndices);
		SPtr<MeshData> cylinderMeshData = bs_shared_ptr_new<MeshData>(cylinderNumVertices, cylinderNumIndices, vertexDesc);

		ShapeMeshes3D::solidCylinder(Vector3::ZERO, Vector3::UNIT_Y, 1.0f, 1.0f, Vector2::ONE, cylinderMeshData, 0, 0);
		SPtr<Mesh> cylinderMesh = Mesh::_createPtr(RendererMeshData::convert(cylinderMeshData));

		UINT32 quadNumVertices = 8;
		UINT32 quadNumIndices = 12;
		ShapeMeshes3D::getNumElementsQuad(quadNumVertices, quadNumIndices);
		SPtr<MeshData> quadMeshData = bs_shared_ptr_new<MeshData>(quadNumVertices, quadNumIndices, vertexDesc);

		std::array<Vector3, 2> axes = {{ Vector3::UNIT_X, Vector3::UNIT_Z }};
		std::array<float, 2> sizes = {{ 1.0f, 1.0f }};
		Rect3 rect(Vector3::ZERO, axes, sizes);
		ShapeMeshes3D::solidQuad(rect, quadMeshData, 0, 0);
		SPtr<Mesh> quadMesh = Mesh::_createPtr(RendererMeshData::convert(quadMeshData));

		UINT32 discNumVertices = 0;
		UINT32 discNumIndices = 0;
		ShapeMeshes3D::getNumElementsDisc(10, discNumVertices, discNumIndices);
		SPtr<MeshData> discMeshData = bs_shared_ptr_new<MeshData>(discNumVertices, discNumIndices, vertexDesc);

		ShapeMeshes3D::solidDisc(Vector3::ZERO, 1.0f, Vector3::UNIT_Y, discMeshData, 0, 0);
		SPtr<Mesh> discMesh = Mesh::_createPtr(RendererMeshData::convert(discMeshData));

		// Save all meshes
		Path outputDir = mEngineMeshFolder;

		auto saveMesh = [&](const Path& path, const SPtr<Mesh>& mesh, const String& uuid)
		{
			HResource meshResource = gResources()._createResourceHandle(mesh, UUID(uuid));

			gResources().save(meshResource, path, true);
			mResourceManifest->registerResource(meshResource.getUUID(), path);
		};

		Path boxPath = outputDir + MeshBoxFile;
		saveMesh(boxPath, boxMesh, "bc1d20ca-7fe6-489b-8b5c-dbf798badc95");

		Path spherePath = outputDir + MeshSphereFile;
		saveMesh(spherePath, sphereMesh, "040642f3-04d6-419e-9dba-f7824161c205");

		Path conePath = outputDir + MeshConeFile;
		saveMesh(conePath, coneMesh, "b8cf6db5-1736-47ac-852f-82ecd88b4d46");

		Path cylinderPath = outputDir + MeshCylinderFile;
		saveMesh(cylinderPath, cylinderMesh, "e6b2b797-4e72-7e49-61ba-4e7275bd561d");

		Path quadPath = outputDir + MeshQuadFile;
		saveMesh(quadPath, quadMesh, "06592bf3-f82a-472e-a034-26a98225fbe1");

		Path discPath = outputDir + MeshDiscFile;
		saveMesh(discPath, discMesh, "6f496313-344a-495c-83e8-152e3053c52d");
	}

	HSpriteTexture BuiltinResources::getSkinTexture(const String& name) const
	{
		Path texturePath = mEngineSkinSpritesFolder;
		texturePath.append(u8"sprite_" + name + u8".asset");

		return gResources().load<SpriteTexture>(texturePath);
	}

	HShader BuiltinResources::getShader(const Path& path) const
	{
		Path programPath = mEngineShaderFolder;
		programPath.append(path);
		programPath.setExtension(programPath.getExtension() + ".asset");

		return gResources().load<Shader>(programPath);
	}

	HTexture BuiltinResources::getCursorTexture(const String& name) const
	{
		Path cursorPath = mEngineCursorFolder;
		cursorPath.append(name + u8".asset");

		return gResources().load<Texture>(cursorPath);
	}

	const PixelData& BuiltinResources::getCursorArrow(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowHotspot;
		return *mCursorArrow.get();
	}

	const PixelData& BuiltinResources::getCursorArrowDrag(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowDragHotspot;
		return *mCursorArrowDrag.get();
	}

	const PixelData& BuiltinResources::getCursorWait(Vector2I& hotSpot)
	{
		hotSpot = CursorWaitHotspot;
		return *mCursorWait.get();
	}

	const PixelData& BuiltinResources::getCursorIBeam(Vector2I& hotSpot)
	{
		hotSpot = CursorIBeamHotspot;
		return *mCursorIBeam.get();
	}

	const PixelData& BuiltinResources::getCursorSizeNESW(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNESWHotspot;
		return *mCursorSizeNESW.get();
	}

	const PixelData& BuiltinResources::getCursorSizeNS(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNSHotspot;
		return *mCursorSizeNS.get();
	}

	const PixelData& BuiltinResources::getCursorSizeNWSE(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNWSEHotspot;
		return *mCursorSizeNWSE.get();
	}

	const PixelData& BuiltinResources::getCursorSizeWE(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeWEHotspot;
		return *mCursorSizeWE.get();
	}

	const PixelData& BuiltinResources::getCursorDeny(Vector2I& hotSpot)
	{
		hotSpot = CursorDenyHotspot;
		return *mCursorDeny.get();
	}

	const PixelData& BuiltinResources::getCursorMoveLeftRight(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowLeftRightHotspot;
		return *mCursorArrowLeftRight.get();
	}

	const PixelData& BuiltinResources::getFrameworkIcon()
	{
		return *mFrameworkIcon.get();
	}

	Path BuiltinResources::getRawShaderFolder()
	{
		return Paths::getDataPath() + "Raw/" + SHADER_FOLDER;
	}

	Path BuiltinResources::getShaderIncludeFolder()
	{
		return Paths::getDataPath() + SHADER_INCLUDE_FOLDER;
	}

	Path BuiltinResources::getIconFolder()
	{
		return Paths::getDataPath() + ICON_FOLDER;
	}

	HMesh BuiltinResources::getMesh(BuiltinMesh mesh) const
	{
		Path meshPath = mEngineMeshFolder;

		switch (mesh)
		{
		case BuiltinMesh::Box:
			meshPath.append(MeshBoxFile);
			break;
		case BuiltinMesh::Sphere:
			meshPath.append(MeshSphereFile);
			break;
		case BuiltinMesh::Cone:
			meshPath.append(MeshConeFile);
			break;
		case BuiltinMesh::Cylinder:
			meshPath.append(MeshCylinderFile);
			break;
		case BuiltinMesh::Quad:
			meshPath.append(MeshQuadFile);
			break;
		case BuiltinMesh::Disc:
			meshPath.append(MeshDiscFile);
			break;
		}

		return gResources().load<Mesh>(meshPath);
	}

	HShader BuiltinResources::getBuiltinShader(BuiltinShader type) const
	{
		switch(type)
		{
		case BuiltinShader::Standard:
			return mShaderDiffuse;
		case BuiltinShader::Transparent:
			return mShaderTransparent;
		case BuiltinShader::ParticlesUnlit:
			return mShaderParticlesUnlit;
		default:
			break;
		}

		return HShader();
	}

	HTexture BuiltinResources::getTexture(BuiltinTexture type)
	{
		Path texturePath = Paths::getDataPath();
		texturePath.append(TEXTURE_FOLDER);

		switch (type)
		{
		case BuiltinTexture::Black:
			texturePath.append(TextureBlackFile);
			break;
		case BuiltinTexture::White:
			texturePath.append(TextureWhiteFile);
			break;
		case BuiltinTexture::Normal:
			texturePath.append(TextureNormalFile);
			break;
		}

		return gResources().load<Texture>(texturePath);
	}

	HMaterial BuiltinResources::createSpriteTextMaterial() const
	{
		return Material::create(mShaderSpriteText);
	}

	HMaterial BuiltinResources::createSpriteImageMaterial() const
	{
		return Material::create(mShaderSpriteImage);
	}

	HMaterial BuiltinResources::createSpriteNonAlphaImageMaterial() const
	{
		return Material::create(mShaderSpriteNonAlphaImage);
	}

	HMaterial BuiltinResources::createSpriteLineMaterial() const
	{
		return Material::create(mShaderSpriteLine);
	}

	BuiltinResources& gBuiltinResources()
	{
		return BuiltinResources::instance();
	}
}