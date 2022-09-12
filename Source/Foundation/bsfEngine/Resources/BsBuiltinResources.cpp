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
	const String BuiltinResources::ShaderSpriteImageFile = u8"SpriteImage.bsl";
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

		gResources().RegisterResourceManifest(mResourceManifest);

		// Load basic resources
		mShaderSpriteText = getShader(ShaderSpriteTextFile);
		mShaderSpriteImage = getShader(ShaderSpriteImageFile);
		mShaderSpriteLine = getShader(ShaderSpriteLineFile);
		mShaderDiffuse = getShader(ShaderDiffuseFile);
		mShaderTransparent = getShader(ShaderTransparentFile);
		mShaderParticlesUnlit = getShader(ShaderParticlesUnlitFile);
		mShaderParticlesLit = getShader(ShaderParticlesLitFile);
		mShaderParticlesLitOpaque = getShader(ShaderParticlesLitOpaqueFile);
		mShaderDecal = getShader(ShaderDecalFile);

		SPtr<PixelData> dummyPixelData = PixelData::create(2, 2, 1, PF_RGBA8);

		dummyPixelData->SetColorAt(Color::Red, 0, 0);
		dummyPixelData->SetColorAt(Color::Red, 0, 1);
		dummyPixelData->SetColorAt(Color::Red, 1, 0);
		dummyPixelData->SetColorAt(Color::Red, 1, 1);

		mDummyTexture = Texture::create(dummyPixelData);

		mWhiteSpriteTexture = getSkinTexture(WhiteTex);
		mDummySpriteTexture = SpriteTexture::create(mDummyTexture);

		mFont = gResources().load<Font>(mBuiltinDataFolder + (String(DEFAULT_FONT_NAME) + u8".asset"));
		mSkin = gResources().load<GUISkin>(mBuiltinDataFolder + (String(GUI_SKIN_FILE) + u8".json.asset"));
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

		mCursorArrow = cursorArrowTex->GetProperties().AllocBuffer(0, 0);
		cursorArrowTex->ReadData(mCursorArrow);

		mCursorArrowDrag = cursorArrowDragTex->GetProperties().AllocBuffer(0, 0);
		cursorArrowDragTex->ReadData(mCursorArrowDrag);

		mCursorArrowLeftRight = cursorArrowLeftRightTex->GetProperties().AllocBuffer(0, 0);
		cursorArrowLeftRightTex->ReadData(mCursorArrowLeftRight);

		mCursorIBeam = cursorIBeamTex->GetProperties().AllocBuffer(0, 0);
		cursorIBeamTex->ReadData(mCursorIBeam);

		mCursorDeny = cursorDenyTex->GetProperties().AllocBuffer(0, 0);
		cursorDenyTex->ReadData(mCursorDeny);

		mCursorWait = cursorWaitTex->GetProperties().AllocBuffer(0, 0);
		cursorWaitTex->ReadData(mCursorWait);

		mCursorSizeNESW = cursorSizeNESWTex->GetProperties().AllocBuffer(0, 0);
		cursorSizeNESWTex->ReadData(mCursorSizeNESW);

		mCursorSizeNS = cursorSizeNSTex->GetProperties().AllocBuffer(0, 0);
		cursorSizeNSTex->ReadData(mCursorSizeNS);

		mCursorSizeNWSE = cursorSizeNWSETex->GetProperties().AllocBuffer(0, 0);
		cursorSizeNWSETex->ReadData(mCursorSizeNWSE);

		mCursorSizeWE = cursorSizeWETex->GetProperties().AllocBuffer(0, 0);
		cursorSizeWETex->ReadData(mCursorSizeWE);

		/************************************************************************/
		/* 								ICON		                     		*/
		/************************************************************************/

		Path iconPath = mBuiltinDataFolder + ICON_FOLDER;
		iconPath.Append(String(IconTextureName) + u8".asset");

		HTexture iconTex = gResources().load<Texture>(iconPath);

		mFrameworkIcon = iconTex->GetProperties().AllocBuffer(0, 0);
		iconTex->ReadData(mFrameworkIcon);

		gCoreThread().Submit(true);
	}

	HSpriteTexture BuiltinResources::GetSkinTexture(const String& name) const
	{
		Path texturePath = mEngineSkinSpritesFolder;
		texturePath.Append(u8"sprite_" + name + u8".asset");

		return GResources().load<SpriteTexture>(texturePath);
	}

	HShader BuiltinResources::GetShader(const Path& path) const
	{
		Path programPath = mEngineShaderFolder;
		programPath.Append(path);
		programPath.SetExtension(programPath.getExtension() + ".asset");

		return GResources().load<Shader>(programPath);
	}

	HTexture BuiltinResources::GetCursorTexture(const String& name) const
	{
		Path cursorPath = mEngineCursorFolder;
		cursorPath.Append(name + u8".asset");

		return GResources().load<Texture>(cursorPath);
	}

	const PixelData& BuiltinResources::GetCursorArrow(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowHotspot;
		return *mCursorArrow.Get();
	}

	const PixelData& BuiltinResources::GetCursorArrowDrag(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowDragHotspot;
		return *mCursorArrowDrag.Get();
	}

	const PixelData& BuiltinResources::GetCursorWait(Vector2I& hotSpot)
	{
		hotSpot = CursorWaitHotspot;
		return *mCursorWait.Get();
	}

	const PixelData& BuiltinResources::GetCursorIBeam(Vector2I& hotSpot)
	{
		hotSpot = CursorIBeamHotspot;
		return *mCursorIBeam.Get();
	}

	const PixelData& BuiltinResources::GetCursorSizeNESW(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNESWHotspot;
		return *mCursorSizeNESW.Get();
	}

	const PixelData& BuiltinResources::GetCursorSizeNS(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNSHotspot;
		return *mCursorSizeNS.Get();
	}

	const PixelData& BuiltinResources::GetCursorSizeNWSE(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeNWSEHotspot;
		return *mCursorSizeNWSE.Get();
	}

	const PixelData& BuiltinResources::GetCursorSizeWE(Vector2I& hotSpot)
	{
		hotSpot = CursorSizeWEHotspot;
		return *mCursorSizeWE.Get();
	}

	const PixelData& BuiltinResources::GetCursorDeny(Vector2I& hotSpot)
	{
		hotSpot = CursorDenyHotspot;
		return *mCursorDeny.Get();
	}

	const PixelData& BuiltinResources::GetCursorMoveLeftRight(Vector2I& hotSpot)
	{
		hotSpot = CursorArrowLeftRightHotspot;
		return *mCursorArrowLeftRight.Get();
	}

	const PixelData& BuiltinResources::GetFrameworkIcon()
	{
		return *mFrameworkIcon.Get();
	}

	Path BuiltinResources::GetRawShaderFolder()
	{
		return Paths::GetDataPath() + "Raw/" + SHADER_FOLDER;
	}

	Path BuiltinResources::GetShaderIncludeFolder()
	{
		return Paths::GetDataPath() + SHADER_INCLUDE_FOLDER;
	}

	Path BuiltinResources::GetIconFolder()
	{
		return Paths::GetDataPath() + ICON_FOLDER;
	}

