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
#include "Material/BsShader.h"
#include "Material/BsMaterial.h"
#include "Reflection/BsRTTIType.h"
#include "FileSystem/BsFileSystem.h"
#include "CoreObject/BsRenderThread.h"
#include "GUI/StyleSheet/BsGUIStyleSheet.h"
#include "Material/BsShaderCompiler.h"
#include "Utility/BsShapeMeshes3D.h"
#include "Mesh/BsMesh.h"

using json = nlohmann::json;

using namespace bs;

constexpr const char* BuiltinResources::kIconTextureName;
constexpr const char* BuiltinResources::kMultiLineLabelStyle;

/************************************************************************/
/* 								GUI TEXTURES                      		*/
/************************************************************************/

const String BuiltinResources::kWhiteTex = u8"White.psd";

/************************************************************************/
/* 							CURSOR TEXTURES                      		*/
/************************************************************************/

const String BuiltinResources::kCursorArrowTex = u8"Arrow.psd";
const String BuiltinResources::kCursorArrowDragTex = u8"ArrowDrag.psd";
const String BuiltinResources::kCursorArrowLeftRightTex = u8"ArrowLeftRight.psd";
const String BuiltinResources::kCursorIBeamTex = u8"IBeam.psd";
const String BuiltinResources::kCursorDenyTex = u8"Deny.psd";
const String BuiltinResources::kCursorWaitTex = u8"Wait.psd";
const String BuiltinResources::kCursorSizeNeswTex = u8"SizeNESW.psd";
const String BuiltinResources::kCursorSizeNsTex = u8"SizeNS.psd";
const String BuiltinResources::kCursorSizeNwseTex = u8"SizeNWSE.psd";
const String BuiltinResources::kCursorSizeWeTex = u8"SizeWE.psd";

const Vector2I BuiltinResources::kCursorArrowHotspot = Vector2I(10, 8);
const Vector2I BuiltinResources::kCursorArrowDragHotspot = Vector2I(8, 4);
const Vector2I BuiltinResources::kCursorArrowLeftRightHotspot = Vector2I(13, 9);
const Vector2I BuiltinResources::kCursorIBeamHotspot = Vector2I(16, 15);
const Vector2I BuiltinResources::kCursorDenyHotspot = Vector2I(15, 15);
const Vector2I BuiltinResources::kCursorWaitHotspot = Vector2I(15, 15);
const Vector2I BuiltinResources::kCursorSizeNeswHotspot = Vector2I(16, 15);
const Vector2I BuiltinResources::kCursorSizeNsHotspot = Vector2I(16, 15);
const Vector2I BuiltinResources::kCursorSizeNwseHotspot = Vector2I(16, 15);
const Vector2I BuiltinResources::kCursorSizeWeHotspot = Vector2I(16, 15);

/************************************************************************/
/* 									SHADERS                      		*/
/************************************************************************/

const String BuiltinResources::kShaderSpriteTextFile = u8"SpriteText.bsl";
const String BuiltinResources::kShaderSpriteImageFile = u8"SpriteImage.bsl";
const String BuiltinResources::kShaderSpriteLineFile = u8"SpriteLine.bsl";

constexpr const char* kShaderDiffuseFile = u8"Diffuse.bsl";
constexpr const char* kShaderTransparentFile = u8"Transparent.bsl";
constexpr const char* kShaderParticlesUnlitFile = u8"ParticlesUnlit.bsl";
constexpr const char* kShaderParticlesLitFile = u8"ParticlesLit.bsl";
constexpr const char* kShaderParticlesLitOpaqueFile = u8"ParticlesLitOpaque.bsl";
constexpr const char* kShaderDecalFile = u8"Decal.bsl";

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

	GetRenderThread().PostCommand([]() { ct::BuiltinResources::ShutDown(); }, "Shutting down builtin resources");
}

