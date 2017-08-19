//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsBuiltinResources.h"
#include "BsGUIElementStyle.h"
#include "BsGUILabel.h"
#include "BsGUIButton.h"
#include "BsGUIInputBox.h"
#include "BsGUIToggle.h"
#include "BsGUIDropDownContent.h"
#include "BsTextSprite.h"
#include "BsSpriteTexture.h"
#include "BsFont.h"
#include "BsTexture.h"
#include "BsImporter.h"
#include "BsResources.h"
#include "BsShader.h"
#include "BsMaterial.h"
#include "BsRTTIType.h"
#include "BsFileSystem.h"
#include "BsCoreThread.h"
#include "BsDataStream.h"
#include "BsResourceManifest.h"
#include "BsVertexDataDesc.h"
#include "BsShapeMeshes3D.h"
#include "BsMesh.h"
#include "BsGUITooltip.h"
#include "BsFileSerializer.h"
#include "BsTextureImportOptions.h"
#include "BsBuiltinResourcesHelper.h"
#include "BsRendererMeshData.h"
#include "BsGUISlider.h"
#include "BsGUIScrollBar.h"

using json = nlohmann::json;

namespace bs
{
	const WString BuiltinResources::DefaultFontFilename = L"arial.ttf";
	const UINT32 BuiltinResources::DefaultFontSize = 8;

	const Color BuiltinResources::TextNormalColor = Color(0.7f, 0.7f, 0.7f);
	const Color BuiltinResources::TextActiveColor = Color(0.0f, 0.0f, 0.0f);

	const String BuiltinResources::MultiLineLabelStyle = "MultiLineLabel";

	const WString BuiltinResources::GUISkinFile = L"GUISkin";

	const char* BuiltinResources::DataListFile = "DataList.json";
	const char* BuiltinResources::CursorFolder = "Cursors\\";
	const char* BuiltinResources::IconFolder = "Icons\\";
	const char* BuiltinResources::ShaderFolder = "Shaders\\";
	const char* BuiltinResources::SkinFolder = "Skin\\";
	const char* BuiltinResources::ShaderIncludeFolder = "Includes\\";
	const char* BuiltinResources::MeshFolder = "Meshes\\";
	const char* BuiltinResources::TextureFolder = "Textures\\";
	const char* BuiltinResources::SpriteSubFolder = "Sprites\\";

	/************************************************************************/
	/* 								GUI TEXTURES                      		*/
	/************************************************************************/

	const WString BuiltinResources::WhiteTex = L"White.psd";
	const wchar_t* BuiltinResources::SplashScreenName = L"SplashScreen.png";

	const WString BuiltinResources::ButtonNormalTex = L"ButtonNormal.png";
	const WString BuiltinResources::ButtonHoverTex = L"ButtonHover.png";
	const WString BuiltinResources::ButtonActiveTex = L"ButtonActive.png";

	const WString BuiltinResources::ToggleNormalTex = L"ToggleNormal.png";
	const WString BuiltinResources::ToggleHoverTex = L"ToggleHover.png";
	const WString BuiltinResources::ToggleNormalOnTex = L"ToggleNormalOn.png";
	const WString BuiltinResources::ToggleHoverOnTex = L"ToggleHoverOn.png";

	const WString BuiltinResources::InputBoxNormalTex = L"InputBoxNormal.png";
	const WString BuiltinResources::InputBoxHoverTex = L"InputBoxHover.png";
	const WString BuiltinResources::InputBoxFocusedTex = L"InputBoxActive.png";

	const WString BuiltinResources::ScrollBarUpNormalTex = L"ScrollArrowUpNormal.png";
	const WString BuiltinResources::ScrollBarUpHoverTex = L"ScrollArrowUpHover.png";
	const WString BuiltinResources::ScrollBarUpActiveTex = L"ScrollArrowUpActive.png";

	const WString BuiltinResources::ScrollBarDownNormalTex = L"ScrollArrowDownNormal.png";
	const WString BuiltinResources::ScrollBarDownHoverTex = L"ScrollArrowDownHover.png";
	const WString BuiltinResources::ScrollBarDownActiveTex = L"ScrollArrowDownActive.png";

	const WString BuiltinResources::ScrollBarLeftNormalTex = L"ScrollArrowLeftNormal.png";
	const WString BuiltinResources::ScrollBarLeftHoverTex = L"ScrollArrowLeftHover.png";
	const WString BuiltinResources::ScrollBarLeftActiveTex = L"ScrollArrowLeftActive.png";

	const WString BuiltinResources::ScrollBarRightNormalTex = L"ScrollArrowRightNormal.png";
	const WString BuiltinResources::ScrollBarRightHoverTex = L"ScrollArrowRightHover.png";
	const WString BuiltinResources::ScrollBarRightActiveTex = L"ScrollArrowRightActive.png";

	const WString BuiltinResources::ScrollBarHandleHorzNormalTex = L"ScrollBarHHandleNormal.png";
	const WString BuiltinResources::ScrollBarHandleHorzHoverTex = L"ScrollBarHHandleHover.png";
	const WString BuiltinResources::ScrollBarHandleHorzActiveTex = L"ScrollBarHHandleActive.png";

	const WString BuiltinResources::ScrollBarHandleVertNormalTex = L"ScrollBarVHandleNormal.png";
	const WString BuiltinResources::ScrollBarHandleVertHoverTex = L"ScrollBarVHandleHover.png";
	const WString BuiltinResources::ScrollBarHandleVertActiveTex = L"ScrollBarVHandleActive.png";

	const WString BuiltinResources::ScrollBarResizeableHandleHorzNormalTex = L"ScrollBarHHandleResizeableNormal.png";
	const WString BuiltinResources::ScrollBarResizeableHandleHorzHoverTex = L"ScrollBarHHandleResizeableHover.png";
	const WString BuiltinResources::ScrollBarResizeableHandleHorzActiveTex = L"ScrollBarHHandleResizeableActive.png";

	const WString BuiltinResources::ScrollBarResizeableHandleVertNormalTex = L"ScrollBarVHandleResizeableNormal.png";
	const WString BuiltinResources::ScrollBarResizeableHandleVertHoverTex = L"ScrollBarVHandleResizeableHover.png";
	const WString BuiltinResources::ScrollBarResizeableHandleVertActiveTex = L"ScrollBarVHandleResizeableActive.png";

	const WString BuiltinResources::ScrollBarHBgTex = L"ScrollBarHBackground.png";
	const WString BuiltinResources::ScrollBarVBgTex = L"ScrollBarVBackground.png";

	const WString BuiltinResources::SliderHBackgroundTex = L"SliderHBackground.png";
	const WString BuiltinResources::SliderHFillTex = L"SliderHFill.png";
	const WString BuiltinResources::SliderVBackgroundTex = L"SliderVBackground.png";
	const WString BuiltinResources::SliderVFillTex = L"SliderVFill.png";
	const WString BuiltinResources::SliderHandleNormalTex = L"SliderHandleNormal.png";
	const WString BuiltinResources::SliderHandleHoverTex = L"SliderHandleHover.png";
	const WString BuiltinResources::SliderHandleActiveTex = L"SliderHandleActive.png";

	const WString BuiltinResources::DropDownBtnNormalTex = L"DropDownButtonNormal.png";
	const WString BuiltinResources::DropDownBtnHoverTex = L"DropDownButtonHover.png";
	const WString BuiltinResources::DropDownBtnActiveTex = L"DropDownButtonActive.png";

