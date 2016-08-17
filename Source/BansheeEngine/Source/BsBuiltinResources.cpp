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
#include "BsFontImportOptions.h"
#include "BsTexture.h"
#include "BsImporter.h"
#include "BsResources.h"
#include "BsShader.h"
#include "BsMaterial.h"
#include "BsRTTIType.h"
#include "BsFileSystem.h"
#include "BsCoreApplication.h"
#include "BsCoreThread.h"
#include "BsDataStream.h"
#include "BsResourceManifest.h"
#include "BsVertexDataDesc.h"
#include "BsShapeMeshes3D.h"
#include "BsMesh.h"
#include "BsGUITooltip.h"
#include "BsFileSerializer.h"
#include "BsTextureImportOptions.h"
#include "BsShaderImportOptions.h"
#include "BsRendererMaterialManager.h"

namespace BansheeEngine
{
	const WString BuiltinResources::DefaultFontFilename = L"arial.ttf";
	const UINT32 BuiltinResources::DefaultFontSize = 11;

	const Color BuiltinResources::TextNormalColor = Color(0.7f, 0.7f, 0.7f);
	const Color BuiltinResources::TextActiveColor = Color(0.0f, 0.0f, 0.0f);

	const String BuiltinResources::MultiLineLabelStyle = "MultiLineLabel";

	const WString BuiltinResources::GUISkinFile = L"GUISkin";

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

	const WString BuiltinResources::ScrollBarHBgTex = L"ScrollBarHBackground.png";
	const WString BuiltinResources::ScrollBarVBgTex = L"ScrollBarVBackground.png";

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
		mEngineRawSkinFolder = mBuiltinRawDataFolder + SkinFolder;
		mEngineRawCursorFolder = mBuiltinRawDataFolder + CursorFolder;
		mEngineRawIconFolder = mBuiltinRawDataFolder + IconFolder;
		mEngineRawShaderFolder = mBuiltinRawDataFolder + ShaderFolder;
		mEngineRawShaderIncludeFolder = mBuiltinRawDataFolder + ShaderIncludeFolder;

		mBuiltinDataFolder = Paths::getEngineDataPath();
		mEngineSkinFolder = mBuiltinDataFolder + SkinFolder;
		mEngineSkinSpritesFolder = mEngineSkinFolder + SpriteSubFolder;
		mEngineCursorFolder = mBuiltinDataFolder + CursorFolder;
		mEngineIconFolder = mBuiltinDataFolder + IconFolder;
		mEngineShaderFolder = mBuiltinDataFolder + ShaderFolder;
		mEngineShaderIncludeFolder = mBuiltinDataFolder + ShaderIncludeFolder;
		mEngineMeshFolder = mBuiltinDataFolder + MeshFolder;
		mEngineTextureFolder = mBuiltinDataFolder + TextureFolder;

		ResourceManifestPath = mBuiltinDataFolder + "ResourceManifest.asset";

