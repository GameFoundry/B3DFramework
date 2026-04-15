//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFBXImporter.h"
#include "Resources/B3DResource.h"
#include "Debug/B3DDebug.h"
#include "FileSystem/B3DDataStream.h"
#include "Mesh/B3DMeshData.h"
#include "Mesh/B3DMesh.h"
#include "Math/B3DVector2.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "B3DFBXUtility.h"
#include "Mesh/B3DMeshUtility.h"
#include "Renderer/B3DRendererMeshData.h"
#include "Importer/B3DMeshImportOptions.h"
#include "Physics/B3DPhysicsMesh.h"
#include "Animation/B3DAnimationCurve.h"
#include "Animation/B3DAnimationClip.h"
#include "Animation/B3DAnimationUtility.h"
#include "Animation/B3DSkeleton.h"
#include "Animation/B3DMorphShapes.h"
#include "Physics/B3DPhysics.h"
#include "FileSystem/B3DFileSystem.h"

using namespace b3d;

Matrix4 FBXToNativeType(const FbxAMatrix& value)
{
	Matrix4 native;
	for(u32 row = 0; row < 4; row++)
		for(u32 col = 0; col < 4; col++)
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
	native.R = (float)value[0];
	native.G = (float)value[1];
	native.B = (float)value[2];
	native.A = (float)value[3];

	return native.GetAsRgba();
}

FbxSurfaceMaterial* FBXToNativeType(FbxSurfaceMaterial* const& value)
{
	return value;
}

int FBXToNativeType(const int& value)
{
	return value;
}

FBXImporter::FBXImporter()
{
	mExtensions.push_back(u8"fbx");
	mExtensions.push_back(u8"obj");
	mExtensions.push_back(u8"dae");
}

bool FBXImporter::IsExtensionSupported(const String& ext) const
{
	String lowerCaseExt = ext;
	StringUtility::ToLowerCase(lowerCaseExt);

	return find(mExtensions.begin(), mExtensions.end(), lowerCaseExt) != mExtensions.end();
}

bool FBXImporter::IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const
{
	return true; // FBX files can be plain-text so I don't even check for magic number
}

SPtr<ImportOptions> FBXImporter::CreateImportOptions() const
{
	return B3DMakeShared<MeshImportOptions>();
}

SPtr<Resource> FBXImporter::Import(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	MeshCreateInformation meshCreateInformation;

	Vector<FBXAnimationClipData> dummy;
	SPtr<RendererMeshData> rendererMeshData = ImportMeshData(filePath, importOptions, meshCreateInformation.SubMeshes, dummy, meshCreateInformation.Skeleton, meshCreateInformation.MorphShapes);

	const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());

	meshCreateInformation.Flags = MeshFlag::Static;
	if(meshImportOptions->CpuCached)
		meshCreateInformation.Flags |= MeshFlag::KeepCPUCopy;

	SPtr<Mesh> mesh = Mesh::CreateShared(rendererMeshData->GetData(), meshCreateInformation);

	const String fileName = filePath.GetFilename(false);
	mesh->SetName(fileName);

	return mesh;
}

Vector<SubResourceRaw> FBXImporter::ImportAll(const Path& filePath, SPtr<const ImportOptions> importOptions)
{
	MeshCreateInformation desc;

	Vector<FBXAnimationClipData> animationClips;
	SPtr<RendererMeshData> rendererMeshData = ImportMeshData(filePath, importOptions, desc.SubMeshes, animationClips, desc.Skeleton, desc.MorphShapes);

	const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());

	desc.Flags = MeshFlag::Static;
	if(meshImportOptions->CpuCached)
		desc.Flags |= MeshFlag::KeepCPUCopy;

	SPtr<Mesh> mesh = Mesh::CreateShared(rendererMeshData->GetData(), desc);

	const String fileName = filePath.GetFilename(false);
	mesh->SetName(fileName);

	Vector<SubResourceRaw> output;
	if(mesh != nullptr)
	{
		output.push_back({ SubResourceRaw::kPrimaryResourceName, mesh });

		CollisionMeshType collisionMeshType = meshImportOptions->CollisionMeshType;
		if(collisionMeshType != CollisionMeshType::None)
		{
			if(Physics::IsStarted())
			{
				PhysicsMeshType type = collisionMeshType == CollisionMeshType::Convex ? PhysicsMeshType::Convex : PhysicsMeshType::Triangle;

				SPtr<PhysicsMesh> physicsMesh = PhysicsMesh::CreateShared(rendererMeshData->GetData(), type);

				output.push_back({ u8"collision", physicsMesh });
			}
			else
			{
				B3D_LOG(Warning, LogFBXImporter, "Cannot generate a collision mesh as the physics module was not started.");
			}
		}

		Vector<ImportedAnimationEvents> events = meshImportOptions->AnimationEvents;
		for(auto& entry : animationClips)
		{
			SPtr<AnimationClip> clip = AnimationClip::CreatePtrInternal(entry.Curves, entry.IsAdditive, entry.SampleRate, entry.RootMotion);
			clip->SetName(entry.Name);

			for(auto& eventsEntry : events)
			{
				if(entry.Name == eventsEntry.Name)
				{
					clip->SetEvents(eventsEntry.Events);
					break;
				}
			}

			output.push_back({ entry.Name, clip });
		}
	}

	return output;
}

SPtr<RendererMeshData> FBXImporter::ImportMeshData(const Path& filePath, SPtr<const ImportOptions> importOptions, Vector<SubMesh>& subMeshes, Vector<FBXAnimationClipData>& animation, SPtr<Skeleton>& skeleton, SPtr<MorphShapes>& morphShapes)
{
	FbxScene* fbxScene = nullptr;

	if(!StartUpSdk(fbxScene))
		return nullptr;

	if(!LoadFbxFile(fbxScene, filePath))
		return nullptr;

	const MeshImportOptions* meshImportOptions = static_cast<const MeshImportOptions*>(importOptions.get());
	FBXImportOptions fbxImportOptions;
	fbxImportOptions.ImportNormals = meshImportOptions->ImportNormals;
	fbxImportOptions.ImportTangents = meshImportOptions->ImportTangents;
	fbxImportOptions.ImportAnimation = meshImportOptions->ImportAnimation;
	fbxImportOptions.ImportBlendShapes = meshImportOptions->ImportBlendShapes;
	fbxImportOptions.ImportSkin = meshImportOptions->ImportSkin;
	fbxImportOptions.ImportScale = meshImportOptions->ImportScale;
	fbxImportOptions.ReduceKeyframes = meshImportOptions->ReduceKeyFrames;

	FBXImportScene importedScene;
	BakeTransforms(fbxScene);
	ParseScene(fbxScene, fbxImportOptions, importedScene);

	if(fbxImportOptions.ImportBlendShapes)
		ImportBlendShapes(importedScene, fbxImportOptions);

	if(fbxImportOptions.ImportSkin)
		ImportSkin(importedScene, fbxImportOptions);

	if(fbxImportOptions.ImportAnimation)
		ImportAnimations(fbxScene, fbxImportOptions, importedScene);

	SplitMeshVertices(importedScene);
	GenerateMissingTangentSpace(importedScene, fbxImportOptions);

	SPtr<RendererMeshData> rendererMeshData = GenerateMeshData(importedScene, fbxImportOptions, subMeshes);

	skeleton = CreateSkeleton(importedScene, subMeshes.size() > 1);
	morphShapes = CreateMorphShapes(importedScene);

	// Import animation clips
	if(!importedScene.Clips.empty())
	{
		const Vector<AnimationSplitInfo>& splits = meshImportOptions->AnimationSplits;
		ConvertAnimations(importedScene.Clips, splits, skeleton, meshImportOptions->ImportRootMotion, animation);
	}

	// TODO - Later: Optimize mesh: Remove bad and degenerate polygons, weld nearby vertices, optimize for vertex cache

	ShutDownSdk();

	return rendererMeshData;
}

SPtr<Skeleton> FBXImporter::CreateSkeleton(const FBXImportScene& scene, bool sharedRoot)
{
	Vector<BoneInformation> allBones;
	UnorderedMap<FBXImportNode*, u32> boneMap;

	for(auto& mesh : scene.Meshes)
	{
		// Create bones
		for(auto& fbxBone : mesh->Bones)
		{
			u32 boneIdx = (u32)allBones.size();

			auto iterFind = boneMap.find(fbxBone.Node);
			if(iterFind != boneMap.end())
				continue; // Duplicate

			boneMap[fbxBone.Node] = boneIdx;

			allBones.push_back(BoneInformation());
			BoneInformation& bone = allBones.back();

			bone.Name = fbxBone.Node->Name;
			bone.LocalTfrm = fbxBone.LocalTfrm;
			bone.InvBindPose = fbxBone.BindPose;
		}
	}

	// Generate skeleton
	if(allBones.size() > 0)
	{
		// Find bone parents
		u32 numProcessedBones = 0;

		// Generate common root bone for all meshes
		u32 rootBoneIdx = (u32)-1;
		if(sharedRoot)
		{
			rootBoneIdx = (u32)allBones.size();

			allBones.push_back(BoneInformation());
			BoneInformation& bone = allBones.back();

			bone.Name = "MultiMeshRoot";
			bone.LocalTfrm = Transform();
			bone.InvBindPose = Matrix4::kIdentity;
			bone.Parent = (u32)-1;

			numProcessedBones++;
		}

		Stack<std::pair<FBXImportNode*, u32>> todo;
		todo.push({ scene.RootNode, rootBoneIdx });

		while(!todo.empty())
		{
			auto entry = todo.top();
			todo.pop();

			FBXImportNode* node = entry.first;
			u32 parentBoneIdx = entry.second;

			auto boneIter = boneMap.find(node);
			if(boneIter != boneMap.end())
			{
				u32 boneIdx = boneIter->second;
				allBones[boneIdx].Parent = parentBoneIdx;

				parentBoneIdx = boneIdx;
				numProcessedBones++;
			}
			else
			{
				// Node is not a bone, but it still needs to be part of the hierarchy. It wont be animated, nor will
				// it directly influence any vertices, but its transform must be applied to any child bones.
				u32 boneIdx = (u32)allBones.size();

				allBones.push_back(BoneInformation());
				BoneInformation& bone = allBones.back();

				bone.Name = node->Name;
				bone.LocalTfrm = node->LocalTransform;
				bone.InvBindPose = Matrix4::kIdentity;
				bone.Parent = parentBoneIdx;

				parentBoneIdx = boneIdx;
				numProcessedBones++;
			}

			for(auto& child : node->Children)
				todo.push({ child, parentBoneIdx });
		}

		u32 numAllBones = (u32)allBones.size();
		if(numProcessedBones == numAllBones)
			return Skeleton::Create(allBones.data(), numAllBones);

		B3D_LOG(Error, LogFBXImporter, "Not all bones were found in the node hierarchy. Skeleton invalid.");
	}

	return nullptr;
}

