#include "BsApplication.h"
#include "BsEngineConfig.h"
#include "Resources/BsResources.h"
#include "Resources/BsBuiltinResources.h"
#include "BsBuiltinResourcesHelper.h"
#include "FileSystem/BsFileSystem.h"
#include "FileSystem/BsPath.h"
#include "FileSystem/BsDataStream.h"
#include "Resources/BsResourceManifest.h"
#include "ThirdParty/json.hpp"
#include "Utility/BsShapeMeshes3D.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsRendererMeshData.h"
#include "Image/BsTexture.h"
#include "RenderAPI/BsVertexDataDesc.h"

namespace bs
{
	static constexpr const char* TIMESTAMP_NAME = u8"Timestamp.asset";
	static constexpr const char* MANIFEST_NAME = u8"ResourceManifest.asset";
	static constexpr const char* DEPENDENCIES_JSON_NAME = u8"ShaderDependencies.json";
	static constexpr const char* DATA_LIST_JSON = u8"DataList.json";
	static constexpr const char* GUI_SKIN_JSON = u8"GUISkin.json";

	static Path sInputFolder;
	static Path sOutputFolder;
	static Path sManifestPath;
	static SPtr<ResourceManifest> sManifest;

	void processAssets(bool, time_t);
}

int main(int argc, char * argv[]) 
{
	using namespace bs;

	if(argc < 3)
		return 2;

	START_UP_DESC desc;
	desc.renderAPI = BS_RENDER_API_MODULE;
	desc.renderer = BS_RENDERER_MODULE;
	desc.audio = BS_AUDIO_MODULE;
	desc.physics = BS_PHYSICS_MODULE;
	desc.scripting = false;

	desc.importers.push_back("bsfFreeImgImporter");
	desc.importers.push_back("bsfFBXImporter");
	desc.importers.push_back("bsfFontImporter");
	desc.importers.push_back("bsfSL");

	desc.primaryWindowDesc.videoMode = VideoMode (64, 64);
	desc.primaryWindowDesc.fullscreen = false;
	desc.primaryWindowDesc.title = "bsf importer";
	desc.primaryWindowDesc.hidden = true;

	Application::startUp(desc);

	sInputFolder = argv[1];
	sOutputFolder = argv[2];
	sManifestPath = sOutputFolder + MANIFEST_NAME;

	if (FileSystem::exists(sInputFolder))
	{
		time_t lastUpdateTime;
		UINT32 modifications = BuiltinResourcesHelper::checkForModifications(
			sInputFolder,
			sOutputFolder + TIMESTAMP_NAME,
			lastUpdateTime);

		// Check if manifest needs to be rebuilt
		if (modifications == 0 && !FileSystem::exists(sManifestPath))
			modifications = 1;

		if (modifications > 0)
		{
			const bool fullReimport = modifications == 2;

			sManifest = ResourceManifest::create("BuiltinResources");
			gResources().registerResourceManifest(sManifest);

			processAssets(fullReimport, lastUpdateTime);
			BuiltinResourcesHelper::writeTimestamp(sOutputFolder + TIMESTAMP_NAME);

			ResourceManifest::save(sManifest, sManifestPath, sOutputFolder);

			Application::shutDown();
			return 1;
		}
	}

	Application::shutDown();
	return 0;
}

namespace bs
{
	void generateTextures()
	{
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
		Path outputDir = sOutputFolder + BuiltinResources::TEXTURE_FOLDER;

		auto saveTexture = [&](const Path& path, const SPtr<Texture>& texture, const String& uuid)
		{
			HResource textureResource = gResources()._createResourceHandle(texture, UUID(uuid));

			gResources().save(textureResource, path, true);
			sManifest->registerResource(textureResource.getUUID(), path);
		};

		Path whitePath = outputDir + BuiltinResources::TEXTURE_WHITE_FILE;
		saveTexture(whitePath, whiteTexture, "1f7d0e3f-d81b-42ee-9d31-cb6c6fc55824");

		Path blackPath = outputDir + BuiltinResources::TEXTURE_BLACK_FILE;
		saveTexture(blackPath, blackTexture, "149a5c05-9570-4915-9dbd-69acf88b865b");

		Path normalPath = outputDir + BuiltinResources::TEXTURE_NORMAL_FILE;
		saveTexture(normalPath, normalTexture, "afb29163-1ef0-4440-9cfb-c1ebb3b3d452");
	}

	void generateMeshes()
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
		const Path outputDir = sOutputFolder + BuiltinResources::MESH_FOLDER;

		auto saveMesh = [&](const Path& path, const SPtr<Mesh>& mesh, const String& uuid)
		{
			HResource meshResource = gResources()._createResourceHandle(mesh, UUID(uuid));

			gResources().save(meshResource, path, true);
			sManifest->registerResource(meshResource.getUUID(), path);
		};

		Path boxPath = outputDir + BuiltinResources::MESH_BOX_FILE;
		saveMesh(boxPath, boxMesh, "bc1d20ca-7fe6-489b-8b5c-dbf798badc95");

		Path spherePath = outputDir + BuiltinResources::MESH_SPHERE_FILE;
		saveMesh(spherePath, sphereMesh, "040642f3-04d6-419e-9dba-f7824161c205");

		Path conePath = outputDir + BuiltinResources::MESH_CONE_FILE;
		saveMesh(conePath, coneMesh, "b8cf6db5-1736-47ac-852f-82ecd88b4d46");