		// Update from raw assets if needed
#if BS_DEBUG_MODE
		if (FileSystem::exists(mBuiltinRawDataFolder))
		{
			if (BuiltinResourcesHelper::checkForModifications(mBuiltinRawDataFolder, mBuiltinDataFolder + L"Timestamp.asset"))
			{
				SPtr<ResourceManifest> oldResourceManifest;
				if (FileSystem::exists(ResourceManifestPath))
				{
					oldResourceManifest = ResourceManifest::load(ResourceManifestPath, mBuiltinDataFolder);
					if (oldResourceManifest != nullptr)
						gResources().registerResourceManifest(oldResourceManifest);
				}

				mResourceManifest = ResourceManifest::create("BuiltinResources");
				gResources().registerResourceManifest(mResourceManifest);

				preprocess();
				BuiltinResourcesHelper::writeTimestamp(mBuiltinDataFolder + L"Timestamp.asset");

				ResourceManifest::save(mResourceManifest, ResourceManifestPath, mBuiltinDataFolder);

				if (oldResourceManifest != nullptr)
					gResources().unregisterResourceManifest(oldResourceManifest);
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

		SPtr<PixelData> dummyPixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);

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

		mCursorArrow = cursorArrowTex->getProperties().allocateSubresourceBuffer(0);
		cursorArrowTex->readSubresource(gCoreAccessor(), 0, mCursorArrow);

		mCursorArrowDrag = cursorArrowDragTex->getProperties().allocateSubresourceBuffer(0);
		cursorArrowDragTex->readSubresource(gCoreAccessor(), 0, mCursorArrowDrag);

		mCursorArrowLeftRight = cursorArrowLeftRightTex->getProperties().allocateSubresourceBuffer(0);
		cursorArrowLeftRightTex->readSubresource(gCoreAccessor(), 0, mCursorArrowLeftRight);

		mCursorIBeam = cursorIBeamTex->getProperties().allocateSubresourceBuffer(0);
		cursorIBeamTex->readSubresource(gCoreAccessor(), 0, mCursorIBeam);

		mCursorDeny = cursorDenyTex->getProperties().allocateSubresourceBuffer(0);
		cursorDenyTex->readSubresource(gCoreAccessor(), 0, mCursorDeny);

		mCursorWait = cursorWaitTex->getProperties().allocateSubresourceBuffer(0);
		cursorWaitTex->readSubresource(gCoreAccessor(), 0, mCursorWait);

		mCursorSizeNESW = cursorSizeNESWTex->getProperties().allocateSubresourceBuffer(0);
		cursorSizeNESWTex->readSubresource(gCoreAccessor(), 0, mCursorSizeNESW);

		mCursorSizeNS = cursorSizeNSTex->getProperties().allocateSubresourceBuffer(0);
		cursorSizeNSTex->readSubresource(gCoreAccessor(), 0, mCursorSizeNS);

		mCursorSizeNWSE = cursorSizeNWSETex->getProperties().allocateSubresourceBuffer(0);
		cursorSizeNWSETex->readSubresource(gCoreAccessor(), 0, mCursorSizeNWSE);

		mCursorSizeWE = cursorSizeWETex->getProperties().allocateSubresourceBuffer(0);
		cursorSizeWETex->readSubresource(gCoreAccessor(), 0, mCursorSizeWE);

		/************************************************************************/
		/* 								ICON		                     		*/
		/************************************************************************/

		Path iconPath = mEngineIconFolder;
		iconPath.append(IconTextureName + L".asset");

		HTexture iconTex = gResources().load<Texture>(iconPath);

		mBansheeIcon = iconTex->getProperties().allocateSubresourceBuffer(0);
		iconTex->readSubresource(gCoreAccessor(), 0, mBansheeIcon);

		gCoreAccessor().submitToCoreThread(true);
	}

	void BuiltinResources::preprocess()
	{
		// Hidden dependency: Textures need to be generated before shaders as they may use the default textures
		generateTextures();

		BuiltinResourcesHelper::importAssets(mEngineRawCursorFolder, mEngineCursorFolder, mResourceManifest);
		BuiltinResourcesHelper::importAssets(mEngineRawIconFolder, mEngineIconFolder, mResourceManifest);
		BuiltinResourcesHelper::importAssets(mEngineRawShaderIncludeFolder, mEngineShaderIncludeFolder, mResourceManifest); // Hidden dependency: Includes must be imported before shaders
		BuiltinResourcesHelper::importAssets(mEngineRawShaderFolder, mEngineShaderFolder, mResourceManifest);
		BuiltinResourcesHelper::importAssets(mEngineRawSkinFolder, mEngineSkinFolder, mResourceManifest);

		// Import font
		BuiltinResourcesHelper::importFont(mBuiltinRawDataFolder + DefaultFontFilename, DefaultFontFilename, mBuiltinDataFolder,
			{ DefaultFontSize }, false, mResourceManifest);

		// Import splash screen
		{
			Path inputPath = mBuiltinRawDataFolder + WString(SplashScreenName);
			Path outputPath = mBuiltinDataFolder + (WString(SplashScreenName) + L".asset");

			auto textureIO = gImporter().createImportOptions<TextureImportOptions>(inputPath);
			textureIO->setCPUReadable(true);
			textureIO->setGenerateMipmaps(false);
			HTexture splashTexture = gImporter().import<Texture>(inputPath, textureIO);

			SPtr<PixelData> splashPixelData = splashTexture->getProperties().allocateSubresourceBuffer(0);
			splashTexture->readData(*splashPixelData);

			FileEncoder fe(outputPath);
			fe.encode(splashPixelData.get());
		}

		// Generate & save GUI sprite textures
		BuiltinResourcesHelper::generateSpriteTextures(mEngineSkinFolder, mEngineSkinSpritesFolder, mResourceManifest);

		// Generate & save GUI skin
		{
			SPtr<GUISkin> skin = generateGUISkin();
			Path outputPath = mBuiltinDataFolder + (GUISkinFile + L".asset");

			HResource skinResource;
			if (FileSystem::exists(outputPath))
				skinResource = gResources().load(outputPath);

			if (skinResource.isLoaded())
				gResources().update(skinResource, skin);
			else
				skinResource = gResources()._createResourceHandle(skin);

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
		inputBoxStyle.border.left = 4;
		inputBoxStyle.border.right = 4;
		inputBoxStyle.border.top = 4;
		inputBoxStyle.border.bottom = 6;
		inputBoxStyle.contentOffset.left = 4;
		inputBoxStyle.contentOffset.right = 4;
		inputBoxStyle.contentOffset.top = 4;
		inputBoxStyle.contentOffset.bottom = 4;
		inputBoxStyle.margins.bottom = 2;
		inputBoxStyle.fixedHeight = true;
		inputBoxStyle.height = 21;
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

		skin->setStyle("ScrollBarHorz", horzScrollBarStyle);

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
		SPtr<PixelData> blackPixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);
		blackPixelData->setColorAt(Color::Black, 0, 0);
		blackPixelData->setColorAt(Color::Black, 0, 1);
		blackPixelData->setColorAt(Color::Black, 1, 0);
		blackPixelData->setColorAt(Color::Black, 1, 1);

		SPtr<Texture> blackTexture = Texture::_createPtr(blackPixelData);

		SPtr<PixelData> whitePixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);
		whitePixelData->setColorAt(Color::White, 0, 0);
		whitePixelData->setColorAt(Color::White, 0, 1);
		whitePixelData->setColorAt(Color::White, 1, 0);
		whitePixelData->setColorAt(Color::White, 1, 1);

