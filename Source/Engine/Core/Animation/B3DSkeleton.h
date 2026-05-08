//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Script/B3DIScriptExportable.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"
#include "Animation/B3DCurveCache.h"
#include "Math/B3DTransform.h"

namespace b3d
{
	class SkeletonMask;

	/** @addtogroup Animation
	 *  @{
	 */

	/**
	 * Contains indices for position/rotation/scale animation curves. Used for quick mapping of bones in a skeleton to
	 * relevant animation curves.
	 */
	struct AnimationCurveMapping
	{
		u32 Position;
		u32 Rotation;
		u32 Scale;
	};

	/** Information about a single bone used for constructing a skeleton. */
	struct BoneInformation
	{
		String Name; /**< Unique name of the bone. */
		u32 Parent; /**< Index of the parent bone, if any. -1 if root bone. */

		Transform LocalTfrm; /**< Local transform of the bone, relative to other bones in the hierarchy. */
		Matrix4 InvBindPose; /**< Inverse bind pose which transforms vertices from their bind pose into local space. */
	};

	/** Contains information about a single playing animation clip. */
	struct AnimationState
	{
		TShared<AnimationCurves> Curves; /**< All curves in the animation clip. */
		float Length; /**< Total length of the animation clip in seconds (same as the length of the longest animation curve). */
		AnimationCurveMapping* BoneToCurveMapping; /**< Mapping of bone indices to curve indices for quick lookup .*/
		AnimationCurveMapping* SceneObjectToCurveMapping; /**< Mapping of scene object indices to curve indices for quick lookup. */

		TCurveCache<Vector3>* PositionCaches; /**< Cache used for evaluating position curves. */
		TCurveCache<Quaternion>* RotationCaches; /**< Cache used for evaluating rotation curves. */
		TCurveCache<Vector3>* ScaleCaches; /**< Cache used for evaluating scale curves. */
		TCurveCache<float>* GenericCaches; /**< Cache used for evaluating generic curves. */

		float Time; /**< Time to evaluate the curve at. */
		float Weight; /**< Determines how much of an influence will this clip have in regard to others in the same layer. */
		bool Loop; /**< Determines should the animation loop (wrap) once ending or beginning frames are passed. */
		bool Disabled; /**< If true the clip state will not be evaluated. */
	};

	/** Contains animation states for a single animation layer. */
	struct AnimationStateLayer
	{
		AnimationState* States; /**< Array of animation states in the layer. */
		u32 StateCount; /**< Number of states in @p states. */

		u8 Index; /**< Unique index of the animation layer. */

		/**
		 * If true animations from this layer will be added on top of other layers using the per-state weights. If false
		 * the weights will be normalized, animations will be blended with each other according to the normalized weights
		 * and then added on top of other layers.
		 */
		bool Additive;
	};

	/**
	 * Contains local translation, rotation and scale values for each bone in a skeleton, after being evaluated at a
	 * specific time of an animation.  All values are stored in the same order as the bones in the skeleton they were
	 * created by.
	 */
	struct LocalSkeletonPose
	{
		LocalSkeletonPose() = default;
		LocalSkeletonPose(u32 numBones, bool individualOverride = false);
		LocalSkeletonPose(u32 numPos, u32 numRot, u32 numScale);
		LocalSkeletonPose(const LocalSkeletonPose& other) = delete;
		LocalSkeletonPose(LocalSkeletonPose&& other);
		~LocalSkeletonPose();

		LocalSkeletonPose& operator=(const LocalSkeletonPose& other) = delete;
		LocalSkeletonPose& operator=(LocalSkeletonPose&& other);

		Vector3* Positions = nullptr; /**< Local bone positions at specific animation time. */
		Quaternion* Rotations = nullptr; /**< Local bone rotations at specific animation time. */
		Vector3* Scales = nullptr; /**< Local bone scales at specific animation time. */
		bool* HasOverride = nullptr; /**< True if the bone transform was overriden externally (local pose was ignored). */
		u32 NumBones = 0; /**< Number of bones in the pose. */
	};