		Path cylinderPath = outputDir + BuiltinResources::MESH_CYLINDER_FILE;
		saveMesh(cylinderPath, cylinderMesh, "e6b2b797-4e72-7e49-61ba-4e7275bd561d");

		Path quadPath = outputDir + BuiltinResources::MESH_QUAD_FILE;
		saveMesh(quadPath, quadMesh, "06592bf3-f82a-472e-a034-26a98225fbe1");

		Path discPath = outputDir + BuiltinResources::MESH_DISC_FILE;
		saveMesh(discPath, discMesh, "6f496313-344a-495c-83e8-152e3053c52d");
	}

	SPtr<GUISkin> generateGUISkin()
	{
		using nlohmann::json;

		const Path skinFolder = sOutputFolder + BuiltinResources::SKIN_FOLDER + BuiltinResources::SPRITE_FOLDER;
		BuiltinResourceGUIElementStyleLoader loader(sOutputFolder, skinFolder);

		Path guiSkinPath = sInputFolder + GUI_SKIN_JSON;
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

	void processAssets(bool forceImport, time_t lastUpdateTime)
	{
		using nlohmann::json;

		// Hidden dependency: Textures need to be generated before shaders as they may use the default textures
		generateTextures();

		const Path dataListsFilePath = sInputFolder + DATA_LIST_JSON;
		SPtr<DataStream> dataListStream = FileSystem::openFile(dataListsFilePath);
		json dataListJSON = json::parse(dataListStream->getAsString().c_str());

		json skinJSON = dataListJSON["Skin"];
		json cursorsJSON = dataListJSON["Cursors"];
		json iconsJSON = dataListJSON["Icons"];
		json includesJSON = dataListJSON["Includes"];
		json shadersJSON = dataListJSON["Shaders"];

		const Path rawSkinFolder = sInputFolder + BuiltinResources::SKIN_FOLDER;
		const Path rawCursorFolder = sInputFolder + BuiltinResources::CURSOR_FOLDER;
		const Path rawIconFolder = sInputFolder + BuiltinResources::ICON_FOLDER;
		const Path rawShaderFolder = sInputFolder + BuiltinResources::SHADER_FOLDER;
		const Path rawShaderIncludeFolder = sInputFolder + BuiltinResources::SHADER_INCLUDE_FOLDER;

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

		const Path skinFolder = sOutputFolder + BuiltinResources::SKIN_FOLDER;
		const Path cursorFolder = sOutputFolder + BuiltinResources::CURSOR_FOLDER;
		const Path iconFolder = sOutputFolder + BuiltinResources::ICON_FOLDER;
		const Path shaderFolder = sOutputFolder + BuiltinResources::SHADER_FOLDER;
		const Path shaderIncludeFolder = sOutputFolder + BuiltinResources::SHADER_INCLUDE_FOLDER;
		const Path shaderDependenciesFile = sInputFolder + DEPENDENCIES_JSON_NAME;

		// If forcing import, clear all data folders since everything will be recreated anyway
		if(forceImport)
		{
			FileSystem::remove(cursorFolder);
			FileSystem::remove(iconFolder);
			FileSystem::remove(shaderIncludeFolder);
			FileSystem::remove(shaderFolder);
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
				cursorFolder,
				cursorsJSON,
				sManifest,
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
				cursorFolder,
				sManifest,
				BuiltinResourcesHelper::AssetType::Normal);
		}

		// Import icons
		{
			BuiltinResourcesHelper::updateManifest(
				iconFolder,
				iconsJSON,
				sManifest,
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
				sManifest,
				BuiltinResourcesHelper::AssetType::Normal);
		}

		// Import shaders
		{
			BuiltinResourcesHelper::updateManifest(
				shaderIncludeFolder,
				includesJSON,
				sManifest,
				BuiltinResourcesHelper::AssetType::Normal);

			BuiltinResourcesHelper::updateManifest(
				shaderFolder,
				shadersJSON,
				sManifest,
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
				sManifest,
				BuiltinResourcesHelper::AssetType::Normal,
				nullptr,
				true);


			BuiltinResourcesHelper::importAssets(
				shadersJSON,
				shaderImportFlags,
				rawShaderFolder,
				shaderFolder,
				sManifest,
				BuiltinResourcesHelper::AssetType::Normal,
				&shaderDependenciesJSON,
				true);
		}

		// Import GUI sprites
		{
			BuiltinResourcesHelper::updateManifest(
				skinFolder,
				skinJSON,
				sManifest,
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
				sManifest,
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
		const Path fontSourcePath = sInputFolder + BuiltinResources::DEFAULT_FONT_NAME;

		BuiltinResourcesHelper::importFont(
			fontSourcePath,
			BuiltinResources::DEFAULT_FONT_NAME,
			sOutputFolder,
			{ BuiltinResources::DEFAULT_FONT_SIZE },
			false,
			UUID("c9f08cab-f9c9-47c4-96e0-1066a8d4455b"),
			sManifest);

		// Generate & save GUI skin
		{
			const SPtr<GUISkin> skin = generateGUISkin();
			const Path outputPath = sOutputFolder + (String(BuiltinResources::GUI_SKIN_FILE) + u8".asset");

			HResource skinResource = gResources()._createResourceHandle(skin, UUID("c1bf9a9d-4355-4841-a538-25e67730ec4b"));

			gResources().save(skinResource, outputPath, true);
			sManifest->registerResource(skinResource.getUUID(), outputPath);
		}

		// Generate & save meshes
		generateMeshes();
	}
}