	const WString BuiltinResources::DropDownBoxBgTex = L"DropDownBoxBg.png";
	const WString BuiltinResources::DropDownBoxSideBgTex = L"DropDownBoxSideBg.png";
	const WString BuiltinResources::DropDownBoxHandleTex = L"DropDownBoxScrollHandle.png";

	const WString BuiltinResources::DropDownBoxEntryNormalTex = L"DropDownBoxEntryNormal.png";
	const WString BuiltinResources::DropDownBoxEntryHoverTex = L"DropDownBoxEntryHover.png";

	const WString BuiltinResources::DropDownBoxBtnUpNormalTex = L"DropDownBoxArrowUpNormal.png";
	const WString BuiltinResources::DropDownBoxBtnUpHoverTex = L"DropDownBoxArrowUpHover.png";

	const WString BuiltinResources::DropDownBoxBtnDownNormalTex = L"DropDownBoxArrowDownNormal.png";
	const WString BuiltinResources::DropDownBoxBtnDownHoverTex = L"DropDownBoxArrowDownHover.png";

	const WString BuiltinResources::DropDownBoxEntryExpNormalTex = L"DropDownBoxExpandBtnNormal.png";
	const WString BuiltinResources::DropDownBoxEntryExpHoverTex = L"DropDownBoxExpandBtnHover.png";

	const WString BuiltinResources::DropDownBoxEntryToggleNormalTex = L"DropDownBoxEntryToggleNormal.png";
	const WString BuiltinResources::DropDownBoxEntryToggleHoverTex = L"DropDownBoxEntryToggleHover.png";
	const WString BuiltinResources::DropDownBoxEntryToggleNormalOnTex = L"DropDownBoxEntryToggleNormalOn.png";
	const WString BuiltinResources::DropDownBoxEntryToggleHoverOnTex = L"DropDownBoxEntryToggleHoverOn.png";

	const WString BuiltinResources::DropDownSeparatorTex = L"DropDownBoxSeparator.png";

	/************************************************************************/
	/* 							CURSOR TEXTURES                      		*/
	/************************************************************************/

	const WString BuiltinResources::CursorArrowTex = L"Arrow.psd";
	const WString BuiltinResources::CursorArrowDragTex = L"ArrowDrag.psd";
	const WString BuiltinResources::CursorArrowLeftRightTex = L"ArrowLeftRight.psd";
	const WString BuiltinResources::CursorIBeamTex = L"IBeam.psd";
	const WString BuiltinResources::CursorDenyTex = L"Deny.psd";
	const WString BuiltinResources::CursorWaitTex = L"Wait.psd";
	const WString BuiltinResources::CursorSizeNESWTex = L"SizeNESW.psd";
	const WString BuiltinResources::CursorSizeNSTex = L"SizeNS.psd";
	const WString BuiltinResources::CursorSizeNWSETex = L"SizeNWSE.psd";
	const WString BuiltinResources::CursorSizeWETex = L"SizeWE.psd";

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
	/* 							ICON TEXTURES					    		*/
	/************************************************************************/

	const WString BuiltinResources::IconTextureName = L"BansheeIcon.png";

	/************************************************************************/
	/* 									SHADERS                      		*/
	/************************************************************************/

	const WString BuiltinResources::ShaderSpriteTextFile = L"SpriteText.bsl";
	const WString BuiltinResources::ShaderSpriteImageAlphaFile = L"SpriteImageAlpha.bsl";
	const WString BuiltinResources::ShaderSpriteImageNoAlphaFile = L"SpriteImageNoAlpha.bsl";
	const WString BuiltinResources::ShaderSpriteLineFile = L"SpriteLine.bsl";
	const WString BuiltinResources::ShaderDiffuseFile = L"Diffuse.bsl";
	const WString BuiltinResources::ShaderTransparentFile = L"Transparent.bsl";

	/************************************************************************/
	/* 								MESHES							  		*/
	/************************************************************************/

	const WString BuiltinResources::MeshSphereFile = L"Sphere.asset";
	const WString BuiltinResources::MeshBoxFile = L"Box.asset";
	const WString BuiltinResources::MeshConeFile = L"Cone.asset";
	const WString BuiltinResources::MeshQuadFile = L"Quad.asset";
	const WString BuiltinResources::MeshDiscFile = L"Disc.asset";

	/************************************************************************/
	/* 								TEXTURES							  	*/
	/************************************************************************/

	const WString BuiltinResources::TextureWhiteFile = L"White.asset";
	const WString BuiltinResources::TextureBlackFile = L"Black.asset";
	const WString BuiltinResources::TextureNormalFile = L"Normal.asset";

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
		mBansheeIcon = nullptr;
	}

	BuiltinResources::BuiltinResources()
	{
		// Set up paths
		mBuiltinRawDataFolder = Paths::getRuntimeDataPath() + L"Raw\\Engine\\";

		mBuiltinDataFolder = Paths::getEngineDataPath();
		mEngineSkinSpritesFolder = mBuiltinDataFolder + SkinFolder + SpriteSubFolder;
		mEngineShaderFolder = mBuiltinDataFolder + ShaderFolder;
		mEngineMeshFolder = mBuiltinDataFolder + MeshFolder;
		mEngineCursorFolder = mBuiltinDataFolder + CursorFolder;

		ResourceManifestPath = mBuiltinDataFolder + "ResourceManifest.asset";

		// Update from raw assets if needed
#if BS_DEBUG_MODE
		if (FileSystem::exists(mBuiltinRawDataFolder))
		{
			time_t lastUpdateTime;
			UINT32 modifications = BuiltinResourcesHelper::checkForModifications(mBuiltinRawDataFolder, 
				mBuiltinDataFolder + L"Timestamp.asset", lastUpdateTime);

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
				BuiltinResourcesHelper::writeTimestamp(mBuiltinDataFolder + L"Timestamp.asset");

				ResourceManifest::save(mResourceManifest, ResourceManifestPath, mBuiltinDataFolder);
			}
		}
