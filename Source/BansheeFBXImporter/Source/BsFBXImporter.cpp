//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFBXImporter.h"
#include "BsResource.h"
#include "BsCoreApplication.h"
#include "BsDebug.h"
#include "BsDataStream.h"
#include "BsMeshData.h"
#include "BsMesh.h"
#include "BsVector2.h"
#include "BsVector3.h"
#include "BsVector4.h"
#include "BsQuaternion.h"
#include "BsVertexDataDesc.h"
#include "BsFBXUtility.h"
#include "BsMeshUtility.h"
#include "BsRendererMeshData.h"
#include "BsMeshImportOptions.h"
#include "BsPhysicsMesh.h"
#include "BsAnimationCurve.h"
#include "BsAnimationClip.h"
#include "BsSkeleton.h"
#include "BsPhysics.h"

namespace BansheeEngine
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
		Vector3 native;
		native.x = (float)value[0];
		native.y = (float)value[1];
		native.z = (float)value[2];

		return native;
	}

	Vector3 FBXToNativeType(const FbxDouble3& value)
	{
		Vector3 native;
		native.x = (float)value[0];
		native.y = (float)value[1];
		native.z = (float)value[2];

		return native;
	}

	Vector2 FBXToNativeType(const FbxVector2& value)
	{
		Vector2 native;
		native.x = (float)value[0];
		native.y = (float)value[1];

		return native;
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
		:SpecificImporter(), mFBXManager(nullptr)
	{
		mExtensions.push_back(L"fbx");
	}

	FBXImporter::~FBXImporter() 
	{

	}

	bool FBXImporter::isExtensionSupported(const WString& ext) const
	{
		WString lowerCaseExt = ext;
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
		Vector<SubMesh> subMeshes;
		UnorderedMap<String, SPtr<AnimationCurves>> dummy;
		SPtr<Skeleton> skeleton;
		SPtr<RendererMeshData> rendererMeshData = importMeshData(filePath, importOptions, subMeshes, dummy, skeleton);

		const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());

		INT32 usage = MU_STATIC;
		if (meshImportOptions->getCPUReadable())
			usage |= MU_CPUCACHED;

		SPtr<Mesh> mesh = Mesh::_createPtr(rendererMeshData->getData(), subMeshes, usage, skeleton);

		WString fileName = filePath.getWFilename(false);
		mesh->setName(fileName);

		return mesh;
	}

	Vector<SubResourceRaw> FBXImporter::importAll(const Path& filePath, SPtr<const ImportOptions> importOptions)
	{
		Vector<SubMesh> subMeshes;
		UnorderedMap<String, SPtr<AnimationCurves>> animationClips;
		SPtr<Skeleton> skeleton;
		SPtr<RendererMeshData> rendererMeshData = importMeshData(filePath, importOptions, subMeshes, animationClips, skeleton);

		const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());

		INT32 usage = MU_STATIC;
		if (meshImportOptions->getCPUReadable())
			usage |= MU_CPUCACHED;

		SPtr<Mesh> mesh = Mesh::_createPtr(rendererMeshData->getData(), subMeshes, usage, skeleton);

		WString fileName = filePath.getWFilename(false);
		mesh->setName(fileName);

		Vector<SubResourceRaw> output;
		if(mesh != nullptr)
		{
			output.push_back({ L"primary", mesh });

			CollisionMeshType collisionMeshType = meshImportOptions->getCollisionMeshType();
			if(collisionMeshType != CollisionMeshType::None)
			{
				if(Physics::isStarted())
				{
					PhysicsMeshType type = collisionMeshType == CollisionMeshType::Convex ? 
						PhysicsMeshType::Convex : PhysicsMeshType::Triangle;

					SPtr<PhysicsMesh> physicsMesh = PhysicsMesh::_createPtr(rendererMeshData->getData(), type);

					output.push_back({ L"collision", physicsMesh });
				}
				else
				{
					LOGWRN("Cannot generate a collision mesh as the physics module was not started.");
				}
			}

			for(auto& entry : animationClips)
			{
				SPtr<AnimationClip> clip = AnimationClip::_createPtr(entry.second);

				output.push_back({ toWString(entry.first), clip });
			}
		}

		return output;
	}

	SPtr<RendererMeshData> FBXImporter::importMeshData(const Path& filePath, SPtr<const ImportOptions> importOptions, 
		Vector<SubMesh>& subMeshes, UnorderedMap<String, SPtr<AnimationCurves>>& animation, SPtr<Skeleton>& skeleton)
	{
		FbxScene* fbxScene = nullptr;

		if (!startUpSdk(fbxScene))
			return nullptr;

		if (!loadFBXFile(fbxScene, filePath))
			return nullptr;

		const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());
		FBXImportOptions fbxImportOptions;
		fbxImportOptions.importNormals = meshImportOptions->getImportNormals();
		fbxImportOptions.importTangents = meshImportOptions->getImportTangents();
		fbxImportOptions.importAnimation = meshImportOptions->getImportAnimation();
		fbxImportOptions.importBlendShapes = meshImportOptions->getImportBlendShapes();
		fbxImportOptions.importSkin = meshImportOptions->getImportSkin();
		fbxImportOptions.importScale = meshImportOptions->getImportScale();

		FBXImportScene importedScene;
		parseScene(fbxScene, fbxImportOptions, importedScene);

		if (fbxImportOptions.importBlendShapes)
			importBlendShapes(importedScene, fbxImportOptions);

		if (fbxImportOptions.importSkin)
			importSkin(importedScene);

		if (fbxImportOptions.importAnimation)
			importAnimations(fbxScene, fbxImportOptions, importedScene);

		splitMeshVertices(importedScene);
		generateMissingTangentSpace(importedScene, fbxImportOptions);

		SPtr<RendererMeshData> rendererMeshData = generateMeshData(importedScene, fbxImportOptions, subMeshes, skeleton);

		// Import animation clips
		if (!importedScene.clips.empty())
		{
			Vector<AnimationSplitInfo> splits = meshImportOptions->getAnimationClipSplits();
			convertAnimations(importedScene.clips, splits, animation);
		}

		// TODO - Later: Optimize mesh: Remove bad and degenerate polygons, weld nearby vertices, optimize for vertex cache

		shutDownSdk();

		return rendererMeshData;
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

		Vector3 translation = FBXToNativeType(fbxNode->LclTranslation.Get());
		Vector3 rotationEuler = FBXToNativeType(fbxNode->LclRotation.Get());
		Vector3 scale = FBXToNativeType(fbxNode->LclScaling.Get());

		Quaternion rotation((Radian)rotationEuler.x, (Radian)rotationEuler.y, (Radian)rotationEuler.z);

		node->localTransform.setTRS(translation, rotation, scale);
		node->name = fbxNode->GetNameWithoutNameSpacePrefix().Buffer();
		node->fbxNode = fbxNode;

		if (parent != nullptr)
		{
			node->worldTransform = node->localTransform * parent->worldTransform;

			parent->children.push_back(node);
		}
		else
			node->worldTransform = node->localTransform;

		scene.nodeMap.insert(std::make_pair(fbxNode, node));

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
			FBXUtility::flipWindingOrder(*splitMesh);
			splitMeshes.push_back(splitMesh);

			bs_delete(mesh);
		}

		scene.meshes = splitMeshes;
	}

	void FBXImporter::convertAnimations(const Vector<FBXAnimationClip>& clips, const Vector<AnimationSplitInfo>& splits,
		UnorderedMap<String, SPtr<AnimationCurves>>& output)
	{
		bool isFirstClip = true;
		for (auto& clip : clips)
		{
			SPtr<AnimationCurves> curves = bs_shared_ptr_new<AnimationCurves>();
			
			for (auto& bone : clip.boneAnimations)
			{
				// Translation curves
				{
					assert((bone.translation[0].keyframes.size() == bone.translation[1].keyframes.size()) &&
						(bone.translation[0].keyframes.size() == bone.translation[2].keyframes.size()));

					UINT32 numKeyframes = (UINT32)bone.translation[0].keyframes.size();
					Vector <TKeyframe<Vector3>> keyFrames(numKeyframes);
					for (UINT32 i = 0; i < numKeyframes; i++)
					{
						const FBXKeyFrame& keyFrameX = bone.translation[0].keyframes[i];
						const FBXKeyFrame& keyFrameY = bone.translation[1].keyframes[i];
						const FBXKeyFrame& keyFrameZ = bone.translation[2].keyframes[i];

						keyFrames[i].value = Vector3(keyFrameX.value, keyFrameY.value, keyFrameZ.value);

						assert((keyFrameX.time == keyFrameY.time) && (keyFrameX.time == keyFrameZ.time));
						keyFrames[i].time = keyFrameX.time;
						keyFrames[i].inTangent = Vector3(keyFrameX.inTangent, keyFrameY.inTangent, keyFrameZ.inTangent);
						keyFrames[i].outTangent = Vector3(keyFrameX.outTangent, keyFrameY.outTangent, keyFrameZ.outTangent);
					}

					curves->position.push_back({ bone.node->name, keyFrames });
				}

				// Rotation curves
				{
					assert((bone.rotation[0].keyframes.size() == bone.rotation[1].keyframes.size()) &&
						(bone.rotation[0].keyframes.size() == bone.rotation[2].keyframes.size()) &&
						(bone.rotation[0].keyframes.size() == bone.rotation[3].keyframes.size()));

					UINT32 numKeyframes = (UINT32)bone.rotation[0].keyframes.size();
					Vector <TKeyframe<Quaternion>> keyFrames(numKeyframes);
					for (UINT32 i = 0; i < numKeyframes; i++)
					{
						const FBXKeyFrame& keyFrameW = bone.rotation[0].keyframes[i];
						const FBXKeyFrame& keyFrameX = bone.rotation[1].keyframes[i];
						const FBXKeyFrame& keyFrameY = bone.rotation[2].keyframes[i];
						const FBXKeyFrame& keyFrameZ = bone.rotation[3].keyframes[i];

						keyFrames[i].value = Quaternion(keyFrameW.value, keyFrameX.value, keyFrameY.value, keyFrameZ.value);

						assert((keyFrameX.time == keyFrameY.time) && (keyFrameX.time == keyFrameZ.time) && (keyFrameX.time == keyFrameW.time));
						keyFrames[i].time = keyFrameX.time;
						keyFrames[i].inTangent = Quaternion(keyFrameW.inTangent, keyFrameX.inTangent, keyFrameY.inTangent, keyFrameZ.inTangent);
						keyFrames[i].outTangent = Quaternion(keyFrameW.outTangent, keyFrameX.outTangent, keyFrameY.outTangent, keyFrameZ.outTangent);
					}

					curves->rotation.push_back({ bone.node->name, keyFrames });
				}

				// Scale curves
				{
					assert((bone.scale[0].keyframes.size() == bone.scale[1].keyframes.size()) &&
						(bone.scale[0].keyframes.size() == bone.scale[2].keyframes.size()));

					UINT32 numKeyframes = (UINT32)bone.scale[0].keyframes.size();
					Vector <TKeyframe<Vector3>> keyFrames(numKeyframes);
					for (UINT32 i = 0; i < numKeyframes; i++)
					{
						const FBXKeyFrame& keyFrameX = bone.scale[0].keyframes[i];
						const FBXKeyFrame& keyFrameY = bone.scale[1].keyframes[i];
						const FBXKeyFrame& keyFrameZ = bone.scale[2].keyframes[i];

						keyFrames[i].value = Vector3(keyFrameX.value, keyFrameY.value, keyFrameZ.value);

						assert((keyFrameX.time == keyFrameY.time) && (keyFrameX.time == keyFrameZ.time));
						keyFrames[i].time = keyFrameX.time;
						keyFrames[i].inTangent = Vector3(keyFrameX.inTangent, keyFrameY.inTangent, keyFrameZ.inTangent);
						keyFrames[i].outTangent = Vector3(keyFrameX.outTangent, keyFrameY.outTangent, keyFrameZ.outTangent);
					}

					curves->scale.push_back({ bone.node->name, keyFrames });
				}
			}

			// See if any splits are required. We only split the first clip as it is assumed if FBX has multiple clips the
			// user has the ability to split them externally.
			if(isFirstClip && !splits.empty())
			{
				for(auto& split : splits)
				{
					SPtr<AnimationCurves> splitClipCurve = bs_shared_ptr_new<AnimationCurves>();

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

							UINT32 lastFrame = numFrames - 1;
							float startTime = animCurve.getKeyFrame(std::min(split.startFrame, lastFrame)).time;
							float endTime = animCurve.getKeyFrame(std::min(split.endFrame, lastFrame)).time;

							outCurves[i].curve = inCurves[i].curve.split(startTime, endTime);

							if (split.isAdditive)
								outCurves[i].curve.makeAdditive();
						}
					};

					splitCurves(curves->position, splitClipCurve->position);
					splitCurves(curves->rotation, splitClipCurve->rotation);
					splitCurves(curves->scale, splitClipCurve->scale);
					splitCurves(curves->generic, splitClipCurve->generic);

					// Search for a unique name
					String name = split.name;
					UINT32 attemptIdx = 0;
					while (output.find(name) != output.end())
					{
						name = clip.name + "_" + toString(attemptIdx);
						attemptIdx++;
					}

					output.insert(std::make_pair(name, splitClipCurve));
				}
			}
			else
			{
				// Search for a unique name
				String name = clip.name;
				UINT32 attemptIdx = 0;
				while(output.find(name) != output.end())
				{
					name = clip.name + "_" + toString(attemptIdx);
					attemptIdx++;
				}

				output.insert(std::make_pair(name, curves));
			}

			isFirstClip = false;
		}
	}

	SPtr<RendererMeshData> FBXImporter::generateMeshData(const FBXImportScene& scene, const FBXImportOptions& options, 
		Vector<SubMesh>& outputSubMeshes, SPtr<Skeleton>& outputSkeleton)
	{
		Matrix4 importScale = Matrix4::scaling(options.importScale);

		Vector<SPtr<MeshData>> allMeshData;
		Vector<Vector<SubMesh>> allSubMeshes;
		Vector<BONE_DESC> allBones;
		UnorderedMap<FBXImportNode*, UINT32> boneMap;
		UINT32 boneIndexOffset = 0;

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
				Matrix4 worldTransform = node->worldTransform * importScale;
				Matrix4 worldTransformIT = worldTransform.transpose();
				worldTransformIT = worldTransformIT.inverse();

				SPtr<RendererMeshData> meshData = RendererMeshData::create((UINT32)numVertices, numIndices, (VertexLayout)vertexLayout);

				// Copy indices
				meshData->setIndices((UINT32*)mesh->indices.data(), numIndices * sizeof(UINT32));

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

				// Copy bone influences
				if(hasBoneInfluences)
				{
					UINT32 bufferSize = sizeof(BoneWeight) * (UINT32)numVertices;
					BoneWeight* weights = (BoneWeight*)bs_stack_alloc(bufferSize);
					for(UINT32 i = 0; i < (UINT32)numVertices; i++)
					{
						weights[i].index0 = mesh->boneInfluences[i].indices[0] + boneIndexOffset;
						weights[i].index1 = mesh->boneInfluences[i].indices[1] + boneIndexOffset;
						weights[i].index2 = mesh->boneInfluences[i].indices[2] + boneIndexOffset;
						weights[i].index3 = mesh->boneInfluences[i].indices[3] + boneIndexOffset;

						weights[i].weight0 = mesh->boneInfluences[i].weights[0];
						weights[i].weight1 = mesh->boneInfluences[i].weights[1];
						weights[i].weight2 = mesh->boneInfluences[i].weights[2];
						weights[i].weight3 = mesh->boneInfluences[i].weights[3];
					}

					meshData->setBoneWeights(weights, bufferSize);
					bs_stack_free(weights);
				}

				// TODO - Transform blend shapes?

				allMeshData.push_back(meshData->getData());
				allSubMeshes.push_back(subMeshes);
			}

			// Create bones
			UINT32 numBones = (UINT32)mesh->bones.size();
			for(auto& fbxBone : mesh->bones)
			{
				UINT32 boneIdx = (UINT32)allBones.size();
				boneMap[fbxBone.node] = boneIdx;

				allBones.push_back(BONE_DESC());
				BONE_DESC& bone = allBones.back();

				bone.name = fbxBone.node->name;
				bone.invBindPose = fbxBone.bindPose;
			}

			boneIndexOffset += numBones;
		}

		// Generate skeleton
		if (allBones.size() > 0)
		{
			// Find bone parents
			UINT32 numProcessedBones = 0;

			// Generate common root bone for all meshes
			UINT32 rootBoneIdx = (UINT32)-1;
			if(allMeshData.size() > 1)
			{
				rootBoneIdx = (UINT32)allBones.size();

				allBones.push_back(BONE_DESC());
				BONE_DESC& bone = allBones.back();

				bone.name = "MultiMeshRoot";
				bone.invBindPose = Matrix4::IDENTITY;
				bone.parent = (UINT32)-1;

				numProcessedBones++;
			}

			Stack<std::pair<FBXImportNode*, UINT32>> todo;
			todo.push({ scene.rootNode, rootBoneIdx });

			while(!todo.empty())
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
					numProcessedBones++;

					parentBoneIdx = boneIdx;
				}

				for (auto& child : node->children)
					todo.push({ child, parentBoneIdx });
			}

			UINT32 numAllBones = (UINT32)allBones.size();
			if (numProcessedBones == numAllBones)
				outputSkeleton = Skeleton::create(allBones.data(), numAllBones);
			else
				LOGERR("Not all bones were found in the node hierarchy. Skeleton invalid.");
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
				TNative value;
				indexer.get(i, value);

				output[index++] = value;
				output[index++] = value;
				output[index++] = value;
			}
		}
			break;
		case FbxLayerElement::eAllSame:
		{
			TNative value;
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

					for (UINT32 k = 0; k < frameCount; k++)
					{
						FbxShape* fbxShape = channel->GetTargetShape(k);

						FBXBlendShapeFrame& frame = blendShape.frames[k];
						frame.weight = (float)weights[k];

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

	void FBXImporter::importSkin(FBXImportScene& scene)
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

				importSkin(scene, deformer, *mesh);
			}
		}
	}

	void FBXImporter::importSkin(FBXImportScene& scene, FbxSkin* skin, FBXImportMesh& mesh)
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

			FbxAMatrix clusterTransform;
			cluster->GetTransformMatrix(clusterTransform);

			FbxAMatrix linkTransform;
			cluster->GetTransformLinkMatrix(linkTransform);

			FbxAMatrix bindPose = linkTransform.Inverse() * clusterTransform;
			bone.bindPose = FBXToNativeType(bindPose);

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
						mesh->tangents.resize(numVertices);
						mesh->bitangents.resize(numVertices);

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

			importCurve(translation[0], importOptions, boneAnim.translation[0], clip.start, clip.end);
			importCurve(translation[1], importOptions, boneAnim.translation[1], clip.start, clip.end);
			importCurve(translation[2], importOptions, boneAnim.translation[2], clip.start, clip.end);

			importCurve(scale[0], importOptions, boneAnim.scale[0], clip.start, clip.end);
			importCurve(scale[1], importOptions, boneAnim.scale[1], clip.start, clip.end);
			importCurve(scale[2], importOptions, boneAnim.scale[2], clip.start, clip.end);

			FBXAnimationCurve tempCurveRotation[3];
			importCurve(rotation[0], importOptions, tempCurveRotation[0], clip.start, clip.end);
			importCurve(rotation[1], importOptions, tempCurveRotation[1], clip.start, clip.end);
			importCurve(rotation[2], importOptions, tempCurveRotation[2], clip.start, clip.end);

			eulerToQuaternionCurves(tempCurveRotation, boneAnim.rotation);
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

							importCurve(curve, importOptions, blendShapeAnim.curve, clip.start, clip.end);
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

	void FBXImporter::eulerToQuaternionCurves(FBXAnimationCurve(&eulerCurves)[3], FBXAnimationCurve(&quatCurves)[4])
	{
		const float FIT_TIME = 0.33f;

		INT32 numKeys = (INT32)eulerCurves[0].keyframes.size();

		if (numKeys != (INT32)eulerCurves[1].keyframes.size() || numKeys != (INT32)eulerCurves[2].keyframes.size())
			return;

		auto eulerToQuaternion = [&](INT32 keyIdx, float time, const Quaternion& lastQuat)
		{
			Degree x = (Degree)eulerCurves[0].evaluate(time);
			Degree y = (Degree)eulerCurves[1].evaluate(time);
			Degree z = (Degree)eulerCurves[2].evaluate(time);

			Quaternion quat(x, y, z);

			// Flip quaternion in case rotation is over 180 degrees
			if (keyIdx > 0)
			{
				float dot = quat.dot(lastQuat);
				if (dot < 0.0f)
					quat = Quaternion(-quat.x, -quat.y, -quat.z, -quat.w);
			}

			return quat;
		};

		struct FitKeyframe
		{
			float time;
			Quaternion value;
		};

		Vector<FitKeyframe> fitQuaternions(numKeys * 2);

		Quaternion lastQuat;
		for (INT32 i = 0; i < numKeys; i++)
		{
			float time = eulerCurves[0].keyframes[i].time;
			Quaternion quat = eulerToQuaternion(i, time, lastQuat);

			// Calculate extra values between keys so we can better approximate tangents
			if ((i + 1) < numKeys)
			{
				float nextTime = eulerCurves[0].keyframes[i + 1].time;
				float dt = nextTime - time;

				FitKeyframe& fitStart = fitQuaternions[i * 2 + 0];
				FitKeyframe& fitEnd = fitQuaternions[i * 2 + 1];

				fitStart.time = time + dt * FIT_TIME;
				fitEnd.time = time + dt * (1.0f - FIT_TIME);

				fitStart.value = eulerToQuaternion(i, fitStart.time, quat);
				fitEnd.value = eulerToQuaternion(i, fitEnd.time, fitStart.value);

				lastQuat = fitStart.value;
			}

			// TODO - If animation is looping I should also compare last and first for continuity

			for (INT32 j = 0; j < 4; j++)
			{
				quatCurves[j].keyframes.push_back(FBXKeyFrame());
				FBXKeyFrame& keyFrame = quatCurves[j].keyframes.back();
				keyFrame.time = time;
				keyFrame.value = quat[j];

				keyFrame.inTangent = 0;
				keyFrame.outTangent = 0;
			}
		}

		// Recalculate tangents for quaternion curves

		// TODO - There must be an analytical way to convert euler angle tangents
		//        to quaternion tangents, but I don't want to bother figuring it out
		//        until I have a test-bed for animation.
		if (numKeys > 1)
		{
			// TODO - I could check per-key curve interpolation originally assigned in FBX
			//        and use that to generate linear/constant slopes. Currently I assume 
			//        its all cubic.

			// First key
			{
				const FitKeyframe& fitKeyFrame = fitQuaternions[0];

				for (INT32 j = 0; j < 4; j++)
				{
					FBXKeyFrame& keyFrame = quatCurves[j].keyframes[0];

					float dt = fitKeyFrame.time - keyFrame.time;

					keyFrame.inTangent = (fitKeyFrame.value[j] - keyFrame.value) / dt;
					keyFrame.outTangent = keyFrame.inTangent;
				}
			}

			// In-between keys
			{
				for (INT32 i = 1; i < (numKeys - 1); i++)
				{
					const FitKeyframe& fitPointStart = fitQuaternions[i * 2 - 1];
					const FitKeyframe& fitPointEnd = fitQuaternions[i * 2 + 0];

					for (INT32 j = 0; j < 4; j++)
					{
						FBXKeyFrame& keyFrame = quatCurves[j].keyframes[i];

						float dt0 = fitPointEnd.time - keyFrame.time;
						float dt1 = keyFrame.time - fitPointStart.time;

						float t0 = fitPointEnd.value[j] - keyFrame.value;
						float t1 = keyFrame.value - fitPointStart.value[j];

						keyFrame.inTangent = t0 / (0.5f * dt0) + t1 / (0.5f * dt1);
						keyFrame.outTangent = keyFrame.inTangent;
					}
				}
			}

			// Last key
			{
				const FitKeyframe& fitKeyFrame = fitQuaternions[(numKeys - 2) * 2];

				for (INT32 j = 0; j < 4; j++)
				{
					FBXKeyFrame& keyFrame = quatCurves[j].keyframes[numKeys - 2];

					float dt = keyFrame.time - fitKeyFrame.time;

					keyFrame.inTangent = (keyFrame.value - fitKeyFrame.value[j]) / dt;
					keyFrame.outTangent = keyFrame.inTangent;
				}
			}
		}
	}

	void FBXImporter::importCurve(FbxAnimCurve* fbxCurve, FBXImportOptions& importOptions, FBXAnimationCurve& curve, float start, float end)
	{
		if (fbxCurve == nullptr)
			return;

		INT32 keyCount = fbxCurve->KeyGetCount();
		if (importOptions.animResample)
		{
			float curveStart = std::numeric_limits<float>::infinity();
			float curveEnd = -std::numeric_limits<float>::infinity();

			for (INT32 i = 0; i < keyCount; i++)
			{
				FbxTime fbxTime = fbxCurve->KeyGetTime(i);
				float time = (float)fbxTime.GetSecondDouble();

				curveStart = std::min(time, curveStart);
				curveEnd = std::max(time, curveEnd);
			}

			curveStart = Math::clamp(curveStart, start, end);
			curveEnd = Math::clamp(curveEnd, start, end);

			float curveLength = curveEnd - curveStart;
			INT32 numSamples = Math::ceilToInt(curveLength / importOptions.animSampleRate);

			// We don't use the exact provided sample rate but instead modify it slightly so it
			// completely covers the curve range including start/end points while maintaining
			// constant time step between keyframes.
			float dt = curveLength / (float)numSamples; 

			INT32 lastKeyframe = 0;
			INT32 lastLeftTangent = 0;
			INT32 lastRightTangent = 0;
			for (INT32 i = 0; i < numSamples; i++)
			{
				float sampleTime = std::min(curveStart + i * dt, curveEnd);
				FbxTime fbxSampleTime;
				fbxSampleTime.SetSecondDouble(sampleTime);

				curve.keyframes.push_back(FBXKeyFrame());
				FBXKeyFrame& keyFrame = curve.keyframes.back();
				keyFrame.time = sampleTime;
				keyFrame.value = fbxCurve->Evaluate(fbxSampleTime, &lastKeyframe);
				keyFrame.inTangent = fbxCurve->EvaluateLeftDerivative(fbxSampleTime, &lastLeftTangent);
				keyFrame.outTangent = fbxCurve->EvaluateRightDerivative(fbxSampleTime, &lastRightTangent);
			}
		}
		else
		{
			for (int i = 0; i < keyCount; i++)
			{
				FbxTime fbxTime = fbxCurve->KeyGetTime(i);
				float time = (float)fbxTime.GetSecondDouble();

				if (time < start || time > end)
					continue;

				curve.keyframes.push_back(FBXKeyFrame());
				FBXKeyFrame& keyFrame = curve.keyframes.back();
				keyFrame.time = time;
				keyFrame.value = fbxCurve->KeyGetValue(i);
				keyFrame.inTangent = fbxCurve->KeyGetLeftDerivative(i);
				keyFrame.outTangent = fbxCurve->KeyGetRightDerivative(i);
			}
		}
	}
}