void BuiltinResources::OnStartUp()
{
	// Set up paths
	mBuiltinRawDataFolder = Paths::GetDataPath() + u8"Raw/";

	mBuiltinDataFolder = Paths::GetDataPath();
	mEngineShaderFolder = mBuiltinDataFolder + kShaderFolder;

	GetPackageManager().LoadPackages(mBuiltinDataFolder, true, kVirtualPathPrefix);
	ShaderCompilers::Instance().RegisterSearchPath(GetRawShaderFolder());

	// Load basic resources
	mShaderSpriteText = GetShader(kShaderSpriteTextFile);
	mShaderSpriteImage = GetShader(kShaderSpriteImageFile);
	mShaderSpriteLine = GetShader(kShaderSpriteLineFile);
	mShaderDiffuse = GetShader(kShaderDiffuseFile);
	mShaderTransparent = GetShader(kShaderTransparentFile);
	mShaderParticlesUnlit = GetShader(kShaderParticlesUnlitFile);
	mShaderParticlesLit = GetShader(kShaderParticlesLitFile);
	mShaderParticlesLitOpaque = GetShader(kShaderParticlesLitOpaqueFile);
	mShaderDecal = GetShader(kShaderDecalFile);

	SPtr<PixelData> dummyPixelData = PixelData::Create(2, 2, 1, PF_RGBA8);

	dummyPixelData->SetColorAt(Color::kRed, 0, 0);
	dummyPixelData->SetColorAt(Color::kRed, 0, 1);
	dummyPixelData->SetColorAt(Color::kRed, 1, 0);
	dummyPixelData->SetColorAt(Color::kRed, 1, 1);

	mDummyTexture = Texture::Create(dummyPixelData);

	mWhiteSpriteTexture = GetSkinTexture(kWhiteTex);

	const Path& fontsFolderVirtualPath = Path::Combine(kVirtualPathPrefix, kFontsFolder);
	const Path& defaultFontVirtualPath = Path::Combine(fontsFolderVirtualPath, kDefaultFontName);

	mFont = GetResources().Load<Font>(defaultFontVirtualPath, ResourceLoadOptions(false));

	mDefaultGUIStyleSheet = GUIStyleSheet::Parse(mBuiltinRawDataFolder + "GUI.css");
	mDefaultGUIStyleSheetCascade = B3DMakeShared<GUIStyleSheetCascade>();
	mDefaultGUIStyleSheetCascade->RegisterStyleSheet(mDefaultGUIStyleSheet, GUIStyleSheet::kBuiltinImportance);

	const HTexture whiteTexture2D = GetTexture(BuiltinTexture::White);
	const HTexture blackTexture2D = GetTexture(BuiltinTexture::Black);
	const HTexture normalTexture = GetTexture(BuiltinTexture::Normal);
	const HTexture whiteTexture3D = GetTexture(BuiltinTexture::White3D);
	const HTexture blackTexture3D = GetTexture(BuiltinTexture::Black3D);

	auto fnInitializeBuiltinResourceRenderProxies = [whiteTexture2D = B3DGetRenderProxy(whiteTexture2D), blackTexture2D = B3DGetRenderProxy(blackTexture2D), normalTexture = B3DGetRenderProxy(normalTexture), whiteTexture3D = B3DGetRenderProxy(whiteTexture3D), blackTexture3D = B3DGetRenderProxy(blackTexture3D)]()

	{
		ct::BuiltinResources::StartUp();
		ct::BuiltinResources& renderThreadBuiltinResources = ct::BuiltinResources::Instance();
		renderThreadBuiltinResources.WhiteTexture2D = whiteTexture2D;
		renderThreadBuiltinResources.BlackTexture2D = blackTexture2D;
		renderThreadBuiltinResources.NormalTexture2D = normalTexture;
		renderThreadBuiltinResources.WhiteTexture3D = whiteTexture3D;
		renderThreadBuiltinResources.BlackTexture3D = blackTexture3D;
	};

	GetRenderThread().PostCommand(fnInitializeBuiltinResourceRenderProxies, "Initialize builtin resource render proxies");

	/************************************************************************/
	/* 								CURSOR		                     		*/
	/************************************************************************/

	HTexture cursorArrowTex = GetCursorTexture(kCursorArrowTex);
	HTexture cursorArrowDragTex = GetCursorTexture(kCursorArrowDragTex);
	HTexture cursorArrowLeftRightTex = GetCursorTexture(kCursorArrowLeftRightTex);
	HTexture cursorIBeamTex = GetCursorTexture(kCursorIBeamTex);
	HTexture cursorDenyTex = GetCursorTexture(kCursorDenyTex);
	HTexture cursorWaitTex = GetCursorTexture(kCursorWaitTex);
	HTexture cursorSizeNESWTex = GetCursorTexture(kCursorSizeNeswTex);
	HTexture cursorSizeNSTex = GetCursorTexture(kCursorSizeNsTex);
	HTexture cursorSizeNWSETex = GetCursorTexture(kCursorSizeNwseTex);
	HTexture cursorSizeWETex = GetCursorTexture(kCursorSizeWeTex);

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

	const Path& iconFolderVirtualPath = Path::Combine(kVirtualPathPrefix, kIconFolder);
	const Path& iconVirtualPath = Path::Combine(iconFolderVirtualPath, kIconTextureName);

	HTexture iconTex = GetResources().Load<Texture>(iconVirtualPath, ResourceLoadOptions(false));

	mFrameworkIcon = iconTex->GetProperties().AllocBuffer(0, 0);
	iconTex->ReadData(mFrameworkIcon);

	GetRenderThread().PostCommand([] {}, "Reading back cursor and icon pixels",true);
}

