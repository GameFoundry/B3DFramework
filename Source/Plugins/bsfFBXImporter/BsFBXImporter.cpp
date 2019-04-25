//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFBXImporter.h"
#include "Resources/BsResource.h"
#include "BsCoreApplication.h"
#include "Debug/BsDebug.h"
#include "FileSystem/BsDataStream.h"
#include "Mesh/BsMeshData.h"
#include "Mesh/BsMesh.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "BsFBXUtility.h"
#include "Mesh/BsMeshUtility.h"
#include "Renderer/BsRendererMeshData.h"
#include "Importer/BsMeshImportOptions.h"
#include "Physics/BsPhysicsMesh.h"
#include "Animation/BsAnimationCurve.h"
#include "Animation/BsAnimationClip.h"
#include "Animation/BsAnimationUtility.h"
#include "Animation/BsSkeleton.h"
#include "Animation/BsMorphShapes.h"
#include "Physics/BsPhysics.h"
#include "FileSystem/BsFileSystem.h"

namespace bs
{
	Matrix4 FBXToNativeType(const FbxAMatrix& value)
	{
		Matrix4 native;
		for (UINT32 row = 0; row < 4; row++)
			for (UINT32 col = 0; col < 4; col++)
			native[row][col] = (float)value[col][row];

		return native;
	}

	Vector3 FBXToNativeType(const FbxVector4& value)
	{
		return Vector3((float)value[0], (float)value[1], (float)value[2]);
	}

	Vector3 FBXToNativeType(const FbxDouble3& value)
	{
		return Vector3((float)value[0], (float)value[1], (float)value[2]);
	}

	Vector2 FBXToNativeType(const FbxVector2& value)
	{
		return Vector2((float)value[0], (float)value[1]);
	}

	RGBA FBXToNativeType(const FbxColor& value)
	{
		Color native;
		native.r = (float)value[0];
		native.g = (float)value[1];
		native.b = (float)value[2];
		native.a = (float)value[3];

		return native.getAsRGBA();
	}

	FbxSurfaceMaterial* FBXToNativeType(FbxSurfaceMaterial* const& value)
	{
		return value;
	}

	int FBXToNativeType(const int & value)
	{
		return value;
	}

	FBXImporter::FBXImporter()
	{
		mExtensions.push_back(u8"fbx");
		mExtensions.push_back(u8"obj");
		mExtensions.push_back(u8"dae");
	}

	bool FBXImporter::isExtensionSupported(const String& ext) const
	{
		String lowerCaseExt = ext;
		StringUtil::toLowerCase(lowerCaseExt);

		return find(mExtensions.begin(), mExtensions.end(), lowerCaseExt) != mExtensions.end();
	}

	bool FBXImporter::isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const
	{
		return true; // FBX files can be plain-text so I don't even check for magic number
	}

	SPtr<ImportOptions> FBXImporter::createImportOptions() const
	{
		return bs_shared_ptr_new<MeshImportOptions>();
	}

	SPtr<Resource> FBXImporter::import(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		MESH_DESC desc;

		Vector<FBXAnimationClipData> dummy;
		SPtr<RendererMeshData> rendererMeshData = importMeshData(filePath, importOptions, desc.subMeshes, dummy, 
			desc.skeleton, desc.morphShapes);

		const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());

		desc.usage = MU_STATIC;
		if (meshImportOptions->cpuCached)
			desc.usage |= MU_CPUCACHED;

		SPtr<Mesh> mesh = Mesh::_createPtr(rendererMeshData->getData(), desc);

		const String fileName = filePath.getFilename(false);
		mesh->setName(fileName);

