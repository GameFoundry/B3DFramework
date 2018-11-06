//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsBuiltinResources.h"
#include "GUI/BsGUILabel.h"
#include "Image/BsSpriteTexture.h"
#include "Text/BsFont.h"
#include "Image/BsTexture.h"
#include "Importer/BsImporter.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResourcesHelper.h"
#include "Resources/BsResourceManifest.h"
#include "Material/BsShader.h"
#include "Material/BsMaterial.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsFileSystem.h"
#include "CoreThread/BsCoreThread.h"
#include "Utility/BsShapeMeshes3D.h"
#include "Mesh/BsMesh.h"

using json = nlohmann::json;

namespace bs
{
	constexpr const char* BuiltinResources::IconTextureName;
	constexpr const char* BuiltinResources::MultiLineLabelStyle;

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

	constexpr const char* ShaderDiffuseFile = u8"Diffuse.bsl";
	constexpr const char* ShaderTransparentFile = u8"Transparent.bsl";
	constexpr const char* ShaderParticlesUnlitFile = u8"ParticlesUnlit.bsl";
	constexpr const char* ShaderParticlesLitFile = u8"ParticlesLit.bsl";
	constexpr const char* ShaderParticlesLitOpaqueFile = u8"ParticlesLitOpaque.bsl";
	constexpr const char* ShaderDecalFile = u8"Decal.bsl";

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

		// Load manifest
		if (FileSystem::exists(ResourceManifestPath))
			mResourceManifest = ResourceManifest::load(ResourceManifestPath, mBuiltinDataFolder);

		if (mResourceManifest == nullptr)
			mResourceManifest = ResourceManifest::create("BuiltinResources");

		gResources().registerResourceManifest(mResourceManifest);

		// Load basic resources
		mShaderSpriteText = getShader(ShaderSpriteTextFile);
		mShaderSpriteImage = getShader(ShaderSpriteImageAlphaFile);
		mShaderSpriteNonAlphaImage = getShader(ShaderSpriteImageNoAlphaFile);
		mShaderSpriteLine = getShader(ShaderSpriteLineFile);
		mShaderDiffuse = getShader(ShaderDiffuseFile);
		mShaderTransparent = getShader(ShaderTransparentFile);
		mShaderParticlesUnlit = getShader(ShaderParticlesUnlitFile);
		mShaderParticlesLit = getShader(ShaderParticlesLitFile);
		mShaderParticlesLitOpaque = getShader(ShaderParticlesLitOpaqueFile);
		mShaderDecal = getShader(ShaderDecalFile);

		SPtr<PixelData> dummyPixelData = PixelData::create(2, 2, 1, PF_RGBA8);

		dummyPixelData->setColorAt(Color::Red, 0, 0);
		dummyPixelData->setColorAt(Color::Red, 0, 1);
		dummyPixelData->setColorAt(Color::Red, 1, 0);
		dummyPixelData->setColorAt(Color::Red, 1, 1);

		mDummyTexture = Texture::create(dummyPixelData);

		mWhiteSpriteTexture = getSkinTexture(WhiteTex);
		mDummySpriteTexture = SpriteTexture::create(mDummyTexture);

		mFont = gResources().load<Font>(mBuiltinDataFolder + (String(DEFAULT_FONT_NAME) + u8".asset"));
		mSkin = gResources().load<GUISkin>(mBuiltinDataFolder + (String(GUI_SKIN_FILE) + u8".asset"));
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
			meshPath.append(MESH_BOX_FILE);
			break;
		case BuiltinMesh::Sphere:
			meshPath.append(MESH_SPHERE_FILE);
			break;
		case BuiltinMesh::Cone:
			meshPath.append(MESH_CONE_FILE);
			break;
		case BuiltinMesh::Cylinder:
			meshPath.append(MESH_CYLINDER_FILE);
			break;
		case BuiltinMesh::Quad:
			meshPath.append(MESH_QUAD_FILE);
			break;
		case BuiltinMesh::Disc:
			meshPath.append(MESH_DISC_FILE);
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
		case BuiltinShader::ParticlesLit:
			return mShaderParticlesLit;
		case BuiltinShader::ParticlesLitOpaque:
			return mShaderParticlesLitOpaque;
		case BuiltinShader::Decal:
			return mShaderDecal;
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
			texturePath.append(TEXTURE_BLACK_FILE);
			break;
		case BuiltinTexture::White:
			texturePath.append(TEXTURE_WHITE_FILE);
			break;
		case BuiltinTexture::Normal:
			texturePath.append(TEXTURE_NORMAL_FILE);
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