HSpriteTexture BuiltinResources::GetSkinTexture(const String& name) const
{
	Path texturePath = Path::Combine(Path::Combine(kVirtualPathPrefix, kSkinFolder), kSpriteFolder);
	texturePath.Append(u8"sprite_" + name);

	return GetResources().Load<SpriteTexture>(texturePath, ResourceLoadOptions(false));
}

HShader BuiltinResources::GetShader(const Path& path) const
{
	const Path fullShaderPath = GetRawShaderFolder() + path;
	return GetOrCompileShader(fullShaderPath);
}

HFont BuiltinResources::GetFont(const String& font) const
{
	const Path& fontFolderVirtualPath = Path::Combine(kVirtualPathPrefix, kFontsFolder);
	Path fontVirtualFilePath = Path::Combine(fontFolderVirtualPath, font);

	const PackageManager& packageManager = GetPackageManager();
	if(!packageManager.TryResolveVirtualResourcePath(fontVirtualFilePath).has_value())
	{
		fontVirtualFilePath.SetFilename(font + ".ttf");

		if(!packageManager.TryResolveVirtualResourcePath(fontVirtualFilePath).has_value())
			fontVirtualFilePath.SetFilename(font + ".otf");

		if(!packageManager.TryResolveVirtualResourcePath(fontVirtualFilePath).has_value())
		{
			B3D_LOG(Warning, GUI, "Cannot find font of the requested font : {0}. Using default font instead.", font);
			return GetDefaultFont();
		}
	}

	// TODO: This needs to perform a lookup in the project library. Likely need to enumerate all fonts from data packages on start-up, and register them in FontManager for lookup.
	// - Other alternative is to integrate ProjectLibrary into the framework, but in my mind that should remain editor only functionality. We can perhaps pull some generic
	// package manipulation in a helper library, for use in the framework.

	return GetResources().Load<Font>(fontVirtualFilePath, ResourceLoadOptions(false));
}

HShader BuiltinResources::GetOrCompileShader(const Path& path) const
{
#ifndef BS_IS_ASSET_TOOL
	auto found = mCompiledShaders.find(path);
	if(found != mCompiledShaders.end())
		return found->second;

	HShader shader;
	if(shader == nullptr)
	{
		static const String kBuiltinShaderCachePrefix = "BuiltinShaders/";
		const SPtr<Shader> shaderShared = ShaderCompilers::Instance().GetOrCompileShader<false>(path, kBuiltinShaderCachePrefix, {});

		if(shaderShared != nullptr)
		{
			shader = B3DStaticResourceCast<Shader>(GetResources().CreateResourceHandle(shaderShared));
		}
	}

	mCompiledShaders[path] = shader;
	return shader;
#endif

	return HShader();
}

HTexture BuiltinResources::GetCursorTexture(const String& name) const
{
	const Path& cursorFolderVirtualPath = Path::Combine(kVirtualPathPrefix, kCursorFolder);
	const Path& cursorVirtualPath = Path::Combine(cursorFolderVirtualPath, name);

	return GetResources().Load<Texture>(cursorVirtualPath, ResourceLoadOptions(false));
}

const PixelData& BuiltinResources::GetCursorArrow(Vector2I& hotSpot)
{
	hotSpot = kCursorArrowHotspot;
	return *mCursorArrow.get();
}

const PixelData& BuiltinResources::GetCursorArrowDrag(Vector2I& hotSpot)
{
	hotSpot = kCursorArrowDragHotspot;
	return *mCursorArrowDrag.get();
}

const PixelData& BuiltinResources::GetCursorWait(Vector2I& hotSpot)
{
	hotSpot = kCursorWaitHotspot;
	return *mCursorWait.get();
}

const PixelData& BuiltinResources::GetCursorIBeam(Vector2I& hotSpot)
{
	hotSpot = kCursorIBeamHotspot;
	return *mCursorIBeam.get();
}

