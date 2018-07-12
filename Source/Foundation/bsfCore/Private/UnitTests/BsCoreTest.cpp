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
				TKeyframe<float>{ -9.81f, 0.0f, 0.0f, 0.0f }
			});

		{
			TCurveIntegrationCache<float> cache;
			BS_TEST_ASSERT(Math::approxEquals(curveConstant.evaluateIntegrated(0.0f, cache), 0.0f));
			BS_TEST_ASSERT(Math::approxEquals(curveConstant.evaluateIntegrated(0.5f, cache), 0.5f));
			BS_TEST_ASSERT(Math::approxEquals(curveConstant.evaluateIntegrated(1.0f, cache), 1.0f));
		}

		{
			TCurveIntegrationCache<float> cache;
			BS_TEST_ASSERT(Math::approxEquals(curveLinear.evaluateIntegrated(0.0f, cache), 0.0f));
			BS_TEST_ASSERT(Math::approxEquals(curveLinear.evaluateIntegrated(0.5f, cache), 0.125f));
			BS_TEST_ASSERT(Math::approxEquals(curveLinear.evaluateIntegrated(1.0f, cache), 0.5f));
		}

		{
			TCurveIntegrationCache<float> cache;

			float times[] = { 0.0f, 0.5f, 1.0f };
			for(auto time : times)
			{
				BS_TEST_ASSERT(Math::approxEquals(curveConstant.evaluateIntegratedDouble(time, cache), 
					evalPosition(1.0f, 0.0f, time)));
			}
		}

		{
			TCurveIntegrationCache<float> cache;

			float times[] = { 0.0f, 0.5f, 1.0f, 2.0f, 3.0f, 5.0f, 10.0f };
			for(auto time : times)
			{
				BS_TEST_ASSERT(Math::approxEquals(curveAcceleration.evaluateIntegratedDouble(time, cache), 
					evalPosition(-9.81f, 0.0f, time)));
			}
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