SPtr<MorphShapes> FBXImporter::CreateMorphShapes(const FBXImportScene& scene)
{
	// Combine morph shapes from all sub-meshes, and transform them
	struct RawMorphShape
	{
		String Name;
		float Weight;
		Vector<MorphVertex> Vertices;
	};

	UnorderedMap<String, UnorderedMap<String, RawMorphShape>> allRawMorphShapes;
	u32 totalNumVertices = 0;

	// Note: Order in which we combine meshes must match the order in MeshData::combine
	for(auto& mesh : scene.Meshes)
	{
		u32 numVertices = (u32)mesh->Positions.size();
		u32 numNormals = (u32)mesh->Normals.size();
		bool hasNormals = numVertices == numNormals;

		for(auto& node : mesh->ReferencedBy)
		{
			Matrix4 worldTransform = node->WorldTransform * node->GeomTransform;
			Matrix4 worldTransformIT = worldTransform.Inverse();
			worldTransformIT = worldTransformIT.Transpose();

			// Copy & transform positions
			for(auto& blendShape : mesh->BlendShapes)
			{
				UnorderedMap<String, RawMorphShape>& channelShapes = allRawMorphShapes[blendShape.Name];

				for(auto& blendFrame : blendShape.Frames)
				{
					RawMorphShape& shape = channelShapes[blendFrame.Name];
					shape.Name = blendFrame.Name;
					shape.Weight = blendFrame.Weight;

					u32 frameNumVertices = (u32)blendFrame.Positions.size();
					if(frameNumVertices == numVertices)
					{
						for(u32 i = 0; i < numVertices; i++)
						{
							Vector3 meshPosition = worldTransform.MultiplyAffine(mesh->Positions[i]);
							Vector3 blendPosition = worldTransform.MultiplyAffine(blendFrame.Positions[i]);

							Vector3 positionDelta = blendPosition - meshPosition;
							Vector3 normalDelta;
							if(hasNormals)
							{
								Vector3 blendNormal = worldTransformIT.MultiplyDirection(blendFrame.Normals[i]);
								blendNormal = Vector3::Normalize(blendNormal);

								Vector3 meshNormal = worldTransformIT.MultiplyDirection(mesh->Normals[i]);
								meshNormal = Vector3::Normalize(meshNormal);

								normalDelta = blendNormal - meshNormal;
							}
							else
								normalDelta = Vector3::kZero;

							if(positionDelta.SquaredLength() > 0.000001f || normalDelta.SquaredLength() > 0.0001f)
								shape.Vertices.push_back(MorphVertex(positionDelta, normalDelta, totalNumVertices + i));
						}
					}
					else
					{
						B3D_LOG(Error, LogFBXImporter, "Corrupt blend shape frame. Number of vertices doesn't match the number of mesh vertices.");
					}
				}
			}

			totalNumVertices += numVertices;
		}
	}

	// Create morph shape object from combined shape data
	SPtr<MorphShapes> morphShapes;
	Vector<SPtr<MorphChannel>> allChannels;
	for(auto& channel : allRawMorphShapes)
	{
		Vector<SPtr<MorphShape>> channelShapes;
		for(auto& entry : channel.second)
		{
			RawMorphShape& shape = entry.second;
			shape.Vertices.shrink_to_fit();

			SPtr<MorphShape> morphShape = MorphShape::Create(shape.Name, shape.Weight, shape.Vertices);
			channelShapes.push_back(morphShape);
		}

		if(channelShapes.size() > 0)
		{
			SPtr<MorphChannel> morphChannel = MorphChannel::Create(channel.first, channelShapes);
			allChannels.push_back(morphChannel);
		}
	}

	if(!allChannels.empty())
		return MorphShapes::Create(allChannels, totalNumVertices);

	return morphShapes;
}

bool FBXImporter::StartUpSdk(FbxScene*& scene)
{
	mFBXManager = FbxManager::Create();
	if(mFBXManager == nullptr)
	{
		B3D_LOG(Error, LogFBXImporter, "FBX import failed: FBX SDK failed to initialize. FbxManager::Create() failed.");
		return false;
	}

	FbxIOSettings* ios = FbxIOSettings::Create(mFBXManager, IOSROOT);
	mFBXManager->SetIOSettings(ios);

	scene = FbxScene::Create(mFBXManager, "Import Scene");
	if(scene == nullptr)
	{
		B3D_LOG(Warning, LogFBXImporter, "FBX import failed: Failed to create FBX scene.");
		return false;
	}

	return true;
}

void FBXImporter::ShutDownSdk()
{
	mFBXManager->Destroy();
	mFBXManager = nullptr;
}

bool FBXImporter::LoadFbxFile(FbxScene* scene, const Path& filePath)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor, lSDKMinor, lSDKRevision;
	FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	Lock fileLock = FileScheduler::GetLock(filePath);
	FbxImporter* importer = FbxImporter::Create(mFBXManager, "");
	bool importStatus = importer->Initialize(filePath.ToString().c_str(), -1, mFBXManager->GetIOSettings());

	importer->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if(!importStatus)
	{
		B3D_LOG(Error, LogFBXImporter, "FBX import failed: Call to FbxImporter::Initialize() failed.\n"
								   "Error returned: %s\n\n{0}",
			   importer->GetStatus().GetErrorString());
		return false;
	}

	mFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, false);
	mFBXManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, false);

	importStatus = importer->Import(scene);

	if(!importStatus)
	{
		importer->Destroy();

		B3D_LOG(Error, LogFBXImporter, "FBX import failed: Call to FbxImporter::Import() failed.\n"
								   "Error returned: %s\n\n{0}",
			   importer->GetStatus().GetErrorString());
		return false;
	}

	FbxAxisSystem fileCoordSystem = scene->GetGlobalSettings().GetAxisSystem();
	FbxAxisSystem bsCoordSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
	if(fileCoordSystem != bsCoordSystem)
		bsCoordSystem.ConvertScene(scene);

	importer->Destroy();
	return true;
}

void FBXImporter::ParseScene(FbxScene* scene, const FBXImportOptions& options, FBXImportScene& outputScene)
{
	// Scale from file units to engine units, and apply optional user scale
	float importScale = 1.0f;
	if(options.ImportScale > 0.0001f)
		importScale = options.ImportScale;

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

	outputScene.RootNode = CreateImportNode(outputScene, scene->GetRootNode(), nullptr);

	Stack<FbxNode*> todo;
	todo.push(scene->GetRootNode());

	while(!todo.empty())
	{
		FbxNode* curNode = todo.top();
		FBXImportNode* curImportNode = outputScene.NodeMap[curNode];
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

					if(attrib->GetAttributeType() == FbxNodeAttribute::eMesh)
					{
						FbxMesh* mesh = static_cast<FbxMesh*>(attrib);
						mesh->RemoveBadPolygons();

						ParseMesh(mesh, curImportNode, options, outputScene);
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

					ParseMesh(mesh, curImportNode, options, outputScene);
				}
				break;
			default:
				break;
			}
		}

		for(int i = 0; i < curNode->GetChildCount(); i++)
		{
			FbxNode* childNode = curNode->GetChild(i);
			CreateImportNode(outputScene, childNode, curImportNode);

			todo.push(childNode);
		}
	}
}