const PixelData& BuiltinResources::GetCursorSizeNesw(Vector2I& hotSpot)
{
	hotSpot = kCursorSizeNeswHotspot;
	return *mCursorSizeNESW.get();
}

const PixelData& BuiltinResources::GetCursorSizeNs(Vector2I& hotSpot)
{
	hotSpot = kCursorSizeNsHotspot;
	return *mCursorSizeNS.get();
}

const PixelData& BuiltinResources::GetCursorSizeNwse(Vector2I& hotSpot)
{
	hotSpot = kCursorSizeNwseHotspot;
	return *mCursorSizeNWSE.get();
}

const PixelData& BuiltinResources::GetCursorSizeWe(Vector2I& hotSpot)
{
	hotSpot = kCursorSizeWeHotspot;
	return *mCursorSizeWE.get();
}

const PixelData& BuiltinResources::GetCursorDeny(Vector2I& hotSpot)
{
	hotSpot = kCursorDenyHotspot;
	return *mCursorDeny.get();
}

const PixelData& BuiltinResources::GetCursorMoveLeftRight(Vector2I& hotSpot)
{
	hotSpot = kCursorArrowLeftRightHotspot;
	return *mCursorArrowLeftRight.get();
}

const PixelData& BuiltinResources::GetFrameworkIcon()
{
	return *mFrameworkIcon.get();
}

Path BuiltinResources::GetRawShaderFolder()
{
	return Paths::GetDataPath() + "Raw/" + kShaderFolder;
}

Path BuiltinResources::GetUnitTestDataFolder()
{
	return Paths::GetDataPath() + "Raw/" + kUnitTestDataFolder;
}

Path BuiltinResources::GetRawShaderIncludeFolder()
{
	return Paths::GetDataPath() + "Raw/" + kShaderIncludeFolder;
}

Path BuiltinResources::GetShaderIncludeFolder()
{
	return Paths::GetDataPath() + kShaderIncludeFolder;
}

Path BuiltinResources::GetIconFolder()
{
	return Paths::GetDataPath() + kIconFolder;
}

#if B3D_IS_ENGINE || defined BS_IS_ASSET_TOOL
Path BuiltinResources::GetEditorShaderIncludeFolder()
{
	return Paths::GetEditorDataPath() + kShaderIncludeFolder;
}

Path BuiltinResources::GetEditorRawShaderIncludeFolder()
{
	return Paths::GetEditorDataPath() + "Raw/" + kShaderIncludeFolder;
}
#endif

HMesh BuiltinResources::GetMesh(BuiltinMesh mesh) const
{
	Path meshVirtualPath = Path::Combine(kVirtualPathPrefix, kMeshFolder);

	switch(mesh)
	{
	case BuiltinMesh::Box:
		meshVirtualPath.Append(kMeshBoxFile);
		break;
	case BuiltinMesh::Sphere:
		meshVirtualPath.Append(kMeshSphereFile);
		break;
	case BuiltinMesh::Cone:
		meshVirtualPath.Append(kMeshConeFile);
		break;
	case BuiltinMesh::Cylinder:
		meshVirtualPath.Append(kMeshCylinderFile);
		break;
	case BuiltinMesh::Quad:
		meshVirtualPath.Append(kMeshQuadFile);
		break;
	case BuiltinMesh::Disc:
		meshVirtualPath.Append(kMeshDiscFile);
		break;
	}

	return GetResources().Load<Mesh>(meshVirtualPath, ResourceLoadOptions(false));
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
	Path textureVirtualPath = Path::Combine(kVirtualPathPrefix, kTextureFolder);

	switch(type)
	{
	case BuiltinTexture::Black:
		textureVirtualPath.Append(kTextureBlackFile);
		break;
	case BuiltinTexture::White:
		textureVirtualPath.Append(kTextureWhiteFile);
		break;
	case BuiltinTexture::Normal:
		textureVirtualPath.Append(kTextureNormalFile);
		break;
	case BuiltinTexture::BokehFlare:
		textureVirtualPath.Append(u8"BokehHex.png");
		break;
	case BuiltinTexture::Black3D:
		textureVirtualPath.Append(kTextureBlack3DFile);
		break;
	case BuiltinTexture::White3D:
		textureVirtualPath.Append(kTextureWhite3DFile);
		break;
	}

	return GetResources().Load<Texture>(textureVirtualPath, ResourceLoadOptions(false));
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

namespace bs
{
BuiltinResources& GetBuiltinResources()
{
	return BuiltinResources::Instance();
}
} // namespace bs
