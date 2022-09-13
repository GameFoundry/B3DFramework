//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"
#include "Testing/BsTestSuite.h"
#include "Animation/BsAnimationCurve.h"
#include "Particles/BsParticleDistribution.h"

namespace bs
{
	float EvalPosition(float acceleration, float velocity, float time)
	{
		return acceleration * time * time * 0.5f + velocity * time;
	}

	float EvalVelocity(float acceleration, float time)
	{
		return acceleration * time;
	}

	class CoreTestSuite : public TestSuite
	{
	public:
		CoreTestSuite();

	private:
		void TestAnimCurveIntegration();
		void TestLookupTable();
	};

	CoreTestSuite::CoreTestSuite()
	{
		BS_ADD_TEST(CoreTestSuite::testAnimCurveIntegration);
		BS_ADD_TEST(CoreTestSuite::testLookupTable);
	}

	void CoreTestSuite::TestAnimCurveIntegration()
	{
		static constexpr float EPSILON = 0.0001f;

		// Construct some curves
		TAnimationCurve<float> curveConstant(
			{
				TKeyframe<float>{ 1.0f, 0.0f, 0.0f, 0.0f },
			});

		TAnimationCurve<float> curveLinear(
			{
				TKeyframe<float>{ 0.0f, 0.0f, 1.0f, 0.0f },
				TKeyframe<float>{ 1.0f, 1.0f, 0.0f, 1.0f }
			});

		TAnimationCurve<float> curveAcceleration(
			{
				TKeyframe<float>{ -9.81f, 0.0f, 0.0f, 0.0f },
				TKeyframe<float>{ -9.81f, 0.0f, 0.0f, 10.0f }
			});

		{
			TCurveIntegrationCache<float> cache;
			BS_TEST_ASSERT(Math::ApproxEquals(curveConstant.EvaluateIntegrated(0.0f, cache), 0.0f, EPSILON));
			BS_TEST_ASSERT(Math::ApproxEquals(curveConstant.EvaluateIntegrated(0.5f, cache), 0.5f, EPSILON));
			BS_TEST_ASSERT(Math::ApproxEquals(curveConstant.EvaluateIntegrated(1.0f, cache), 1.0f, EPSILON));
		}

		{
			TCurveIntegrationCache<float> cache;
			BS_TEST_ASSERT(Math::ApproxEquals(curveLinear.EvaluateIntegrated(0.0f, cache), 0.0f, EPSILON));
			BS_TEST_ASSERT(Math::ApproxEquals(curveLinear.EvaluateIntegrated(0.5f, cache), 0.125f, EPSILON));
			BS_TEST_ASSERT(Math::ApproxEquals(curveLinear.EvaluateIntegrated(1.0f, cache), 0.5f, EPSILON));
		}

		{
			TCurveIntegrationCache<float> cache;

			float times[] = { 0.0f, 0.5f, 1.0f };
			for(auto time : times)
			{
				BS_TEST_ASSERT(Math::ApproxEquals(curveConstant.EvaluateIntegratedDouble(time, cache),
					evalPosition(1.0f, 0.0f, time), EPSILON));
			}
		}

		{
			TCurveIntegrationCache<float> cache;

			float times[] = { 0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f };
			for(auto time : times)
			{
				BS_TEST_ASSERT(Math::ApproxEquals(curveAcceleration.EvaluateIntegrated(time, cache),
					evalVelocity(-9.81f, time), EPSILON));
			}

			std::pair<float, float> range = curveAcceleration.CalculateRangeIntegrated(cache);
			BS_TEST_ASSERT(Math::ApproxEquals(range.first, -98.1f, EPSILON));
			BS_TEST_ASSERT(Math::ApproxEquals(range.second, 0.0f, EPSILON));
		}

		{
			TCurveIntegrationCache<float> cache;

			float times[] = { 0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f };
			for(auto time : times)
			{
				BS_TEST_ASSERT(Math::ApproxEquals(curveAcceleration.EvaluateIntegratedDouble(time, cache),
					evalPosition(-9.81f, 0.0f, time)));
			}

			std::pair<float, float> range = curveAcceleration.CalculateRangeIntegratedDouble(cache);
			BS_TEST_ASSERT(Math::ApproxEquals(range.first, -490.5f, EPSILON));
			BS_TEST_ASSERT(Math::ApproxEquals(range.second, 0.0f, EPSILON));
		}
	}

	void CoreTestSuite::TestLookupTable()
	{
		static constexpr float EPSILON = 0.0001f;

		TAnimationCurve<Vector3> curve
		({
			TKeyframe<Vector3>{ Vector3(0.0f, 0.0f, 0.0f), Vector3::ZERO, Vector3::ONE, 0.0f },
			TKeyframe<Vector3>{ Vector3(5.0f, 3.0f, 10.0f), Vector3::ONE, Vector3::ZERO, 10.0f },
		});

		Vector3Distribution dist = curve;
		auto lookupTable = dist.ToLookupTable(128);

		for(UINT32 i = 0; i < 10; i++)
		{
			const float* left;
			const float* right;
			float lerp;

			float t = (i / 9.0f) * 1.0f;
			lookupTable.Evaluate(t, left, right, lerp);

			Vector3* leftVec = (Vector3*)left;
			Vector3* rightVec = (Vector3*)right;

			Vector3 valueLookup = Vector3::lerp(lerp, *leftVec, *rightVec);
			Vector3 valueCurve = curve.Evaluate(t);

			for(UINT32 j = 0; j < 3; j++)
				BS_TEST_ASSERT(Math::ApproxEquals(valueLookup[j], valueCurve[j], EPSILON));
		}
	}
}

using namespace bs;

int main()
{
	SPtr<TestSuite> tests = CoreTestSuite::create<CoreTestSuite>();

	ExceptionTestOutput testOutput;
	tests->Run(testOutput);

	return 0;
}