		return mesh;
	}

	Vector<SubResourceRaw> FBXImporter::importAll(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		MESH_DESC desc;

		Vector<FBXAnimationClipData> animationClips;
		SPtr<RendererMeshData> rendererMeshData = importMeshData(filePath, importOptions, desc.subMeshes, animationClips, 
			desc.skeleton, desc.morphShapes);

		const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());

		desc.usage = MU_STATIC;
		if (meshImportOptions->cpuCached)
			desc.usage |= MU_CPUCACHED;

		SPtr<Mesh> mesh = Mesh::_createPtr(rendererMeshData->getData(), desc);

		const String fileName = filePath.getFilename(false);
		mesh->setName(fileName);

		Vector<SubResourceRaw> output;
		if(mesh != nullptr)
		{
			output.push_back({ u8"primary", mesh });

			CollisionMeshType collisionMeshType = meshImportOptions->collisionMeshType;
			if(collisionMeshType != CollisionMeshType::None)
			{
				if(Physics::isStarted())
				{
					PhysicsMeshType type = collisionMeshType == CollisionMeshType::Convex ? 
						PhysicsMeshType::Convex : PhysicsMeshType::Triangle;

					SPtr<PhysicsMesh> physicsMesh = PhysicsMesh::_createPtr(rendererMeshData->getData(), type);

					output.push_back({ u8"collision", physicsMesh });
				}
				else
				{
					LOGWRN("Cannot generate a collision mesh as the physics module was not started.");
				}
			}

			Vector<ImportedAnimationEvents> events = meshImportOptions->animationEvents;
			for(auto& entry : animationClips)
			{
				SPtr<AnimationClip> clip = AnimationClip::_createPtr(entry.curves, entry.isAdditive, entry.sampleRate, 
					entry.rootMotion);
				
				for(auto& eventsEntry : events)
				{
					if(entry.name == eventsEntry.name)
					{
						clip->setEvents(eventsEntry.events);
						break;
					}
				}

				output.push_back({ entry.name, clip });
			}
		}

		return output;
	}

	SPtr<RendererMeshData> FBXImporter::importMeshData(const Path& filePath, SPtr<const ImportOptions> importOptions, 
		Vector<SubMesh>& subMeshes, Vector<FBXAnimationClipData>& animation, SPtr<Skeleton>& skeleton, 
		SPtr<MorphShapes>& morphShapes)
	{
		FbxScene* fbxScene = nullptr;

		if (!startUpSdk(fbxScene))
			return nullptr;

		if (!loadFBXFile(fbxScene, filePath))
			return nullptr;

		const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());
		FBXImportOptions fbxImportOptions;
		fbxImportOptions.importNormals = meshImportOptions->importNormals;
		fbxImportOptions.importTangents = meshImportOptions->importTangents;
		fbxImportOptions.importAnimation = meshImportOptions->importAnimation;
		fbxImportOptions.importBlendShapes = meshImportOptions->importBlendShapes;
		fbxImportOptions.importSkin = meshImportOptions->importSkin;
		fbxImportOptions.importScale = meshImportOptions->importScale;
		fbxImportOptions.reduceKeyframes = meshImportOptions->reduceKeyFrames;

		FBXImportScene importedScene;
		bakeTransforms(fbxScene);
		parseScene(fbxScene, fbxImportOptions, importedScene);

		if (fbxImportOptions.importBlendShapes)
			importBlendShapes(importedScene, fbxImportOptions);

		if (fbxImportOptions.importSkin)
			importSkin(importedScene, fbxImportOptions);

		if (fbxImportOptions.importAnimation)
			importAnimations(fbxScene, fbxImportOptions, importedScene);

		splitMeshVertices(importedScene);
		generateMissingTangentSpace(importedScene, fbxImportOptions);

		SPtr<RendererMeshData> rendererMeshData = generateMeshData(importedScene, fbxImportOptions, subMeshes);

		skeleton = createSkeleton(importedScene, subMeshes.size() > 1);
		morphShapes = createMorphShapes(importedScene);

		// Import animation clips
		if (!importedScene.clips.empty())
		{
			const Vector<AnimationSplitInfo>& splits = meshImportOptions->animationSplits;
			convertAnimations(importedScene.clips, splits, skeleton, meshImportOptions->importRootMotion, animation);
		}

		// TODO - Later: Optimize mesh: Remove bad and degenerate polygons, weld nearby vertices, optimize for vertex cache

		shutDownSdk();

		return rendererMeshData;
	}

	SPtr<Skeleton> FBXImporter::createSkeleton(const FBXImportScene& scene, bool sharedRoot)
	{
		Vector<BONE_DESC> allBones;
		UnorderedMap<FBXImportNode*, UINT32> boneMap;

		for (auto& mesh : scene.meshes)
		{
			// Create bones
			for (auto& fbxBone : mesh->bones)
			{
				UINT32 boneIdx = (UINT32)allBones.size();

				auto iterFind = boneMap.find(fbxBone.node);
				if(iterFind != boneMap.end())
					continue; // Duplicate

				boneMap[fbxBone.node] = boneIdx;

				allBones.push_back(BONE_DESC());
				BONE_DESC& bone = allBones.back();

				bone.name = fbxBone.node->name;
				bone.localTfrm = fbxBone.localTfrm;
				bone.invBindPose = fbxBone.bindPose;
			}
		}

		// Generate skeleton
		if (allBones.size() > 0)
		{
			// Find bone parents
			UINT32 numProcessedBones = 0;

			// Generate common root bone for all meshes
			UINT32 rootBoneIdx = (UINT32)-1;
			if (sharedRoot)
			{
				rootBoneIdx = (UINT32)allBones.size();

				allBones.push_back(BONE_DESC());
				BONE_DESC& bone = allBones.back();

				bone.name = "MultiMeshRoot";
				bone.localTfrm = Transform();
				bone.invBindPose = Matrix4::IDENTITY;
				bone.parent = (UINT32)-1;

				numProcessedBones++;
			}

			Stack<std::pair<FBXImportNode*, UINT32>> todo;
			todo.push({ scene.rootNode, rootBoneIdx });

			while (!todo.empty())
			{
				auto entry = todo.top();
				todo.pop();

				FBXImportNode* node = entry.first;
				UINT32 parentBoneIdx = entry.second;

				auto boneIter = boneMap.find(node);
				if (boneIter != boneMap.end())
				{
					UINT32 boneIdx = boneIter->second;
					allBones[boneIdx].parent = parentBoneIdx;

					parentBoneIdx = boneIdx;
					numProcessedBones++;
				}
				else
				{
					// Node is not a bone, but it still needs to be part of the hierarchy. It wont be animated, nor will
					// it directly influence any vertices, but its transform must be applied to any child bones.
					UINT32 boneIdx = (UINT32)allBones.size();

					allBones.push_back(BONE_DESC());
					BONE_DESC& bone = allBones.back();

					bone.name = node->name;
					bone.localTfrm = node->localTransform;
					bone.invBindPose = Matrix4::IDENTITY;
					bone.parent = parentBoneIdx;

					parentBoneIdx = boneIdx;
					numProcessedBones++;
				}

				for (auto& child : node->children)
					todo.push({ child, parentBoneIdx });
			}

			UINT32 numAllBones = (UINT32)allBones.size();
			if (numProcessedBones == numAllBones)
				return Skeleton::create(allBones.data(), numAllBones);

			LOGERR("Not all bones were found in the node hierarchy. Skeleton invalid.");
		}

		return nullptr;
	}

	SPtr<MorphShapes> FBXImporter::createMorphShapes(const FBXImportScene& scene)
	{
		// Combine morph shapes from all sub-meshes, and transform them
		struct RawMorphShape
		{
			String name;
			float weight;
			Vector<MorphVertex> vertices;
		};

		UnorderedMap<String, UnorderedMap<String, RawMorphShape>> allRawMorphShapes;
		UINT32 totalNumVertices = 0;

		// Note: Order in which we combine meshes must match the order in MeshData::combine
		for (auto& mesh : scene.meshes)
		{
			UINT32 numVertices = (UINT32)mesh->positions.size();
			UINT32 numNormals = (UINT32)mesh->normals.size();
			bool hasNormals = numVertices == numNormals;

			for (auto& node : mesh->referencedBy)
			{
				Matrix4 worldTransform = node->worldTransform * node->geomTransform;
				Matrix4 worldTransformIT = worldTransform.inverse();
				worldTransformIT = worldTransformIT.transpose();

				// Copy & transform positions
				for(auto& blendShape : mesh->blendShapes)
				{
					UnorderedMap<String, RawMorphShape>& channelShapes = allRawMorphShapes[blendShape.name];

					for(auto& blendFrame : blendShape.frames)
					{
						RawMorphShape& shape = channelShapes[blendFrame.name];
						shape.name = blendFrame.name;
						shape.weight = blendFrame.weight;

						UINT32 frameNumVertices = (UINT32)blendFrame.positions.size();
						if (frameNumVertices == numVertices)
						{
							for (UINT32 i = 0; i < numVertices; i++)
							{
								Vector3 meshPosition = worldTransform.multiplyAffine(mesh->positions[i]);
								Vector3 blendPosition = worldTransform.multiplyAffine(blendFrame.positions[i]);

								Vector3 positionDelta = blendPosition - meshPosition;
								Vector3 normalDelta;
								if (hasNormals)
								{
									Vector3 blendNormal = worldTransformIT.multiplyDirection(blendFrame.normals[i]);
									blendNormal = Vector3::normalize(blendNormal);

									Vector3 meshNormal = worldTransformIT.multiplyDirection(mesh->normals[i]);
									meshNormal = Vector3::normalize(meshNormal);

									normalDelta = blendNormal - meshNormal;
								}
								else
									normalDelta = Vector3::ZERO;

								if (positionDelta.squaredLength() > 0.000001f || normalDelta.squaredLength() > 0.0001f)
									shape.vertices.push_back(MorphVertex(positionDelta, normalDelta, totalNumVertices + i));
							}
						}
						else
						{
							LOGERR("Corrupt blend shape frame. Number of vertices doesn't match the number of mesh vertices.");
						}
					}
				}

				totalNumVertices += numVertices;
			}
		}

		// Create morph shape object from combined shape data
		SPtr<MorphShapes> morphShapes;
		Vector<SPtr<MorphChannel>> allChannels;
		for (auto& channel : allRawMorphShapes)
		{
			Vector<SPtr<MorphShape>> channelShapes;
			for (auto& entry : channel.second)
			{
				RawMorphShape& shape = entry.second;
				shape.vertices.shrink_to_fit();

				SPtr<MorphShape> morphShape = MorphShape::create(shape.name, shape.weight, shape.vertices);
				channelShapes.push_back(morphShape);
			}

			if(channelShapes.size() > 0)
			{
				SPtr<MorphChannel> morphChannel = MorphChannel::create(channel.first, channelShapes);
				allChannels.push_back(morphChannel);
			}
		}

		if (!allChannels.empty())
			return MorphShapes::create(allChannels, totalNumVertices);

		return morphShapes;
	}

	bool FBXImporter::startUpSdk(FbxScene*& scene)
	{
		mFBXManager = FbxManager::Create();
		if (mFBXManager == nullptr)
		{
			LOGERR("FBX import failed: FBX SDK failed to initialize. FbxManager::Create() failed.");
			return false;
		}

		FbxIOSettings* ios = FbxIOSettings::Create(mFBXManager, IOSROOT);
		mFBXManager->SetIOSettings(ios);

		scene = FbxScene::Create(mFBXManager, "Import Scene");
		if (scene == nullptr)
		{
			LOGWRN("FBX import failed: Failed to create FBX scene.");
			return false;
		}

		return true;
	}

	void FBXImporter::shutDownSdk()
	{
		mFBXManager->Destroy();
		mFBXManager = nullptr;
	}

	bool FBXImporter::loadFBXFile(FbxScene* scene, const Path& filePath)
	{
		int lFileMajor, lFileMinor, lFileRevision;
		int lSDKMajor,  lSDKMinor,  lSDKRevision;
		FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

		Lock fileLock = FileScheduler::getLock(filePath);
		FbxImporter* importer = FbxImporter::Create(mFBXManager, "");
		bool importStatus = importer->Initialize(filePath.toString().c_str(), -1, mFBXManager->GetIOSettings());
		
		importer->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

		if(!importStatus)
		{
			LOGERR("FBX import failed: Call to FbxImporter::Initialize() failed.\n" +
				String("Error returned: %s\n\n") + String(importer->GetStatus().GetErrorString()));
			return false;
		}

		mFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, false);
		mFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, false);

		importStatus = importer->Import(scene);

		if(!importStatus)
		{
			importer->Destroy();
			
			LOGERR("FBX import failed: Call to FbxImporter::Import() failed.\n" +
				String("Error returned: %s\n\n") + String(importer->GetStatus().GetErrorString()));
			return false;
		}

		FbxAxisSystem fileCoordSystem = scene->GetGlobalSettings().GetAxisSystem();
		FbxAxisSystem bsCoordSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
		if (fileCoordSystem != bsCoordSystem)
			bsCoordSystem.ConvertScene(scene);

		importer->Destroy();
		return true;
	}

	void FBXImporter::parseScene(FbxScene* scene, const FBXImportOptions& options, FBXImportScene& outputScene)
	{
		// Scale from file units to engine units, and apply optional user scale
		float importScale = 1.0f;
		if (options.importScale > 0.0001f)
			importScale = options.importScale;

		FbxSystemUnit units = scene->GetGlobalSettings().GetSystemUnit();
		FbxSystemUnit bsScaledUnits(100.0f / importScale);

		const FbxSystemUnit::ConversionOptions convOptions = {
			false,
			true,
			true,
			true,
			true,
			true
		};

		bsScaledUnits.ConvertScene(scene, convOptions);

		outputScene.rootNode = createImportNode(outputScene, scene->GetRootNode(), nullptr);

		Stack<FbxNode*> todo;
		todo.push(scene->GetRootNode());

		while(!todo.empty())
		{
			FbxNode* curNode = todo.top();
			FBXImportNode* curImportNode = outputScene.nodeMap[curNode];
			todo.pop();

			FbxNodeAttribute* attrib = curNode->GetNodeAttribute();
			if(attrib != nullptr)
			{
				FbxNodeAttribute::EType attribType = attrib->GetAttributeType();

				switch(attribType)
				{
				case FbxNodeAttribute::eNurbs:
				case FbxNodeAttribute::eNurbsSurface:
				case FbxNodeAttribute::ePatch:
				{
					FbxGeometryConverter geomConverter(mFBXManager);
					attrib = geomConverter.Triangulate(attrib, true);

					if (attrib->GetAttributeType() == FbxNodeAttribute::eMesh)
					{
						FbxMesh* mesh = static_cast<FbxMesh*>(attrib);
						mesh->RemoveBadPolygons();

						parseMesh(mesh, curImportNode, options, outputScene);
					}
				}
					break;
				case FbxNodeAttribute::eMesh:
					{
						FbxMesh* mesh = static_cast<FbxMesh*>(attrib);
						mesh->RemoveBadPolygons();

						if(!mesh->IsTriangleMesh())
						{
							FbxGeometryConverter geomConverter(mFBXManager);
							geomConverter.Triangulate(mesh, true);
							attrib = curNode->GetNodeAttribute();
							mesh = static_cast<FbxMesh*>(attrib);
						}

						parseMesh(mesh, curImportNode, options, outputScene);
					}
					break;
				default:
					break;
				}
			}

			for (int i = 0; i < curNode->GetChildCount(); i++)
			{
				FbxNode* childNode = curNode->GetChild(i);
				createImportNode(outputScene, childNode, curImportNode);

				todo.push(childNode);
			}
		}
	}

	FBXImportNode* FBXImporter::createImportNode(FBXImportScene& scene, FbxNode* fbxNode, FBXImportNode* parent)
	{
		FBXImportNode* node = bs_new<FBXImportNode>();

		Vector3 translation = FBXToNativeType(fbxNode->EvaluateLocalTranslation(FbxTime(0)));
		Vector3 rotationEuler = FBXToNativeType(fbxNode->EvaluateLocalRotation(FbxTime(0)));
		Vector3 scale = FBXToNativeType(fbxNode->EvaluateLocalScaling(FbxTime(0)));

		Quaternion rotation((Degree)rotationEuler.x, (Degree)rotationEuler.y, (Degree)rotationEuler.z, 
			EulerAngleOrder::XYZ);

		node->name = fbxNode->GetNameWithoutNameSpacePrefix().Buffer();
		node->fbxNode = fbxNode;
		node->localTransform = Transform(translation, rotation, scale);

		if (parent != nullptr)
		{
			node->worldTransform = parent->worldTransform * node->localTransform.getMatrix();

			parent->children.push_back(node);
		}
		else
			node->worldTransform = node->localTransform.getMatrix();

		// Geometry transform is applied to geometry (mesh data) only, it is not inherited by children, so we store it
		// separately
		Vector3 geomTrans = FBXToNativeType(fbxNode->GeometricTranslation.Get());
		Vector3 geomRotEuler = FBXToNativeType(fbxNode->GeometricRotation.Get());
		Vector3 geomScale = FBXToNativeType(fbxNode->GeometricScaling.Get());

		Quaternion geomRotation((Degree)geomRotEuler.x, (Degree)geomRotEuler.y, (Degree)geomRotEuler.z, EulerAngleOrder::XYZ);
		node->geomTransform = Matrix4::TRS(geomTrans, geomRotation, geomScale);

		scene.nodeMap.insert(std::make_pair(fbxNode, node));

		// Determine if geometry winding needs to be flipped to match the engine convention. This is true by default, but
		// each negative scaling factor changes the winding.
		if (parent != nullptr)
			node->flipWinding = parent->flipWinding;
		else
			node->flipWinding = true;

		for (UINT32 i = 0; i < 3; i++)
		{
			if (scale[i] < 0.0f) node->flipWinding = !node->flipWinding;
			if (geomScale[i] < 0.0f) node->flipWinding = !node->flipWinding;
		}

		return node;
	}

	void FBXImporter::splitMeshVertices(FBXImportScene& scene)
	{
		Vector<FBXImportMesh*> splitMeshes;

		for (auto& mesh : scene.meshes)
		{
			FBXImportMesh* splitMesh = bs_new<FBXImportMesh>();
			splitMesh->fbxMesh = mesh->fbxMesh;
			splitMesh->referencedBy = mesh->referencedBy;
			splitMesh->bones = mesh->bones;

			FBXUtility::splitVertices(*mesh, *splitMesh);
			splitMeshes.push_back(splitMesh);

			bs_delete(mesh);
		}

		scene.meshes = splitMeshes;
	}

	void FBXImporter::convertAnimations(const Vector<FBXAnimationClip>& clips, const Vector<AnimationSplitInfo>& splits,
		const SPtr<Skeleton>& skeleton, bool importRootMotion, Vector<FBXAnimationClipData>& output)
	{
		UnorderedSet<String> names;

		String rootBoneName;
		if (skeleton == nullptr)
			importRootMotion = false;
		else
		{
			UINT32 rootBoneIdx = skeleton->getRootBoneIndex();
			if (rootBoneIdx == (UINT32)-1)
				importRootMotion = false;
			else
				rootBoneName = skeleton->getBoneInfo(rootBoneIdx).name;
		}

		bool isFirstClip = true;
		for (auto& clip : clips)
		{
			SPtr<AnimationCurves> curves = bs_shared_ptr_new<AnimationCurves>();
			SPtr<RootMotion> rootMotion;

			// Find offset so animations start at time 0
			float animStart = std::numeric_limits<float>::infinity();

			for (auto& bone : clip.boneAnimations)
			{
				if(bone.translation.getNumKeyFrames() > 0)
					animStart = std::min(bone.translation.getKeyFrame(0).time, animStart);

				if (bone.rotation.getNumKeyFrames() > 0)
					animStart = std::min(bone.rotation.getKeyFrame(0).time, animStart);

				if (bone.scale.getNumKeyFrames() > 0)
					animStart = std::min(bone.scale.getKeyFrame(0).time, animStart);
			}

			for (auto& anim : clip.blendShapeAnimations)
			{
				if (anim.curve.getNumKeyFrames() > 0)
					animStart = std::min(anim.curve.getKeyFrame(0).time, animStart);
			}

			AnimationCurveFlags blendShapeFlags = AnimationCurveFlag::ImportedCurve | AnimationCurveFlag::MorphFrame;
			if (animStart != 0.0f && animStart != std::numeric_limits<float>::infinity())
			{
				for (auto& bone : clip.boneAnimations)
				{
					TAnimationCurve<Vector3> translation = AnimationUtility::offsetCurve(bone.translation, -animStart);
					TAnimationCurve<Quaternion> rotation = AnimationUtility::offsetCurve(bone.rotation, -animStart);
					TAnimationCurve<Vector3> scale = AnimationUtility::offsetCurve(bone.scale, -animStart);

					if(importRootMotion && bone.node->name == rootBoneName)
						rootMotion = bs_shared_ptr_new<RootMotion>(translation, rotation);
					else
					{
						curves->position.push_back({ bone.node->name, AnimationCurveFlag::ImportedCurve, translation });
						curves->rotation.push_back({ bone.node->name, AnimationCurveFlag::ImportedCurve, rotation });
						curves->scale.push_back({ bone.node->name, AnimationCurveFlag::ImportedCurve, scale });
					}
				}

				for (auto& anim : clip.blendShapeAnimations)
				{
					TAnimationCurve<float> curve = AnimationUtility::offsetCurve(anim.curve, -animStart);
					curves->generic.push_back({ anim.blendShape, blendShapeFlags, curve });
				}
			}
			else
			{
				for (auto& bone : clip.boneAnimations)
				{
					if (importRootMotion && bone.node->name == rootBoneName)
						rootMotion = bs_shared_ptr_new<RootMotion>(bone.translation, bone.rotation);
					else
					{
						curves->position.push_back({ bone.node->name, AnimationCurveFlag::ImportedCurve, bone.translation });
						curves->rotation.push_back({ bone.node->name, AnimationCurveFlag::ImportedCurve, bone.rotation });
						curves->scale.push_back({ bone.node->name, AnimationCurveFlag::ImportedCurve, bone.scale });
					}
				}

				for (auto& anim : clip.blendShapeAnimations)
					curves->generic.push_back({ anim.blendShape, blendShapeFlags, anim.curve });
			}

			// See if any splits are required. We only split the first clip as it is assumed if FBX has multiple clips the
			// user has the ability to split them externally.
			if(isFirstClip && !splits.empty())
			{
				float secondsPerFrame = 1.0f / clip.sampleRate;

				for(auto& split : splits)
				{
					SPtr<AnimationCurves> splitClipCurve = bs_shared_ptr_new<AnimationCurves>();
					SPtr<RootMotion> splitRootMotion;

					auto splitCurves = [&](auto& inCurves, auto& outCurves)
					{
						UINT32 numCurves = (UINT32)inCurves.size();
						outCurves.resize(numCurves);

						for (UINT32 i = 0; i < numCurves; i++)
						{
							auto& animCurve = inCurves[i].curve;
							outCurves[i].name = inCurves[i].name;

							UINT32 numFrames = animCurve.getNumKeyFrames();
							if (numFrames == 0)
								continue;

							float startTime = split.startFrame * secondsPerFrame;
							float endTime = split.endFrame * secondsPerFrame;

							outCurves[i].curve = inCurves[i].curve.split(startTime, endTime);

							if (split.isAdditive)
								outCurves[i].curve.makeAdditive();
						}
					};

					splitCurves(curves->position, splitClipCurve->position);
					splitCurves(curves->rotation, splitClipCurve->rotation);
					splitCurves(curves->scale, splitClipCurve->scale);
					splitCurves(curves->generic, splitClipCurve->generic);

					if(rootMotion != nullptr)
					{
						auto splitCurve = [&](auto& inCurve, auto& outCurve)
						{
							UINT32 numFrames = inCurve.getNumKeyFrames();
							if (numFrames > 0)
							{
								float startTime = split.startFrame * secondsPerFrame;
								float endTime = split.endFrame * secondsPerFrame;

								outCurve = inCurve.split(startTime, endTime);

								if (split.isAdditive)
									outCurve.makeAdditive();
							}
						};

						splitRootMotion = bs_shared_ptr_new<RootMotion>();
						splitCurve(rootMotion->position, splitRootMotion->position);
						splitCurve(rootMotion->rotation, splitRootMotion->rotation);
					}

					// Search for a unique name
					String name = split.name;
					UINT32 attemptIdx = 0;
					while (names.find(name) != names.end())
					{
						name = clip.name + "_" + toString(attemptIdx);
						attemptIdx++;
					}

					names.insert(name);
					output.push_back(FBXAnimationClipData(name, split.isAdditive, clip.sampleRate, splitClipCurve,
						splitRootMotion));
				}
			}
			else
			{
				// Search for a unique name
				String name = clip.name;
				UINT32 attemptIdx = 0;
				while(names.find(name) != names.end())
				{
					name = clip.name + "_" + toString(attemptIdx);
					attemptIdx++;
				}

				names.insert(name);
				output.push_back(FBXAnimationClipData(name, false, clip.sampleRate, curves, rootMotion));
			}

			isFirstClip = false;
		}
	}

	SPtr<RendererMeshData> FBXImporter::generateMeshData(const FBXImportScene& scene, const FBXImportOptions& options, 
		Vector<SubMesh>& outputSubMeshes)
	{
		Vector<SPtr<MeshData>> allMeshData;
		Vector<Vector<SubMesh>> allSubMeshes;
		UINT32 boneIndexOffset = 0;

		// Generate unique indices for all the bones. This is mirrored in createSkeleton().
		UnorderedMap<FBXImportNode*, UINT32> boneMap;
		for (auto& mesh : scene.meshes)
		{
			// Create bones
			for (auto& fbxBone : mesh->bones)
			{
				UINT32 boneIdx = (UINT32)boneMap.size();

				auto iterFind = boneMap.find(fbxBone.node);
				if(iterFind != boneMap.end())
					continue; // Duplicate

				boneMap[fbxBone.node] = boneIdx;
			}
		}

		for (auto& mesh : scene.meshes)
		{
			Vector<Vector<UINT32>> indicesPerMaterial;
			for (UINT32 i = 0; i < (UINT32)mesh->indices.size(); i++)
			{
				while ((UINT32)mesh->materials[i] >= (UINT32)indicesPerMaterial.size())
					indicesPerMaterial.push_back(Vector<UINT32>());

				indicesPerMaterial[mesh->materials[i]].push_back(mesh->indices[i]);
			}

			UINT32* orderedIndices = (UINT32*)bs_alloc((UINT32)mesh->indices.size() * sizeof(UINT32));
			Vector<SubMesh> subMeshes;
			UINT32 currentIndex = 0;

			for (auto& subMeshIndices : indicesPerMaterial)
			{
				UINT32 indexCount = (UINT32)subMeshIndices.size();
				UINT32* dest = orderedIndices + currentIndex;
				memcpy(dest, subMeshIndices.data(), indexCount * sizeof(UINT32));

				subMeshes.push_back(SubMesh(currentIndex, indexCount, DOT_TRIANGLE_LIST));

				currentIndex += indexCount;
			}

			UINT32 vertexLayout = (UINT32)VertexLayout::Position;

			size_t numVertices = mesh->positions.size();
			bool hasColors = mesh->colors.size() == numVertices;
			bool hasNormals = mesh->normals.size() == numVertices;
			bool hasBoneInfluences = mesh->boneInfluences.size() == numVertices;

			if (hasColors)
				vertexLayout |= (UINT32)VertexLayout::Color;

			bool hasTangents = false;
			if (hasNormals)
			{
				vertexLayout |= (UINT32)VertexLayout::Normal;

				if (mesh->tangents.size() == numVertices &&
					mesh->bitangents.size() == numVertices)
				{
					vertexLayout |= (UINT32)VertexLayout::Tangent;
					hasTangents = true;
				}
			}

			if (hasBoneInfluences)
				vertexLayout |= (UINT32)VertexLayout::BoneWeights;

			for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
			{
				if (mesh->UV[i].size() == numVertices)
				{
					if (i == 0)
						vertexLayout |= (UINT32)VertexLayout::UV0;
					else if (i == 1)
						vertexLayout |= (UINT32)VertexLayout::UV1;
				}
			}

			UINT32 numIndices = (UINT32)mesh->indices.size();
			for (auto& node : mesh->referencedBy)
			{
				Matrix4 worldTransform = node->worldTransform * node->geomTransform;
				Matrix4 worldTransformIT = worldTransform.inverse();
				worldTransformIT = worldTransformIT.transpose();

				SPtr<RendererMeshData> meshData = RendererMeshData::create((UINT32)numVertices, numIndices, (VertexLayout)vertexLayout);

				// Copy indices
				if(!node->flipWinding)
					meshData->setIndices(orderedIndices, numIndices * sizeof(UINT32));
				else
				{
					UINT32* flippedIndices = bs_stack_alloc<UINT32>(numIndices);

					for (UINT32 i = 0; i < numIndices; i += 3)
					{
						flippedIndices[i + 0] = orderedIndices[i + 0];
						flippedIndices[i + 1] = orderedIndices[i + 2];
						flippedIndices[i + 2] = orderedIndices[i + 1];
					}

					meshData->setIndices(flippedIndices, numIndices * sizeof(UINT32));
					bs_stack_free(flippedIndices);
				}

				// Copy & transform positions
				UINT32 positionsSize = sizeof(Vector3) * (UINT32)numVertices;
				Vector3* transformedPositions = (Vector3*)bs_stack_alloc(positionsSize);

				for (UINT32 i = 0; i < (UINT32)numVertices; i++)
					transformedPositions[i] = worldTransform.multiplyAffine((Vector3)mesh->positions[i]);

				meshData->setPositions(transformedPositions, positionsSize);
				bs_stack_free(transformedPositions);

				// Copy & transform normals
				if (hasNormals)
				{
					UINT32 normalsSize = sizeof(Vector3) * (UINT32)numVertices;
					Vector3* transformedNormals = (Vector3*)bs_stack_alloc(normalsSize);

					// Copy, convert & transform tangents & bitangents
					if (hasTangents)
					{
						UINT32 tangentsSize = sizeof(Vector4) * (UINT32)numVertices;
						Vector4* transformedTangents = (Vector4*)bs_stack_alloc(tangentsSize);

						for (UINT32 i = 0; i < (UINT32)numVertices; i++)
						{
							Vector3 normal = (Vector3)mesh->normals[i];
							normal = worldTransformIT.multiplyDirection(normal);
							transformedNormals[i] = Vector3::normalize(normal);

							Vector3 tangent = (Vector3)mesh->tangents[i];
							tangent = Vector3::normalize(worldTransformIT.multiplyDirection(tangent));

							Vector3 bitangent = (Vector3)mesh->bitangents[i];
							bitangent = worldTransformIT.multiplyDirection(bitangent);

							Vector3 engineBitangent = Vector3::cross(normal, tangent);
							float sign = Vector3::dot(engineBitangent, bitangent);

							transformedTangents[i] = Vector4(tangent.x, tangent.y, tangent.z, sign > 0 ? 1.0f : -1.0f);
						}

						meshData->setTangents(transformedTangents, tangentsSize);
						bs_stack_free(transformedTangents);
					}
					else // Just normals
					{
						for (UINT32 i = 0; i < (UINT32)numVertices; i++)
							transformedNormals[i] = Vector3::normalize(worldTransformIT.multiplyDirection((Vector3)mesh->normals[i]));
					}

					meshData->setNormals(transformedNormals, normalsSize);
					bs_stack_free(transformedNormals);
				}

				// Copy colors
				if (hasColors)
				{
					meshData->setColors(mesh->colors.data(), sizeof(UINT32) * (UINT32)numVertices);
				}

				// Copy UV
				int writeUVIDx = 0;
				for (auto& uvLayer : mesh->UV)
				{
					if (uvLayer.size() == numVertices)
					{
						UINT32 size = sizeof(Vector2) * (UINT32)numVertices;
						Vector2* transformedUV = (Vector2*)bs_stack_alloc(size);

						UINT32 i = 0;
						for (auto& uv : uvLayer)
						{
							transformedUV[i] = uv;
							transformedUV[i].y = 1.0f - uv.y;

							i++;
						}

						if (writeUVIDx == 0)
							meshData->setUV0(transformedUV, size);
						else if (writeUVIDx == 1)
							meshData->setUV1(transformedUV, size);

						bs_stack_free(transformedUV);

						writeUVIDx++;
					}
				}

				// Copy bone influences & remap bone indices
				if(hasBoneInfluences)
				{
					UINT32 bufferSize = sizeof(BoneWeight) * (UINT32)numVertices;
					BoneWeight* weights = (BoneWeight*)bs_stack_alloc(bufferSize);
					for(UINT32 i = 0; i < (UINT32)numVertices; i++)
					{
						int* indices[] = { &weights[i].index0, &weights[i].index1, &weights[i].index2, &weights[i].index3};
						float* amounts[] = { &weights[i].weight0, &weights[i].weight1, &weights[i].weight2, &weights[i].weight3};

						for(UINT32 j = 0; j < 4; j++)
						{
							int boneIdx = mesh->boneInfluences[i].indices[j];
							if(boneIdx != -1)
							{
								FBXImportNode* boneNode = mesh->bones[boneIdx].node;

								auto iterFind = boneMap.find(boneNode);
								if(iterFind != boneMap.end())
									*indices[j] = iterFind->second;
								else
									*indices[j] = -1;
							}
							else
								*indices[j] = boneIdx;

							*amounts[j] = mesh->boneInfluences[i].weights[j];
						}
					}

					meshData->setBoneWeights(weights, bufferSize);
					bs_stack_free(weights);
				}

				allMeshData.push_back(meshData->getData());
				allSubMeshes.push_back(subMeshes);
			}

			bs_free(orderedIndices);

			UINT32 numBones = (UINT32)mesh->bones.size();
			boneIndexOffset += numBones;
		}

		if (allMeshData.size() > 1)
		{
			return RendererMeshData::create(MeshData::combine(allMeshData, allSubMeshes, outputSubMeshes));
		}
		else if (allMeshData.size() == 1)
		{
			outputSubMeshes = allSubMeshes[0];
			return RendererMeshData::create(allMeshData[0]);
		}

		return nullptr;
	}

	template<class TFBX, class TNative>
	class FBXDirectIndexer
	{
	public:
		FBXDirectIndexer(const FbxLayerElementTemplate<TFBX>& layer)
			:mElementArray(layer.GetDirectArray()),
			mElementCount(mElementArray.GetCount())
		{}

		bool get(int index, TNative& output) const
		{
			if (index < 0 || index >= mElementCount)
				return false;

			output = FBXToNativeType(mElementArray.GetAt(index));
			return true;
		}

		bool isEmpty() const
		{
			return mElementCount == 0;
		}

	private:
		const FbxLayerElementArrayTemplate<TFBX>& mElementArray;
		int mElementCount;
	};

	template<class TFBX, class TNative>
	class FBXIndexIndexer
	{
	public:
		FBXIndexIndexer(const FbxLayerElementTemplate<TFBX>& layer)
			:mElementArray(layer.GetDirectArray()),
			mIndexArray(layer.GetIndexArray()),
			mElementCount(mElementArray.GetCount()),
			mIndexCount(mIndexArray.GetCount())
		{}

		bool get(int index, TNative& output) const
		{
			if (index < 0 || index >= mIndexCount)
				return false;

			int actualIndex = mIndexArray.GetAt(index);

			if (actualIndex < 0 || actualIndex >= mElementCount)
				return false;

			output = FBXToNativeType(mElementArray.GetAt(actualIndex));
			return true;
		}

		bool isEmpty() const
		{
			return mElementCount == 0 || mIndexCount == 0;
		}

	private:
		const FbxLayerElementArrayTemplate<TFBX>& mElementArray;
		const FbxLayerElementArrayTemplate<int>& mIndexArray;
		int mElementCount;
		int mIndexCount;
	};

	template<class TFBX, class TNative, class TIndexer>
	void readLayerData(FbxLayerElementTemplate<TFBX>& layer, Vector<TNative>& output, const Vector<int>& indices)
	{
		TIndexer indexer(layer);
		if (indexer.isEmpty())
			return;

		output.resize(indices.size());

		FbxLayerElement::EMappingMode mappingMode = layer.GetMappingMode();

		UINT32 indexCount = (UINT32)indices.size();
		switch (mappingMode)
		{
		case FbxLayerElement::eByControlPoint:
			for (UINT32 i = 0; i < indexCount; i++)
			{
				int index = indices[i];
				indexer.get(index, output[i]);
			}
			break;
		case FbxLayerElement::eByPolygonVertex:
			for (UINT32 i = 0; i < indexCount; i++)
				indexer.get(i, output[i]);
			break;
		case FbxLayerElement::eByPolygon:
			// We expect mesh to be triangulated here
		{
			UINT32 polygonCount = indexCount / 3;
			UINT32 index = 0;

			for (UINT32 i = 0; i < polygonCount; i++)
			{
				TNative value{};
				indexer.get(i, value);

				output[index++] = value;
				output[index++] = value;
				output[index++] = value;
			}
		}
			break;
		case FbxLayerElement::eAllSame:
		{
			TNative value{};
			indexer.get(0, value);

			for (UINT32 i = 0; i < indexCount; i++)
				output[i] = value;
		}
			break;
		default:
			LOGWRN("FBX Import: Unsupported layer mapping mode.");
			break;
		}
	}

	template<class TFBX, class TNative>
	void readLayerData(FbxLayerElementTemplate<TFBX>& layer, Vector<TNative>& output, const Vector<int>& indices)
	{
		FbxLayerElement::EReferenceMode refMode = layer.GetReferenceMode();

		if (refMode == FbxLayerElement::eDirect)
			readLayerData<TFBX, TNative, FBXDirectIndexer<TFBX, TNative> >(layer, output, indices);
		else if (refMode == FbxLayerElement::eIndexToDirect)
			readLayerData<TFBX, TNative, FBXIndexIndexer<TFBX, TNative> >(layer, output, indices);
		else
			LOGWRN("FBX Import: Unsupported layer reference mode.");
	}

	void FBXImporter::parseMesh(FbxMesh* mesh, FBXImportNode* parentNode, const FBXImportOptions& options, FBXImportScene& outputScene)
	{
		// Check if valid
		if (!mesh->IsTriangleMesh())
			return;

		UINT32 vertexCount = mesh->GetControlPointsCount();
		UINT32 triangleCount = mesh->GetPolygonCount();

		if (vertexCount == 0 || triangleCount == 0)
			return;

		// Register in global mesh array
		FBXImportMesh* importMesh = nullptr;

		auto iterFindMesh = outputScene.meshMap.find(mesh);
		if (iterFindMesh != outputScene.meshMap.end())
		{
			UINT32 meshIdx = iterFindMesh->second;
			outputScene.meshes[meshIdx]->referencedBy.push_back(parentNode);

			return;
		}
		else
		{
			importMesh = bs_new<FBXImportMesh>();
			outputScene.meshes.push_back(importMesh);

			importMesh->referencedBy.push_back(parentNode);
			importMesh->fbxMesh = mesh;

			outputScene.meshMap[mesh] = (UINT32)outputScene.meshes.size() - 1;
		}

		// Import vertices
		importMesh->positions.resize(vertexCount);
		FbxVector4* controlPoints = mesh->GetControlPoints();

		for (UINT32 i = 0; i < vertexCount; i++)
			importMesh->positions[i] = FBXToNativeType(controlPoints[i]);

		// Import triangles
		UINT32 indexCount = triangleCount * 3;
		importMesh->indices.resize(indexCount);

		int* fbxIndices = mesh->GetPolygonVertices();
		importMesh->indices.assign(fbxIndices, fbxIndices + indexCount);

		// Import UVs
		Vector<FbxLayerElementUV*> fbxUVLayers;

		//// Search the diffuse layers first
		for (UINT32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			FbxLayer* layer = mesh->GetLayer(i, FbxLayerElement::eUV);
			if (layer == nullptr)
				continue;

			for (int j = FbxLayerElement::eTextureDiffuse; j < FbxLayerElement::eTypeCount; j++)
			{
				FbxLayerElementUV* uvLayer = layer->GetUVs((FbxLayerElement::EType)j);
				if (uvLayer == nullptr)
					continue;

				fbxUVLayers.push_back(uvLayer);

				if (fbxUVLayers.size() == FBX_IMPORT_MAX_UV_LAYERS)
					break;
			}

			if (fbxUVLayers.size() == FBX_IMPORT_MAX_UV_LAYERS)
				break;
		}

		//// If there's room, search all others too
		if (fbxUVLayers.size() < FBX_IMPORT_MAX_UV_LAYERS)
		{
			UINT32 numLayers = mesh->GetLayerCount();
			for (UINT32 i = 0; i < numLayers; i++)
			{
				FbxLayer* layer = mesh->GetLayer(i);
				if (layer == nullptr)
					continue;

				for (int j = FbxLayerElement::eTextureDiffuse; j < FbxLayerElement::eTypeCount; j++)
				{
					FbxLayerElementUV* uvLayer = layer->GetUVs((FbxLayerElement::EType)j);
					if (uvLayer == nullptr)
						continue;

					auto iterFind = std::find(fbxUVLayers.begin(), fbxUVLayers.end(), uvLayer);
					if (iterFind != fbxUVLayers.end())
						continue;

					fbxUVLayers.push_back(uvLayer);

					if (fbxUVLayers.size() == FBX_IMPORT_MAX_UV_LAYERS)
						break;
				}

				if (fbxUVLayers.size() == FBX_IMPORT_MAX_UV_LAYERS)
					break;
			}
		}

		for (size_t i = 0; i < fbxUVLayers.size(); i++)
			readLayerData(*fbxUVLayers[i], importMesh->UV[i], importMesh->indices);

		FbxLayer* mainLayer = mesh->GetLayer(0);
		if (mainLayer != nullptr)
		{
			// Import colors
			if (mainLayer->GetVertexColors() != nullptr)
				readLayerData(*mainLayer->GetVertexColors(), importMesh->colors, importMesh->indices);

			// Import normals
			if (options.importNormals)
			{
				bool hasNormals = mainLayer->GetNormals() != nullptr;

				if (!hasNormals)
				{
					if (mainLayer->GetSmoothing() != nullptr)
					{
						FbxLayerElementSmoothing* smoothing = mainLayer->GetSmoothing();

						if (smoothing->GetMappingMode() == FbxLayerElement::eByEdge)
						{
							FbxGeometryConverter converter(mFBXManager);
							converter.ComputePolygonSmoothingFromEdgeSmoothing(mesh, 0);
						}

						readLayerData(*smoothing, importMesh->smoothingGroups, importMesh->indices);

						if (!importMesh->smoothingGroups.empty())
						{
							FBXUtility::normalsFromSmoothing(importMesh->positions, importMesh->indices, 
								importMesh->smoothingGroups, importMesh->normals);
						}
					}
				}
				else
					readLayerData(*mainLayer->GetNormals(), importMesh->normals, importMesh->indices);
			}

			// Import tangents
			if (options.importTangents)
			{
				bool hasTangents = mainLayer->GetTangents() != nullptr && mainLayer->GetBinormals() != nullptr;

				if (!hasTangents)
				{
					if (fbxUVLayers.size() > 0)
						hasTangents = mesh->GenerateTangentsData(0, false);
				}

				if (hasTangents)
				{
					readLayerData(*mainLayer->GetTangents(), importMesh->tangents, importMesh->indices);
					readLayerData(*mainLayer->GetBinormals(), importMesh->bitangents, importMesh->indices);
				}
			}

			// Import material indexes
			if (mainLayer->GetMaterials() != nullptr)
			{
				Vector<FbxSurfaceMaterial*> fbxMaterials;

				readLayerData(*mainLayer->GetMaterials(), fbxMaterials, importMesh->indices);

				UnorderedMap<FbxSurfaceMaterial*, int> materialLookup;
				int nextMaterialIdx = 0;
				for (UINT32 i = 0; i < (UINT32)fbxMaterials.size(); i++)
				{
					auto iterFind = materialLookup.find(fbxMaterials[i]);

					int materialIdx = 0;
					if (iterFind != materialLookup.end())
						materialIdx = iterFind->second;
					else
					{
						materialIdx = nextMaterialIdx++;
						materialLookup[fbxMaterials[i]] = materialIdx;
					}

					importMesh->materials.push_back(materialIdx);
				}
			}
			else
			{
				importMesh->materials.resize(importMesh->indices.size(), 0);
			}
		}
	}

	void FBXImporter::importBlendShapes(FBXImportScene& scene, const FBXImportOptions& options)
	{
		for (auto& mesh : scene.meshes)
		{
			FbxMesh* fbxMesh = mesh->fbxMesh;

			UINT32 deformerCount = (UINT32)fbxMesh->GetDeformerCount(FbxDeformer::eBlendShape);
			for (UINT32 i = 0; i < deformerCount; i++)
			{
				FbxBlendShape* deformer = static_cast<FbxBlendShape*>(fbxMesh->GetDeformer(i, FbxDeformer::eBlendShape));

				UINT32 blendShapeChannelCount = (UINT32)deformer->GetBlendShapeChannelCount();
				for (UINT32 j = 0; j < blendShapeChannelCount; ++j)
				{
					FbxBlendShapeChannel* channel = deformer->GetBlendShapeChannel(j);
					double* weights = channel->GetTargetShapeFullWeights();

					UINT32 frameCount = channel->GetTargetShapeCount();
					if (frameCount == 0)
						continue;

					mesh->blendShapes.push_back(FBXBlendShape());
					FBXBlendShape& blendShape = mesh->blendShapes.back();
					blendShape.name = channel->GetName();
					blendShape.frames.resize(frameCount);

					// Get name without invalid characters
					blendShape.name = StringUtil::replaceAll(blendShape.name, ".", "_");
					blendShape.name = StringUtil::replaceAll(blendShape.name, "/", "_");

					for (UINT32 k = 0; k < frameCount; k++)
					{
						FbxShape* fbxShape = channel->GetTargetShape(k);

						FBXBlendShapeFrame& frame = blendShape.frames[k];
						frame.name = fbxShape->GetName();
						frame.weight = (float)(weights[k] / 100.0);

						// Get name without invalid characters
						frame.name = StringUtil::replaceAll(frame.name, ".", "_");
						frame.name = StringUtil::replaceAll(frame.name, "/", "_");
						
						importBlendShapeFrame(fbxShape, *mesh, options, frame);
					}
				}
			}
		}
	}

	void FBXImporter::importBlendShapeFrame(FbxShape* shape, const FBXImportMesh& mesh, const FBXImportOptions& options, FBXBlendShapeFrame& outFrame)
	{
		UINT32 vertexCount = (UINT32)shape->GetControlPointsCount();
		outFrame.positions.resize(vertexCount);
		FbxVector4* controlPoints = shape->GetControlPoints();

		for (UINT32 i = 0; i < vertexCount; i++)
			outFrame.positions[i] = FBXToNativeType(controlPoints[i]);

		FbxLayer* mainLayer = shape->GetLayer(0);
		if (options.importNormals)
		{
			bool hasNormals = mainLayer->GetNormals() != nullptr;

			if (!hasNormals)
			{
				if (!mesh.smoothingGroups.empty())
				{
					FBXUtility::normalsFromSmoothing(outFrame.positions, mesh.indices,
						mesh.smoothingGroups, outFrame.normals);
				}
			}
			else
				readLayerData(*mainLayer->GetNormals(), outFrame.normals, mesh.indices);
		}

		if (options.importTangents)
		{
			bool hasTangents = mainLayer->GetTangents() != nullptr && mainLayer->GetBinormals() != nullptr;

			if (hasTangents)
			{
				readLayerData(*mainLayer->GetTangents(), outFrame.tangents, mesh.indices);
				readLayerData(*mainLayer->GetBinormals(), outFrame.bitangents, mesh.indices);
			}
		}
	}

	void FBXImporter::importSkin(FBXImportScene& scene, const FBXImportOptions& options)
	{
		for (auto& mesh : scene.meshes)
		{
			FbxMesh* fbxMesh = mesh->fbxMesh;

			UINT32 deformerCount = (UINT32)fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
			if (deformerCount > 0)
			{
				// We ignore other deformers if there's more than one
				FbxSkin* deformer = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
				UINT32 boneCount = (UINT32)deformer->GetClusterCount();

				if (boneCount == 0)
					continue;

				// If only one bone and it links to itself, ignore the bone
				if (boneCount == 1)
				{
					FbxCluster* cluster = deformer->GetCluster(0);
					if (mesh->referencedBy.size() == 1 && mesh->referencedBy[0]->fbxNode == cluster->GetLink())
						continue;
				}

				importSkin(scene, deformer, *mesh, options);
			}
		}
	}

	void FBXImporter::importSkin(FBXImportScene& scene, FbxSkin* skin, FBXImportMesh& mesh, const FBXImportOptions& options)
	{
		Vector<FBXBoneInfluence>& influences = mesh.boneInfluences;
		influences.resize(mesh.positions.size());

		UnorderedSet<FbxNode*> existingBones;
		UINT32 boneCount = (UINT32)skin->GetClusterCount();
		for (UINT32 i = 0; i < boneCount; i++)
		{
			FbxCluster* cluster = skin->GetCluster(i);
			FbxNode* link = cluster->GetLink();

			// The bone node doesn't exist, skip it
			auto iterFind = scene.nodeMap.find(link);
			if (iterFind == scene.nodeMap.end())
				continue;

			mesh.bones.push_back(FBXBone());

			FBXBone& bone = mesh.bones.back();
			bone.node = iterFind->second;

			if(mesh.referencedBy.size() > 1)
			{
				// Note: If this becomes a relevant issue (unlikely), then I will have to duplicate skeleton bones for
				// each such mesh, since they will all require their own bind poses. Animation curves will also need to be
				// handled specially (likely by allowing them to be applied to multiple bones at once). The other option is
				// not to bake the node transform into mesh vertices and handle it on a Scene Object level.
				LOGWRN("Skinned mesh has multiple different instances. This is not supported.");
			}

			FBXImportNode* parentNode = mesh.referencedBy[0];

			// Calculate bind pose
			////  Grab the transform of the node linked to this cluster (should be equivalent to bone.node->worldTransform)
			FbxAMatrix linkTransform;
			cluster->GetTransformLinkMatrix(linkTransform);
			
			FbxAMatrix clusterTransform;
			cluster->GetTransformMatrix(clusterTransform);

			bone.localTfrm = bone.node->localTransform;

			FbxAMatrix invLinkTransform = linkTransform.Inverse();
			bone.bindPose = FBXToNativeType(invLinkTransform * clusterTransform);
			
			// Undo the transform we baked into the mesh
			bone.bindPose = bone.bindPose * (parentNode->worldTransform * parentNode->geomTransform).inverseAffine();
			
			bool isDuplicate = !existingBones.insert(link).second;
			bool isAdditive = cluster->GetLinkMode() == FbxCluster::eAdditive;

			// We avoid importing weights twice for duplicate bones and we don't
			// support additive link mode.
			bool importWeights = !isDuplicate && !isAdditive;
			if (!importWeights)
				continue;

			double* weights = cluster->GetControlPointWeights();
			INT32* indices = cluster->GetControlPointIndices();
			UINT32 numIndices = (UINT32)cluster->GetControlPointIndicesCount();
			INT32 numVertices = (INT32)influences.size();

			// Add new weights while keeping them in order and removing the smallest ones
			// if number of influences exceeds the set maximum value
			for (UINT32 j = 0; j < numIndices; j++)
			{
				INT32 vertexIndex = indices[j];
				float weight = (float)weights[j];

				for (INT32 k = 0; k < FBX_IMPORT_MAX_BONE_INFLUENCES; k++)
				{
					if (vertexIndex < 0 || vertexIndex >= numVertices)
						continue;

					if (weight >= influences[vertexIndex].weights[k])
					{
						for (INT32 l = FBX_IMPORT_MAX_BONE_INFLUENCES - 2; l >= k; l--)
						{
							influences[vertexIndex].weights[l + 1] = influences[vertexIndex].weights[l];
							influences[vertexIndex].indices[l + 1] = influences[vertexIndex].indices[l];
						}

						influences[vertexIndex].weights[k] = weight;
						influences[vertexIndex].indices[k] = i;
						break;
					}
				}
			}
		}
		
		if (mesh.bones.empty())
			mesh.boneInfluences.clear();

		UINT32 numBones = (UINT32)mesh.bones.size();
		if (numBones > 256)
			LOGWRN("A maximum of 256 bones per skeleton are supported. Imported skeleton has " + toString(numBones) + " bones");

		// Normalize weights
		UINT32 numInfluences = (UINT32)mesh.boneInfluences.size();
		for (UINT32 i = 0; i < numInfluences; i++)
		{
			float sum = 0.0f;
			for (UINT32 j = 0; j < FBX_IMPORT_MAX_BONE_INFLUENCES; j++)
				sum += influences[i].weights[j];

			float invSum = 1.0f / sum;
			for (UINT32 j = 0; j < FBX_IMPORT_MAX_BONE_INFLUENCES; j++)
				influences[i].weights[j] *= invSum;
		}
	}

	void FBXImporter::generateMissingTangentSpace(FBXImportScene& scene, const FBXImportOptions& options)
	{
		for (auto& mesh : scene.meshes)
		{
			UINT32 numVertices = (UINT32)mesh->positions.size();
			UINT32 numIndices = (UINT32)mesh->indices.size();

			if ((options.importNormals || options.importTangents) && mesh->normals.empty())
			{
				mesh->normals.resize(numVertices);

				MeshUtility::calculateNormals(mesh->positions.data(), (UINT8*)mesh->indices.data(), numVertices, numIndices, mesh->normals.data());
			}

			if (options.importTangents && !mesh->UV[0].empty() && (mesh->tangents.empty() || mesh->bitangents.empty()))
			{
				mesh->tangents.resize(numVertices);
				mesh->bitangents.resize(numVertices);

				MeshUtility::calculateTangents(mesh->positions.data(), mesh->normals.data(), mesh->UV[0].data(), (UINT8*)mesh->indices.data(), 
					numVertices, numIndices, mesh->tangents.data(), mesh->bitangents.data());
			}

			for (auto& shape : mesh->blendShapes)
			{
				for (auto& frame : shape.frames)
				{
					if ((options.importNormals || options.importTangents) && frame.normals.empty())
					{
						frame.normals.resize(numVertices);

						MeshUtility::calculateNormals(mesh->positions.data(), (UINT8*)mesh->indices.data(), numVertices, numIndices, frame.normals.data());
					}

					if (options.importTangents && !mesh->UV[0].empty() && (frame.tangents.empty() || frame.bitangents.empty()))
					{
						frame.tangents.resize(numVertices);
						frame.bitangents.resize(numVertices);

						MeshUtility::calculateTangents(mesh->positions.data(), frame.normals.data(), mesh->UV[0].data(), (UINT8*)mesh->indices.data(),
							numVertices, numIndices, frame.tangents.data(), frame.bitangents.data());
					}
				}
			}
		}
	}

	void FBXImporter::importAnimations(FbxScene* scene, FBXImportOptions& importOptions, FBXImportScene& importScene)
	{
		FbxNode* root = scene->GetRootNode();

		UINT32 numAnimStacks = (UINT32)scene->GetSrcObjectCount<FbxAnimStack>();
		for (UINT32 i = 0; i < numAnimStacks; i++)
		{
			FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);

			importScene.clips.push_back(FBXAnimationClip());
			FBXAnimationClip& clip = importScene.clips.back();
			clip.name = animStack->GetName();

			FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
			clip.start = (float)timeSpan.GetStart().GetSecondDouble();
			clip.end = (float)timeSpan.GetStop().GetSecondDouble();

			clip.sampleRate = (UINT32)FbxTime::GetFrameRate(scene->GetGlobalSettings().GetTimeMode());

			UINT32 layerCount = animStack->GetMemberCount<FbxAnimLayer>();
			if (layerCount > 1)
			{
				FbxAnimEvaluator* evaluator = scene->GetAnimationEvaluator();

				FbxTime startTime;
				startTime.SetSecondDouble(clip.start);

				FbxTime endTime;
				endTime.SetSecondDouble(clip.end);

				FbxTime sampleRate;

				if (importOptions.animResample)
					sampleRate.SetSecondDouble(importOptions.animSampleRate);
				else
				{
					FbxTime::EMode timeMode = scene->GetGlobalSettings().GetTimeMode();
					sampleRate.SetSecondDouble(1.0f / FbxTime::GetFrameRate(timeMode));
				}

				if (!animStack->BakeLayers(evaluator, startTime, endTime, sampleRate))
					continue;

				layerCount = animStack->GetMemberCount<FbxAnimLayer>();
			}

			if (layerCount == 1)
			{
				FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);

				importAnimations(animLayer, root, importOptions, clip, importScene);
			}
		}
	}

	void FBXImporter::importAnimations(FbxAnimLayer* layer, FbxNode* node, FBXImportOptions& importOptions,
		FBXAnimationClip& clip, FBXImportScene& importScene)
	{
		FbxAnimCurve* translation[3];
		translation[0] = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
		translation[1] = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
		translation[2] = node->LclTranslation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);

		FbxAnimCurve* rotation[3];
		rotation[0] = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
		rotation[1] = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
		rotation[2] = node->LclRotation.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);

		FbxAnimCurve* scale[3];
		scale[0] = node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_X);
		scale[1] = node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Y);
		scale[2] = node->LclScaling.GetCurve(layer, FBXSDK_CURVENODE_COMPONENT_Z);

		Vector3 defaultTranslation = FBXToNativeType(node->LclTranslation.Get());
		Vector3 defaultRotation = FBXToNativeType(node->LclRotation.Get());
		Vector3 defaultScale = FBXToNativeType(node->LclScaling.Get());

		auto hasCurveValues = [](FbxAnimCurve* curves[3])
		{
			for (UINT32 i = 0; i < 3; i++)
			{
				if (curves[i] != nullptr && curves[i]->KeyGetCount() > 0)
					return true;
			}

			return false;
		};

		bool hasBoneAnimation = hasCurveValues(translation) || hasCurveValues(rotation) || hasCurveValues(scale);
		if (hasBoneAnimation)
		{
			clip.boneAnimations.push_back(FBXBoneAnimation());
			FBXBoneAnimation& boneAnim = clip.boneAnimations.back();
			boneAnim.node = importScene.nodeMap[node];

			if (hasCurveValues(translation))
			{
				float defaultValues[3];
				memcpy(defaultValues, &defaultTranslation, sizeof(defaultValues));

				boneAnim.translation = importCurve<Vector3, 3>(translation, defaultValues, importOptions, 
					clip.start, clip.end);
			}
			else
			{
				Vector<TKeyframe<Vector3>> keyframes(1);
				keyframes[0].value = defaultTranslation;
				keyframes[0].inTangent = Vector3::ZERO;
				keyframes[0].outTangent = Vector3::ZERO;

				boneAnim.translation = TAnimationCurve<Vector3>(keyframes);
			}

			if (hasCurveValues(scale))
			{
				float defaultValues[3];
				memcpy(defaultValues, &defaultScale, sizeof(defaultValues));

				boneAnim.scale = importCurve<Vector3, 3>(scale, defaultValues, importOptions, clip.start, clip.end);
			}
			else
			{
				Vector<TKeyframe<Vector3>> keyframes(1);
				keyframes[0].value = defaultScale;
				keyframes[0].inTangent = Vector3::ZERO;
				keyframes[0].outTangent = Vector3::ZERO;

				boneAnim.scale = TAnimationCurve<Vector3>(keyframes);
			}

			SPtr<TAnimationCurve<Vector3>> eulerAnimation = bs_shared_ptr_new<TAnimationCurve<Vector3>>();
			if (hasCurveValues(rotation))
			{
				float defaultValues[3];
				memcpy(defaultValues, &defaultRotation, sizeof(defaultValues));

				*eulerAnimation = importCurve<Vector3, 3>(rotation, defaultValues, importOptions, clip.start, clip.end);
			}
			else
			{
				Vector<TKeyframe<Vector3>> keyframes(1);
				keyframes[0].value = defaultRotation;
				keyframes[0].inTangent = Vector3::ZERO;
				keyframes[0].outTangent = Vector3::ZERO;

				*eulerAnimation = TAnimationCurve<Vector3>(keyframes);
			}

			if(importOptions.reduceKeyframes)
			{
				boneAnim.translation = reduceKeyframes(boneAnim.translation);
				boneAnim.scale = reduceKeyframes(boneAnim.scale);
				*eulerAnimation = reduceKeyframes(*eulerAnimation);
			}

			boneAnim.rotation = *AnimationUtility::eulerToQuaternionCurve(eulerAnimation, EulerAngleOrder::XYZ);
		}

		if (importOptions.importBlendShapes)
		{
			FbxMesh* fbxMesh = node->GetMesh();
			if (fbxMesh != nullptr)
			{
				INT32 deformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eBlendShape);
				for (INT32 i = 0; i < deformerCount; i++)
				{
					FbxBlendShape* deformer = static_cast<FbxBlendShape*>(fbxMesh->GetDeformer(i, FbxDeformer::eBlendShape));

					INT32 channelCount = deformer->GetBlendShapeChannelCount();
					for (INT32 j = 0; j < channelCount; j++)
					{
						FbxBlendShapeChannel* channel = deformer->GetBlendShapeChannel(j);

						FbxAnimCurve* curve = fbxMesh->GetShapeChannel(i, j, layer);
						if (curve != nullptr && curve->KeyGetCount() > 0)
						{
							clip.blendShapeAnimations.push_back(FBXBlendShapeAnimation());
							FBXBlendShapeAnimation& blendShapeAnim = clip.blendShapeAnimations.back();
							blendShapeAnim.blendShape = channel->GetName();

							// Get name without invalid characters
							blendShapeAnim.blendShape = StringUtil::replaceAll(blendShapeAnim.blendShape, ".", "_");
							blendShapeAnim.blendShape = StringUtil::replaceAll(blendShapeAnim.blendShape, "/", "_");

							FbxAnimCurve* curves[1] = { curve };
							float defaultValues[1] = { 0.0f };
							blendShapeAnim.curve = importCurve<float, 1>(curves, defaultValues, importOptions, clip.start, 
								clip.end);

							// FBX contains data in [0, 100] range, but we need it in [0, 1] range
							blendShapeAnim.curve = AnimationUtility::scaleCurve(blendShapeAnim.curve, 0.01f);
						}
					}
				}
			}
		}

		UINT32 childCount = (UINT32)node->GetChildCount();
		for (UINT32 i = 0; i < childCount; i++)
		{
			FbxNode* child = node->GetChild(i);
			importAnimations(layer, child, importOptions, clip, importScene);
		}
	}

	void FBXImporter::bakeTransforms(FbxScene* scene)
	{
		// FBX stores transforms in a more complex way than just translation-rotation-scale as used by the framework.
		// Instead they also support rotations offsets and pivots, scaling pivots and more. We wish to bake all this data
		// into a standard transform so we can access it using node's local TRS properties (e.g. FbxNode::LclTranslation).

		double frameRate = FbxTime::GetFrameRate(scene->GetGlobalSettings().GetTimeMode());

		bs_frame_mark();
		{
			FrameStack<FbxNode*> todo;
			todo.push(scene->GetRootNode());

			while(todo.size() > 0)
			{
				FbxNode* node = todo.top();
				todo.pop();

				FbxVector4 zero(0, 0, 0);
				FbxVector4 one(1, 1, 1);

				// Activate pivot converting
				node->SetPivotState(FbxNode::eSourcePivot, FbxNode::ePivotActive);
				node->SetPivotState(FbxNode::eDestinationPivot, FbxNode::ePivotActive);

				// We want to set all these to 0 (1 for scale) and bake them into the transforms
				node->SetPostRotation(FbxNode::eDestinationPivot, zero);
				node->SetPreRotation(FbxNode::eDestinationPivot, zero);
				node->SetRotationOffset(FbxNode::eDestinationPivot, zero);
				node->SetScalingOffset(FbxNode::eDestinationPivot, zero);
				node->SetRotationPivot(FbxNode::eDestinationPivot, zero);
				node->SetScalingPivot(FbxNode::eDestinationPivot, zero);

				// We account for geometric properties separately during node traversal
				node->SetGeometricTranslation(FbxNode::eDestinationPivot, node->GetGeometricTranslation(FbxNode::eSourcePivot));
				node->SetGeometricRotation(FbxNode::eDestinationPivot, node->GetGeometricRotation(FbxNode::eSourcePivot));
				node->SetGeometricScaling(FbxNode::eDestinationPivot, node->GetGeometricScaling(FbxNode::eSourcePivot));

				// Use XYZ as that appears to be the default for FBX (other orders sometimes have artifacts)
				node->SetRotationOrder(FbxNode::eDestinationPivot, FbxEuler::eOrderXYZ);

				// Keep interpolation as is
				node->SetQuaternionInterpolation(FbxNode::eDestinationPivot, node->GetQuaternionInterpolation(FbxNode::eSourcePivot));

				for (int i = 0; i < node->GetChildCount(); i++)
				{
					FbxNode* childNode = node->GetChild(i);
					todo.push(childNode);
				}
			}

			scene->GetRootNode()->ConvertPivotAnimationRecursive(nullptr, FbxNode::eDestinationPivot, frameRate, false);
		}
		bs_frame_clear();
	}

	TAnimationCurve<Vector3> FBXImporter::reduceKeyframes(TAnimationCurve<Vector3>& curve)
	{
		UINT32 keyCount = curve.getNumKeyFrames();

		Vector<TKeyframe<Vector3>> newKeyframes;

		bool lastWasEqual = false;
		for (UINT32 i = 0; i < keyCount; i++)
		{
			bool isEqual = true;

			const TKeyframe<Vector3>& curKey = curve.getKeyFrame(i);
			if (i > 0)
			{
				TKeyframe<Vector3>& prevKey = newKeyframes.back();

				isEqual = Math::approxEquals(prevKey.value, curKey.value) &&
					Math::approxEquals(prevKey.outTangent, curKey.inTangent) && isEqual;
			}
			else
				isEqual = false;

			// More than two keys in a row are equal, remove previous key by replacing it with this one
			if (lastWasEqual && isEqual)
			{
				TKeyframe<Vector3>& prevKey = newKeyframes.back();

				// Other properties are guaranteed unchanged
				prevKey.time = curKey.time;
				prevKey.outTangent = curKey.outTangent;

				continue;
			}

			newKeyframes.push_back(curKey);
			lastWasEqual = isEqual;
		}

		return TAnimationCurve<Vector3>(newKeyframes);
	}

	template<class T>
	void setKeyframeValues(TKeyframe<T>& keyFrame, int idx, float value, float inTangent, float outTangent)
	{
		keyFrame.value = value;
		keyFrame.inTangent = inTangent;
		keyFrame.outTangent = outTangent;
	}

	template<>
	void setKeyframeValues<Vector3>(TKeyframe<Vector3>& keyFrame, int idx, float value, float inTangent, float outTangent)
	{
		keyFrame.value[idx] = value;
		keyFrame.inTangent[idx] = inTangent;
		keyFrame.outTangent[idx] = outTangent;
	}

	template<class T, int C>
	TAnimationCurve<T> FBXImporter::importCurve(FbxAnimCurve*(&fbxCurve)[C], float (&defaultValues)[C],
		FBXImportOptions& importOptions, float clipStart, float clipEnd)
	{
		int keyCounts[C];
		for (int i = 0; i < C; i++)
		{
			if (fbxCurve[i] != nullptr)
				keyCounts[i] = fbxCurve[i]->KeyGetCount();
			else
				keyCounts[i] = 0;
		}

		// If curve key-counts don't match, we need to force resampling 
		bool forceResample = false;
		if (!forceResample)
		{
			for (int i = 1; i < C; i++)
			{
				forceResample |= keyCounts[i - 1] != keyCounts[i];
				if (forceResample)
					break;
			}
		}

		// Determine curve length
		float curveStart = std::numeric_limits<float>::infinity();
		float curveEnd = -std::numeric_limits<float>::infinity();

		for (INT32 i = 0; i < C; i++)
		{
			if(fbxCurve[i] == nullptr)
			{
				curveStart = std::min(0.0f, curveStart);
				curveEnd = std::max(0.0f, curveEnd);

				continue;
			}

			int keyCount = keyCounts[i];
			for (INT32 j = 0; j < keyCount; j++)
			{
				FbxTime fbxTime = fbxCurve[i]->KeyGetTime(j);
				float time = (float)fbxTime.GetSecondDouble();

				curveStart = std::min(time, curveStart);
				curveEnd = std::max(time, curveEnd);
			}
		}

		// Read keys directly
		if(!importOptions.animResample && !forceResample)
		{
			bool foundMismatch = false;
			int keyCount = keyCounts[0];
			Vector<TKeyframe<T>> keyframes;

			// All curves must match the length of the clip, so add a keyframe if first keyframe doesn't match the start time
			if(curveStart > clipStart)
			{
				keyframes.push_back(TKeyframe<T>());
				TKeyframe<T>& keyFrame = keyframes.back();

				keyFrame.time = clipStart;

				FbxTime fbxSampleTime;
				fbxSampleTime.SetSecondDouble(clipStart);

				for (int j = 0; j < C; j++)
				{
					setKeyframeValues(keyFrame, j,
						fbxCurve[j]->Evaluate(fbxSampleTime),
						fbxCurve[j]->EvaluateLeftDerivative(fbxSampleTime),
						fbxCurve[j]->EvaluateRightDerivative(fbxSampleTime));
				}
			}

			for (int i = 0; i < keyCount; i++)
			{
				FbxTime fbxTime = fbxCurve[0]->KeyGetTime(i);
				float time = (float)fbxTime.GetSecondDouble();

				// Ensure times from other curves match
				for (int j = 1; j < C; j++)
				{
					fbxTime = fbxCurve[j]->KeyGetTime(i);
					float otherTime = (float)fbxTime.GetSecondDouble();

					if (!Math::approxEquals(time, otherTime))
					{
						foundMismatch = true;
						break;
					}
				}

				if(foundMismatch)
					break;

				if (time < clipStart || time > clipEnd)
					continue;

				keyframes.push_back(TKeyframe<T>());
				TKeyframe<T>& keyFrame = keyframes.back();

				keyFrame.time = time;

				for (int j = 0; j < C; j++)
				{
					setKeyframeValues(keyFrame, j,
						fbxCurve[j]->KeyGetValue(i),
						fbxCurve[j]->KeyGetLeftDerivative(i),
						fbxCurve[j]->KeyGetRightDerivative(i));
				}
			}

			// All curves must match the length of the clip, so add a keyframe if last keyframe doesn't match the end time
			if(curveEnd < clipEnd)
			{
				keyframes.push_back(TKeyframe<T>());
				TKeyframe<T>& keyFrame = keyframes.back();

				keyFrame.time = clipEnd;

				FbxTime fbxSampleTime;
				fbxSampleTime.SetSecondDouble(clipEnd);

				for (int j = 0; j < C; j++)
				{
					setKeyframeValues(keyFrame, j,
						fbxCurve[j]->Evaluate(fbxSampleTime),
						fbxCurve[j]->EvaluateLeftDerivative(fbxSampleTime),
						fbxCurve[j]->EvaluateRightDerivative(fbxSampleTime));
				}
			}

			if (!foundMismatch)
				return TAnimationCurve<T>(keyframes);
			else
				forceResample = true;
		}

		// Resample keys
		if (!importOptions.animResample && forceResample)
			LOGWRN_VERBOSE("Animation has different keyframes for different curve components, forcing resampling.");

		// Make sure to resample along the length of the entire clip
		curveStart = std::min(curveStart, clipStart);
		curveEnd = std::max(curveEnd, clipEnd);

		float curveLength = curveEnd - curveStart;
		INT32 numSamples = Math::ceilToInt(curveLength / importOptions.animSampleRate) + 1;

		// We don't use the exact provided sample rate but instead modify it slightly so it
		// completely covers the curve range including start/end points while maintaining
		// constant time step between keyframes.
		float dt = curveLength / (float)(numSamples - 1); 

		INT32 lastKeyframe[] = { 0, 0, 0 };
		INT32 lastLeftTangent[] = { 0, 0, 0 };
		INT32 lastRightTangent[] = { 0, 0, 0 };

		Vector<TKeyframe<T>> keyframes(numSamples);
		for (INT32 i = 0; i < numSamples; i++)
		{
			float sampleTime = std::min(curveStart + i * dt, curveEnd);
			FbxTime fbxSampleTime;
			fbxSampleTime.SetSecondDouble(sampleTime);

			TKeyframe<T>& keyFrame = keyframes[i];
			keyFrame.time = sampleTime;

			for (int j = 0; j < C; j++)
			{
				if (fbxCurve[j] != nullptr)
				{
					setKeyframeValues(keyFrame, j,
									  fbxCurve[j]->Evaluate(fbxSampleTime, &lastKeyframe[j]),
									  fbxCurve[j]->EvaluateLeftDerivative(fbxSampleTime, &lastLeftTangent[j]),
									  fbxCurve[j]->EvaluateRightDerivative(fbxSampleTime, &lastRightTangent[j]));
				}
				else
				{
					setKeyframeValues(keyFrame, j, defaultValues[j], 0.0f, 0.0f);
				}
			}
		}

		return TAnimationCurve<T>(keyframes);
	}
}