FBXImportNode* FBXImporter::CreateImportNode(FBXImportScene& scene, FbxNode* fbxNode, FBXImportNode* parent)
{
	FBXImportNode* node = B3DNew<FBXImportNode>();

	Vector3 translation = FBXToNativeType(fbxNode->EvaluateLocalTranslation(FbxTime(0)));
	Vector3 rotationEuler = FBXToNativeType(fbxNode->EvaluateLocalRotation(FbxTime(0)));
	Vector3 scale = FBXToNativeType(fbxNode->EvaluateLocalScaling(FbxTime(0)));

	Quaternion rotation((Degree)rotationEuler.X, (Degree)rotationEuler.Y, (Degree)rotationEuler.Z, EulerAngleOrder::XYZ);

	node->Name = fbxNode->GetNameWithoutNameSpacePrefix().Buffer();
	node->FbxNode = fbxNode;
	node->LocalTransform = Transform(translation, rotation, scale);

	if(parent != nullptr)
	{
		node->WorldTransform = parent->WorldTransform * node->LocalTransform.GetMatrix();

		parent->Children.push_back(node);
	}
	else
		node->WorldTransform = node->LocalTransform.GetMatrix();

	// Geometry transform is applied to geometry (mesh data) only, it is not inherited by children, so we store it
	// separately
	Vector3 geomTrans = FBXToNativeType(fbxNode->GeometricTranslation.Get());
	Vector3 geomRotEuler = FBXToNativeType(fbxNode->GeometricRotation.Get());
	Vector3 geomScale = FBXToNativeType(fbxNode->GeometricScaling.Get());

	Quaternion geomRotation((Degree)geomRotEuler.X, (Degree)geomRotEuler.Y, (Degree)geomRotEuler.Z, EulerAngleOrder::XYZ);
	node->GeomTransform = Matrix4::TRS(geomTrans, geomRotation, geomScale);

	scene.NodeMap.insert(std::make_pair(fbxNode, node));

	// Determine if geometry winding needs to be flipped to match the engine convention. This is true by default, but
	// each negative scaling factor changes the winding.
	if(parent != nullptr)
		node->FlipWinding = parent->FlipWinding;
	else
		node->FlipWinding = true;

	for(u32 i = 0; i < 3; i++)
	{
		if(scale[i] < 0.0f) node->FlipWinding = !node->FlipWinding;
		if(geomScale[i] < 0.0f) node->FlipWinding = !node->FlipWinding;
	}

	return node;
}

void FBXImporter::SplitMeshVertices(FBXImportScene& scene)
{
	Vector<FBXImportMesh*> splitMeshes;

	for(auto& mesh : scene.Meshes)
	{
		FBXImportMesh* splitMesh = B3DNew<FBXImportMesh>();
		splitMesh->FbxMesh = mesh->FbxMesh;
		splitMesh->ReferencedBy = mesh->ReferencedBy;
		splitMesh->Bones = mesh->Bones;

		FBXUtility::SplitVertices(*mesh, *splitMesh);
		splitMeshes.push_back(splitMesh);

		B3DDelete(mesh);
	}

	scene.Meshes = splitMeshes;
}

void FBXImporter::ConvertAnimations(const Vector<FBXAnimationClip>& clips, const Vector<AnimationSplitInfo>& splits, const SPtr<Skeleton>& skeleton, bool importRootMotion, Vector<FBXAnimationClipData>& output)
{
	UnorderedSet<String> names;

	String rootBoneName;
	if(skeleton == nullptr)
		importRootMotion = false;
	else
	{
		u32 rootBoneIdx = skeleton->GetRootBoneIndex();
		if(rootBoneIdx == (u32)-1)
			importRootMotion = false;
		else
			rootBoneName = skeleton->GetBoneInfo(rootBoneIdx).Name;
	}

	bool isFirstClip = true;
	for(auto& clip : clips)
	{
		SPtr<AnimationCurves> curves = B3DMakeShared<AnimationCurves>();
		SPtr<RootMotion> rootMotion;

		// Find offset so animations start at time 0
		float animStart = std::numeric_limits<float>::infinity();

		for(auto& bone : clip.BoneAnimations)
		{
			if(bone.Translation.GetNumKeyFrames() > 0)
				animStart = std::min(bone.Translation.GetKeyFrame(0).Time, animStart);

			if(bone.Rotation.GetNumKeyFrames() > 0)
				animStart = std::min(bone.Rotation.GetKeyFrame(0).Time, animStart);

			if(bone.Scale.GetNumKeyFrames() > 0)
				animStart = std::min(bone.Scale.GetKeyFrame(0).Time, animStart);
		}

		for(auto& anim : clip.BlendShapeAnimations)
		{
			if(anim.Curve.GetNumKeyFrames() > 0)
				animStart = std::min(anim.Curve.GetKeyFrame(0).Time, animStart);
		}

		AnimationCurveFlags blendShapeFlags = AnimationCurveFlag::ImportedCurve | AnimationCurveFlag::MorphFrame;
		if(animStart != 0.0f && animStart != std::numeric_limits<float>::infinity())
		{
			for(auto& bone : clip.BoneAnimations)
			{
				TAnimationCurve<Vector3> translation = AnimationUtility::OffsetCurve(bone.Translation, -animStart);
				TAnimationCurve<Quaternion> rotation = AnimationUtility::OffsetCurve(bone.Rotation, -animStart);
				TAnimationCurve<Vector3> scale = AnimationUtility::OffsetCurve(bone.Scale, -animStart);

				if(importRootMotion && bone.Node->Name == rootBoneName)
					rootMotion = B3DMakeShared<RootMotion>(translation, rotation);
				else
				{
					curves->Position.push_back({ bone.Node->Name, AnimationCurveFlag::ImportedCurve, translation });
					curves->Rotation.push_back({ bone.Node->Name, AnimationCurveFlag::ImportedCurve, rotation });
					curves->Scale.push_back({ bone.Node->Name, AnimationCurveFlag::ImportedCurve, scale });
				}
			}

			for(auto& anim : clip.BlendShapeAnimations)
			{
				TAnimationCurve<float> curve = AnimationUtility::OffsetCurve(anim.Curve, -animStart);
				curves->Generic.push_back({ anim.BlendShape, blendShapeFlags, curve });
			}
		}
		else
		{
			for(auto& bone : clip.BoneAnimations)
			{
				if(importRootMotion && bone.Node->Name == rootBoneName)
					rootMotion = B3DMakeShared<RootMotion>(bone.Translation, bone.Rotation);
				else
				{
					curves->Position.push_back({ bone.Node->Name, AnimationCurveFlag::ImportedCurve, bone.Translation });
					curves->Rotation.push_back({ bone.Node->Name, AnimationCurveFlag::ImportedCurve, bone.Rotation });
					curves->Scale.push_back({ bone.Node->Name, AnimationCurveFlag::ImportedCurve, bone.Scale });
				}
			}

			for(auto& anim : clip.BlendShapeAnimations)
				curves->Generic.push_back({ anim.BlendShape, blendShapeFlags, anim.Curve });
		}

		// See if any splits are required. We only split the first clip as it is assumed if FBX has multiple clips the
		// user has the ability to split them externally.
		if(isFirstClip && !splits.empty())
		{
			float secondsPerFrame = 1.0f / clip.SampleRate;

			for(auto& split : splits)
			{
				SPtr<AnimationCurves> splitClipCurve = B3DMakeShared<AnimationCurves>();
				SPtr<RootMotion> splitRootMotion;

				auto splitCurves = [&](auto& inCurves, auto& outCurves)
				{
					u32 numCurves = (u32)inCurves.size();
					outCurves.resize(numCurves);

					for(u32 i = 0; i < numCurves; i++)
					{
						auto& animCurve = inCurves[i].Curve;
						outCurves[i].Name = inCurves[i].Name;

						u32 numFrames = animCurve.GetNumKeyFrames();
						if(numFrames == 0)
							continue;

						float startTime = split.StartFrame * secondsPerFrame;
						float endTime = split.EndFrame * secondsPerFrame;

						outCurves[i].Curve = inCurves[i].Curve.Split(startTime, endTime);

						if(split.IsAdditive)
							outCurves[i].Curve.MakeAdditive();
					}
				};

				splitCurves(curves->Position, splitClipCurve->Position);
				splitCurves(curves->Rotation, splitClipCurve->Rotation);
				splitCurves(curves->Scale, splitClipCurve->Scale);
				splitCurves(curves->Generic, splitClipCurve->Generic);

				if(rootMotion != nullptr)
				{
					auto splitCurve = [&](auto& inCurve, auto& outCurve)
					{
						u32 numFrames = inCurve.GetNumKeyFrames();
						if(numFrames > 0)
						{
							float startTime = split.StartFrame * secondsPerFrame;
							float endTime = split.EndFrame * secondsPerFrame;

							outCurve = inCurve.Split(startTime, endTime);

							if(split.IsAdditive)
								outCurve.MakeAdditive();
						}
					};

					splitRootMotion = B3DMakeShared<RootMotion>();
					splitCurve(rootMotion->Position, splitRootMotion->Position);
					splitCurve(rootMotion->Rotation, splitRootMotion->Rotation);
				}

				// Search for a unique name
				String name = split.Name;
				u32 attemptIdx = 0;
				while(names.find(name) != names.end())
				{
					name = clip.Name + "_" + ToString(attemptIdx);
					attemptIdx++;
				}

				names.insert(name);
				output.push_back(FBXAnimationClipData(name, split.IsAdditive, clip.SampleRate, splitClipCurve, splitRootMotion));
			}
		}
		else
		{
			// Search for a unique name
			String name = clip.Name;
			u32 attemptIdx = 0;
			while(names.find(name) != names.end())
			{
				name = clip.Name + "_" + ToString(attemptIdx);
				attemptIdx++;
			}

			names.insert(name);
			output.push_back(FBXAnimationClipData(name, false, clip.SampleRate, curves, rootMotion));
		}

		isFirstClip = false;
	}
}

