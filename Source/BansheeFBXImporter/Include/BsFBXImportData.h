//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFBXPrerequisites.h"
#include "BsMatrix4.h"
#include "BsColor.h"
#include "BsVector2.h"
#include "BsVector4.h"
#include "BsSubMesh.h"

namespace BansheeEngine
{
	/** @addtogroup FBX
	 *  @{
	 */

	/**	Options that control FBX import */
	struct FBXImportOptions
	{
		bool importAnimation = true;
		bool importSkin = true;
		bool importBlendShapes = true;
		bool importNormals = true;
		bool importTangents = true;
		float importScale = 0.01f;
		float animSampleRate = 1.0f / 60.0f;
		bool animResample = false;
		bool reduceKeyframes = true;
	};

	/**	Represents a single node in the FBX transform hierarchy. */
	struct FBXImportNode
	{
		~FBXImportNode();

		Matrix4 localTransform;
		Matrix4 worldTransform;
		String name;
		FbxNode* fbxNode;

		Vector<FBXImportNode*> children;
	};

	/**	Contains geometry from one blend shape frame. */
	struct FBXBlendShapeFrame
	{
		Vector<Vector3> positions;
		Vector<Vector3> normals;
		Vector<Vector3> tangents;
		Vector<Vector3> bitangents;

		float weight;
	};

	/**	Contains all geometry for a single blend shape. */
	struct FBXBlendShape
	{
		String name;
		Vector<FBXBlendShapeFrame> frames;
	};

	/**	Contains data about a single bone in a skinned mesh. */
	struct FBXBone
	{
		FBXImportNode* node;
		Matrix4 bindPose;
	};

	/** Contains a set of bone weights and indices for a single vertex, used in a skinned mesh. */
	struct FBXBoneInfluence
	{
		FBXBoneInfluence()
		{
			for (UINT32 i = 0; i < FBX_IMPORT_MAX_BONE_INFLUENCES; i++)
			{
				weights[i] = 0.0f;
				indices[i] = -1;
			}
		}

		float weights[FBX_IMPORT_MAX_BONE_INFLUENCES];
		INT32 indices[FBX_IMPORT_MAX_BONE_INFLUENCES];
	};

	/**
	 * Represents a single frame in an animation curve. Contains a value at a specific time as well as the in and out 
	 * tangents at that position.
	 */
	struct FBXKeyFrame
	{
		float time;
		float value;
		float inTangent;
		float outTangent;
	};

	/**	Curve with a set of key frames used for animation of a single value. */
	struct FBXAnimationCurve
	{
		Vector<FBXKeyFrame> keyframes;

		float evaluate(float time);
	};

	/**	Animation curves required to animate a single bone. */
	struct FBXBoneAnimation
	{
		FBXImportNode* node;

		FBXAnimationCurve translation[3];
		FBXAnimationCurve rotation[4];
		FBXAnimationCurve scale[3];
	};

	/**	Animation curve required to animate a blend shape. */
	struct FBXBlendShapeAnimation
	{
		String blendShape;
		FBXAnimationCurve curve;
	};

	/** Animation clip containing a set of bone or blend shape animations. */
	struct FBXAnimationClip
	{
		String name;
		float start;
		float end;

		Vector<FBXBoneAnimation> boneAnimations;
		Vector<FBXBlendShapeAnimation> blendShapeAnimations;
	};

	/** All information required for creating an animation clip. */
	struct FBXAnimationClipData
	{
		FBXAnimationClipData(const String& name, bool isAdditive, const SPtr<AnimationCurves>& curves)
			:name(name), isAdditive(isAdditive), curves(curves)
		{ }

		String name;
		bool isAdditive;
		SPtr<AnimationCurves> curves;
	};

	/**	Imported mesh data. */
	struct FBXImportMesh
	{
		FbxMesh* fbxMesh;

		Vector<int> indices;
		Vector<Vector3> positions;
		Vector<Vector3> normals;
		Vector<Vector3> tangents;
		Vector<Vector3> bitangents;
		Vector<RGBA> colors;
		Vector<Vector2> UV[FBX_IMPORT_MAX_UV_LAYERS];
		Vector<int> materials;

		Vector<int> smoothingGroups;
		Vector<FBXBlendShape> blendShapes;

		Vector<FBXBoneInfluence> boneInfluences;
		Vector<FBXBone> bones;

		SPtr<MeshData> meshData;
		Vector<SubMesh> subMeshes;

		Vector<FBXImportNode*> referencedBy;
	};

	/**	Scene information used and modified during FBX import. */
	struct FBXImportScene
	{
		FBXImportScene();
		~FBXImportScene();

		Vector<FBXImportMesh*> meshes;
		FBXImportNode* rootNode;

		UnorderedMap<FbxNode*, FBXImportNode*> nodeMap;
		UnorderedMap<FbxMesh*, UINT32> meshMap;

		Vector<FBXAnimationClip> clips;
	};

	/** @} */
}