	/** Contains internal information about a single bone in a Skeleton. */
	struct SkeletonBoneInfo
	{
		String Name; /**< Unique name of the bone. */
		u32 Parent; /**< Index of the bone parent, or -1 if root (no parent). */
	};

	/**
	 * @native
	 * Contains information about bones required for skeletal animation. Allows caller to evaluate a set of animation
	 * clips at a specific time and output the relevant skeleton pose.
	 * @endnative
	 * @script
	 * Contains information about bones required for skeletal animation.
	 * @endscript
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) Skeleton : public IReflectable, public IScriptExportable // Note: Must be immutable in order to be usable on multiple threads
	{
	public:
		/**
		 * Outputs a skeleton pose containing required transforms for transforming the skeleton to the values specified by
		 * the provided animation clip evaluated at the specified time.
		 *
		 * @param	outPose			Output pose containing the requested transforms. Must be pre-allocated with enough space
		 *							to hold all the bone matrices of this skeleton.
		 * @param	mask			Mask that filters which skeleton bones are enabled or disabled.
		 * @param	outLocalPose	Output pose containing the local transforms. Must be pre-allocated with enough space
		 *							to hold all the bone data of this skeleton.
		 * @param	clip			Clip to evaluate.
		 * @param	time			Time to evaluate the clip with.
		 * @param	loop			Determines should the time be looped (wrapped) if it goes past the clip start/end.
		 *
		 * @note	It is more efficient to use the other GetPose overload as sequential calls can benefit from animation
		 *			evaluator cache.
		 */
		void GetPose(Matrix4* outPose, LocalSkeletonPose& outLocalPose, const SkeletonMask& mask, const AnimationClip& clip, float time, bool loop = true);

		/**
		 * Outputs a skeleton pose containing required transforms for transforming the skeleton to the values specified by
		 * the provided set of animation curves.
		 *
		 * @param	outPose			Output pose containing the requested transforms. Must be pre-allocated with enough space
		 *							to hold all the bone matrices of this skeleton.
		 * @param	mask			Mask that filters which skeleton bones are enabled or disabled.
		 * @param	outLocalPose	Output pose containing the local transforms. Must be pre-allocated with enough space
		 *							to hold all the bone data of this skeleton.
		 * @param	layers			One or multiple layers, containing one or multiple animation states to evaluate.
		 * @param	layerCount		Number of layers in the @p layers array.
		 */
		void GetPose(Matrix4* outPose, LocalSkeletonPose& outLocalPose, const SkeletonMask& mask, const AnimationStateLayer* layers, u32 layerCount);

		/** Returns the total number of bones in the skeleton. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(NumBones))
		u32 GetBoneCount() const { return mNumBones; }

		/** Returns information about a bone at the provided index. */
		const SkeletonBoneInfo& GetBoneInfo(u32 index) const { return mBoneInfo[index]; }

		/** Searches all bones to find a root bone. Returns -1 if no root can be found. */
		u32 GetRootBoneIndex() const;

		/** Returns the inverse bind pose for the bone at the provided index. */
		const Matrix4& GetInvBindPose(u32 index) const { return mInvBindPoses[index]; }

		/** Calculates the bind-pose transform of the bone at the specified index. */
		Transform CalcBoneTransform(u32 index) const;

		/**
		 * Creates a new Skeleton.
		 *
		 * @param	bones		An array of bones to initialize the skeleton with. Data will be copied.
		 * @param	boneCount	Number of bones in the @p bones array.
		 */
		static TShared<Skeleton> Create(BoneInformation* bones, u32 boneCount);

	private:
		Skeleton() = default;
		Skeleton(BoneInformation* bones, u32 boneCount);

		u32 mNumBones = 0;
		TArray<Transform> mBoneTransforms;
		TArray<Matrix4> mInvBindPoses;
		TArray<SkeletonBoneInfo> mBoneInfo;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class SkeletonRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

		/**
		 * Creates a Skeleton with no data. You must populate its data manually.
		 *
		 * @note	For serialization use only.
		 */
		static TShared<Skeleton> CreateEmpty();
	};

	/** @} */
} // namespace b3d
