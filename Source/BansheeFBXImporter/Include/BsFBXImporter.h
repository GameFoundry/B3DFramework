//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFBXPrerequisites.h"
#include "BsSpecificImporter.h"
#include "BsSubMesh.h"
#include "BsFBXImportData.h"

#define FBX_IMPORT_MAX_UV_LAYERS 2

namespace BansheeEngine
{
	/** @addtogroup FBX
	 *  @{
	 */

	/** Importer implementation that handles FBX/OBJ/DAE/3DS file import by using the FBX SDK. */
	class BS_FBX_EXPORT FBXImporter : public SpecificImporter
	{
	public:
		FBXImporter();
		virtual ~FBXImporter();

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const WString& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;

		/** @copydoc SpecificImporter::importAll */
		Vector<SubResourceRaw> importAll(const Path& filePath, SPtr<const ImportOptions> importOptions) override;

		/** @copydoc SpecificImporter::createImportOptions */
		SPtr<ImportOptions> createImportOptions() const override;
	private:
		/**
		 * Starts up FBX SDK. Must be called before any other operations. Outputs an FBX manager and FBX scene instances
		 * you should use in further operations. Returns false if the SDK wasn't started properly.
		 */
		bool startUpSdk(FbxScene*& scene);

		/**	Shuts down FBX SDK. Must be called after any other operations. */
		void shutDownSdk();

		/** 
		 * Reads the FBX file and outputs mesh data from the read file. Sub-mesh information will be output in @p subMeshes.
		 */
		SPtr<RendererMeshData> importMeshData(const Path& filePath, SPtr<const ImportOptions> importOptions, 
			Vector<SubMesh>& subMeshes, UnorderedMap<String, SPtr<AnimationCurves>>& animationClips);

		/**
		 * Loads the data from the file at the provided path into the provided FBX scene. Returns false if the file
		 * couldn't be loaded.
		 */
		bool loadFBXFile(FbxScene* scene, const Path& filePath);

		/**
		 * Parses an FBX scene. Find all meshes in the scene and returns mesh data object containing all vertices, indexes
		 * and other mesh information. Also outputs a sub-mesh array that allows you locate specific sub-meshes within the
		 * returned mesh data object. If requested animation and blend shape data is output as well.
		 */
		void parseScene(FbxScene* scene, const FBXImportOptions& options, FBXImportScene& outputScene);

		/**
		 * Parses an FBX mesh. Converts it from FBX SDK format into a mesh data object containing one or multiple sub-meshes.
		 */
		void parseMesh(FbxMesh* mesh, FBXImportNode* parentNode, const FBXImportOptions& options, FBXImportScene& outputScene);

		/**	Imports blend shapes for all the meshes that are part of the scene. */
		void importBlendShapes(FBXImportScene& scene, const FBXImportOptions& options);

		/**
		 * Parses a single FBX blend shape frame. Converts it from FBX SDK format into a shape data object containing
		 * position and tangent frame.
		 */
		void importBlendShapeFrame(FbxShape* shape, const FBXImportMesh& mesh, const FBXImportOptions& options, FBXBlendShapeFrame& outFrame);

		/**	Imports skinning information and bones for all meshes. */
		void importSkin(FBXImportScene& scene);

		/**	Imports skinning information and bones for the specified mesh. */
		void importSkin(FBXImportScene& scene, FbxSkin* skin, FBXImportMesh& mesh);

		/**	Imports all bone and blend shape animations from the FBX. */
		void importAnimations(FbxScene* scene, FBXImportOptions& importOptions, FBXImportScene& importScene);

		/**
		 * Imports all animations for the specified animation layer and outputs them in the provided clip. Child nodes will
		 * be iterated recursively.
		 */
		void importAnimations(FbxAnimLayer* layer, FbxNode* node, FBXImportOptions& importOptions, 
			FBXAnimationClip& clip, FBXImportScene& importScene);

		/**	Converts a single FBX animation curve into an engine curve format, resampling it if necessary. */
		void importCurve(FbxAnimCurve* fbxCurve, FBXImportOptions& importOptions, FBXAnimationCurve& curve, float start, float end);

		/** Converts FBX animation clips into engine-ready animation curve format. */
		void convertAnimations(const Vector<FBXAnimationClip>& clips, UnorderedMap<String, SPtr<AnimationCurves>>& output);

		/**	Converts a set of curves containing rotation in euler angles into a set of curves using	quaternion rotation. */
		void eulerToQuaternionCurves(FBXAnimationCurve(&eulerCurves)[3], FBXAnimationCurve(&quatCurves)[4]);

		/**
		 * Converts all the meshes from per-index attributes to per-vertex attributes.
		 *
		 * @note	
		 * This method will replace all meshes in the scene with new ones, and delete old ones so be sure not to keep any
		 * mesh references.
		 */
		void splitMeshVertices(FBXImportScene& scene);

		/**
		 * Traverses over all meshes in the scene and generates normals, tangents and bitangents if they're missing.
		 *
		 * @note	This assumes vertices have already been split and shouldn't be called on pre-split meshes.
		 */
		void generateMissingTangentSpace(FBXImportScene& scene, const FBXImportOptions& options);

		/**Converts the mesh data from the imported FBX scene into mesh data that can be used for initializing a mesh. */
		SPtr<RendererMeshData> generateMeshData(const FBXImportScene& scene, const FBXImportOptions& options, Vector<SubMesh>& subMeshes);

		/**	Creates an internal representation of an FBX node from an FbxNode object. */
		FBXImportNode* createImportNode(FBXImportScene& scene, FbxNode* fbxNode, FBXImportNode* parent);

	private:
		Vector<WString> mExtensions;
		FbxManager* mFBXManager;
	};

	/** @} */
}