SPtr<RendererMeshData> FBXImporter::GenerateMeshData(const FBXImportScene& scene, const FBXImportOptions& options, Vector<SubMesh>& outputSubMeshes)
{
	Vector<SPtr<MeshData>> allMeshData;
	Vector<Vector<SubMesh>> allSubMeshes;
	u32 boneIndexOffset = 0;

	// Generate unique indices for all the bones. This is mirrored in createSkeleton().
	UnorderedMap<FBXImportNode*, u32> boneMap;
	for(auto& mesh : scene.Meshes)
	{
		// Create bones
		for(auto& fbxBone : mesh->Bones)
		{
			u32 boneIdx = (u32)boneMap.size();

			auto iterFind = boneMap.find(fbxBone.Node);
			if(iterFind != boneMap.end())
				continue; // Duplicate

			boneMap[fbxBone.Node] = boneIdx;
		}
	}

	for(auto& mesh : scene.Meshes)
	{
		Vector<Vector<u32>> indicesPerMaterial;
		for(u32 i = 0; i < (u32)mesh->Indices.size(); i++)
		{
			u32 materialIdx = 0;
			if(i < (u32)mesh->Materials.size())
				materialIdx = (u32)mesh->Materials[i];

			while(materialIdx >= (u32)indicesPerMaterial.size())
				indicesPerMaterial.push_back(Vector<u32>());

			indicesPerMaterial[materialIdx].push_back(mesh->Indices[i]);
		}

		u32* orderedIndices = (u32*)B3DAllocate((u32)mesh->Indices.size() * sizeof(u32));
		Vector<SubMesh> subMeshes;
		u32 currentIndex = 0;

		for(auto& subMeshIndices : indicesPerMaterial)
		{
			u32 indexCount = (u32)subMeshIndices.size();
			u32* dest = orderedIndices + currentIndex;
			memcpy(dest, subMeshIndices.data(), indexCount * sizeof(u32));

			subMeshes.push_back(SubMesh(currentIndex, indexCount, DOT_TRIANGLE_LIST));

			currentIndex += indexCount;
		}

		u32 vertexLayout = (u32)VertexLayout::Position;

		size_t numVertices = mesh->Positions.size();
		bool hasColors = mesh->Colors.size() == numVertices;
		bool hasNormals = mesh->Normals.size() == numVertices;
		bool hasBoneInfluences = mesh->BoneInfluences.size() == numVertices;

		if(hasColors)
			vertexLayout |= (u32)VertexLayout::Color;

		bool hasTangents = false;
		if(hasNormals)
		{
			vertexLayout |= (u32)VertexLayout::Normal;

			if(mesh->Tangents.size() == numVertices &&
			   mesh->Bitangents.size() == numVertices)
			{
				vertexLayout |= (u32)VertexLayout::Tangent;
				hasTangents = true;
			}
		}

		if(hasBoneInfluences)
			vertexLayout |= (u32)VertexLayout::BoneWeights;

		for(u32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
		{
			if(mesh->UV[i].size() == numVertices)
			{
				if(i == 0)
					vertexLayout |= (u32)VertexLayout::UV0;
				else if(i == 1)
					vertexLayout |= (u32)VertexLayout::UV1;
			}
		}

		u32 numIndices = (u32)mesh->Indices.size();
		for(auto& node : mesh->ReferencedBy)
		{
			Matrix4 worldTransform = node->WorldTransform * node->GeomTransform;
			Matrix4 worldTransformIT = worldTransform.Inverse();
			worldTransformIT = worldTransformIT.Transpose();

			SPtr<RendererMeshData> meshData = RendererMeshData::Create((u32)numVertices, numIndices, (VertexLayout)vertexLayout);

			// Copy indices
			if(!node->FlipWinding)
				meshData->SetIndices(orderedIndices, numIndices * sizeof(u32));
			else
			{
				u32* flippedIndices = B3DStackAllocate<u32>(numIndices);

				for(u32 i = 0; i < numIndices; i += 3)
				{
					flippedIndices[i + 0] = orderedIndices[i + 0];
					flippedIndices[i + 1] = orderedIndices[i + 2];
					flippedIndices[i + 2] = orderedIndices[i + 1];
				}

				meshData->SetIndices(flippedIndices, numIndices * sizeof(u32));
				B3DStackFree(flippedIndices);
			}

			// Copy & transform positions
			u32 positionsSize = sizeof(Vector3) * (u32)numVertices;
			Vector3* transformedPositions = (Vector3*)B3DStackAllocate(positionsSize);

			for(u32 i = 0; i < (u32)numVertices; i++)
				transformedPositions[i] = worldTransform.MultiplyAffine((Vector3)mesh->Positions[i]);

			meshData->SetPositions(transformedPositions, positionsSize);
			B3DStackFree(transformedPositions);

			// Copy & transform normals
			if(hasNormals)
			{
				u32 normalsSize = sizeof(Vector3) * (u32)numVertices;
				Vector3* transformedNormals = (Vector3*)B3DStackAllocate(normalsSize);

				// Copy, convert & transform tangents & bitangents
				if(hasTangents)
				{
					u32 tangentsSize = sizeof(Vector4) * (u32)numVertices;
					Vector4* transformedTangents = (Vector4*)B3DStackAllocate(tangentsSize);

					for(u32 i = 0; i < (u32)numVertices; i++)
					{
						Vector3 normal = (Vector3)mesh->Normals[i];
						normal = worldTransformIT.MultiplyDirection(normal);
						transformedNormals[i] = Vector3::Normalize(normal);

						Vector3 tangent = (Vector3)mesh->Tangents[i];
						tangent = Vector3::Normalize(worldTransformIT.MultiplyDirection(tangent));

						Vector3 bitangent = (Vector3)mesh->Bitangents[i];
						bitangent = worldTransformIT.MultiplyDirection(bitangent);

						Vector3 engineBitangent = Vector3::Cross(normal, tangent);
						float sign = Vector3::Dot(engineBitangent, bitangent);

						transformedTangents[i] = Vector4(tangent.X, tangent.Y, tangent.Z, sign > 0 ? 1.0f : -1.0f);
					}

					meshData->SetTangents(transformedTangents, tangentsSize);
					B3DStackFree(transformedTangents);
				}
				else // Just normals
				{
					for(u32 i = 0; i < (u32)numVertices; i++)
						transformedNormals[i] = Vector3::Normalize(worldTransformIT.MultiplyDirection((Vector3)mesh->Normals[i]));
				}

				meshData->SetNormals(transformedNormals, normalsSize);
				B3DStackFree(transformedNormals);
			}

			// Copy colors
			if(hasColors)
			{
				meshData->SetColors(mesh->Colors.data(), sizeof(u32) * (u32)numVertices);
			}

			// Copy UV
			int writeUVIDx = 0;
			for(auto& uvLayer : mesh->UV)
			{
				if(uvLayer.size() == numVertices)
				{
					u32 size = sizeof(Vector2) * (u32)numVertices;
					Vector2* transformedUV = (Vector2*)B3DStackAllocate(size);

					u32 i = 0;
					for(auto& uv : uvLayer)
					{
						transformedUV[i] = uv;
						transformedUV[i].Y = 1.0f - uv.Y;

						i++;
					}

					if(writeUVIDx == 0)
						meshData->SetUV0(transformedUV, size);
					else if(writeUVIDx == 1)
						meshData->SetUV1(transformedUV, size);

					B3DStackFree(transformedUV);

					writeUVIDx++;
				}
			}

			// Copy bone influences & remap bone indices
			if(hasBoneInfluences)
			{
				u32 bufferSize = sizeof(BoneWeight) * (u32)numVertices;
				BoneWeight* weights = (BoneWeight*)B3DStackAllocate(bufferSize);
				for(u32 i = 0; i < (u32)numVertices; i++)
				{
					int* indices[] = { &weights[i].Index0, &weights[i].Index1, &weights[i].Index2, &weights[i].Index3 };
					float* amounts[] = { &weights[i].Weight0, &weights[i].Weight1, &weights[i].Weight2, &weights[i].Weight3 };

					for(u32 j = 0; j < 4; j++)
					{
						int boneIdx = mesh->BoneInfluences[i].Indices[j];
						if(boneIdx != -1)
						{
							FBXImportNode* boneNode = mesh->Bones[boneIdx].Node;

							auto iterFind = boneMap.find(boneNode);
							if(iterFind != boneMap.end())
								*indices[j] = iterFind->second;
							else
								*indices[j] = -1;
						}
						else
							*indices[j] = boneIdx;

						*amounts[j] = mesh->BoneInfluences[i].Weights[j];
					}
				}

				meshData->SetBoneWeights(weights, bufferSize);
				B3DStackFree(weights);
			}

			allMeshData.push_back(meshData->GetData());
			allSubMeshes.push_back(subMeshes);
		}

		B3DFree(orderedIndices);

		u32 numBones = (u32)mesh->Bones.size();
		boneIndexOffset += numBones;
	}

	if(allMeshData.size() > 1)
	{
		return RendererMeshData::Create(MeshData::Combine(allMeshData, allSubMeshes, outputSubMeshes));
	}
	else if(allMeshData.size() == 1)
	{
		outputSubMeshes = allSubMeshes[0];
		return RendererMeshData::Create(allMeshData[0]);
	}

	return nullptr;
}

template <class TFBX, class TNative>
class FBXDirectIndexer
{
public:
	FBXDirectIndexer(const FbxLayerElementTemplate<TFBX>& layer)
		: mElementArray(layer.GetDirectArray()), mElementCount(mElementArray.GetCount())
	{}

	bool Get(int index, TNative& output) const
	{
		if(index < 0 || index >= mElementCount)
			return false;

		output = FBXToNativeType(mElementArray.GetAt(index));
		return true;
	}

	bool IsEmpty() const
	{
		return mElementCount == 0;
	}

private:
	const FbxLayerElementArrayTemplate<TFBX>& mElementArray;
	int mElementCount;
};

template <class TFBX, class TNative>
class FBXIndexIndexer
{
public:
	FBXIndexIndexer(const FbxLayerElementTemplate<TFBX>& layer)
		: mElementArray(layer.GetDirectArray()), mIndexArray(layer.GetIndexArray()), mElementCount(mElementArray.GetCount()), mIndexCount(mIndexArray.GetCount())
	{}

	bool Get(int index, TNative& output) const
	{
		if(index < 0 || index >= mIndexCount)
			return false;

		int actualIndex = mIndexArray.GetAt(index);

		if(actualIndex < 0 || actualIndex >= mElementCount)
			return false;

		output = FBXToNativeType(mElementArray.GetAt(actualIndex));
		return true;
	}

	bool IsEmpty() const
	{
		return mElementCount == 0 || mIndexCount == 0;
	}

private:
	const FbxLayerElementArrayTemplate<TFBX>& mElementArray;
	const FbxLayerElementArrayTemplate<int>& mIndexArray;
	int mElementCount;
	int mIndexCount;
};

template <class TFBX, class TNative, class TIndexer>
void ReadLayerData(FbxLayerElementTemplate<TFBX>& layer, Vector<TNative>& output, const Vector<int>& indices)
{
	TIndexer indexer(layer);
	if(indexer.IsEmpty())
		return;

	output.resize(indices.size());

	FbxLayerElement::EMappingMode mappingMode = layer.GetMappingMode();

	u32 indexCount = (u32)indices.size();
	switch(mappingMode)
	{
	case FbxLayerElement::eByControlPoint:
		for(u32 i = 0; i < indexCount; i++)
		{
			int index = indices[i];
			indexer.Get(index, output[i]);
		}
		break;
	case FbxLayerElement::eByPolygonVertex:
		for(u32 i = 0; i < indexCount; i++)
			indexer.Get(i, output[i]);
		break;
	case FbxLayerElement::eByPolygon:
		// We expect mesh to be triangulated here
		{
			u32 polygonCount = indexCount / 3;
			u32 index = 0;

			for(u32 i = 0; i < polygonCount; i++)
			{
				TNative value{};
				indexer.Get(i, value);

				output[index++] = value;
				output[index++] = value;
				output[index++] = value;
			}
		}
		break;
	case FbxLayerElement::eAllSame:
		{
			TNative value{};
			indexer.Get(0, value);

			for(u32 i = 0; i < indexCount; i++)
				output[i] = value;
		}
		break;
	default:
		B3D_LOG(Warning, LogFBXImporter, "FBX Import: Unsupported layer mapping mode.");
		break;
	}
}

template <class TFBX, class TNative>
void ReadLayerData(FbxLayerElementTemplate<TFBX>& layer, Vector<TNative>& output, const Vector<int>& indices)
{
	FbxLayerElement::EReferenceMode refMode = layer.GetReferenceMode();

	if(refMode == FbxLayerElement::eDirect)
		ReadLayerData<TFBX, TNative, FBXDirectIndexer<TFBX, TNative>>(layer, output, indices);
	else if(refMode == FbxLayerElement::eIndexToDirect)
		ReadLayerData<TFBX, TNative, FBXIndexIndexer<TFBX, TNative>>(layer, output, indices);
	else
		B3D_LOG(Warning, LogFBXImporter, "FBX Import: Unsupported layer reference mode.");
}

void FBXImporter::ParseMesh(FbxMesh* mesh, FBXImportNode* parentNode, const FBXImportOptions& options, FBXImportScene& outputScene)
{
	// Check if valid
	if(!mesh->IsTriangleMesh())
		return;

	u32 vertexCount = mesh->GetControlPointsCount();
	u32 triangleCount = mesh->GetPolygonCount();

	if(vertexCount == 0 || triangleCount == 0)
		return;

	// Register in global mesh array
	FBXImportMesh* importMesh = nullptr;

	auto iterFindMesh = outputScene.MeshMap.find(mesh);
	if(iterFindMesh != outputScene.MeshMap.end())
	{
		u32 meshIdx = iterFindMesh->second;
		outputScene.Meshes[meshIdx]->ReferencedBy.push_back(parentNode);

		return;
	}
	else
	{
		importMesh = B3DNew<FBXImportMesh>();
		outputScene.Meshes.push_back(importMesh);

		importMesh->ReferencedBy.push_back(parentNode);
		importMesh->FbxMesh = mesh;

		outputScene.MeshMap[mesh] = (u32)outputScene.Meshes.size() - 1;
	}

	// Import vertices
	importMesh->Positions.resize(vertexCount);
	FbxVector4* controlPoints = mesh->GetControlPoints();

	for(u32 i = 0; i < vertexCount; i++)
		importMesh->Positions[i] = FBXToNativeType(controlPoints[i]);

	// Import triangles
	u32 indexCount = triangleCount * 3;
	importMesh->Indices.resize(indexCount);

	int* fbxIndices = mesh->GetPolygonVertices();
	importMesh->Indices.assign(fbxIndices, fbxIndices + indexCount);

	// Import UVs
	Vector<FbxLayerElementUV*> fbxUVLayers;

	//// Search the diffuse layers first
	for(u32 i = 0; i < FBX_IMPORT_MAX_UV_LAYERS; i++)
	{
		FbxLayer* layer = mesh->GetLayer(i, FbxLayerElement::eUV);
		if(layer == nullptr)
			continue;

		for(int j = FbxLayerElement::eTextureDiffuse; j < FbxLayerElement::eTypeCount; j++)
		{
			FbxLayerElementUV* uvLayer = layer->GetUVs((FbxLayerElement::EType)j);
			if(uvLayer == nullptr)
				continue;

			fbxUVLayers.push_back(uvLayer);

			if(fbxUVLayers.size() == FBX_IMPORT_MAX_UV_LAYERS)
				break;
		}

		if(fbxUVLayers.size() == FBX_IMPORT_MAX_UV_LAYERS)
			break;
	}

	//// If there's room, search all others too
	if(fbxUVLayers.size() < FBX_IMPORT_MAX_UV_LAYERS)
	{
		u32 numLayers = mesh->GetLayerCount();
		for(u32 i = 0; i < numLayers; i++)
		{
			FbxLayer* layer = mesh->GetLayer(i);
			if(layer == nullptr)
				continue;

			for(int j = FbxLayerElement::eTextureDiffuse; j < FbxLayerElement::eTypeCount; j++)
			{
				FbxLayerElementUV* uvLayer = layer->GetUVs((FbxLayerElement::EType)j);
				if(uvLayer == nullptr)
					continue;

				auto iterFind = std::find(fbxUVLayers.begin(), fbxUVLayers.end(), uvLayer);
				if(iterFind != fbxUVLayers.end())
					continue;

				fbxUVLayers.push_back(uvLayer);

				if(fbxUVLayers.size() == FBX_IMPORT_MAX_UV_LAYERS)
					break;
			}

			if(fbxUVLayers.size() == FBX_IMPORT_MAX_UV_LAYERS)
				break;
		}
	}

	for(size_t i = 0; i < fbxUVLayers.size(); i++)
		ReadLayerData(*fbxUVLayers[i], importMesh->UV[i], importMesh->Indices);

	FbxLayer* mainLayer = mesh->GetLayer(0);
	if(mainLayer != nullptr)
	{
		// Import colors
		if(mainLayer->GetVertexColors() != nullptr)
			ReadLayerData(*mainLayer->GetVertexColors(), importMesh->Colors, importMesh->Indices);

		// Import normals
		if(options.ImportNormals)
		{
			bool hasNormals = mainLayer->GetNormals() != nullptr;

			if(!hasNormals)
			{
				if(mainLayer->GetSmoothing() != nullptr)
				{
					FbxLayerElementSmoothing* smoothing = mainLayer->GetSmoothing();

					if(smoothing->GetMappingMode() == FbxLayerElement::eByEdge)
					{
						FbxGeometryConverter converter(mFBXManager);
						converter.ComputePolygonSmoothingFromEdgeSmoothing(mesh, 0);
					}

					ReadLayerData(*smoothing, importMesh->SmoothingGroups, importMesh->Indices);

					if(!importMesh->SmoothingGroups.empty())
					{
						FBXUtility::NormalsFromSmoothing(importMesh->Positions, importMesh->Indices, importMesh->SmoothingGroups, importMesh->Normals);
					}
				}
			}
			else
				ReadLayerData(*mainLayer->GetNormals(), importMesh->Normals, importMesh->Indices);
		}

		// Import tangents
		if(options.ImportTangents)
		{
			bool hasTangents = mainLayer->GetTangents() != nullptr && mainLayer->GetBinormals() != nullptr;

			if(!hasTangents)
			{
				if(fbxUVLayers.size() > 0)
					hasTangents = mesh->GenerateTangentsData(0, false);
			}

			if(hasTangents)
			{
				ReadLayerData(*mainLayer->GetTangents(), importMesh->Tangents, importMesh->Indices);
				ReadLayerData(*mainLayer->GetBinormals(), importMesh->Bitangents, importMesh->Indices);
			}
		}

		// Import material indexes
		if(mainLayer->GetMaterials() != nullptr)
		{
			Vector<FbxSurfaceMaterial*> fbxMaterials;

			ReadLayerData(*mainLayer->GetMaterials(), fbxMaterials, importMesh->Indices);

			UnorderedMap<FbxSurfaceMaterial*, int> materialLookup;
			int nextMaterialIdx = 0;
			for(u32 i = 0; i < (u32)fbxMaterials.size(); i++)
			{
				auto iterFind = materialLookup.find(fbxMaterials[i]);

				int materialIdx = 0;
				if(iterFind != materialLookup.end())
					materialIdx = iterFind->second;
				else
				{
					materialIdx = nextMaterialIdx++;
					materialLookup[fbxMaterials[i]] = materialIdx;
				}

				importMesh->Materials.push_back(materialIdx);
			}
		}
		else
		{
			importMesh->Materials.resize(importMesh->Indices.size(), 0);
		}
	}
}

void FBXImporter::ImportBlendShapes(FBXImportScene& scene, const FBXImportOptions& options)
{
	for(auto& mesh : scene.Meshes)
	{
		FbxMesh* fbxMesh = mesh->FbxMesh;

		u32 deformerCount = (u32)fbxMesh->GetDeformerCount(FbxDeformer::eBlendShape);
		for(u32 i = 0; i < deformerCount; i++)
		{
			FbxBlendShape* deformer = static_cast<FbxBlendShape*>(fbxMesh->GetDeformer(i, FbxDeformer::eBlendShape));

			u32 blendShapeChannelCount = (u32)deformer->GetBlendShapeChannelCount();
			for(u32 j = 0; j < blendShapeChannelCount; ++j)
			{
				FbxBlendShapeChannel* channel = deformer->GetBlendShapeChannel(j);
				double* weights = channel->GetTargetShapeFullWeights();

				u32 frameCount = channel->GetTargetShapeCount();
				if(frameCount == 0)
					continue;

				mesh->BlendShapes.push_back(FBXBlendShape());
				FBXBlendShape& blendShape = mesh->BlendShapes.back();
				blendShape.Name = channel->GetName();
				blendShape.Frames.resize(frameCount);

				// Get name without invalid characters
				blendShape.Name = StringUtility::ReplaceAll(blendShape.Name, ".", "_");
				blendShape.Name = StringUtility::ReplaceAll(blendShape.Name, "/", "_");

				for(u32 k = 0; k < frameCount; k++)
				{
					FbxShape* fbxShape = channel->GetTargetShape(k);

					FBXBlendShapeFrame& frame = blendShape.Frames[k];
					frame.Name = fbxShape->GetName();
					frame.Weight = (float)(weights[k] / 100.0);

					// Get name without invalid characters
					frame.Name = StringUtility::ReplaceAll(frame.Name, ".", "_");
					frame.Name = StringUtility::ReplaceAll(frame.Name, "/", "_");

					ImportBlendShapeFrame(fbxShape, *mesh, options, frame);
				}
			}
		}
	}
}

void FBXImporter::ImportBlendShapeFrame(FbxShape* shape, const FBXImportMesh& mesh, const FBXImportOptions& options, FBXBlendShapeFrame& outFrame)
{
	u32 vertexCount = (u32)shape->GetControlPointsCount();
	outFrame.Positions.resize(vertexCount);
	FbxVector4* controlPoints = shape->GetControlPoints();

	for(u32 i = 0; i < vertexCount; i++)
		outFrame.Positions[i] = FBXToNativeType(controlPoints[i]);

	FbxLayer* mainLayer = shape->GetLayer(0);
	if(options.ImportNormals)
	{
		bool hasNormals = mainLayer->GetNormals() != nullptr;

		if(!hasNormals)
		{
			if(!mesh.SmoothingGroups.empty())
			{
				FBXUtility::NormalsFromSmoothing(outFrame.Positions, mesh.Indices, mesh.SmoothingGroups, outFrame.Normals);
			}
		}
		else
			ReadLayerData(*mainLayer->GetNormals(), outFrame.Normals, mesh.Indices);
	}

	if(options.ImportTangents)
	{
		bool hasTangents = mainLayer->GetTangents() != nullptr && mainLayer->GetBinormals() != nullptr;

		if(hasTangents)
		{
			ReadLayerData(*mainLayer->GetTangents(), outFrame.Tangents, mesh.Indices);
			ReadLayerData(*mainLayer->GetBinormals(), outFrame.Bitangents, mesh.Indices);
		}
	}
}

void FBXImporter::ImportSkin(FBXImportScene& scene, const FBXImportOptions& options)
{
	for(auto& mesh : scene.Meshes)
	{
		FbxMesh* fbxMesh = mesh->FbxMesh;

		u32 deformerCount = (u32)fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
		if(deformerCount > 0)
		{
			// We ignore other deformers if there's more than one
			FbxSkin* deformer = static_cast<FbxSkin*>(fbxMesh->GetDeformer(0, FbxDeformer::eSkin));
			u32 boneCount = (u32)deformer->GetClusterCount();

			if(boneCount == 0)
				continue;

			// If only one bone and it links to itself, ignore the bone
			if(boneCount == 1)
			{
				FbxCluster* cluster = deformer->GetCluster(0);
				if(mesh->ReferencedBy.size() == 1 && mesh->ReferencedBy[0]->FbxNode == cluster->GetLink())
					continue;
			}

			ImportSkin(scene, deformer, *mesh, options);
		}
	}
}

void FBXImporter::ImportSkin(FBXImportScene& scene, FbxSkin* skin, FBXImportMesh& mesh, const FBXImportOptions& options)
{
	Vector<FBXBoneInfluence>& influences = mesh.BoneInfluences;
	influences.resize(mesh.Positions.size());

	UnorderedSet<FbxNode*> existingBones;
	u32 boneCount = (u32)skin->GetClusterCount();
	for(u32 i = 0; i < boneCount; i++)
	{
		FbxCluster* cluster = skin->GetCluster(i);
		FbxNode* link = cluster->GetLink();

		// The bone node doesn't exist, skip it
		auto iterFind = scene.NodeMap.find(link);
		if(iterFind == scene.NodeMap.end())
			continue;

		mesh.Bones.push_back(FBXBone());

		FBXBone& bone = mesh.Bones.back();
		bone.Node = iterFind->second;

		if(mesh.ReferencedBy.size() > 1)
		{
			// Note: If this becomes a relevant issue (unlikely), then I will have to duplicate skeleton bones for
			// each such mesh, since they will all require their own bind poses. Animation curves will also need to be
			// handled specially (likely by allowing them to be applied to multiple bones at once). The other option is
			// not to bake the node transform into mesh vertices and handle it on a Scene Object level.
			B3D_LOG(Warning, LogFBXImporter, "Skinned mesh has multiple different instances. This is not supported.");
		}

		FBXImportNode* parentNode = mesh.ReferencedBy[0];

		// Calculate bind pose
		////  Grab the transform of the node linked to this cluster (should be equivalent to bone.node->worldTransform)
		FbxAMatrix linkTransform;
		cluster->GetTransformLinkMatrix(linkTransform);

		FbxAMatrix clusterTransform;
		cluster->GetTransformMatrix(clusterTransform);

		bone.LocalTfrm = bone.Node->LocalTransform;

		FbxAMatrix invLinkTransform = linkTransform.Inverse();
		bone.BindPose = FBXToNativeType(invLinkTransform * clusterTransform);

		// Undo the transform we baked into the mesh
		bone.BindPose = bone.BindPose * (parentNode->WorldTransform * parentNode->GeomTransform).InverseAffine();

		bool isDuplicate = !existingBones.insert(link).second;
		bool isAdditive = cluster->GetLinkMode() == FbxCluster::eAdditive;

		// We avoid importing weights twice for duplicate bones and we don't
		// support additive link mode.
		bool importWeights = !isDuplicate && !isAdditive;
		if(!importWeights)
			continue;

		double* weights = cluster->GetControlPointWeights();
		i32* indices = cluster->GetControlPointIndices();
		u32 numIndices = (u32)cluster->GetControlPointIndicesCount();
		i32 numVertices = (i32)influences.size();

		// Add new weights while keeping them in order and removing the smallest ones
		// if number of influences exceeds the set maximum value
		for(u32 j = 0; j < numIndices; j++)
		{
			i32 vertexIndex = indices[j];
			float weight = (float)weights[j];

			for(i32 k = 0; k < FBX_IMPORT_MAX_BONE_INFLUENCES; k++)
			{
				if(vertexIndex < 0 || vertexIndex >= numVertices)
					continue;

				if(weight >= influences[vertexIndex].Weights[k])
				{
					for(i32 l = FBX_IMPORT_MAX_BONE_INFLUENCES - 2; l >= k; l--)
					{
						influences[vertexIndex].Weights[l + 1] = influences[vertexIndex].Weights[l];
						influences[vertexIndex].Indices[l + 1] = influences[vertexIndex].Indices[l];
					}

					influences[vertexIndex].Weights[k] = weight;
					influences[vertexIndex].Indices[k] = i;
					break;
				}
			}
		}
	}

	if(mesh.Bones.empty())
		mesh.BoneInfluences.clear();

	u32 numBones = (u32)mesh.Bones.size();
	if(numBones > 256)
	{
		B3D_LOG(Warning, LogFBXImporter, "A maximum of 256 bones per skeleton are supported. Imported skeleton has {0} bones.", numBones);
	}

	// Normalize weights
	u32 numInfluences = (u32)mesh.BoneInfluences.size();
	for(u32 i = 0; i < numInfluences; i++)
	{
		float sum = 0.0f;
		for(u32 j = 0; j < FBX_IMPORT_MAX_BONE_INFLUENCES; j++)
			sum += influences[i].Weights[j];

		float invSum = 1.0f / sum;
		for(u32 j = 0; j < FBX_IMPORT_MAX_BONE_INFLUENCES; j++)
			influences[i].Weights[j] *= invSum;
	}
}

void FBXImporter::GenerateMissingTangentSpace(FBXImportScene& scene, const FBXImportOptions& options)
{
	for(auto& mesh : scene.Meshes)
	{
		u32 numVertices = (u32)mesh->Positions.size();
		u32 numIndices = (u32)mesh->Indices.size();

		if((options.ImportNormals || options.ImportTangents) && mesh->Normals.empty())
		{
			mesh->Normals.resize(numVertices);

			MeshUtility::CalculateNormals(mesh->Positions.data(), (u8*)mesh->Indices.data(), numVertices, numIndices, mesh->Normals.data());
		}

		if(options.ImportTangents && !mesh->UV[0].empty() && (mesh->Tangents.empty() || mesh->Bitangents.empty()))
		{
			mesh->Tangents.resize(numVertices);
			mesh->Bitangents.resize(numVertices);

			MeshUtility::CalculateTangents(mesh->Positions.data(), mesh->Normals.data(), mesh->UV[0].data(), (u8*)mesh->Indices.data(), numVertices, numIndices, mesh->Tangents.data(), mesh->Bitangents.data());
		}

		for(auto& shape : mesh->BlendShapes)
		{
			for(auto& frame : shape.Frames)
			{
				if((options.ImportNormals || options.ImportTangents) && frame.Normals.empty())
				{
					frame.Normals.resize(numVertices);

					MeshUtility::CalculateNormals(mesh->Positions.data(), (u8*)mesh->Indices.data(), numVertices, numIndices, frame.Normals.data());
				}

				if(options.ImportTangents && !mesh->UV[0].empty() && (frame.Tangents.empty() || frame.Bitangents.empty()))
				{
					frame.Tangents.resize(numVertices);
					frame.Bitangents.resize(numVertices);

					MeshUtility::CalculateTangents(mesh->Positions.data(), frame.Normals.data(), mesh->UV[0].data(), (u8*)mesh->Indices.data(), numVertices, numIndices, frame.Tangents.data(), frame.Bitangents.data());
				}
			}
		}
	}
}

void FBXImporter::ImportAnimations(FbxScene* scene, FBXImportOptions& importOptions, FBXImportScene& importScene)
{
	FbxNode* root = scene->GetRootNode();

	u32 numAnimStacks = (u32)scene->GetSrcObjectCount<FbxAnimStack>();
	for(u32 i = 0; i < numAnimStacks; i++)
	{
		FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(i);

		importScene.Clips.push_back(FBXAnimationClip());
		FBXAnimationClip& clip = importScene.Clips.back();
		clip.Name = animStack->GetName();

		FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
		clip.Start = (float)timeSpan.GetStart().GetSecondDouble();
		clip.End = (float)timeSpan.GetStop().GetSecondDouble();

		clip.SampleRate = (u32)FbxTime::GetFrameRate(scene->GetGlobalSettings().GetTimeMode());

		u32 layerCount = animStack->GetMemberCount<FbxAnimLayer>();
		if(layerCount > 1)
		{
			FbxAnimEvaluator* evaluator = scene->GetAnimationEvaluator();

			FbxTime startTime;
			startTime.SetSecondDouble(clip.Start);

			FbxTime endTime;
			endTime.SetSecondDouble(clip.End);

			FbxTime sampleRate;

			if(importOptions.AnimResample)
				sampleRate.SetSecondDouble(importOptions.AnimSampleRate);
			else
			{
				FbxTime::EMode timeMode = scene->GetGlobalSettings().GetTimeMode();
				sampleRate.SetSecondDouble(1.0f / FbxTime::GetFrameRate(timeMode));
			}

			if(!animStack->BakeLayers(evaluator, startTime, endTime, sampleRate))
				continue;

			layerCount = animStack->GetMemberCount<FbxAnimLayer>();
		}

		if(layerCount == 1)
		{
			FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);

			ImportAnimations(animLayer, root, importOptions, clip, importScene);
		}
	}
}