		SPtr<Texture> whiteTexture = Texture::_createPtr(whitePixelData);

		SPtr<PixelData> normalPixelData = PixelData::create(2, 2, 1, PF_R8G8B8A8);

		Color encodedNormal(0.5f, 0.5f, 1.0f);
		normalPixelData->setColorAt(encodedNormal, 0, 0);
		normalPixelData->setColorAt(encodedNormal, 0, 1);
		normalPixelData->setColorAt(encodedNormal, 1, 0);
		normalPixelData->setColorAt(encodedNormal, 1, 1);

		SPtr<Texture> normalTexture = Texture::_createPtr(normalPixelData);

		// Save all textures
		Path outputDir = mEngineTextureFolder;

		auto saveTexture = [&](const Path& path, const SPtr<Texture>& texture)
		{
			HResource textureResource;
			if (FileSystem::exists(path))
				textureResource = gResources().load(path);

			if (textureResource.isLoaded())
				gResources().update(textureResource, texture);
			else
				textureResource = gResources()._createResourceHandle(texture);

			gResources().save(textureResource, path, true);
			mResourceManifest->registerResource(textureResource.getUUID(), path);
		};

		Path whitePath = outputDir + TextureWhiteFile;
		saveTexture(whitePath, whiteTexture);

		Path blackPath = outputDir + TextureBlackFile;
		saveTexture(blackPath, blackTexture);

