//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class GUIStyleSheetTestSuite : public TestSuite
	{
	public:
		GUIStyleSheetTestSuite();

	private:
		void TestChainedPseudoClassParsing();
		void TestChainedPseudoClassSpecificity();
		void TestChainedPseudoClassStateResolution();
		void TestDisabledStateResolution();
		void TestComments();
		void TestUnexpectedTokenIsReportedAsError();
		void TestTextMetricParsing();
		void TestFontWeightSelectsBoldFace();
		void TestFontMetaDataReportsFaceIdentity();
		void TestFontFamilyMatchesClosestFace();
		void TestFontFamilyLoadsFacesOnDemand();
		void TestFontStyleParsing();
		void TestTextMetricsAffectMeasuredSize();
		void TestMultipleStyleClassResolution();
		void TestResolvedRulesetCaching();
		void TestAncestorStateRestylesDescendants();
		void TestStateChangeOnlyRestylesWhenRuleDependsOnIt();
		void TestStyleKeyDistinguishesElementParts();
		void TestStyleKeyIgnoresUnobservedAncestors();
		void TestDescendantsFollowAncestorChanges();
		void TestParentOfCombinatorKeepsChildLevel();
	};
} // namespace b3d