#if BS_IS_BANSHEE3D || defined BS_IS_ASSET_TOOL
	Path BuiltinResources::GetEditorShaderIncludeFolder()
	{
		return Paths::GetEditorDataPath() + SHADER_INCLUDE_FOLDER;
	}
#endif

	HMesh BuiltinResources::GetMesh(BuiltinMesh mesh) const
	{
		Path meshPath = mEngineMeshFolder;

		switch (mesh)
		{
		case BuiltinMesh::Box:
			meshPath.Append(MESH_BOX_FILE);
			break;
		case BuiltinMesh::Sphere:
			meshPath.Append(MESH_SPHERE_FILE);
			break;
		case BuiltinMesh::Cone:
			meshPath.Append(MESH_CONE_FILE);
			break;
		case BuiltinMesh::Cylinder:
			meshPath.Append(MESH_CYLINDER_FILE);
			break;
		case BuiltinMesh::Quad:
			meshPath.Append(MESH_QUAD_FILE);
			break;
		case BuiltinMesh::Disc:
			meshPath.Append(MESH_DISC_FILE);
			break;
		}

		return GResources().load<Mesh>(meshPath);
	}

	HShader BuiltinResources::GetBuiltinShader(BuiltinShader type) const
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

	HTexture BuiltinResources::GetTexture(BuiltinTexture type)
	{
		Path texturePath = Paths::getDataPath();
		texturePath.Append(TEXTURE_FOLDER);

		switch (type)
		{
		case BuiltinTexture::Black:
			texturePath.Append(TEXTURE_BLACK_FILE);
			break;
		case BuiltinTexture::White:
			texturePath.Append(TEXTURE_WHITE_FILE);
			break;
		case BuiltinTexture::Normal:
			texturePath.Append(TEXTURE_NORMAL_FILE);
			break;
		case BuiltinTexture::BokehFlare:
			texturePath.Append(u8"BokehHex.png.asset");
			break;
		}

		return GResources().load<Texture>(texturePath);
	}

	HMaterial BuiltinResources::CreateSpriteTextMaterial() const
	{
		return Material::Create(mShaderSpriteText);
	}

	HMaterial BuiltinResources::CreateSpriteImageMaterial() const
	{
		return Material::Create(mShaderSpriteImage);
	}

	HMaterial BuiltinResources::CreateSpriteLineMaterial() const
	{
		return Material::Create(mShaderSpriteLine);
	}

	BuiltinResources& GBuiltinResources()
	{
		return BuiltinResources::Instance();
	}
}
