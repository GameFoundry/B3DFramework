//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsFBXPrerequisites.h"
#include "Importer/BsSpecificImporter.h"
#include "RenderAPI/BsSubMesh.h"
#include "BsFBXImportData.h"

#define FBX_IMPORT_MAX_UV_LAYERS 2

namespace bs
{
	/** @addtogroup FBX
	 *  @{
	 */

	struct AnimationSplitInfo;
	class MorphShapes;

	/** Importer implementation that handles FBX/OBJ/DAE/3DS file import by using the FBX SDK. */
	class FBXImporter : public SpecificImporter
	{
	public:
		FBXImporter();
		virtual ~FBXImporter() = default;

		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const String& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::getAsyncMode */
		ImporterAsyncMode getAsyncMode() const override { return ImporterAsyncMode::Single; }

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
			Vector<SubMesh>& subMeshes, Vector<FBXAnimationClipData>& animationClips, SPtr<Skeleton>& skeleton, 
			SPtr<MorphShapes>& morphShapes);

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
		void importSkin(FBXImportScene& scene, const FBXImportOptions& options);

		/**	Imports skinning information and bones for the specified mesh. */
		void importSkin(FBXImportScene& scene, FbxSkin* skin, FBXImportMesh& mesh, const FBXImportOptions& options);

		/**	Imports all bone and blend shape animations from the FBX. */
		void importAnimations(FbxScene* scene, FBXImportOptions& importOptions, FBXImportScene& importScene);

		/**
		 * Imports all animations for the specified animation layer and outputs them in the provided clip. Child nodes will
		 * be iterated recursively.
		 */
		void importAnimations(FbxAnimLayer* layer, FbxNode* node, FBXImportOptions& importOptions, 
			FBXAnimationClip& clip, FBXImportScene& importScene);

		/** Bakes all FBX node transforms into standard translation-rotation-scale transform components. */
		void bakeTransforms(FbxScene* scene);

		/**	Converts a single FBX animation curve into an engine curve format, resampling it if necessary. */
		template<class T, int C>
		TAnimationCurve<T> importCurve(FbxAnimCurve*(&fbxCurve)[C], float(&defaultValues)[C], 
			FBXImportOptions& importOptions, float clipStart, float clipEnd);

		/** Converts FBX animation clips into engine-ready animation curve format. */
		void convertAnimations(const Vector<FBXAnimationClip>& clips, const Vector<AnimationSplitInfo>& splits, 
			const SPtr<Skeleton>& skeleton, bool importRootMotion, Vector<FBXAnimationClipData>& output);

		/** 
		 * Removes identical sequential keyframes for the provided set of curves. The keyframe must be identical over all
		 * the curves in order for it to be removed.
		 */
		TAnimationCurve<Vector3> reduceKeyframes(TAnimationCurve<Vector3>& curve);

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

		/** Converts the mesh data from the imported FBX scene into mesh data that can be used for initializing a mesh. */
		SPtr<RendererMeshData> generateMeshData(const FBXImportScene& scene, const FBXImportOptions& options, 
			Vector<SubMesh>& outputSubMeshes);

		/** 
		 * Parses the scene and outputs a skeleton for the imported meshes using the imported raw data. 
		 *
		 * @param[in]	scene		Scene whose meshes to parse.
		 * @param[in]	sharedRoot	Determines should a shared root bone be created. Set this to true if the scene contains
		 *							multiple sub-meshes (as there can't be multiple roots).
		 * @return					Skeleton containing a set of bones, or null if meshes don't contain a skeleton.
		 */
		SPtr<Skeleton> createSkeleton(const FBXImportScene& scene, bool sharedRoot);

		/** Parses the scene and generates morph shapes for the imported meshes using the imported raw data. */
		SPtr<MorphShapes> createMorphShapes(const FBXImportScene& scene);

		/**	Creates an internal representation of an FBX node from an FbxNode object. */
		FBXImportNode* createImportNode(FBXImportScene& scene, FbxNode* fbxNode, FBXImportNode* parent);

	private:
		Vector<String> mExtensions;
		FbxManager* mFBXManager = nullptr;
	};

	/** @} */
}