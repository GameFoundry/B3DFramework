//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DAnimationTestSuite.h"
#include "Animation/B3DAnimationClip.h"
#include "Animation/B3DAnimationScene.h"
#include "Animation/B3DMorphShapes.h"
#include "Components/B3DAnimation.h"
#include "Components/B3DRenderable.h"
#include "GpuBackend/B3DSubMesh.h"
#include "Math/B3DVector3.h"
#include "Mesh/B3DMesh.h"
#include "Renderer/B3DRendererMeshData.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneObject.h"

using namespace b3d;

namespace
{
	/** Number of vertices of the morph test mesh: one per morph shape, which is the only vertex the shape moves. */
	constexpr u32 kMorphVertexCount = 4;

	/** Weights the two morph channels of the test mesh are set to. */
	constexpr float kFirstChannelWeight = 0.8f;
	constexpr float kSecondChannelWeight = 0.3f;

	/** Largest difference between an evaluated morph shape weight and the expected one. */
	constexpr float kWeightTolerance = 1e-5f;

	/** Returns a morph channel named @p name with shapes at frame weights 0.5 and 1.0. */
	TShared<MorphChannel> CreateMorphChannel(const String& name)
	{
		return MorphChannel::Create(name, { MorphShape::Create(name + "0", 0.5f), MorphShape::Create(name + "1", 1.0f) });
	}

	/** Returns a curve that runs linearly from @p startValue at time 0 to @p endValue at time 1. */
	TAnimationCurve<float> CreateLinearCurve(float startValue, float endValue)
	{
		const float slope = endValue - startValue;

		Vector<TKeyframe<float>> keyframes(2);
		keyframes[0].Value = startValue;
		keyframes[0].Time = 0.0f;
		keyframes[1].Value = endValue;
		keyframes[1].Time = 1.0f;

		for(TKeyframe<float>& keyframe : keyframes)
		{
			keyframe.InTangent = slope;
			keyframe.OutTangent = slope;
		}

		return TAnimationCurve<float>(keyframes);
	}

	/** Returns a curve that moves morph channel @p channelName linearly from position @p startValue at time 0 to @p endValue at time 1. */
	TNamedAnimationCurve<float> CreateMorphFrameCurve(const String& channelName, float startValue, float endValue)
	{
		return TNamedAnimationCurve<float>(channelName, AnimationCurveFlag::MorphFrame, CreateLinearCurve(startValue, endValue));
	}

	/**
	 * Returns the weights of the two shapes of a test channel at channel position @p position, before the channel's weight: the first shape
	 * rises from 0 at position 0 to 1 at its frame weight of 0.5, then fades out as the second rises to 1 at its frame weight of 1.
	 */
	std::pair<float, float> GetExpectedShapeWeights(float position)
	{
		if(position <= 0.5f)
			return std::make_pair(position / 0.5f, 0.0f);

		const float blend = (position - 0.5f) / 0.5f;
		return std::make_pair(1.0f - blend, blend);
	}
} // namespace

AnimationTestSuite::AnimationTestSuite()
	: TestSuite("AnimationTestSuite")
{
	B3D_ADD_TEST(AnimationTestSuite::TestMorphShapeWeights)
}

