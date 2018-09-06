//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"
#include "Testing/BsTestSuite.h"
#include "Animation/BsAnimationCurve.h"
#include "Particles/BsParticleDistribution.h"

namespace bs
{
	float evalPosition(float acceleration, float velocity, float time)
	{
		return acceleration * time * time * 0.5f + velocity * time;
	}

	float evalVelocity(float acceleration, float time)
	{
		return acceleration * time;
	}

	class CoreTestSuite : public TestSuite
	{
	public:
		CoreTestSuite();

	private:
		void testAnimCurveIntegration();
		void testLookupTable();
	};

	CoreTestSuite::CoreTestSuite()
	{
		BS_ADD_TEST(CoreTestSuite::testAnimCurveIntegration);
		BS_ADD_TEST(CoreTestSuite::testLookupTable);
	}

	void CoreTestSuite::testAnimCurveIntegration()
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
			BS_TEST_ASSERT(Math::approxEquals(curveConstant.evaluateIntegrated(0.0f, cache), 0.0f, EPSILON));
			BS_TEST_ASSERT(Math::approxEquals(curveConstant.evaluateIntegrated(0.5f, cache), 0.5f, EPSILON));
			BS_TEST_ASSERT(Math::approxEquals(curveConstant.evaluateIntegrated(1.0f, cache), 1.0f, EPSILON));
		}

		{
			TCurveIntegrationCache<float> cache;
			BS_TEST_ASSERT(Math::approxEquals(curveLinear.evaluateIntegrated(0.0f, cache), 0.0f, EPSILON));
			BS_TEST_ASSERT(Math::approxEquals(curveLinear.evaluateIntegrated(0.5f, cache), 0.125f, EPSILON));
			BS_TEST_ASSERT(Math::approxEquals(curveLinear.evaluateIntegrated(1.0f, cache), 0.5f, EPSILON));
		}

		{
			TCurveIntegrationCache<float> cache;

			float times[] = { 0.0f, 0.5f, 1.0f };
			for(auto time : times)
			{
				BS_TEST_ASSERT(Math::approxEquals(curveConstant.evaluateIntegratedDouble(time, cache), 
					evalPosition(1.0f, 0.0f, time), EPSILON));
			}
		}

		{
			TCurveIntegrationCache<float> cache;

			float times[] = { 0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f };
			for(auto time : times)
			{
				BS_TEST_ASSERT(Math::approxEquals(curveAcceleration.evaluateIntegrated(time, cache), 
					evalVelocity(-9.81f, time), EPSILON));
			}

			std::pair<float, float> range = curveAcceleration.calculateRangeIntegrated(cache);
			BS_TEST_ASSERT(Math::approxEquals(range.first, -98.1f, EPSILON));
			BS_TEST_ASSERT(Math::approxEquals(range.second, 0.0f, EPSILON));
		}

		{
			TCurveIntegrationCache<float> cache;

			float times[] = { 0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f };
			for(auto time : times)
			{
				BS_TEST_ASSERT(Math::approxEquals(curveAcceleration.evaluateIntegratedDouble(time, cache), 
					evalPosition(-9.81f, 0.0f, time)));
			}

			std::pair<float, float> range = curveAcceleration.calculateRangeIntegratedDouble(cache);
			BS_TEST_ASSERT(Math::approxEquals(range.first, -490.5f, EPSILON));
			BS_TEST_ASSERT(Math::approxEquals(range.second, 0.0f, EPSILON));
		}
	}

	void CoreTestSuite::testLookupTable()
	{
		static constexpr float EPSILON = 0.0001f;

		TAnimationCurve<Vector3> curve
		({
			TKeyframe<Vector3>{ Vector3(0.0f, 0.0f, 0.0f), Vector3::ZERO, Vector3::ONE, 0.0f },
			TKeyframe<Vector3>{ Vector3(5.0f, 3.0f, 10.0f), Vector3::ONE, Vector3::ZERO, 10.0f },
		});

		Vector3Distribution dist = curve;
		auto lookupTable = dist.toLookupTable(128);

		for(UINT32 i = 0; i < 10; i++)
		{
			const float* left;
			const float* right;
			float lerp;

			float t = (i / 9.0f) * 1.0f;
			lookupTable.evaluate(t, left, right, lerp);

			Vector3* leftVec = (Vector3*)left;
			Vector3* rightVec = (Vector3*)right;

			Vector3 valueLookup = Vector3::lerp(lerp, *leftVec, *rightVec);
			Vector3 valueCurve = curve.evaluate(t);

			for(UINT32 j = 0; j < 3; j++)
				BS_TEST_ASSERT(Math::approxEquals(valueLookup[j], valueCurve[j], EPSILON));
		}
	}
}

using namespace bs;

int main()
{
	SPtr<TestSuite> tests = CoreTestSuite::create<CoreTestSuite>();

	ExceptionTestOutput testOutput;
	tests->run(testOutput);

	return 0;
}