void FBXImporter::ImportAnimations(FbxAnimLayer* layer, FbxNode* node, FBXImportOptions& importOptions, FBXAnimationClip& clip, FBXImportScene& importScene)
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
		for(u32 i = 0; i < 3; i++)
		{
			if(curves[i] != nullptr && curves[i]->KeyGetCount() > 0)
				return true;
		}

		return false;
	};

	bool hasBoneAnimation = hasCurveValues(translation) || hasCurveValues(rotation) || hasCurveValues(scale);
	if(hasBoneAnimation)
	{
		clip.BoneAnimations.push_back(FBXBoneAnimation());
		FBXBoneAnimation& boneAnim = clip.BoneAnimations.back();
		boneAnim.Node = importScene.NodeMap[node];

		if(hasCurveValues(translation))
		{
			float defaultValues[3];
			memcpy(defaultValues, &defaultTranslation, sizeof(defaultValues));

			boneAnim.Translation = ImportCurve<Vector3, 3>(translation, defaultValues, importOptions, clip.Start, clip.End);
		}
		else
		{
			Vector<TKeyframe<Vector3>> keyframes(1);
			keyframes[0].Value = defaultTranslation;
			keyframes[0].InTangent = Vector3::kZero;
			keyframes[0].OutTangent = Vector3::kZero;

			boneAnim.Translation = TAnimationCurve<Vector3>(keyframes);
		}

		if(hasCurveValues(scale))
		{
			float defaultValues[3];
			memcpy(defaultValues, &defaultScale, sizeof(defaultValues));

			boneAnim.Scale = ImportCurve<Vector3, 3>(scale, defaultValues, importOptions, clip.Start, clip.End);
		}
		else
		{
			Vector<TKeyframe<Vector3>> keyframes(1);
			keyframes[0].Value = defaultScale;
			keyframes[0].InTangent = Vector3::kZero;
			keyframes[0].OutTangent = Vector3::kZero;

			boneAnim.Scale = TAnimationCurve<Vector3>(keyframes);
		}

		SPtr<TAnimationCurve<Vector3>> eulerAnimation = B3DMakeShared<TAnimationCurve<Vector3>>();
		if(hasCurveValues(rotation))
		{
			float defaultValues[3];
			memcpy(defaultValues, &defaultRotation, sizeof(defaultValues));

			*eulerAnimation = ImportCurve<Vector3, 3>(rotation, defaultValues, importOptions, clip.Start, clip.End);
		}
		else
		{
			Vector<TKeyframe<Vector3>> keyframes(1);
			keyframes[0].Value = defaultRotation;
			keyframes[0].InTangent = Vector3::kZero;
			keyframes[0].OutTangent = Vector3::kZero;

			*eulerAnimation = TAnimationCurve<Vector3>(keyframes);
		}

		if(importOptions.ReduceKeyframes)
		{
			boneAnim.Translation = ReduceKeyframes(boneAnim.Translation);
			boneAnim.Scale = ReduceKeyframes(boneAnim.Scale);
			*eulerAnimation = ReduceKeyframes(*eulerAnimation);
		}

		boneAnim.Rotation = *AnimationUtility::EulerToQuaternionCurve(eulerAnimation, EulerAngleOrder::XYZ);
	}

	if(importOptions.ImportBlendShapes)
	{
		FbxMesh* fbxMesh = node->GetMesh();
		if(fbxMesh != nullptr)
		{
			i32 deformerCount = fbxMesh->GetDeformerCount(FbxDeformer::eBlendShape);
			for(i32 i = 0; i < deformerCount; i++)
			{
				FbxBlendShape* deformer = static_cast<FbxBlendShape*>(fbxMesh->GetDeformer(i, FbxDeformer::eBlendShape));

				i32 channelCount = deformer->GetBlendShapeChannelCount();
				for(i32 j = 0; j < channelCount; j++)
				{
					FbxBlendShapeChannel* channel = deformer->GetBlendShapeChannel(j);

					FbxAnimCurve* curve = fbxMesh->GetShapeChannel(i, j, layer);
					if(curve != nullptr && curve->KeyGetCount() > 0)
					{
						clip.BlendShapeAnimations.push_back(FBXBlendShapeAnimation());
						FBXBlendShapeAnimation& blendShapeAnim = clip.BlendShapeAnimations.back();
						blendShapeAnim.BlendShape = channel->GetName();

						// Get name without invalid characters
						blendShapeAnim.BlendShape = StringUtility::ReplaceAll(blendShapeAnim.BlendShape, ".", "_");
						blendShapeAnim.BlendShape = StringUtility::ReplaceAll(blendShapeAnim.BlendShape, "/", "_");

						FbxAnimCurve* curves[1] = { curve };
						float defaultValues[1] = { 0.0f };
						blendShapeAnim.Curve = ImportCurve<float, 1>(curves, defaultValues, importOptions, clip.Start, clip.End);

						// FBX contains data in [0, 100] range, but we need it in [0, 1] range
						blendShapeAnim.Curve = AnimationUtility::ScaleCurve(blendShapeAnim.Curve, 0.01f);
					}
				}
			}
		}
	}

	u32 childCount = (u32)node->GetChildCount();
	for(u32 i = 0; i < childCount; i++)
	{
		FbxNode* child = node->GetChild(i);
		ImportAnimations(layer, child, importOptions, clip, importScene);
	}
}

