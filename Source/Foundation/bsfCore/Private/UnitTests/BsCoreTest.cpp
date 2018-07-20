//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"
#include "Testing/BsTestSuite.h"
#include "Animation/BsAnimationCurve.h"

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
	};

	CoreTestSuite::CoreTestSuite()
	{
		BS_ADD_TEST(CoreTestSuite::testAnimCurveIntegration);
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
}

using namespace bs;

int main()
{
	SPtr<TestSuite> tests = CoreTestSuite::create<CoreTestSuite>();

	ExceptionTestOutput testOutput;
	tests->run(testOutput);

	return 0;
}