		Path normalPath = outputDir + TextureNormalFile;
		saveTexture(normalPath, normalTexture);
	}

	void BuiltinResources::generateMeshes()
	{
		SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDesc->addVertElem(VET_FLOAT3, VES_NORMAL);
		vertexDesc->addVertElem(VET_COLOR, VES_COLOR);

		UINT32 boxNumVertices = 0;
		UINT32 boxNumIndices = 0;
		ShapeMeshes3D::getNumElementsAABox(boxNumVertices, boxNumIndices);
		SPtr<MeshData> boxMeshData = bs_shared_ptr_new<MeshData>(boxNumVertices, boxNumIndices, vertexDesc);
		AABox box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));

		ShapeMeshes3D::solidAABox(box, boxMeshData, 0, 0);
		SPtr<Mesh> boxMesh = Mesh::_createPtr(boxMeshData);

		UINT32 sphereNumVertices = 0;
		UINT32 sphereNumIndices = 0;
		ShapeMeshes3D::getNumElementsSphere(3, sphereNumVertices, sphereNumIndices);
		SPtr<MeshData> sphereMeshData = bs_shared_ptr_new<MeshData>(sphereNumVertices, sphereNumIndices, vertexDesc);

		ShapeMeshes3D::solidSphere(Sphere(Vector3::ZERO, 1.0f), sphereMeshData, 0, 0, 3);
		SPtr<Mesh> sphereMesh = Mesh::_createPtr(sphereMeshData);

		UINT32 coneNumVertices = 0;
		UINT32 coneNumIndices = 0;
		ShapeMeshes3D::getNumElementsCone(10, coneNumVertices, coneNumIndices);
		SPtr<MeshData> coneMeshData = bs_shared_ptr_new<MeshData>(coneNumVertices, coneNumIndices, vertexDesc);

		ShapeMeshes3D::solidCone(Vector3::ZERO, Vector3::UNIT_Y, 1.0f, 1.0f, Vector2::ONE, coneMeshData, 0, 0);
		SPtr<Mesh> coneMesh = Mesh::_createPtr(coneMeshData);

		UINT32 quadNumVertices = 8;
		UINT32 quadNumIndices = 12;
		ShapeMeshes3D::getNumElementsQuad(quadNumVertices, quadNumIndices);
		SPtr<MeshData> quadMeshData = bs_shared_ptr_new<MeshData>(quadNumVertices, quadNumIndices, vertexDesc);

		std::array<Vector3, 2> axes = {{ Vector3::UNIT_X, Vector3::UNIT_Y }};
		std::array<float, 2> sizes = {{ 1.0f, 1.0f }};
		Rect3 rect(Vector3::ZERO, axes, sizes);
		ShapeMeshes3D::solidQuad(rect, quadMeshData, 0, 0);
		SPtr<Mesh> quadMesh = Mesh::_createPtr(quadMeshData);

		UINT32 discNumVertices = 0;
		UINT32 discNumIndices = 0;
		ShapeMeshes3D::getNumElementsDisc(10, discNumVertices, discNumIndices);
		SPtr<MeshData> discMeshData = bs_shared_ptr_new<MeshData>(discNumVertices, discNumIndices, vertexDesc);

		ShapeMeshes3D::solidDisc(Vector3::ZERO, 1.0f, Vector3::UNIT_Y, discMeshData, 0, 0);
		SPtr<Mesh> discMesh = Mesh::_createPtr(discMeshData);

		// Save all meshes
		Path outputDir = mEngineMeshFolder;

		auto saveMesh = [&](const Path& path, const SPtr<Mesh>& mesh)
		{
			HResource meshResource;
			if (FileSystem::exists(path))
				meshResource = gResources().load(path);
			
			if (meshResource.isLoaded())
				gResources().update(meshResource, mesh);
			else
				meshResource = gResources()._createResourceHandle(mesh);

			gResources().save(meshResource, path, true);
			mResourceManifest->registerResource(meshResource.getUUID(), path);
		};

		Path boxPath = outputDir + MeshBoxFile;
		saveMesh(boxPath, boxMesh);

		Path spherePath = outputDir + MeshSphereFile;
		saveMesh(spherePath, sphereMesh);

		Path conePath = outputDir + MeshConeFile;
		saveMesh(conePath, coneMesh);

		Path quadPath = outputDir + MeshQuadFile;
		saveMesh(quadPath, quadMesh);

		Path discPath = outputDir + MeshDiscFile;
		saveMesh(discPath, discMesh);
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

		return gResources().load<Shader>(programPath);
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

	void BuiltinResourcesHelper::importAssets(const Path& inputFolder, const Path& outputFolder, const SPtr<ResourceManifest>& manifest)
	{
		if (!FileSystem::exists(inputFolder))
			return;

		UnorderedSet<Path> outputAssets;
		auto importResource = [&](const Path& filePath)
		{
			Vector<std::pair<Path, SPtr<ImportOptions>>> resourcesToSave;

			{
				Path relativePath = filePath.getRelative(inputFolder);
				Path relativeAssetPath = relativePath;
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
						// Check if the shader is used for a renderer material, in which case generate different variations
						// according to #defines (if any are specified).
						Vector<ShaderDefines> variations = RendererMaterialManager::_getVariations(relativePath);

						if(variations.size() == 0) // Not a renderer material or no variations, save normally
						{
							resourcesToSave.push_back(std::make_pair(relativeAssetPath, nullptr));
						}
						else // Renderer material, save a copy for each variation
						{
							// Note: Renderer materials are returned in an undefined order, meaning that renderer materials
							// will not properly persist references. But this should be okay since they are used only in low
							// level systems.

							UINT32 variationIdx = 0;
							for(auto& variation : variations)
							{
								SPtr<ShaderImportOptions> shaderImportOptions = 
									std::static_pointer_cast<ShaderImportOptions>(gImporter().createImportOptions(filePath));

								shaderImportOptions->getDefines() = variation.getAll();

								Path uniquePath = RendererMaterialManager::_getVariationPath(relativePath, variationIdx);
								uniquePath.setFilename(uniquePath.getWFilename() + L".asset");
								
								resourcesToSave.push_back(std::make_pair(uniquePath, shaderImportOptions));
								variationIdx++;
							}
						}
					}
					else
						resourcesToSave.push_back(std::make_pair(relativeAssetPath, nullptr));
				}
				else
					resourcesToSave.push_back(std::make_pair(relativeAssetPath, nullptr));
			}

			for(auto& entry : resourcesToSave)
			{
				Path outputPath = outputFolder + entry.first;;

				HResource resource;
				if (FileSystem::exists(outputPath))
					resource = gResources().load(outputPath);

				if (resource != nullptr)
					gImporter().reimport(resource, filePath, entry.second);
				else
					resource = Importer::instance().import(filePath, entry.second);

				if (resource != nullptr)
				{
					Resources::instance().save(resource, outputPath, true);
					manifest->registerResource(resource.getUUID(), outputPath);

					outputAssets.insert(outputPath);
				}
			}
			
			return true;
		};

		Vector<Path> obsoleteAssets;
		auto gatherObsolete = [&](const Path& filePath)
		{
			auto iterFind = outputAssets.find(filePath);
			if (iterFind == outputAssets.end())
				obsoleteAssets.push_back(filePath);

			return true;
		};

		FileSystem::iterate(inputFolder, importResource, nullptr, false);
		FileSystem::iterate(outputFolder, gatherObsolete, nullptr, false);

		for (auto& obsoleteAssetPath : obsoleteAssets)
			FileSystem::remove(obsoleteAssetPath);
	}

	void BuiltinResourcesHelper::importFont(const Path& inputFile, const WString& outputName, const Path& outputFolder,
		const Vector<UINT32>& fontSizes, bool antialiasing, const SPtr<ResourceManifest>& manifest)
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

		HFont font = Importer::instance().import<Font>(inputFile, fontImportOptions);

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

	void BuiltinResourcesHelper::generateSpriteTextures(const Path& input, const Path& output, 
		const SPtr<ResourceManifest>& manifest)
	{
		if (!FileSystem::exists(input))
			return;

		Vector<Path> filesToProcess;
		auto gather = [&](const Path& filePath)
		{
			filesToProcess.push_back(filePath);

			return true;
		};

		FileSystem::iterate(input, gather, nullptr, false);

		UnorderedSet<Path> outputAssets;
		for (auto& filePath : filesToProcess)
		{
			Path relativePath = filePath.getRelative(input);
			Path outputPath = output + relativePath;

			outputPath.setFilename(L"sprite_" + filePath.getWFilename());

			HTexture source = gResources().load<Texture>(filePath);

			if (source != nullptr)
			{
				HResource resource;
				if (FileSystem::exists(outputPath))
				{
					resource = gResources().load(outputPath);
					SPtr<SpriteTexture> spriteTex = SpriteTexture::_createPtr(source);

					gResources().update(resource, spriteTex);
					Resources::instance().save(resource, outputPath, true);
					manifest->registerResource(resource.getUUID(), outputPath);
				}
				else
				{
					HSpriteTexture spriteTex = SpriteTexture::create(source);
					Resources::instance().save(spriteTex, outputPath, true);
					manifest->registerResource(spriteTex.getUUID(), outputPath);
				}

				outputAssets.insert(outputPath);
			}
		}

		Vector<Path> obsoleteAssets;
		auto gatherObsolete = [&](const Path& filePath)
		{
			auto iterFind = outputAssets.find(filePath);
			if (iterFind == outputAssets.end())
				obsoleteAssets.push_back(filePath);

			return true;
		};

		FileSystem::iterate(output, gatherObsolete, nullptr, false);

		for (auto& obsoleteAssetPath : obsoleteAssets)
			FileSystem::remove(obsoleteAssetPath);
	}

	void BuiltinResourcesHelper::writeTimestamp(const Path& file)
	{
		SPtr<DataStream> fileStream = FileSystem::createAndOpenFile(file);

		time_t currentTime = std::time(nullptr);
		fileStream->write(&currentTime, sizeof(currentTime));
		fileStream->close();
	}

	bool BuiltinResourcesHelper::checkForModifications(const Path& folder, const Path& timeStampFile)
	{
		if (!FileSystem::exists(timeStampFile))
			return true;

		SPtr<DataStream> fileStream = FileSystem::openFile(timeStampFile);
		time_t lastUpdateTime = 0;
		fileStream->read(&lastUpdateTime, sizeof(lastUpdateTime));
		fileStream->close();

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

		FileSystem::iterate(folder, checkUpToDate, checkUpToDate);

		return !upToDate;
	}
}