void FBXImporter::BakeTransforms(FbxScene* scene)
{
	// FBX stores transforms in a more complex way than just translation-rotation-scale as used by the framework.
	// Instead they also support rotations offsets and pivots, scaling pivots and more. We wish to bake all this data
	// into a standard transform so we can access it using node's local TRS properties (e.g. FbxNode::LclTranslation).

	double frameRate = FbxTime::GetFrameRate(scene->GetGlobalSettings().GetTimeMode());

	B3DMarkAllocatorFrame();
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

			for(int i = 0; i < node->GetChildCount(); i++)
			{
				FbxNode* childNode = node->GetChild(i);
				todo.push(childNode);
			}
		}

		scene->GetRootNode()->ConvertPivotAnimationRecursive(nullptr, FbxNode::eDestinationPivot, frameRate, false);
	}
	B3DClearAllocatorFrame();
}

TAnimationCurve<Vector3> FBXImporter::ReduceKeyframes(TAnimationCurve<Vector3>& curve)
{
	u32 keyCount = curve.GetNumKeyFrames();

	Vector<TKeyframe<Vector3>> newKeyframes;

	bool lastWasEqual = false;
	for(u32 i = 0; i < keyCount; i++)
	{
		bool isEqual = true;

		const TKeyframe<Vector3>& curKey = curve.GetKeyFrame(i);
		if(i > 0)
		{
			TKeyframe<Vector3>& prevKey = newKeyframes.back();

			isEqual = Math::ApproxEquals(prevKey.Value, curKey.Value) &&
				Math::ApproxEquals(prevKey.OutTangent, curKey.InTangent) && isEqual;
		}
		else
			isEqual = false;

		// More than two keys in a row are equal, remove previous key by replacing it with this one
		if(lastWasEqual && isEqual)
		{
			TKeyframe<Vector3>& prevKey = newKeyframes.back();

			// Other properties are guaranteed unchanged
			prevKey.Time = curKey.Time;
			prevKey.OutTangent = curKey.OutTangent;

			continue;
		}

		newKeyframes.push_back(curKey);
		lastWasEqual = isEqual;
	}

	return TAnimationCurve<Vector3>(newKeyframes);
}