void AnimationTestSuite::TestMorphShapeWeights()
{
	// A quad whose four vertices are each moved by one shape. The second channel starts at shape 2, so a weight computed without the
	// channel's start lands on the first channel's shapes.
	const TShared<RendererMeshData> meshData = RendererMeshData::Create(kMorphVertexCount, 6, VertexLayout::Position);

	Vector3 positions[kMorphVertexCount] = { Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), Vector3(1.0f, 1.0f, 0.0f) };
	u32 indices[6] = { 0, 2, 1, 1, 2, 3 };
	meshData->SetPositions(positions, sizeof(positions));
	meshData->SetIndices(indices, sizeof(indices));

	MeshCreateInformation meshInformation;
	meshInformation.SubMeshes = { SubMesh(0, 6, DOT_TRIANGLE_LIST) };
	meshInformation.MorphShapes = MorphShapes::Create({ CreateMorphChannel("First"), CreateMorphChannel("Second") }, kMorphVertexCount);

	// Each shape moves the vertex of its own index by one unit
	Vector<Vector<MorphVertex>> shapeVertices;
	for(u32 shapeIndex = 0; shapeIndex < kMorphVertexCount; shapeIndex++)
		shapeVertices.push_back({ MorphVertex(Vector3(1.0f, 0.0f, 0.0f), Vector3::kZero, shapeIndex) });

	meshInformation.MorphShapeDeltas = MorphShapeDeltas::Create(kMorphVertexCount, shapeVertices);
	const HMesh mesh = Mesh::Create(meshData->GetData(), meshInformation);

	// The frame curves of the two channels run in opposite directions, so at most times the channels are at different positions
	const TShared<AnimationCurves> curves = B3DMakeShared<AnimationCurves>();
	curves->Generic.push_back(CreateMorphFrameCurve("First", 0.0f, 1.0f));
	curves->Generic.push_back(CreateMorphFrameCurve("Second", 1.0f, 0.0f));
	const HAnimationClip clip = AnimationClip::Create(curves);

	for(float time : { 0.0f, 0.25f, 0.5f, 0.75f, 1.0f })
	{
		// An animation scene evaluates once per update period, so each time is evaluated in a scene of its own
		const TShared<SceneInstance> scene = SceneInstance::Create("AnimationTestSuite");
		const HSceneObject sceneObject = scene->CreateSceneObject("Morph");
		const HRenderable renderable = sceneObject->AddComponent<Renderable>();
		renderable->SetMesh(mesh);

		const HAnimation animation = sceneObject->AddComponent<Animation>();
		animation->SetEnableCull(false);

		AnimationClipState state;
		state.Time = time;
		state.Speed = 0.0f;
		state.WrapMode = AnimationWrapMode::Clamp;
		animation->SetState(clip, state);
		animation->SetMorphChannelWeight("First", kFirstChannelWeight);
		animation->SetMorphChannelWeight("Second", kSecondChannelWeight);

		const EvaluatedAnimationData* const animationData = scene->GetAnimationScene()->Update(false);
		B3D_TEST_ASSERT(animationData != nullptr && animationData->EvaluationIndex != 0)

		const EvaluatedAnimationData::MorphShapeInfo* morphShapeInfo = nullptr;
		if(animationData != nullptr)
		{
			const auto found = animationData->Infos.find(animation->GetAnimationId());
			if(found != animationData->Infos.end() && found->second.MorphShapeInfo.WeightCount == 4 &&
				found->second.MorphShapeInfo.WeightStartIndex + 4 <= (u32)animationData->MorphShapeWeights.size())
			{
				morphShapeInfo = &found->second.MorphShapeInfo;
			}
		}

		B3D_TEST_ASSERT_MSG(morphShapeInfo != nullptr, StringUtility::Format("Time {0}: the animation has no weights for its four morph shapes.", time))
		if(morphShapeInfo != nullptr)
		{
			const float* const weights = &animationData->MorphShapeWeights[morphShapeInfo->WeightStartIndex];

			const std::pair<float, float> firstShapeWeights = GetExpectedShapeWeights(time);
			const std::pair<float, float> secondShapeWeights = GetExpectedShapeWeights(1.0f - time);
			const float expectedWeights[4] = { firstShapeWeights.first * kFirstChannelWeight, firstShapeWeights.second * kFirstChannelWeight,
				secondShapeWeights.first * kSecondChannelWeight, secondShapeWeights.second * kSecondChannelWeight };

			for(u32 shapeIndex = 0; shapeIndex < 4; shapeIndex++)
			{
				B3D_TEST_ASSERT_MSG(std::abs(weights[shapeIndex] - expectedWeights[shapeIndex]) <= kWeightTolerance, StringUtility::Format("Time {0}: shape {1} weighs {2}, expected {3}.",
					time, shapeIndex, weights[shapeIndex], expectedWeights[shapeIndex]))
			}
		}

		scene->Destroy();
	}
}
