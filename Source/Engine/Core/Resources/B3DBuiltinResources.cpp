//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DBuiltinResources.h"
#include "GUI/B3DGUILabel.h"
#include "Image/B3DSpriteTexture.h"
#include "Text/B3DFont.h"
#include "Image/B3DTexture.h"
#include "Importer/B3DImporter.h"
#include "Resources/B3DResources.h"
#include "Resources/B3DBuiltinResourcesHelper.h"
#include "Material/B3DShader.h"
#include "Material/B3DMaterial.h"
#include "Reflection/B3DRTTIType.h"
#include "FileSystem/B3DFileSystem.h"
#include "CoreObject/B3DRenderThread.h"
#include "GUI/StyleSheet/B3DGUIStyleSheet.h"
#include "Material/B3DShaderCompiler.h"
#include "Utility/B3DShapeMeshes3D.h"
#include "Mesh/B3DMesh.h"

using json = nlohmann::json;

using namespace b3d;

constexpr const char* BuiltinResources::kIconTextureName;
constexpr const char* BuiltinResources::kMultiLineLabelStyle;

/************************************************************************/
/* 								GUI TEXTURES                      		*/
/************************************************************************/

const String BuiltinResources::kWhiteTex = "White.psd";

/************************************************************************/
/* 							CURSOR TEXTURES                      		*/
/************************************************************************/

const String BuiltinResources::kCursorArrowTex = "Arrow.psd";
const String BuiltinResources::kCursorArrowDragTex = "ArrowDrag.psd";
const String BuiltinResources::kCursorArrowLeftRightTex = "ArrowLeftRight.psd";
const String BuiltinResources::kCursorIBeamTex = "IBeam.psd";
const String BuiltinResources::kCursorDenyTex = "Deny.psd";
const String BuiltinResources::kCursorWaitTex = "Wait.psd";
const String BuiltinResources::kCursorSizeNeswTex = "SizeNESW.psd";
const String BuiltinResources::kCursorSizeNsTex = "SizeNS.psd";
const String BuiltinResources::kCursorSizeNwseTex = "SizeNWSE.psd";
const String BuiltinResources::kCursorSizeWeTex = "SizeWE.psd";

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

const String BuiltinResources::kShaderSpriteTextFile = "SpriteText.bsl";
const String BuiltinResources::kShaderSpriteImageFile = "SpriteImage.bsl";
const String BuiltinResources::kShaderSpriteLineFile = "SpriteLine.bsl";

constexpr const char* kShaderDiffuseFile = "Diffuse.bsl";
constexpr const char* kShaderTransparentFile = "Transparent.bsl";
constexpr const char* kShaderParticlesUnlitFile = "ParticlesUnlit.bsl";
constexpr const char* kShaderParticlesLitFile = "ParticlesLit.bsl";
constexpr const char* kShaderParticlesLitOpaqueFile = "ParticlesLitOpaque.bsl";
constexpr const char* kShaderDecalFile = "Decal.bsl";

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

	GetRenderThread().PostCommand([]() { render::BuiltinResources::ShutDown(); }, "Shutting down builtin resources");
}

void BuiltinResources::OnStartUp()
{
	// Set up paths
	mBuiltinRawDataFolder = Paths::GetDataPath() + "Raw/";
	mBuiltinDataFolder = Paths::GetDataPath();

	GetPackageManager().LoadPackages(mBuiltinDataFolder, true, kVirtualPathPrefix);
	ShaderCompilers::Instance().RegisterSearchPath(GetShaderFolder());

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

	TextureCreateInformation dummyTextureCreateInformation = TextureCreateInformation::CreateFromPixelData(dummyPixelData);
	dummyTextureCreateInformation.Name = "DummyTexture";

	mDummyTexture = Texture::Create(dummyTextureCreateInformation);

	mWhiteSpriteTexture = GetSkinTexture(kWhiteTex);

	const Path& fontsFolderVirtualPath = Path::Combine(kVirtualPathPrefix, kFontsFolder);
	const Path& defaultFontVirtualPath = Path::Combine(fontsFolderVirtualPath, kDefaultFontName);

	mFont = GetResources().Load<Font>(defaultFontVirtualPath, ResourceLoadOptions(false));

	mDefaultGUIStyleSheet = GUIStyleSheet::Parse(mBuiltinDataFolder + "GUI.css");
	mDefaultGUIStyleSheetCascade = B3DMakeShared<GUIStyleSheetCascade>();
	mDefaultGUIStyleSheetCascade->RegisterStyleSheet(mDefaultGUIStyleSheet, GUIStyleSheet::kBuiltinImportance);

	const HTexture whiteTexture2D = GetTexture(BuiltinTexture::White);
	const HTexture blackTexture2D = GetTexture(BuiltinTexture::Black);
	const HTexture normalTexture = GetTexture(BuiltinTexture::Normal);
	const HTexture whiteTexture3D = GetTexture(BuiltinTexture::White3D);
	const HTexture blackTexture3D = GetTexture(BuiltinTexture::Black3D);

	auto fnInitializeBuiltinResourceRenderProxies = [whiteTexture2D = B3DGetRenderProxy(whiteTexture2D), blackTexture2D = B3DGetRenderProxy(blackTexture2D), normalTexture = B3DGetRenderProxy(normalTexture), whiteTexture3D = B3DGetRenderProxy(whiteTexture3D), blackTexture3D = B3DGetRenderProxy(blackTexture3D)]()

	{
		render::BuiltinResources::StartUp();
		render::BuiltinResources& renderThreadBuiltinResources = render::BuiltinResources::Instance();
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
	texturePath.Append("sprite_" + name);

	return GetResources().Load<SpriteTexture>(texturePath, ResourceLoadOptions(false));
}

HShader BuiltinResources::GetShader(const Path& path) const
{
	const Path fullShaderPath = GetShaderFolder() + path;
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
			B3D_LOG(Warning, LogGUI, "Cannot find the requested font: {0}. Using default font instead.", font);
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

Path BuiltinResources::GetShaderFolder()
{
	return Paths::GetDataPath() + kShaderFolder;
}

Path BuiltinResources::GetUnitTestDataFolder()
{
	return Paths::GetDataPath() + "Raw/" + kUnitTestDataFolder;
}

Path BuiltinResources::GetShaderIncludeFolder()
{
	return Paths::GetDataPath() + kShaderIncludeFolder;
}

Path BuiltinResources::GetIconFolder()
{
	return Paths::GetDataPath() + kIconFolder;
}

#if B3D_WITH_EDITOR
Path BuiltinResources::GetEditorShaderIncludeFolder()
{
	return Paths::GetEditorDataPath() + kShaderIncludeFolder;
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
		textureVirtualPath.Append("BokehHex.png");
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

namespace b3d
{
BuiltinResources& GetBuiltinResources()
{
	return BuiltinResources::Instance();
}
} // namespace b3d