template <class T>
void SetKeyframeValues(TKeyframe<T>& keyFrame, int idx, float value, float inTangent, float outTangent)
{
	keyFrame.Value = value;
	keyFrame.InTangent = inTangent;
	keyFrame.OutTangent = outTangent;
}

template <>
void SetKeyframeValues<Vector3>(TKeyframe<Vector3>& keyFrame, int idx, float value, float inTangent, float outTangent)
{
	keyFrame.Value[idx] = value;
	keyFrame.InTangent[idx] = inTangent;
	keyFrame.OutTangent[idx] = outTangent;
}

template <class T, int C>
TAnimationCurve<T> FBXImporter::ImportCurve(FbxAnimCurve* (&fbxCurve)[C], float (&defaultValues)[C], FBXImportOptions& importOptions, float clipStart, float clipEnd)
{
	int keyCounts[C];
	for(int i = 0; i < C; i++)
	{
		if(fbxCurve[i] != nullptr)
			keyCounts[i] = fbxCurve[i]->KeyGetCount();
		else
			keyCounts[i] = 0;
	}

	// If curve key-counts don't match, we need to force resampling
	bool forceResample = false;
	if(!forceResample)
	{
		for(int i = 1; i < C; i++)
		{
			forceResample |= keyCounts[i - 1] != keyCounts[i];
			if(forceResample)
				break;
		}
	}

	// Determine curve length
	float curveStart = std::numeric_limits<float>::infinity();
	float curveEnd = -std::numeric_limits<float>::infinity();

	for(i32 i = 0; i < C; i++)
	{
		if(fbxCurve[i] == nullptr)
		{
			curveStart = std::min(0.0f, curveStart);
			curveEnd = std::max(0.0f, curveEnd);

			continue;
		}

		int keyCount = keyCounts[i];
		for(i32 j = 0; j < keyCount; j++)
		{
			FbxTime fbxTime = fbxCurve[i]->KeyGetTime(j);
			float time = (float)fbxTime.GetSecondDouble();

			curveStart = std::min(time, curveStart);
			curveEnd = std::max(time, curveEnd);
		}
	}

	// Read keys directly
	if(!importOptions.AnimResample && !forceResample)
	{
		bool foundMismatch = false;
		int keyCount = keyCounts[0];
		Vector<TKeyframe<T>> keyframes;

		// All curves must match the length of the clip, so add a keyframe if first keyframe doesn't match the start time
		if(curveStart > clipStart)
		{
			keyframes.push_back(TKeyframe<T>());
			TKeyframe<T>& keyFrame = keyframes.back();

			keyFrame.Time = clipStart;

			FbxTime fbxSampleTime;
			fbxSampleTime.SetSecondDouble(clipStart);

			for(int j = 0; j < C; j++)
			{
				SetKeyframeValues(keyFrame, j, fbxCurve[j]->Evaluate(fbxSampleTime), fbxCurve[j]->EvaluateLeftDerivative(fbxSampleTime), fbxCurve[j]->EvaluateRightDerivative(fbxSampleTime));
			}
		}

		for(int i = 0; i < keyCount; i++)
		{
			FbxTime fbxTime = fbxCurve[0]->KeyGetTime(i);
			float time = (float)fbxTime.GetSecondDouble();

			// Ensure times from other curves match
			for(int j = 1; j < C; j++)
			{
				fbxTime = fbxCurve[j]->KeyGetTime(i);
				float otherTime = (float)fbxTime.GetSecondDouble();

				if(!Math::ApproxEquals(time, otherTime))
				{
					foundMismatch = true;
					break;
				}
			}

			if(foundMismatch)
				break;

			if(time < clipStart || time > clipEnd)
				continue;

			keyframes.push_back(TKeyframe<T>());
			TKeyframe<T>& keyFrame = keyframes.back();

			keyFrame.Time = time;

			for(int j = 0; j < C; j++)
			{
				SetKeyframeValues(keyFrame, j, fbxCurve[j]->KeyGetValue(i), fbxCurve[j]->KeyGetLeftDerivative(i), fbxCurve[j]->KeyGetRightDerivative(i));
			}
		}

		// All curves must match the length of the clip, so add a keyframe if last keyframe doesn't match the end time
		if(curveEnd < clipEnd)
		{
			keyframes.push_back(TKeyframe<T>());
			TKeyframe<T>& keyFrame = keyframes.back();

			keyFrame.Time = clipEnd;

			FbxTime fbxSampleTime;
			fbxSampleTime.SetSecondDouble(clipEnd);

			for(int j = 0; j < C; j++)
			{
				SetKeyframeValues(keyFrame, j, fbxCurve[j]->Evaluate(fbxSampleTime), fbxCurve[j]->EvaluateLeftDerivative(fbxSampleTime), fbxCurve[j]->EvaluateRightDerivative(fbxSampleTime));
			}
		}

		if(!foundMismatch)
			return TAnimationCurve<T>(keyframes);
		else
			forceResample = true;
	}

	// Resample keys
	if(!importOptions.AnimResample && forceResample)
		B3D_LOG(Verbose, LogFBXImporter, "Animation has different keyframes for different curve components, forcing resampling.");

	// Make sure to resample along the length of the entire clip
	curveStart = std::min(curveStart, clipStart);
	curveEnd = std::max(curveEnd, clipEnd);

	float curveLength = curveEnd - curveStart;
	i32 numSamples = Math::CeilToInt(curveLength / importOptions.AnimSampleRate) + 1;

	// We don't use the exact provided sample rate but instead modify it slightly so it
	// completely covers the curve range including start/end points while maintaining
	// constant time step between keyframes.
	float dt = curveLength / (float)(numSamples - 1);

	i32 lastKeyframe[] = { 0, 0, 0 };
	i32 lastLeftTangent[] = { 0, 0, 0 };
	i32 lastRightTangent[] = { 0, 0, 0 };

	Vector<TKeyframe<T>> keyframes(numSamples);
	for(i32 i = 0; i < numSamples; i++)
	{
		float sampleTime = std::min(curveStart + i * dt, curveEnd);
		FbxTime fbxSampleTime;
		fbxSampleTime.SetSecondDouble(sampleTime);

		TKeyframe<T>& keyFrame = keyframes[i];
		keyFrame.Time = sampleTime;

		for(int j = 0; j < C; j++)
		{
			if(fbxCurve[j] != nullptr)
			{
				SetKeyframeValues(keyFrame, j, fbxCurve[j]->Evaluate(fbxSampleTime, &lastKeyframe[j]), fbxCurve[j]->EvaluateLeftDerivative(fbxSampleTime, &lastLeftTangent[j]), fbxCurve[j]->EvaluateRightDerivative(fbxSampleTime, &lastRightTangent[j]));
			}
			else
			{
				SetKeyframeValues(keyFrame, j, defaultValues[j], 0.0f, 0.0f);
			}
		}
	}

	return TAnimationCurve<T>(keyframes);
}