#endif

		// Load manifest
		if (mResourceManifest == nullptr)
		{
			if (FileSystem::exists(ResourceManifestPath))
				mResourceManifest = ResourceManifest::load(ResourceManifestPath, mBuiltinDataFolder);

			if (mResourceManifest == nullptr)
				mResourceManifest = ResourceManifest::create("BuiltinResources");

			gResources().registerResourceManifest(mResourceManifest);
		}
		
		// Load basic resources
		mShaderSpriteText = getShader(ShaderSpriteTextFile);
		mShaderSpriteImage = getShader(ShaderSpriteImageAlphaFile);
		mShaderSpriteNonAlphaImage = getShader(ShaderSpriteImageNoAlphaFile);
		mShaderSpriteLine = getShader(ShaderSpriteLineFile);
		mShaderDiffuse = getShader(ShaderDiffuseFile);
		mShaderTransparent = getShader(ShaderTransparentFile);

		SPtr<PixelData> dummyPixelData = PixelData::create(2, 2, 1, PF_RGBA8);

		dummyPixelData->setColorAt(Color::Red, 0, 0);
		dummyPixelData->setColorAt(Color::Red, 0, 1);
		dummyPixelData->setColorAt(Color::Red, 1, 0);
		dummyPixelData->setColorAt(Color::Red, 1, 1);

		mDummyTexture = Texture::create(dummyPixelData);

		mWhiteSpriteTexture = getSkinTexture(WhiteTex);
		mDummySpriteTexture = SpriteTexture::create(mDummyTexture);

		mFont = gResources().load<Font>(mBuiltinDataFolder + (DefaultFontFilename + L".asset"));
		mSkin = gResources().load<GUISkin>(mBuiltinDataFolder + (GUISkinFile + L".asset"));
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

		Path iconPath = mBuiltinDataFolder + IconFolder;
		iconPath.append(IconTextureName + L".asset");

		HTexture iconTex = gResources().load<Texture>(iconPath);

		mBansheeIcon = iconTex->getProperties().allocBuffer(0, 0);
		iconTex->readData(mBansheeIcon);

		gCoreThread().submit(true);
	}

	void BuiltinResources::preprocess(bool forceImport, time_t lastUpdateTime)
	{
		// Hidden dependency: Textures need to be generated before shaders as they may use the default textures
		generateTextures();

		Path dataListsFilePath = mBuiltinRawDataFolder + DataListFile;
		SPtr<DataStream> dataListStream = FileSystem::openFile(dataListsFilePath);
		json dataListJSON = json::parse(dataListStream->getAsString().c_str());

		json skinJSON = dataListJSON["Skin"];
		json cursorsJSON = dataListJSON["Cursors"];
		json iconsJSON = dataListJSON["Icons"];
		json includesJSON = dataListJSON["Includes"];
		json shadersJSON = dataListJSON["Shaders"];

		Path rawSkinFolder = mBuiltinRawDataFolder + SkinFolder;
		Path rawCursorFolder = mBuiltinRawDataFolder + CursorFolder;
		Path rawIconFolder = mBuiltinRawDataFolder + IconFolder;
		Path rawShaderFolder = mBuiltinRawDataFolder + ShaderFolder;
		Path rawShaderIncludeFolder = mBuiltinRawDataFolder + ShaderIncludeFolder;

		// Update DataList.json if needed
		bool updatedDataLists = false;
		updatedDataLists |= BuiltinResourcesHelper::updateJSON(rawCursorFolder, BuiltinResourcesHelper::AssetType::Normal, cursorsJSON);
		updatedDataLists |= BuiltinResourcesHelper::updateJSON(rawIconFolder, BuiltinResourcesHelper::AssetType::Normal, iconsJSON);
		updatedDataLists |= BuiltinResourcesHelper::updateJSON(rawShaderIncludeFolder, BuiltinResourcesHelper::AssetType::Normal, includesJSON);
		updatedDataLists |= BuiltinResourcesHelper::updateJSON(rawShaderFolder, BuiltinResourcesHelper::AssetType::Normal, shadersJSON);
		updatedDataLists |= BuiltinResourcesHelper::updateJSON(rawSkinFolder, BuiltinResourcesHelper::AssetType::Sprite, skinJSON);

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

		Path skinFolder = mBuiltinDataFolder + SkinFolder;
		Path iconFolder = mBuiltinDataFolder + IconFolder;
		Path shaderIncludeFolder = mBuiltinDataFolder + ShaderIncludeFolder;
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

		{
			Vector<bool> importFlags = BuiltinResourcesHelper::generateImportFlags(cursorsJSON, rawCursorFolder,
				lastUpdateTime, forceImport);

			BuiltinResourcesHelper::importAssets(cursorsJSON, importFlags, rawCursorFolder, mEngineCursorFolder, 
				mResourceManifest, BuiltinResourcesHelper::AssetType::Normal);
		}

		{
			Vector<bool> importFlags = BuiltinResourcesHelper::generateImportFlags(iconsJSON, rawIconFolder,
				lastUpdateTime, forceImport);

			BuiltinResourcesHelper::importAssets(iconsJSON, importFlags, rawIconFolder, iconFolder, mResourceManifest, 
				BuiltinResourcesHelper::AssetType::Normal);
		}

		{
			Vector<bool> includeImportFlags = BuiltinResourcesHelper::generateImportFlags(includesJSON, 
				rawShaderIncludeFolder, lastUpdateTime, forceImport);

			Vector<bool> shaderImportFlags = BuiltinResourcesHelper::generateImportFlags(shadersJSON, rawShaderFolder,
				lastUpdateTime, forceImport, &shaderDependenciesJSON, rawShaderIncludeFolder);

			// Hidden dependency: Includes must be imported before shaders, but import flags for shaders must be generated
			// before includes are imported, since the process checks if imports changed
			BuiltinResourcesHelper::importAssets(includesJSON, includeImportFlags, rawShaderIncludeFolder, 
				shaderIncludeFolder, mResourceManifest, BuiltinResourcesHelper::AssetType::Normal);
		
			BuiltinResourcesHelper::importAssets(shadersJSON, shaderImportFlags, rawShaderFolder, mEngineShaderFolder, 
				mResourceManifest, BuiltinResourcesHelper::AssetType::Normal, &shaderDependenciesJSON);
		}

		{
			Vector<bool> includeImportFlags = BuiltinResourcesHelper::generateImportFlags(skinJSON, 
				rawSkinFolder, lastUpdateTime, forceImport);

			BuiltinResourcesHelper::importAssets(skinJSON, includeImportFlags, rawSkinFolder, skinFolder, mResourceManifest, 
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
		BuiltinResourcesHelper::importFont(mBuiltinRawDataFolder + DefaultFontFilename, DefaultFontFilename, 
			mBuiltinDataFolder, { DefaultFontSize }, false, "c9f08cab-f9c9-47c4-96e0-1066a8d4455b", mResourceManifest);

		// Import splash screen
		{
			Path inputPath = mBuiltinRawDataFolder + WString(SplashScreenName);
			Path outputPath = mBuiltinDataFolder + (WString(SplashScreenName) + L".asset");

			auto textureIO = gImporter().createImportOptions<TextureImportOptions>(inputPath);
			textureIO->setCPUCached(true);
			textureIO->setGenerateMipmaps(false);
			HTexture splashTexture = gImporter().import<Texture>(inputPath, textureIO);

			SPtr<PixelData> splashPixelData = splashTexture->getProperties().allocBuffer(0, 0);
			splashTexture->readCachedData(*splashPixelData);

			FileEncoder fe(outputPath);
			fe.encode(splashPixelData.get());
		}

		// Generate & save GUI skin
		{
			SPtr<GUISkin> skin = generateGUISkin();
			Path outputPath = mBuiltinDataFolder + (GUISkinFile + L".asset");

			HResource skinResource = gResources()._createResourceHandle(skin, "c1bf9a9d-4355-4841-a538-25e67730ec4b");

			gResources().save(skinResource, outputPath, true);
			mResourceManifest->registerResource(skinResource.getUUID(), outputPath);
		}

		// Generate & save meshes
		generateMeshes();
		
		Resources::instance().unloadAllUnused();
	}

	SPtr<GUISkin> BuiltinResources::generateGUISkin()
	{
		Path fontPath = mBuiltinDataFolder;
		fontPath.append(DefaultFontFilename + L".asset");

		HFont font = gResources().load<Font>(fontPath);
		SPtr<GUISkin> skin = GUISkin::_createPtr();

		// Label
		GUIElementStyle labelStyle;
		labelStyle.font = font;
		labelStyle.fontSize = DefaultFontSize;
		labelStyle.normal.textColor = TextNormalColor;
		labelStyle.fixedWidth = false;
		labelStyle.fixedHeight = true;
		labelStyle.height = 14;
		labelStyle.minWidth = 10;

		skin->setStyle(GUILabel::getGUITypeName(), labelStyle);

		// Button
		GUIElementStyle buttonStyle;
		buttonStyle.normal.texture = getSkinTexture(ButtonNormalTex);
		buttonStyle.hover.texture = getSkinTexture(ButtonHoverTex);
		buttonStyle.active.texture = getSkinTexture(ButtonActiveTex);
		buttonStyle.normalOn.texture = getSkinTexture(ButtonActiveTex);
		buttonStyle.hoverOn.texture = getSkinTexture(ButtonActiveTex);
		buttonStyle.activeOn.texture = getSkinTexture(ButtonActiveTex);
		buttonStyle.normal.textColor = TextNormalColor;
		buttonStyle.hover.textColor = TextNormalColor;
		buttonStyle.active.textColor = TextActiveColor;
		buttonStyle.normalOn.textColor = TextActiveColor;
		buttonStyle.hoverOn.textColor = TextActiveColor;
		buttonStyle.activeOn.textColor = TextActiveColor;
		buttonStyle.border.left = 2;
		buttonStyle.border.right = 2;
		buttonStyle.border.top = 2;
		buttonStyle.border.bottom = 4;
		buttonStyle.contentOffset.top = 2;
		buttonStyle.contentOffset.left = 3;
		buttonStyle.contentOffset.right = 3;
		buttonStyle.margins.bottom = 2;
		buttonStyle.fixedHeight = true;
		buttonStyle.height = 21;
		buttonStyle.minWidth = 20;
		buttonStyle.font = font;
		buttonStyle.fontSize = DefaultFontSize;
		buttonStyle.textHorzAlign = THA_Center;
		buttonStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIButton::getGUITypeName(), buttonStyle);

		// Toggle
		GUIElementStyle toggleStyle;
		toggleStyle.normal.texture = getSkinTexture(ToggleNormalTex);
		toggleStyle.hover.texture = getSkinTexture(ToggleHoverTex);
		toggleStyle.active.texture = toggleStyle.hover.texture;
		toggleStyle.normalOn.texture = getSkinTexture(ToggleNormalOnTex);
		toggleStyle.hoverOn.texture = getSkinTexture(ToggleHoverOnTex);
		toggleStyle.activeOn.texture = toggleStyle.hoverOn.texture;
		toggleStyle.fixedHeight = true;
		toggleStyle.fixedWidth = true;
		toggleStyle.margins.bottom = 2;
		toggleStyle.height = 15;
		toggleStyle.width = 13;

		skin->setStyle(GUIToggle::getGUITypeName(), toggleStyle);

		// Input box
		GUIElementStyle inputBoxStyle;
		inputBoxStyle.normal.texture = getSkinTexture(InputBoxNormalTex);
		inputBoxStyle.hover.texture = getSkinTexture(InputBoxHoverTex);
		inputBoxStyle.focused.texture = getSkinTexture(InputBoxFocusedTex);
		inputBoxStyle.active.texture = inputBoxStyle.normal.texture;
		inputBoxStyle.normal.textColor = TextNormalColor;
		inputBoxStyle.hover.textColor = TextNormalColor;
		inputBoxStyle.focused.textColor = TextNormalColor;
		inputBoxStyle.active.textColor = TextNormalColor;
		inputBoxStyle.border.left = 1;
		inputBoxStyle.border.right = 3;
		inputBoxStyle.border.top = 4;
		inputBoxStyle.border.bottom = 1;
		inputBoxStyle.contentOffset.left = 4;
		inputBoxStyle.contentOffset.right = 4;
		inputBoxStyle.contentOffset.top = 4;
		inputBoxStyle.contentOffset.bottom = 4;
		inputBoxStyle.fixedHeight = true;
		inputBoxStyle.height = 19;
		inputBoxStyle.minWidth = 10;
		inputBoxStyle.font = font;
		inputBoxStyle.fontSize = DefaultFontSize;
		inputBoxStyle.textHorzAlign = THA_Left;
		inputBoxStyle.textVertAlign = TVA_Top;

		skin->setStyle(GUIInputBox::getGUITypeName(), inputBoxStyle);

		/************************************************************************/
		/* 								SCROLL BAR                      		*/
		/************************************************************************/

		// Up button
		GUIElementStyle scrollUpBtnStyle;
		scrollUpBtnStyle.normal.texture = getSkinTexture(ScrollBarUpNormalTex);
		scrollUpBtnStyle.hover.texture = getSkinTexture(ScrollBarUpHoverTex);
		scrollUpBtnStyle.active.texture = getSkinTexture(ScrollBarUpActiveTex);
		scrollUpBtnStyle.fixedHeight = true;
		scrollUpBtnStyle.fixedWidth = true;
		scrollUpBtnStyle.height = 11;
		scrollUpBtnStyle.width = 13;

		skin->setStyle("ScrollUpBtn", scrollUpBtnStyle);

		// Down button
		GUIElementStyle scrollDownBtnStyle;
		scrollDownBtnStyle.normal.texture = getSkinTexture(ScrollBarDownNormalTex);
		scrollDownBtnStyle.hover.texture = getSkinTexture(ScrollBarDownHoverTex);
		scrollDownBtnStyle.active.texture = getSkinTexture(ScrollBarDownActiveTex);
		scrollDownBtnStyle.fixedHeight = true;
		scrollDownBtnStyle.fixedWidth = true;
		scrollDownBtnStyle.height = 11;
		scrollDownBtnStyle.width = 13;

		skin->setStyle("ScrollDownBtn", scrollDownBtnStyle);

		// Left button
		GUIElementStyle scrollLeftBtnStyle;
		scrollLeftBtnStyle.normal.texture = getSkinTexture(ScrollBarLeftNormalTex);
		scrollLeftBtnStyle.hover.texture = getSkinTexture(ScrollBarLeftHoverTex);
		scrollLeftBtnStyle.active.texture = getSkinTexture(ScrollBarLeftActiveTex);
		scrollLeftBtnStyle.fixedHeight = true;
		scrollLeftBtnStyle.fixedWidth = true;
		scrollLeftBtnStyle.height = 13;
		scrollLeftBtnStyle.width = 11;

		skin->setStyle("ScrollLeftBtn", scrollLeftBtnStyle);

		// Right button
		GUIElementStyle scrollRightBtnStyle;
		scrollRightBtnStyle.normal.texture = getSkinTexture(ScrollBarRightNormalTex);
		scrollRightBtnStyle.hover.texture = getSkinTexture(ScrollBarRightHoverTex);
		scrollRightBtnStyle.active.texture = getSkinTexture(ScrollBarRightActiveTex);
		scrollRightBtnStyle.fixedHeight = true;
		scrollRightBtnStyle.fixedWidth = true;
		scrollRightBtnStyle.height = 13;
		scrollRightBtnStyle.width = 11;

		skin->setStyle("ScrollRightBtn", scrollRightBtnStyle);

		// Horizontal handle
		GUIElementStyle scrollBarHorzBtnStyle;
		scrollBarHorzBtnStyle.normal.texture = getSkinTexture(ScrollBarHandleHorzNormalTex);
		scrollBarHorzBtnStyle.hover.texture = getSkinTexture(ScrollBarHandleHorzHoverTex);
		scrollBarHorzBtnStyle.active.texture = getSkinTexture(ScrollBarHandleHorzActiveTex);
		scrollBarHorzBtnStyle.fixedHeight = true;
		scrollBarHorzBtnStyle.fixedWidth = false;
		scrollBarHorzBtnStyle.width = 10;
		scrollBarHorzBtnStyle.height = 13;
		scrollBarHorzBtnStyle.border.left = 4;
		scrollBarHorzBtnStyle.border.right = 4;

		skin->setStyle("ScrollBarHorzBtn", scrollBarHorzBtnStyle);

		// Vertical handle
		GUIElementStyle scrollBarVertBtnStyle;
		scrollBarVertBtnStyle.normal.texture = getSkinTexture(ScrollBarHandleVertNormalTex);
		scrollBarVertBtnStyle.hover.texture = getSkinTexture(ScrollBarHandleVertHoverTex);
		scrollBarVertBtnStyle.active.texture = getSkinTexture(ScrollBarHandleVertActiveTex);
		scrollBarVertBtnStyle.fixedHeight = false;
		scrollBarVertBtnStyle.fixedWidth = true;
		scrollBarVertBtnStyle.width = 13;
		scrollBarVertBtnStyle.height = 10;
		scrollBarVertBtnStyle.border.top = 4;
		scrollBarVertBtnStyle.border.bottom = 4;

		skin->setStyle("ScrollBarVertBtn", scrollBarVertBtnStyle);

		// Vertical scroll bar
		GUIElementStyle vertScrollBarStyle;
		vertScrollBarStyle.normal.texture = getSkinTexture(ScrollBarVBgTex);
		vertScrollBarStyle.hover.texture = vertScrollBarStyle.normal.texture;
		vertScrollBarStyle.active.texture = vertScrollBarStyle.normal.texture;
		vertScrollBarStyle.fixedHeight = false;
		vertScrollBarStyle.fixedWidth = true;
		vertScrollBarStyle.minHeight = 8;
		vertScrollBarStyle.width = 16;

		vertScrollBarStyle.subStyles[GUIScrollBar::getVScrollHandleType()] = "ScrollBarVertBtn";
		vertScrollBarStyle.subStyles[GUIScrollBar::getHScrollHandleType()] = "ScrollBarHorzBtn";

		skin->setStyle("ScrollBarVert", vertScrollBarStyle);

		// Horizontal scroll bar
		GUIElementStyle horzScrollBarStyle;
		horzScrollBarStyle.normal.texture = getSkinTexture(ScrollBarHBgTex);
		horzScrollBarStyle.hover.texture = horzScrollBarStyle.normal.texture;
		horzScrollBarStyle.active.texture = horzScrollBarStyle.normal.texture;
		horzScrollBarStyle.fixedHeight = true;
		horzScrollBarStyle.fixedWidth = false;
		horzScrollBarStyle.minWidth = 8;
		horzScrollBarStyle.height = 16;

		horzScrollBarStyle.subStyles[GUIScrollBar::getVScrollHandleType()] = "ScrollBarVertBtn";
		horzScrollBarStyle.subStyles[GUIScrollBar::getHScrollHandleType()] = "ScrollBarHorzBtn";

		skin->setStyle("ScrollBarHorz", horzScrollBarStyle);

		// Horizontal resizeable handle
		GUIElementStyle scrollBarHorzResizeableBtnStyle;
		scrollBarHorzResizeableBtnStyle.normal.texture = getSkinTexture(ScrollBarResizeableHandleHorzNormalTex);
		scrollBarHorzResizeableBtnStyle.hover.texture = getSkinTexture(ScrollBarResizeableHandleHorzHoverTex);
		scrollBarHorzResizeableBtnStyle.active.texture = getSkinTexture(ScrollBarResizeableHandleHorzActiveTex);
		scrollBarHorzResizeableBtnStyle.fixedHeight = true;
		scrollBarHorzResizeableBtnStyle.fixedWidth = false;
		scrollBarHorzResizeableBtnStyle.minWidth = 15;
		scrollBarHorzResizeableBtnStyle.height = 13;
		scrollBarHorzResizeableBtnStyle.border.left = 7;
		scrollBarHorzResizeableBtnStyle.border.right = 7;

		skin->setStyle("ScrollBarResizeableHorzBtn", scrollBarHorzResizeableBtnStyle);

		// Vertical resizeable handle
		GUIElementStyle scrollBarVertResizeableBtnStyle;
		scrollBarVertResizeableBtnStyle.normal.texture = getSkinTexture(ScrollBarResizeableHandleVertNormalTex);
		scrollBarVertResizeableBtnStyle.hover.texture = getSkinTexture(ScrollBarResizeableHandleVertHoverTex);
		scrollBarVertResizeableBtnStyle.active.texture = getSkinTexture(ScrollBarResizeableHandleVertActiveTex);
		scrollBarVertResizeableBtnStyle.fixedHeight = false;
		scrollBarVertResizeableBtnStyle.fixedWidth = true;
		scrollBarVertResizeableBtnStyle.width = 13;
		scrollBarVertResizeableBtnStyle.minHeight = 15;
		scrollBarVertResizeableBtnStyle.border.top = 7;
		scrollBarVertResizeableBtnStyle.border.bottom = 7;

		skin->setStyle("ScrollBarResizeableVertBtn", scrollBarVertResizeableBtnStyle);

		// Vertical resizeable scroll bar
		GUIElementStyle vertResizeableScrollBarStyle;
		vertResizeableScrollBarStyle.normal.texture = getSkinTexture(ScrollBarVBgTex);
		vertResizeableScrollBarStyle.hover.texture = vertResizeableScrollBarStyle.normal.texture;
		vertResizeableScrollBarStyle.active.texture = vertResizeableScrollBarStyle.normal.texture;
		vertResizeableScrollBarStyle.fixedHeight = false;
		vertResizeableScrollBarStyle.fixedWidth = true;
		vertResizeableScrollBarStyle.minHeight = 15;
		vertResizeableScrollBarStyle.width = 16;

		vertResizeableScrollBarStyle.subStyles[GUIScrollBar::getVScrollHandleType()] = "ScrollBarResizeableVertBtn";
		vertResizeableScrollBarStyle.subStyles[GUIScrollBar::getHScrollHandleType()] = "ScrollBarResizeableHorzBtn";

		skin->setStyle("ResizeableScrollBarVert", vertResizeableScrollBarStyle);

		// Horizontal resizeable scroll bar
		GUIElementStyle horzResizeableScrollBarStyle;
		horzResizeableScrollBarStyle.normal.texture = getSkinTexture(ScrollBarHBgTex);
		horzResizeableScrollBarStyle.hover.texture = horzResizeableScrollBarStyle.normal.texture;
		horzResizeableScrollBarStyle.active.texture = horzResizeableScrollBarStyle.normal.texture;
		horzResizeableScrollBarStyle.fixedHeight = true;
		horzResizeableScrollBarStyle.fixedWidth = false;
		horzResizeableScrollBarStyle.minWidth = 15;
		horzResizeableScrollBarStyle.height = 16;

		horzResizeableScrollBarStyle.subStyles[GUIScrollBar::getVScrollHandleType()] = "ScrollBarResizeableVertBtn";
		horzResizeableScrollBarStyle.subStyles[GUIScrollBar::getHScrollHandleType()] = "ScrollBarResizeableHorzBtn";

		skin->setStyle("ResizeableScrollBarHorz", horzResizeableScrollBarStyle);

		/************************************************************************/
		/* 								DROP DOWN BOX                      		*/
		/************************************************************************/

		// ListBox button
		GUIElementStyle dropDownListStyle;
		dropDownListStyle.normal.texture = getSkinTexture(DropDownBtnNormalTex);
		dropDownListStyle.hover.texture = getSkinTexture(DropDownBtnHoverTex);
		dropDownListStyle.active.texture = getSkinTexture(DropDownBtnActiveTex);
		dropDownListStyle.normalOn.texture = dropDownListStyle.active.texture;
		dropDownListStyle.hoverOn.texture = dropDownListStyle.active.texture;
		dropDownListStyle.activeOn.texture = dropDownListStyle.active.texture;
		dropDownListStyle.normal.textColor = TextNormalColor;
		dropDownListStyle.hover.textColor = TextNormalColor;
		dropDownListStyle.active.textColor = TextNormalColor;
		dropDownListStyle.normalOn.textColor = TextNormalColor;
		dropDownListStyle.hoverOn.textColor = TextNormalColor;
		dropDownListStyle.activeOn.textColor = TextNormalColor;
		dropDownListStyle.fixedHeight = true;
		dropDownListStyle.fixedWidth = false;
		dropDownListStyle.height = 21;
		dropDownListStyle.minWidth = 20;
		dropDownListStyle.contentOffset.left = 3;
		dropDownListStyle.contentOffset.right = 18;
		dropDownListStyle.contentOffset.top = 2;
		dropDownListStyle.contentOffset.bottom = 2;
		dropDownListStyle.border.left = 2;
		dropDownListStyle.border.right = 16;
		dropDownListStyle.border.top = 2;
		dropDownListStyle.border.bottom = 4;
		dropDownListStyle.margins.bottom = 2;
		dropDownListStyle.font = font;
		dropDownListStyle.fontSize = DefaultFontSize;
		dropDownListStyle.textHorzAlign = THA_Left;
		dropDownListStyle.textVertAlign = TVA_Center;

		skin->setStyle("ListBox", dropDownListStyle);

		// DropDown scroll up button
		GUIElementStyle dropDownScrollUpBtnStyle;
		dropDownScrollUpBtnStyle.normal.texture = getSkinTexture(DropDownBoxBtnUpNormalTex);
		dropDownScrollUpBtnStyle.hover.texture = getSkinTexture(DropDownBoxBtnUpHoverTex);
		dropDownScrollUpBtnStyle.active.texture = dropDownScrollUpBtnStyle.hover.texture;
		dropDownScrollUpBtnStyle.fixedHeight = true;
		dropDownScrollUpBtnStyle.fixedWidth = true;
		dropDownScrollUpBtnStyle.width = 8;
		dropDownScrollUpBtnStyle.height = 12;

		skin->setStyle("ListBoxScrollUpBtn", dropDownScrollUpBtnStyle);
		skin->setStyle("MenuBarScrollUpBtn", dropDownScrollUpBtnStyle);
		skin->setStyle("ContextMenuScrollUpBtn", dropDownScrollUpBtnStyle);

		// DropDown scroll down button
		GUIElementStyle dropDownScrollDownBtnStyle;
		dropDownScrollDownBtnStyle.normal.texture = getSkinTexture(DropDownBoxBtnDownNormalTex);
		dropDownScrollDownBtnStyle.hover.texture = getSkinTexture(DropDownBoxBtnDownHoverTex);
		dropDownScrollDownBtnStyle.active.texture = dropDownScrollDownBtnStyle.hover.texture;
		dropDownScrollDownBtnStyle.fixedHeight = true;
		dropDownScrollDownBtnStyle.fixedWidth = true;
		dropDownScrollDownBtnStyle.width = 8;
		dropDownScrollDownBtnStyle.height = 12;

		skin->setStyle("ListBoxScrollDownBtn", dropDownScrollDownBtnStyle);
		skin->setStyle("MenuBarScrollDownBtn", dropDownScrollDownBtnStyle);
		skin->setStyle("ContextMenuScrollDownBtn", dropDownScrollDownBtnStyle);

		// DropDown handle
		GUIElementStyle dropDownScrollHandleStyle;
		dropDownScrollHandleStyle.normal.texture = getSkinTexture(DropDownBoxHandleTex);
		dropDownScrollHandleStyle.fixedHeight = false;
		dropDownScrollHandleStyle.fixedWidth = true;
		dropDownScrollHandleStyle.height = 8;
		dropDownScrollHandleStyle.width = 8;

		skin->setStyle("ListBoxHandle", dropDownScrollHandleStyle);
		skin->setStyle("MenuBarHandle", dropDownScrollHandleStyle);
		skin->setStyle("ContextMenuHandle", dropDownScrollHandleStyle);

		// DropDown sidebar background
		GUIElementStyle dropDownSidebarBg;
		dropDownSidebarBg.normal.texture = getSkinTexture(DropDownBoxSideBgTex);
		dropDownSidebarBg.fixedHeight = false;
		dropDownSidebarBg.fixedWidth = true;
		dropDownSidebarBg.height = 8;
		dropDownSidebarBg.width = 9;
		dropDownSidebarBg.border.left = 1;
		dropDownSidebarBg.border.top = 1;
		dropDownSidebarBg.border.bottom = 1;

		skin->setStyle("ListBoxSidebarBg", dropDownSidebarBg);
		skin->setStyle("MenuBarSidebarBg", dropDownSidebarBg);
		skin->setStyle("ContextMenuSidebarBg", dropDownSidebarBg);

		// DropDown entry button
		GUIElementStyle dropDownEntryBtnStyle;
		dropDownEntryBtnStyle.normal.texture = getSkinTexture(DropDownBoxEntryNormalTex);
		dropDownEntryBtnStyle.hover.texture = getSkinTexture(DropDownBoxEntryHoverTex);
		dropDownEntryBtnStyle.active.texture = dropDownEntryBtnStyle.hover.texture;
		dropDownEntryBtnStyle.normalOn.texture = dropDownEntryBtnStyle.hover.texture;
		dropDownEntryBtnStyle.hoverOn.texture = dropDownEntryBtnStyle.hover.texture;
		dropDownEntryBtnStyle.activeOn.texture = dropDownEntryBtnStyle.hover.texture;
		dropDownEntryBtnStyle.normal.textColor = TextNormalColor;
		dropDownEntryBtnStyle.hover.textColor = TextNormalColor;
		dropDownEntryBtnStyle.active.textColor = TextNormalColor;
		dropDownEntryBtnStyle.normalOn.textColor = TextNormalColor;
		dropDownEntryBtnStyle.hoverOn.textColor = TextNormalColor;
		dropDownEntryBtnStyle.activeOn.textColor = TextNormalColor;
		dropDownEntryBtnStyle.fixedHeight = true;
		dropDownEntryBtnStyle.fixedWidth = false;
		dropDownEntryBtnStyle.height = 16;
		dropDownEntryBtnStyle.width = 30;
		dropDownEntryBtnStyle.font = font;
		dropDownEntryBtnStyle.fontSize = DefaultFontSize;
		dropDownEntryBtnStyle.textHorzAlign = THA_Left;
		dropDownEntryBtnStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIDropDownContent::ENTRY_STYLE_TYPE, dropDownEntryBtnStyle);

		// DropDown toggle entry button
		GUIElementStyle dropDownToggleEntryBtnStyle;
		dropDownToggleEntryBtnStyle.normal.texture = getSkinTexture(DropDownBoxEntryToggleNormalTex);
		dropDownToggleEntryBtnStyle.hover.texture = getSkinTexture(DropDownBoxEntryToggleHoverTex);
		dropDownToggleEntryBtnStyle.active.texture = dropDownToggleEntryBtnStyle.hover.texture;
		dropDownToggleEntryBtnStyle.normalOn.texture = getSkinTexture(DropDownBoxEntryToggleNormalOnTex);
		dropDownToggleEntryBtnStyle.hoverOn.texture = getSkinTexture(DropDownBoxEntryToggleHoverOnTex);
		dropDownToggleEntryBtnStyle.activeOn.texture = dropDownToggleEntryBtnStyle.hoverOn.texture;
		dropDownToggleEntryBtnStyle.normal.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.hover.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.active.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.normalOn.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.hoverOn.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.activeOn.textColor = TextNormalColor;
		dropDownToggleEntryBtnStyle.fixedHeight = true;
		dropDownToggleEntryBtnStyle.fixedWidth = false;
		dropDownToggleEntryBtnStyle.height = 18;
		dropDownToggleEntryBtnStyle.width = 30;
		dropDownToggleEntryBtnStyle.border.left = 17;
		dropDownToggleEntryBtnStyle.contentOffset.left = 17;
		dropDownToggleEntryBtnStyle.font = font;
		dropDownToggleEntryBtnStyle.fontSize = DefaultFontSize;
		dropDownToggleEntryBtnStyle.textHorzAlign = THA_Left;
		dropDownToggleEntryBtnStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIDropDownContent::ENTRY_TOGGLE_STYLE_TYPE, dropDownToggleEntryBtnStyle);

		// DropDown entry button with expand
		GUIElementStyle dropDownEntryExpBtnStyle;
		dropDownEntryExpBtnStyle.normal.texture = getSkinTexture(DropDownBoxEntryExpNormalTex);
		dropDownEntryExpBtnStyle.hover.texture = getSkinTexture(DropDownBoxEntryExpHoverTex);
		dropDownEntryExpBtnStyle.active.texture = dropDownEntryExpBtnStyle.hover.texture;
		dropDownEntryExpBtnStyle.normalOn.texture = dropDownEntryExpBtnStyle.hover.texture;
		dropDownEntryExpBtnStyle.hoverOn.texture = dropDownEntryExpBtnStyle.hover.texture;
		dropDownEntryExpBtnStyle.activeOn.texture = dropDownEntryExpBtnStyle.hover.texture;
		dropDownEntryExpBtnStyle.normal.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.hover.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.active.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.normalOn.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.hoverOn.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.activeOn.textColor = TextNormalColor;
		dropDownEntryExpBtnStyle.fixedHeight = true;
		dropDownEntryExpBtnStyle.fixedWidth = false;
		dropDownEntryExpBtnStyle.height = 16;
		dropDownEntryExpBtnStyle.width = 30;
		dropDownEntryExpBtnStyle.border.right = 13;
		dropDownEntryExpBtnStyle.font = font;
		dropDownEntryExpBtnStyle.fontSize = DefaultFontSize;
		dropDownEntryExpBtnStyle.textHorzAlign = THA_Left;
		dropDownEntryExpBtnStyle.textVertAlign = TVA_Center;

		skin->setStyle(GUIDropDownContent::ENTRY_EXP_STYLE_TYPE, dropDownEntryExpBtnStyle);

		// Drop down separator
		GUIElementStyle dropDownSeparatorStyle;
		dropDownSeparatorStyle.normal.texture = getSkinTexture(DropDownSeparatorTex);
		dropDownSeparatorStyle.fixedHeight = true;
		dropDownSeparatorStyle.fixedWidth = false;
		dropDownSeparatorStyle.height = 3;
		dropDownSeparatorStyle.width = 30;

		skin->setStyle(GUIDropDownContent::SEPARATOR_STYLE_TYPE, dropDownSeparatorStyle);

		// Drop down content
		GUIElementStyle dropDownContentStyle;
		dropDownContentStyle.minWidth = 50;
		dropDownContentStyle.minHeight = 20;
		dropDownContentStyle.subStyles[GUIDropDownContent::ENTRY_TOGGLE_STYLE_TYPE] = GUIDropDownContent::ENTRY_TOGGLE_STYLE_TYPE;
		dropDownContentStyle.subStyles[GUIDropDownContent::ENTRY_STYLE_TYPE] = GUIDropDownContent::ENTRY_STYLE_TYPE;
		dropDownContentStyle.subStyles[GUIDropDownContent::ENTRY_EXP_STYLE_TYPE] = GUIDropDownContent::ENTRY_EXP_STYLE_TYPE;
		dropDownContentStyle.subStyles[GUIDropDownContent::SEPARATOR_STYLE_TYPE] = GUIDropDownContent::SEPARATOR_STYLE_TYPE;

		skin->setStyle("ListBoxContent", dropDownContentStyle);
		skin->setStyle("MenuBarContent", dropDownContentStyle);
		skin->setStyle("ContextMenuContent", dropDownContentStyle);

		// DropDown box frame
		GUIElementStyle dropDownBoxStyle;
		dropDownBoxStyle.normal.texture = getSkinTexture(DropDownBoxBgTex);
		dropDownBoxStyle.hover.texture = dropDownBoxStyle.normal.texture;
		dropDownBoxStyle.active.texture = dropDownBoxStyle.hover.texture;
		dropDownBoxStyle.fixedHeight = false;
		dropDownBoxStyle.fixedWidth = false;
		dropDownBoxStyle.border.left = 2;
		dropDownBoxStyle.border.right = 2;
		dropDownBoxStyle.border.top = 2;
		dropDownBoxStyle.border.bottom = 4;
		dropDownBoxStyle.margins.left = 6;
		dropDownBoxStyle.margins.right = 6;
		dropDownBoxStyle.margins.top = 4;
		dropDownBoxStyle.margins.bottom = 6;

		skin->setStyle("ListBoxFrame", dropDownBoxStyle);
		skin->setStyle("MenuBarFrame", dropDownBoxStyle);
		skin->setStyle("ContextMenuFrame", dropDownBoxStyle);

		/************************************************************************/
		/* 								TOOLTIP                      			*/
		/************************************************************************/

		GUIElementStyle tooltipFrameStyle;
		tooltipFrameStyle.normal.texture = getSkinTexture(DropDownBoxBgTex);
		tooltipFrameStyle.hover.texture = tooltipFrameStyle.normal.texture;
		tooltipFrameStyle.active.texture = tooltipFrameStyle.hover.texture;
		tooltipFrameStyle.fixedHeight = false;
		tooltipFrameStyle.fixedWidth = false;
		tooltipFrameStyle.border.left = 2;
		tooltipFrameStyle.border.right = 2;
		tooltipFrameStyle.border.top = 2;
		tooltipFrameStyle.border.bottom = 4;
		tooltipFrameStyle.margins.left = 6;
		tooltipFrameStyle.margins.right = 6;
		tooltipFrameStyle.margins.top = 4;
		tooltipFrameStyle.margins.bottom = 6;

		skin->setStyle(GUITooltip::getFrameStyleName(), tooltipFrameStyle);

		/************************************************************************/
		/* 								SLIDER                      			*/
		/************************************************************************/

		GUIElementStyle sliderHandleStyle;
		sliderHandleStyle.fixedHeight = true;
		sliderHandleStyle.fixedWidth = true;
		sliderHandleStyle.width = 12;
		sliderHandleStyle.height = 13;
		sliderHandleStyle.normal.texture = getSkinTexture(SliderHandleNormalTex);
		sliderHandleStyle.hover.texture = getSkinTexture(SliderHandleHoverTex);
		sliderHandleStyle.active.texture = getSkinTexture(SliderHandleActiveTex);

		skin->setStyle(GUISlider::getHandleStyleType(), sliderHandleStyle);

		GUIElementStyle sliderHorizontalBgStyle;
		sliderHorizontalBgStyle.fixedHeight = true;
		sliderHorizontalBgStyle.height = 10;
		sliderHorizontalBgStyle.normal.texture = getSkinTexture(SliderHBackgroundTex);
		sliderHorizontalBgStyle.border.left = 4;
		sliderHorizontalBgStyle.border.right = 4;

		skin->setStyle("SliderHorzBg", sliderHorizontalBgStyle);

		GUIElementStyle sliderHorizontalFillStyle;
		sliderHorizontalFillStyle.fixedHeight = true;
		sliderHorizontalFillStyle.height = 10;
		sliderHorizontalFillStyle.normal.texture = getSkinTexture(SliderHFillTex);
		sliderHorizontalFillStyle.border.left = 6;
		sliderHorizontalFillStyle.border.right = 4;

		skin->setStyle("SliderHorzFill", sliderHorizontalFillStyle);

		GUIElementStyle sliderHorizontalStyle;
		sliderHorizontalStyle.fixedHeight = true;
		sliderHorizontalStyle.height = 13;
		sliderHorizontalStyle.width = 150;
		sliderHorizontalStyle.minWidth = 10;
		sliderHorizontalStyle.subStyles[GUISlider::getHandleStyleType()] = GUISlider::getHandleStyleType();
		sliderHorizontalStyle.subStyles[GUISlider::getBackgroundStyleType()] = "SliderHorzBg";
		sliderHorizontalStyle.subStyles[GUISlider::getFillStyleType()] = "SliderHorzFill";

		skin->setStyle(GUISliderHorz::getGUITypeName(), sliderHorizontalStyle);

		GUIElementStyle sliderVerticalBgStyle;
		sliderVerticalBgStyle.fixedWidth = true;
		sliderVerticalBgStyle.width = 10;
		sliderVerticalBgStyle.normal.texture = getSkinTexture(SliderVBackgroundTex);
		sliderVerticalBgStyle.border.top = 4;
		sliderVerticalBgStyle.border.bottom = 4;

		skin->setStyle("SliderVertBg", sliderVerticalBgStyle);

		GUIElementStyle sliderVerticalFillStyle;
		sliderVerticalFillStyle.fixedWidth = true;
		sliderVerticalFillStyle.width = 10;
		sliderVerticalFillStyle.normal.texture = getSkinTexture(SliderVFillTex);
		sliderVerticalFillStyle.border.top = 6;
		sliderVerticalFillStyle.border.bottom = 4;

		skin->setStyle("SliderVertFill", sliderVerticalFillStyle);

		GUIElementStyle sliderVerticalStyle;
		sliderVerticalStyle.fixedWidth = true;
		sliderVerticalStyle.width = 13;
		sliderVerticalStyle.height = 150;
		sliderVerticalStyle.minHeight = 10;
		sliderVerticalStyle.subStyles[GUISlider::getHandleStyleType()] = GUISlider::getHandleStyleType();
		sliderVerticalStyle.subStyles[GUISlider::getBackgroundStyleType()] = "SliderVertBg";
		sliderVerticalStyle.subStyles[GUISlider::getFillStyleType()] = "SliderVertFill";

		skin->setStyle(GUISliderVert::getGUITypeName(), sliderVerticalStyle);

		/************************************************************************/
		/* 									OTHER                      			*/
		/************************************************************************/

		// Right-aligned label
		GUIElementStyle rightAlignedLabelStyle;
		rightAlignedLabelStyle.font = font;
		rightAlignedLabelStyle.fontSize = DefaultFontSize;
		rightAlignedLabelStyle.fixedWidth = false;
		rightAlignedLabelStyle.fixedHeight = true;
		rightAlignedLabelStyle.height = 11;
		rightAlignedLabelStyle.minWidth = 10;
		rightAlignedLabelStyle.textHorzAlign = THA_Right;
		rightAlignedLabelStyle.normal.textColor = TextNormalColor;

		skin->setStyle("RightAlignedLabel", rightAlignedLabelStyle);

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
		Path outputDir = mBuiltinDataFolder + TextureFolder;

		auto saveTexture = [&](const Path& path, const SPtr<Texture>& texture, const String& UUID)
		{
			HResource textureResource = gResources()._createResourceHandle(texture, UUID);

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

		UINT32 quadNumVertices = 8;
		UINT32 quadNumIndices = 12;
		ShapeMeshes3D::getNumElementsQuad(quadNumVertices, quadNumIndices);
		SPtr<MeshData> quadMeshData = bs_shared_ptr_new<MeshData>(quadNumVertices, quadNumIndices, vertexDesc);

		std::array<Vector3, 2> axes = {{ Vector3::UNIT_X, Vector3::UNIT_Y }};
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

		auto saveMesh = [&](const Path& path, const SPtr<Mesh>& mesh, const String& UUID)
		{
			HResource meshResource = gResources()._createResourceHandle(mesh, UUID);

			gResources().save(meshResource, path, true);
			mResourceManifest->registerResource(meshResource.getUUID(), path);
		};

		Path boxPath = outputDir + MeshBoxFile;
		saveMesh(boxPath, boxMesh, "bc1d20ca-7fe6-489b-8b5c-dbf798badc95");

		Path spherePath = outputDir + MeshSphereFile;
		saveMesh(spherePath, sphereMesh, "040642f3-04d6-419e-9dba-f7824161c205");

		Path conePath = outputDir + MeshConeFile;
		saveMesh(conePath, coneMesh, "b8cf6db5-1736-47ac-852f-82ecd88b4d46");

		Path quadPath = outputDir + MeshQuadFile;
		saveMesh(quadPath, quadMesh, "06592bf3-f82a-472e-a034-26a98225fbe1");

		Path discPath = outputDir + MeshDiscFile;
		saveMesh(discPath, discMesh, "6f496313-344a-495c-83e8-152e3053c52d");
	}

	HSpriteTexture BuiltinResources::getSkinTexture(const WString& name)
	{
		Path texturePath = mEngineSkinSpritesFolder;
		texturePath.append(L"sprite_" + name + L".asset");

		return gResources().load<SpriteTexture>(texturePath);
	}

	HShader BuiltinResources::getShader(const Path& path)
	{
		Path programPath = mEngineShaderFolder;
		programPath.append(path);
		programPath.setExtension(programPath.getExtension() + ".asset");

		HShader shader = gResources().load<Shader>(programPath);

#if BS_DEBUG_MODE
		BuiltinResourcesHelper::verifyAndReportShader(shader);
#endif

		return shader;
	}

	HTexture BuiltinResources::getCursorTexture(const WString& name)
	{
		Path cursorPath = mEngineCursorFolder;
		cursorPath.append(name + L".asset");

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

	const PixelData& BuiltinResources::getBansheeIcon()
	{
		return *mBansheeIcon.get();
	}

	SPtr<PixelData> BuiltinResources::getSplashScreen()
	{
		Path splashScreenPath = Paths::getEngineDataPath() + (WString(SplashScreenName) + L".asset");
		FileDecoder fd(splashScreenPath);

		return std::static_pointer_cast<PixelData>(fd.decode());
	}

	Path BuiltinResources::getShaderIncludeFolder()
	{
		return Paths::getEngineDataPath() + ShaderIncludeFolder;
	}

	Path BuiltinResources::getIconFolder()
	{
		return Paths::getEngineDataPath() + IconFolder;
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
		}

		return HShader();
	}

	HTexture BuiltinResources::getTexture(BuiltinTexture type)
	{
		Path texturePath = Paths::getEngineDataPath();
		texturePath.append(TextureFolder);

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
}