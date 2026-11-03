//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGUIStyleSheetTestSuite.h"

#include "GUI/B3DGUIElementContainer.h"
#include "GUI/B3DGUILabel.h"
#include "GUI/B3DGUILayoutY.h"
#include "GUI/B3DGUIUtility.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/StyleSheet/B3DGUIStyleSheetParser.h"
#include "Resources/B3DBuiltinResources.h"
#include "Text/B3DFontFamily.h"
#include "Text/B3DFontManager.h"
#include "Resources/B3DResources.h"
#include "Scene/B3DSceneObject.h"
#include "String/B3DSourceCode.h"

using namespace b3d;

namespace
{
	/** Style sheet exercising a base rule, two single pseudo-class rules and a chained pseudo-class rule on the same element. */
	constexpr const char* kChainedPseudoClassSource =
		"button\n"
		"{\n"
		"    background-color: #101010;\n"
		"}\n"
		"\n"
		"button:hover\n"
		"{\n"
		"    background-color: #202020;\n"
		"}\n"
		"\n"
		"button:checked\n"
		"{\n"
		"    background-color: #303030;\n"
		"}\n"
		"\n"
		"button:hover:checked\n"
		"{\n"
		"    background-color: #404040;\n"
		"}\n";

	/** Style sheet exercising the disabled state alongside the interaction states it must win over. */
	constexpr const char* kDisabledStateSource =
		"button\n"
		"{\n"
		"    background-color: #101010;\n"
		"}\n"
		"\n"
		"button:hover\n"
		"{\n"
		"    background-color: #202020;\n"
		"}\n"
		"\n"
		"button:disabled\n"
		"{\n"
		"    background-color: #303030;\n"
		"}\n";

	/** Parses the provided style sheet source. Returns null if parsing failed. */
	TShared<GUIStyleSheet> ParseStyleSheet(const char* source)
	{
		GUIStyleSheetParser parser;

		return parser.Parse(B3DMakeShared<SourceCode>(String(source)));
	}

	/** Parses the chained pseudo-class style sheet. Returns null if parsing failed. */
	TShared<GUIStyleSheet> ParseChainedPseudoClassStyleSheet()
	{
		return ParseStyleSheet(kChainedPseudoClassSource);
	}

	/**
	 * Creates an empty font that is distinct from every other font. Intended for tests that need to tell one face of a
	 * font family apart from another, and that never render from them.
	 */
	HFont CreateDistinctFont()
	{
		return B3DStaticResourceCast<Font>(GetResources().CreateResourceHandle(Font::CreateEmpty()));
	}

	/** Returns the rules of the first (and expected to be only) ruleset in the style sheet. */
	const GUIStyleSheetRules& GetOnlyRules(const GUIStyleSheet& styleSheet)
	{
		return styleSheet.GetRulesets().front().Rules;
	}

	/** Returns the ruleset whose selector list matches @p uniqueName (e.g. "button:hover:checked"), or null if there is none. */
	const GUIStyleSheetRuleset* FindRuleset(const GUIStyleSheet& styleSheet, StringView uniqueName)
	{
		for(const GUIStyleSheetRuleset& ruleset : styleSheet.GetRulesets())
		{
			if(ruleset.SelectorList.GetUniqueName() == uniqueName)
				return &ruleset;
		}

		return nullptr;
	}

	/**
	 * Builds the state-ruleset set a GUI element matching every ruleset in @p styleSheet would receive. This mirrors what
	 * GUIStyleSheetCascade::BuildStateRulesets() produces for an element that all the sheet's selectors apply to, without
	 * requiring a live GUI element.
	 */
	GUIStyleSheetStateRulesets BuildAllMatchingStateRulesets(const TShared<GUIStyleSheet>& styleSheet)
	{
		GUIStyleSheetStateRulesets::StyleSheetRulesetIndices styleSheetRulesetIndices;
		styleSheetRulesetIndices.StyleSheet = styleSheet;

		// Rulesets are sorted from least to most specific by GUIStyleSheet::Initialize(), so ascending index order is also
		// the order in which the rules must be overridden
		for(u32 rulesetIndex = 0; rulesetIndex < (u32)styleSheet->GetRulesets().Size(); ++rulesetIndex)
			styleSheetRulesetIndices.RulesetIndices.Add(rulesetIndex);

		GUIStyleSheetStateRulesets stateRulesets;
		stateRulesets.StyleSheets.Add(styleSheetRulesetIndices);

		return stateRulesets;
	}

	/** Style sheet exercising multiple classes on one element and rules keyed on the state of an ancestor. */
	constexpr const char* kStyleClassSource =
		"label.Entry\n"
		"{\n"
		"    padding: 1px 1px 1px 1px;\n"
		"}\n"
		"\n"
		"label.Entry.Alternate\n"
		"{\n"
		"    padding: 2px 2px 2px 2px;\n"
		"}\n"
		"\n"
		"container.Row:hover label\n"
		"{\n"
		"    padding: 7px 7px 7px 7px;\n"
		"}\n"
		"\n"
		"container.Row:checked label.Subtitle\n"
		"{\n"
		"    padding: 9px 9px 9px 9px;\n"
		"}\n";

	/** Minimal styleable, interactive element that can hold child elements. */
	class TestContainer final : public GUIElementContainer
	{
	public:
		explicit TestContainer(const String& styleClass)
			: GUIElementContainer(GUISizeConstraints(), styleClass)
		{ }

		/** Makes @p element a child of the container, removing it from its previous parent. */
		void AddChild(GUIElement* element) { RegisterChildElement(element); }

		const String& GetStyleSheetId() const override { return StyleId; }

		String StyleId; /**< Id used for style lookup. Only read when the style keys are calculated. */
	};

	/**
	 * Same rules as kStyleClassSource, except every descendant combinator is turned into a child combinator, which the
	 * parser has no syntax for.
	 */
	TShared<GUIStyleSheet> CreateParentOfStyleSheet()
	{
		const TShared<GUIStyleSheet> parsedStyleSheet = ParseStyleSheet(kStyleClassSource);
		if(parsedStyleSheet == nullptr)
			return nullptr;

		TArray<GUIStyleSheetRuleset> rulesets = parsedStyleSheet->GetRulesets();
		for(auto& ruleset : rulesets)
		{
			for(auto& selector : ruleset.SelectorList.Selectors)
			{
				if(selector.CombinatorType == GUIStyleSheetCombinatorType::AncestorOf)
					selector.CombinatorType = GUIStyleSheetCombinatorType::ParentOf;
			}
		}

		return GUIStyleSheet::CreateShared(std::move(rulesets));
	}

	/** Holds a scene object and GUI widget that GUI elements can be parented to, so that they resolve styles from a cascade. */
	struct TestWidgetScope
	{
		explicit TestWidgetScope(const TShared<GUIStyleSheet>& styleSheet)
		{
			Cascade = B3DMakeShared<GUIStyleSheetCascade>();
			Cascade->RegisterStyleSheet(B3DStaticResourceCast<GUIStyleSheet>(GetResources().CreateResourceHandle(styleSheet)), GUIStyleSheet::kBuiltinImportance);

			SceneObject = SceneObject::Create("GUIStyleSheetTestWidget");
			Widget = SceneObject->AddComponent<GUIWidget>(HCamera()); // No camera, so the widget never renders - the test only needs it to carry the style sheet cascade
			Widget->SetStyleSheetCascade(Cascade);
		}

		~TestWidgetScope()
		{
			if(SceneObject != nullptr)
				SceneObject->Destroy(true);
		}

		TestWidgetScope(const TestWidgetScope&) = delete;
		TestWidgetScope& operator=(const TestWidgetScope&) = delete;

		/** Parents an element tree to the widget, which is what makes it resolve its styles from the cascade. */
		void Attach(GUIElement* element) const { element->ChangeParentWidget(Widget.Get()); }

		TShared<GUIStyleSheetCascade> Cascade;
		HSceneObject SceneObject;
		HGUIWidget Widget;
	};
} // anonymous namespace

GUIStyleSheetTestSuite::GUIStyleSheetTestSuite()
	: TestSuite("GUIStyleSheetTestSuite")
{
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestChainedPseudoClassParsing)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestChainedPseudoClassSpecificity)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestChainedPseudoClassStateResolution)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestDisabledStateResolution)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestComments)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestUnexpectedTokenIsReportedAsError)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestTextMetricParsing)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestFontWeightSelectsBoldFace)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestFontMetaDataReportsFaceIdentity)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestFontFamilyMatchesClosestFace)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestFontFamilyLoadsFacesOnDemand)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestFontStyleParsing)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestTextMetricsAffectMeasuredSize)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestMultipleStyleClassResolution)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestResolvedRulesetCaching)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestAncestorStateRestylesDescendants)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestStateChangeOnlyRestylesWhenRuleDependsOnIt)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestStyleKeyDistinguishesElementParts)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestStyleKeyIgnoresUnobservedAncestors)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestDescendantsFollowAncestorChanges)
	B3D_ADD_TEST(GUIStyleSheetTestSuite::TestParentOfCombinatorKeepsChildLevel)
}

void GUIStyleSheetTestSuite::TestChainedPseudoClassParsing()
{
	const TShared<GUIStyleSheet> styleSheet = ParseChainedPseudoClassStyleSheet();
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	B3D_TEST_ASSERT((u32)styleSheet->GetRulesets().Size() == 4)

	const GUIStyleSheetRuleset* chainedRuleset = FindRuleset(*styleSheet, "button:hover:checked");
	B3D_TEST_ASSERT(chainedRuleset != nullptr)
	if(chainedRuleset == nullptr)
		return;

	// A chained selector must be kept as separate selectors, not folded into a single pseudo-class
	const auto& selectors = chainedRuleset->SelectorList.Selectors;
	B3D_TEST_ASSERT((u32)selectors.Size() == 3)
	if((u32)selectors.Size() != 3)
		return;

	B3D_TEST_ASSERT(selectors[0].SelectorType == GUIStyleSheetSelectorType::Element)
	B3D_TEST_ASSERT(selectors[0].Name == "button")
	B3D_TEST_ASSERT(selectors[0].CombinatorType == GUIStyleSheetCombinatorType::None)

	B3D_TEST_ASSERT(selectors[1].SelectorType == GUIStyleSheetSelectorType::PseudoClass)
	B3D_TEST_ASSERT(selectors[1].Name == "hover")
	B3D_TEST_ASSERT(selectors[1].CombinatorType == GUIStyleSheetCombinatorType::None)

	B3D_TEST_ASSERT(selectors[2].SelectorType == GUIStyleSheetSelectorType::PseudoClass)
	B3D_TEST_ASSERT(selectors[2].Name == "checked")
	B3D_TEST_ASSERT(selectors[2].CombinatorType == GUIStyleSheetCombinatorType::None)
}

void GUIStyleSheetTestSuite::TestChainedPseudoClassSpecificity()
{
	const TShared<GUIStyleSheet> styleSheet = ParseChainedPseudoClassStyleSheet();
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const GUIStyleSheetRuleset* baseRuleset = FindRuleset(*styleSheet, "button");
	const GUIStyleSheetRuleset* hoverRuleset = FindRuleset(*styleSheet, "button:hover");
	const GUIStyleSheetRuleset* chainedRuleset = FindRuleset(*styleSheet, "button:hover:checked");

	B3D_TEST_ASSERT(baseRuleset != nullptr && hoverRuleset != nullptr && chainedRuleset != nullptr)
	if(baseRuleset == nullptr || hoverRuleset == nullptr || chainedRuleset == nullptr)
		return;

	// Every chained pseudo-class must add specificity, so a chained rule always wins over its single-pseudo-class counterparts
	B3D_TEST_ASSERT(hoverRuleset->SelectorList.CalculateSpecificity() > baseRuleset->SelectorList.CalculateSpecificity())
	B3D_TEST_ASSERT(chainedRuleset->SelectorList.CalculateSpecificity() > hoverRuleset->SelectorList.CalculateSpecificity())
}

void GUIStyleSheetTestSuite::TestChainedPseudoClassStateResolution()
{
	const TShared<GUIStyleSheet> styleSheet = ParseChainedPseudoClassStyleSheet();
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const GUIStyleSheetRuleset* baseRuleset = FindRuleset(*styleSheet, "button");
	const GUIStyleSheetRuleset* hoverRuleset = FindRuleset(*styleSheet, "button:hover");
	const GUIStyleSheetRuleset* checkedRuleset = FindRuleset(*styleSheet, "button:checked");
	const GUIStyleSheetRuleset* chainedRuleset = FindRuleset(*styleSheet, "button:hover:checked");

	B3D_TEST_ASSERT(baseRuleset != nullptr && hoverRuleset != nullptr && checkedRuleset != nullptr && chainedRuleset != nullptr)
	if(baseRuleset == nullptr || hoverRuleset == nullptr || checkedRuleset == nullptr || chainedRuleset == nullptr)
		return;

	const GUIStyleSheetStateRulesets stateRulesets = BuildAllMatchingStateRulesets(styleSheet);

	auto fnResolveBackgroundColor = [&stateRulesets](GUIElementStates state)
	{
		return stateRulesets.BuildStateRuleset(state)->Rules.BackgroundColor;
	};

	// Unrelated states fall back to the base rule
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Normal) == baseRuleset->Rules.BackgroundColor)
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Active) == baseRuleset->Rules.BackgroundColor)

	// A single active state must not activate the chained rule
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Hover) == hoverRuleset->Rules.BackgroundColor)
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Checked) == checkedRuleset->Rules.BackgroundColor)

	// Both states active: the chained rule wins over both single-state rules
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Hover | GUIElementState::Checked) == chainedRuleset->Rules.BackgroundColor)

	// Additional unrelated states must not disturb the chained match
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Hover | GUIElementState::Checked | GUIElementState::Focus) == chainedRuleset->Rules.BackgroundColor)
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Hover | GUIElementState::Focus) == hoverRuleset->Rules.BackgroundColor)
}

void GUIStyleSheetTestSuite::TestDisabledStateResolution()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kDisabledStateSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const GUIStyleSheetRuleset* baseRuleset = FindRuleset(*styleSheet, "button");
	const GUIStyleSheetRuleset* hoverRuleset = FindRuleset(*styleSheet, "button:hover");
	const GUIStyleSheetRuleset* disabledRuleset = FindRuleset(*styleSheet, "button:disabled");

	B3D_TEST_ASSERT(baseRuleset != nullptr && hoverRuleset != nullptr && disabledRuleset != nullptr)
	if(baseRuleset == nullptr || hoverRuleset == nullptr || disabledRuleset == nullptr)
		return;

	const GUIStyleSheetStateRulesets stateRulesets = BuildAllMatchingStateRulesets(styleSheet);

	auto fnResolveBackgroundColor = [&stateRulesets](GUIElementStates state)
	{
		return stateRulesets.BuildStateRuleset(state)->Rules.BackgroundColor;
	};

	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Normal) == baseRuleset->Rules.BackgroundColor)
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Hover) == hoverRuleset->Rules.BackgroundColor)
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Disabled) == disabledRuleset->Rules.BackgroundColor)

	// A disabled element must not pick up any interaction visuals, no matter which interaction states are still set
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Disabled | GUIElementState::Hover) == disabledRuleset->Rules.BackgroundColor)
	B3D_TEST_ASSERT(fnResolveBackgroundColor(GUIElementState::Disabled | GUIElementState::Active | GUIElementState::Focus) == disabledRuleset->Rules.BackgroundColor)

	// The checked state survives, as it describes what the element represents rather than an interaction in progress.
	// State normalization is internal to BuildStateRuleset, so equal normalized states must yield the identical cached ruleset.
	B3D_TEST_ASSERT(stateRulesets.BuildStateRuleset(GUIElementState::Disabled | GUIElementState::Hover | GUIElementState::Active | GUIElementState::Focus | GUIElementState::Checked) ==
		stateRulesets.BuildStateRuleset(GUIElementState::Disabled | GUIElementState::Checked))
}

void GUIStyleSheetTestSuite::TestComments()
{
	constexpr const char* kCommentedSource =
		"/* Leading block comment */\n"
		"// Leading line comment\n"
		"button /* comment between selectors */ label\n"
		"{\n"
		"    /* comment before a property */\n"
		"    background-color: #101010; // trailing line comment\n"
		"    color: /* comment inside a declaration */ #202020;\n"
		"}\n"
		"/* Trailing block\n"
		"   spanning multiple lines */\n";

	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kCommentedSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	B3D_TEST_ASSERT((u32)styleSheet->GetRulesets().Size() == 1)
	if((u32)styleSheet->GetRulesets().Size() != 1)
		return;

	const GUIStyleSheetRuleset& ruleset = styleSheet->GetRulesets().front();
	B3D_TEST_ASSERT(ruleset.Rules.IsPropertySet(GUIStyleSheetPropertyType::BackgroundColor))
	B3D_TEST_ASSERT(ruleset.Rules.IsPropertySet(GUIStyleSheetPropertyType::Color))

	// A comment between two selectors separates them the same way whitespace does, so it must still read as a descendant selector
	B3D_TEST_ASSERT((u32)ruleset.SelectorList.Selectors.Size() == 2)
	if((u32)ruleset.SelectorList.Selectors.Size() != 2)
		return;

	B3D_TEST_ASSERT(ruleset.SelectorList.Selectors[0].Name == "button")
	B3D_TEST_ASSERT(ruleset.SelectorList.Selectors[0].CombinatorType == GUIStyleSheetCombinatorType::AncestorOf)
	B3D_TEST_ASSERT(ruleset.SelectorList.Selectors[1].Name == "label")

	// A slash is still a token of its own, as it separates the alpha channel in a color function
	const TShared<GUIStyleSheet> slashStyleSheet = ParseStyleSheet("button\n{\n    color: rgb(255, 128, 0 / 0.5);\n}\n");
	B3D_TEST_ASSERT(slashStyleSheet != nullptr)
	if(slashStyleSheet != nullptr)
		B3D_TEST_ASSERT(Math::ApproxEquals(GetOnlyRules(*slashStyleSheet).Color.A, 0.5f))

	// An unterminated block comment is a lexer error rather than a silently truncated file
	GUIStyleSheetParser parser;
	const TShared<GUIStyleSheet> unterminatedStyleSheet = parser.Parse(B3DMakeShared<SourceCode>(String("button\n{\n    color: #101010;\n}\n/* never closed\n")));
	B3D_TEST_ASSERT(unterminatedStyleSheet == nullptr)
	B3D_TEST_ASSERT(!parser.GetErrors().empty())
}

void GUIStyleSheetTestSuite::TestUnexpectedTokenIsReportedAsError()
{
	// A token the ruleset body cannot consume must produce a parse error, not spin the parser
	GUIStyleSheetParser parser;
	const TShared<GUIStyleSheet> styleSheet = parser.Parse(B3DMakeShared<SourceCode>(String("button\n{\n    background-color: #101010;\n    ;\n}\n")));

	B3D_TEST_ASSERT(styleSheet == nullptr)
	B3D_TEST_ASSERT(!parser.GetErrors().empty())
}

void GUIStyleSheetTestSuite::TestTextMetricParsing()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(
		"label\n"
		"{\n"
		"    line-height: 1.35;\n"
		"    letter-spacing: -0.2px;\n"
		"    font-weight: bold;\n"
		"}\n");

	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const GUIStyleSheetRules& rules = GetOnlyRules(*styleSheet);
	B3D_TEST_ASSERT(rules.IsPropertySet(GUIStyleSheetPropertyType::LineHeight))
	B3D_TEST_ASSERT(rules.IsPropertySet(GUIStyleSheetPropertyType::LetterSpacing))
	B3D_TEST_ASSERT(rules.IsPropertySet(GUIStyleSheetPropertyType::FontWeight))

	B3D_TEST_ASSERT(Math::ApproxEquals(rules.LineHeight, 1.35f))
	B3D_TEST_ASSERT(Math::ApproxEquals(rules.LetterSpacing, -0.2f))
	B3D_TEST_ASSERT(rules.FontWeight == FontWeight::Bold)

	// Metrics are in logical units, so they must be scaled along with the font size
	const TextMetrics scaledMetrics = rules.GetTextMetrics(2.0f);
	B3D_TEST_ASSERT(Math::ApproxEquals(scaledMetrics.LineHeight, 1.35f))
	B3D_TEST_ASSERT(Math::ApproxEquals(scaledMetrics.LetterSpacing, -0.4f))

	// Defaults must leave the metrics reported by the font untouched
	const TShared<GUIStyleSheet> defaultStyleSheet = ParseStyleSheet("label\n{\n    color: #101010;\n}\n");
	B3D_TEST_ASSERT(defaultStyleSheet != nullptr)
	if(defaultStyleSheet != nullptr)
	{
		const GUIStyleSheetRules& defaultRules = GetOnlyRules(*defaultStyleSheet);
		B3D_TEST_ASSERT(defaultRules.GetTextMetrics() == TextMetrics())
		B3D_TEST_ASSERT(defaultRules.FontWeight == FontWeight::Normal)
	}
}

void GUIStyleSheetTestSuite::TestFontWeightSelectsBoldFace()
{
	const HFont regularFace = GetBuiltinResources().GetDefaultFont();
	B3D_TEST_ASSERT(regularFace != nullptr)
	if(regularFace == nullptr)
		return;

	GUIStyleSheetRules rules;
	rules.FontFamily = FontFamily::CreateFromFace(regularFace);

	// A family providing only a regular face substitutes it for every weight, so a missing face never blanks out the text
	rules.FontWeight = FontWeight::Bold;
	B3D_TEST_ASSERT(rules.GetFont() == regularFace)

	rules.FontWeight = FontWeight::Normal;
	B3D_TEST_ASSERT(rules.GetFont() == regularFace)

	// Once the family provides a bold face, a bold request resolves to it rather than to the substitute
	B3D_TEST_ASSERT(rules.FontFamily->AddFace(regularFace, FontFaceStyle(FontWeight::Bold)))

	rules.FontWeight = FontWeight::Bold;
	B3D_TEST_ASSERT(rules.FontFamily->HasFace(FontFaceStyle(FontWeight::Bold)))
	B3D_TEST_ASSERT(rules.GetFont() == regularFace)

	// An unassigned family reports no face at all instead of asserting
	rules.FontFamily = nullptr;
	B3D_TEST_ASSERT(rules.GetFont() == nullptr)
}

void GUIStyleSheetTestSuite::TestFontMetaDataReportsFaceIdentity()
{
	const HFont font = GetBuiltinResources().GetDefaultFont();
	B3D_TEST_ASSERT(font != nullptr)
	if(font == nullptr)
		return;

	// Identity is read out of the font file's own tables whenever the renderer is initialized, so it is available for fonts
	// loaded from a package as well as for those created at runtime
	String familyName;
	FontFaceStyle style;
	B3D_TEST_ASSERT(FontFamily::TryGetFaceIdentity(font, familyName, style))

	B3D_TEST_ASSERT(!familyName.empty())
	B3D_TEST_ASSERT(style.Weight >= kMinimumFontWeight && style.Weight <= kMaximumFontWeight)

	// A family built from a single face is named after the family that face reports belonging to, not after the file
	const HFontFamily family = FontFamily::CreateFromFace(font);
	B3D_TEST_ASSERT(family != nullptr)
	if(family != nullptr)
	{
		B3D_TEST_ASSERT(family->GetFamilyName() == familyName)
		B3D_TEST_ASSERT(family->HasFace(style))
	}
}

void GUIStyleSheetTestSuite::TestFontFamilyMatchesClosestFace()
{
	// Every face is a distinct font, so the face that was picked can be identified from the font that comes back. None of
	// them are rendered from, as this exercises which style is picked rather than what it draws
	const HFont lightFace = CreateDistinctFont();
	const HFont regularFace = CreateDistinctFont();
	const HFont boldFace = CreateDistinctFont();
	const HFont italicFace = CreateDistinctFont();

	FontFamilyCreateInformation createInformation;
	createInformation.Name = "TestFamily";
	createInformation.Faces.push_back(FontFamilyFace(lightFace, FontFaceStyle(FontWeight::Light)));
	createInformation.Faces.push_back(FontFamilyFace(regularFace, FontFaceStyle(FontWeight::Normal)));
	createInformation.Faces.push_back(FontFamilyFace(boldFace, FontFaceStyle(FontWeight::Bold)));
	createInformation.Faces.push_back(FontFamilyFace(italicFace, FontFaceStyle(FontWeight::Normal, FontSlant::Italic)));

	const HFontFamily family = FontFamily::Create(createInformation);
	B3D_TEST_ASSERT(family != nullptr)
	if(family == nullptr)
		return;

	B3D_TEST_ASSERT(family->GetFamilyName() == "TestFamily")
	B3D_TEST_ASSERT(family->GetFaces().size() == 4)

	// Duplicate styles are rejected rather than shadowing the face that was added first
	B3D_TEST_ASSERT(!family->AddFace(CreateDistinctFont(), FontFaceStyle(FontWeight::Bold)))
	B3D_TEST_ASSERT(family->GetFaces().size() == 4)
	B3D_TEST_ASSERT(family->GetFace(FontFaceStyle(FontWeight::Bold)) == boldFace)

	// A weight heavier than anything on offer falls back to the heaviest available face, never to a lighter one
	B3D_TEST_ASSERT(!family->HasFace(FontFaceStyle(FontWeight::Black)))
	B3D_TEST_ASSERT(family->GetFace(FontFaceStyle(FontWeight::Black)) == boldFace)

	// A weight lighter than anything on offer likewise falls back to the lightest available face
	B3D_TEST_ASSERT(family->GetFace(FontFaceStyle(FontWeight::Thin)) == lightFace)

	// A request above the regular-to-medium band searches heavier before lighter, so semi-bold takes the bold face rather
	// than the regular one that is the same distance away
	B3D_TEST_ASSERT(family->GetFace(FontFaceStyle(FontWeight::SemiBold)) == boldFace)

	// A request within the band steps across it first, so medium takes the regular face rather than the bold one
	B3D_TEST_ASSERT(!family->HasFace(FontFaceStyle(FontWeight::Medium)))
	B3D_TEST_ASSERT(family->GetFace(FontFaceStyle(FontWeight::Medium)) == regularFace)

	// An exact slant match is preferred over a better weight match in the wrong slant
	B3D_TEST_ASSERT(family->GetFace(FontFaceStyle(FontWeight::Bold, FontSlant::Italic)) == italicFace)

	// Removing the only italic face leaves italic requests resolving to an upright substitute of the right weight
	B3D_TEST_ASSERT(family->RemoveFace(FontFaceStyle(FontWeight::Normal, FontSlant::Italic)))
	B3D_TEST_ASSERT(!family->HasFace(FontFaceStyle(FontWeight::Normal, FontSlant::Italic)))
	B3D_TEST_ASSERT(family->GetFace(FontFaceStyle(FontWeight::Bold, FontSlant::Italic)) == boldFace)

	// An empty family has nothing to substitute, so it reports no face rather than an arbitrary one
	B3D_TEST_ASSERT(family->RemoveFace(FontFaceStyle(FontWeight::Light)))
	B3D_TEST_ASSERT(family->RemoveFace(FontFaceStyle(FontWeight::Normal)))
	B3D_TEST_ASSERT(family->RemoveFace(FontFaceStyle(FontWeight::Bold)))
	B3D_TEST_ASSERT(family->GetFace(FontFaceStyle()) == nullptr)
}

void GUIStyleSheetTestSuite::TestFontFamilyLoadsFacesOnDemand()
{
	// Scanning a font folder records what each face is without reading any font data, so a discovered family starts out
	// holding faces that have not been loaded
	const Vector<String> familyNames = GetFontManager().GetFamilyNames();
	B3D_TEST_ASSERT(!familyNames.empty())

	HFontFamily familyWithUnloadedFace;
	FontFaceStyle unloadedFaceStyle;

	for(const String& familyName : familyNames)
	{
		const HFontFamily family = GetFontManager().TryGetFamily(familyName);
		if(family == nullptr)
			continue;

		const auto found = std::find_if(family->GetFaces().begin(), family->GetFaces().end(),
			[](const FontFamilyFace& face) { return !face.Font.IsLoaded(false); });

		if(found == family->GetFaces().end())
			continue;

		familyWithUnloadedFace = family;
		unloadedFaceStyle = found->Style;

		break;
	}

	B3D_TEST_ASSERT(familyWithUnloadedFace != nullptr)
	if(familyWithUnloadedFace == nullptr)
		return;

	// Asking for the face is what loads it, and the family holds on to it from then on
	const HFont face = familyWithUnloadedFace->GetFace(unloadedFaceStyle);
	B3D_TEST_ASSERT(face.IsLoaded(false))
	B3D_TEST_ASSERT(familyWithUnloadedFace->GetFace(unloadedFaceStyle) == face)
}

void GUIStyleSheetTestSuite::TestFontStyleParsing()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(
		"label\n"
		"{\n"
		"    font-weight: 600;\n"
		"    font-style: italic;\n"
		"}\n");

	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const GUIStyleSheetRules& rules = GetOnlyRules(*styleSheet);
	B3D_TEST_ASSERT(rules.IsPropertySet(GUIStyleSheetPropertyType::FontWeight))
	B3D_TEST_ASSERT(rules.IsPropertySet(GUIStyleSheetPropertyType::FontStyle))

	// Numeric weights are accepted on the same scale the font files report
	B3D_TEST_ASSERT(rules.FontWeight == FontWeight::SemiBold)
	B3D_TEST_ASSERT(rules.FontSlant == FontSlant::Italic)

	// Obliques resolve to the same slant as italics, as a family provides both through one face
	const TShared<GUIStyleSheet> obliqueStyleSheet = ParseStyleSheet("label\n{\n    font-style: oblique;\n}\n");
	B3D_TEST_ASSERT(obliqueStyleSheet != nullptr)
	if(obliqueStyleSheet != nullptr)
		B3D_TEST_ASSERT(GetOnlyRules(*obliqueStyleSheet).FontSlant == FontSlant::Italic)

	// A weight outside the 100-900 scale is a parse error rather than a silently clamped value
	GUIStyleSheetParser parser;
	B3D_TEST_ASSERT(parser.Parse(B3DMakeShared<SourceCode>(String("label\n{\n    font-weight: 1200;\n}\n"))) == nullptr)
	B3D_TEST_ASSERT(!parser.GetErrors().empty())
}

void GUIStyleSheetTestSuite::TestTextMetricsAffectMeasuredSize()
{
	const HFont font = GetBuiltinResources().GetDefaultFont();
	B3D_TEST_ASSERT(font != nullptr)
	if(font == nullptr)
		return;

	const String text = "Measured text";

	GUIStyleSheetRules rules;
	rules.FontFamily = FontFamily::CreateFromFace(font);
	rules.FontSize = 12.0f;

	const GUILogicalSize defaultSize = GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(text, rules, 0);
	B3D_TEST_ASSERT(defaultSize.Width > 0 && defaultSize.Height > 0)

	// Letter spacing widens the text without changing its height
	rules.LetterSpacing = 4.0f;
	const GUILogicalSize trackedSize = GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(text, rules, 0);

	B3D_TEST_ASSERT(trackedSize.Width > defaultSize.Width)
	B3D_TEST_ASSERT(trackedSize.Height == defaultSize.Height)

	// Negative letter spacing tightens it again
	rules.LetterSpacing = -1.0f;
	const GUILogicalSize tightenedSize = GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(text, rules, 0);
	B3D_TEST_ASSERT(tightenedSize.Width < defaultSize.Width)

	// Line height scales the height without changing the width
	rules.LetterSpacing = 0.0f;
	rules.LineHeight = 2.0f;
	const GUILogicalSize spacedSize = GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(text, rules, 0);

	B3D_TEST_ASSERT(spacedSize.Height > defaultSize.Height)
	B3D_TEST_ASSERT(spacedSize.Width == defaultSize.Width)
}

void GUIStyleSheetTestSuite::TestMultipleStyleClassResolution()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kStyleClassSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const TestWidgetScope widgetScope(styleSheet);

	GUILabel* const label = GUILabel::Create(GUIContent(HString("Entry")), "Entry");
	widgetScope.Attach(label);

	B3D_TEST_ASSERT(label->GetStyleSheetClass() == "Entry")
	B3D_TEST_ASSERT(label->GetStyleSheetClasses().Size() == 1)
	B3D_TEST_ASSERT(label->GetPadding().Left == 1)

	// The second class brings in the more specific `label.Entry.Alternate` rule
	label->AddStyleClass("Alternate");
	B3D_TEST_ASSERT(label->GetStyleSheetClasses().Size() == 2)
	B3D_TEST_ASSERT(label->HasStyleClass("Entry") && label->HasStyleClass("Alternate"))
	B3D_TEST_ASSERT(label->GetPadding().Left == 2)

	// Adding the same class twice must not duplicate it, and toggling it off must restore the previous look
	label->AddStyleClass("Alternate");
	B3D_TEST_ASSERT(label->GetStyleSheetClasses().Size() == 2)

	label->ToggleStyleClass("Alternate", false);
	B3D_TEST_ASSERT(label->GetStyleSheetClasses().Size() == 1)
	B3D_TEST_ASSERT(!label->HasStyleClass("Alternate"))
	B3D_TEST_ASSERT(label->GetPadding().Left == 1)

	// Setting the single-class property replaces every class
	label->AddStyleClass("Alternate");
	label->SetStyleSheetClass("Alternate");
	B3D_TEST_ASSERT(label->GetStyleSheetClasses().Size() == 1)
	B3D_TEST_ASSERT(label->GetStyleSheetClass() == "Alternate")
	B3D_TEST_ASSERT(label->GetPadding().Left == 0)

	label->ClearStyleClasses();
	B3D_TEST_ASSERT(label->GetStyleSheetClasses().Empty())
	B3D_TEST_ASSERT(label->GetStyleSheetClass().empty())

	label->Destroy();
}

void GUIStyleSheetTestSuite::TestResolvedRulesetCaching()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kStyleClassSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const TestWidgetScope widgetScope(styleSheet);
	GUIStyleSheetCascade& cascade = *widgetScope.Cascade;

	GUILabel* const firstLabel = GUILabel::Create(GUIContent(HString("First")), "Entry");
	GUILabel* const secondLabel = GUILabel::Create(GUIContent(HString("Second")), "Entry");
	widgetScope.Attach(firstLabel);
	widgetScope.Attach(secondLabel);

	cascade.ResetResolvedRulesetCache();
	cascade.ResetResolutionStatistics();

	// Two elements that look the same to every selector must share one resolution
	const TShared<const GUIStyleSheetStateRulesets> firstRulesets = cascade.BuildStateRulesets(*firstLabel);
	const TShared<const GUIStyleSheetStateRulesets> secondRulesets = cascade.BuildStateRulesets(*secondLabel);

	B3D_TEST_ASSERT(firstRulesets == secondRulesets)
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().RequestCount == 2)
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().FullResolutionCount == 1)
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().GetCacheHitCount() == 1)

	// Toggling between two class sets must flip between two cached results, resolving each only once
	firstLabel->AddStyleClass("Alternate");
	const TShared<const GUIStyleSheetStateRulesets> alternateRulesets = cascade.BuildStateRulesets(*firstLabel);
	B3D_TEST_ASSERT(alternateRulesets != firstRulesets)

	const u64 fullResolutionsAfterFirstToggle = cascade.GetResolutionStatistics().FullResolutionCount;

	for(u32 toggleIndex = 0; toggleIndex < 8; ++toggleIndex)
	{
		const bool isAlternate = (toggleIndex % 2) == 0;
		firstLabel->ToggleStyleClass("Alternate", isAlternate);

		const TShared<const GUIStyleSheetStateRulesets> toggledRulesets = cascade.BuildStateRulesets(*firstLabel);
		B3D_TEST_ASSERT(toggledRulesets == (isAlternate ? alternateRulesets : firstRulesets))
	}

	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().FullResolutionCount == fullResolutionsAfterFirstToggle)

	// Registering another style sheet changes what every element resolves to, so the cache must be dropped
	const HGUIStyleSheet extraStyleSheet = B3DStaticResourceCast<GUIStyleSheet>(GetResources().CreateResourceHandle(ParseChainedPseudoClassStyleSheet()));
	widgetScope.Cascade->RegisterStyleSheet(extraStyleSheet, GUIStyleSheet::kUserImportance);
	cascade.ResetResolutionStatistics();

	cascade.BuildStateRulesets(*firstLabel);
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().FullResolutionCount == 1)

	firstLabel->Destroy();
	secondLabel->Destroy();
}

void GUIStyleSheetTestSuite::TestAncestorStateRestylesDescendants()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kStyleClassSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const TestWidgetScope widgetScope(styleSheet);

	// The nested label sits below a layout, which is not a renderable, so restyling has to reach through it
	TestContainer* const row = B3DNew<TestContainer>("Row");
	GUILabel* const label = GUILabel::Create(GUIContent(HString("Entry")));
	GUILabel* const subtitle = GUILabel::Create(GUIContent(HString("Subtitle")), "Subtitle");
	GUILayoutY* const layout = GUILayoutY::Create();
	GUILabel* const nestedLabel = layout->AddNewElement<GUILabel>(GUIContent(HString("Nested")));

	row->AddChild(label);
	row->AddChild(subtitle);
	row->AddChild(layout);
	widgetScope.Attach(row);

	B3D_TEST_ASSERT(label->GetPadding().Left == 0)
	B3D_TEST_ASSERT(nestedLabel->GetPadding().Left == 0)

	// Hovering the container must restyle the labels inside it, not just the container
	row->AddStateFlags(GUIElementState::Hover);
	B3D_TEST_ASSERT(label->GetPadding().Left == 7)
	B3D_TEST_ASSERT(subtitle->GetPadding().Left == 7)
	B3D_TEST_ASSERT(nestedLabel->GetPadding().Left == 7)

	// The more specific `container.Row:checked label.Subtitle` rule only reaches the label carrying the class
	row->AddStateFlags(GUIElementState::Checked);
	B3D_TEST_ASSERT(label->GetPadding().Left == 7)
	B3D_TEST_ASSERT(subtitle->GetPadding().Left == 9)

	row->RemoveStateFlags(GUIElementState::Hover | GUIElementState::Checked);
	B3D_TEST_ASSERT(label->GetPadding().Left == 0)
	B3D_TEST_ASSERT(subtitle->GetPadding().Left == 0)
	B3D_TEST_ASSERT(nestedLabel->GetPadding().Left == 0)

	row->Destroy();
}

void GUIStyleSheetTestSuite::TestStateChangeOnlyRestylesWhenRuleDependsOnIt()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kStyleClassSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const TestWidgetScope widgetScope(styleSheet);
	GUIStyleSheetCascade& cascade = *widgetScope.Cascade;

	TestContainer* const row = B3DNew<TestContainer>("Row");
	GUILabel* const rowLabel = GUILabel::Create(GUIContent(HString("Row")));
	row->AddChild(rowLabel);

	TestContainer* const plain = B3DNew<TestContainer>("Plain");
	GUILabel* const plainLabel = GUILabel::Create(GUIContent(HString("Plain")));
	plain->AddChild(plainLabel);

	widgetScope.Attach(row);
	widgetScope.Attach(plain);

	// The states of both `container.Row` rules are merged, and nothing else depends on an ancestor state
	const GUIElementStates rowStates = GUIElementState::Hover | GUIElementState::Checked;
	B3D_TEST_ASSERT(cascade.MatchAncestorSelectors(*row).StatesAffectingDescendants == rowStates)
	B3D_TEST_ASSERT(row->GetStatesAffectingDescendants() == rowStates)
	B3D_TEST_ASSERT(plain->GetStatesAffectingDescendants() == GUIElementStates(GUIElementState::Normal))
	B3D_TEST_ASSERT(rowLabel->GetStatesAffectingDescendants() == GUIElementStates(GUIElementState::Normal))

	// No rule asks for a state on a `container.Plain`, so hovering it must not restyle its children
	cascade.ResetResolutionStatistics();
	plain->AddStateFlags(GUIElementState::Hover);
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().RequestCount == 0)

	// No rule asks for focus on a `container.Row`, so focusing it must not restyle its children either
	row->AddStateFlags(GUIElementState::Focus);
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().RequestCount == 0)

	// Hover is asked for, so it restyles the label
	row->AddStateFlags(GUIElementState::Hover);
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().RequestCount > 0)
	B3D_TEST_ASSERT(rowLabel->GetPadding().Left == 7)

	// Once both looks were resolved, hovering again is answered from the cache
	row->RemoveStateFlags(GUIElementState::Hover);
	cascade.ResetResolutionStatistics();

	row->AddStateFlags(GUIElementState::Hover);
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().RequestCount > 0)
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().FullResolutionCount == 0)
	B3D_TEST_ASSERT(rowLabel->GetPadding().Left == 7)

	row->Destroy();
	plain->Destroy();
}

void GUIStyleSheetTestSuite::TestStyleKeyDistinguishesElementParts()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kStyleClassSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const TestWidgetScope widgetScope(styleSheet);

	// A class and an id of the same name are different things to the selectors
	TestContainer* const withClass = B3DNew<TestContainer>("Shared");
	TestContainer* const withId = B3DNew<TestContainer>("");
	withId->StyleId = "Shared";

	widgetScope.Attach(withClass);
	widgetScope.Attach(withId);

	B3D_TEST_ASSERT(withClass->GetStyleKey() != withId->GetStyleKey())

	// Selectors only care which classes are present, not the order they were added in
	TestContainer* const firstThenSecond = B3DNew<TestContainer>("First");
	firstThenSecond->AddStyleClass("Second");

	TestContainer* const secondThenFirst = B3DNew<TestContainer>("Second");
	secondThenFirst->AddStyleClass("First");

	widgetScope.Attach(firstThenSecond);
	widgetScope.Attach(secondThenFirst);

	B3D_TEST_ASSERT(firstThenSecond->GetStyleKey() == secondThenFirst->GetStyleKey())

	withClass->Destroy();
	withId->Destroy();
	firstThenSecond->Destroy();
	secondThenFirst->Destroy();
}

void GUIStyleSheetTestSuite::TestStyleKeyIgnoresUnobservedAncestors()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kStyleClassSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const TestWidgetScope widgetScope(styleSheet);
	GUIStyleSheetCascade& cascade = *widgetScope.Cascade;

	GUILabel* const topLevelLabel = GUILabel::Create(GUIContent(HString("Top")));

	TestContainer* const plain = B3DNew<TestContainer>("Plain");
	GUILabel* const plainLabel = GUILabel::Create(GUIContent(HString("Plain")));
	GUILayoutY* const layout = GUILayoutY::Create();
	GUILabel* const nestedLabel = layout->AddNewElement<GUILabel>(GUIContent(HString("Nested")));
	plain->AddChild(plainLabel);
	plain->AddChild(layout);

	TestContainer* const row = B3DNew<TestContainer>("Row");
	GUILabel* const rowLabel = GUILabel::Create(GUIContent(HString("Row")));
	row->AddChild(rowLabel);

	TestContainer* const childless = B3DNew<TestContainer>("Plain");

	widgetScope.Attach(topLevelLabel);
	widgetScope.Attach(plain);
	widgetScope.Attach(row);
	widgetScope.Attach(childless);

	// No selector can tell a label inside a plain container from one without a parent, however deep it sits
	B3D_TEST_ASSERT(plainLabel->GetStyleKey() == topLevelLabel->GetStyleKey())
	B3D_TEST_ASSERT(nestedLabel->GetStyleKey() == topLevelLabel->GetStyleKey())

	// The row only becomes visible to the selectors of its children once it is in a state they ask for
	B3D_TEST_ASSERT(rowLabel->GetStyleKey() == topLevelLabel->GetStyleKey())

	row->AddStateFlags(GUIElementState::Hover);
	B3D_TEST_ASSERT(rowLabel->GetStyleKey() != topLevelLabel->GetStyleKey())
	B3D_TEST_ASSERT(rowLabel->GetPadding().Left == 7)

	// Changing a class that no ancestor selector can observe restyles the container, but nothing below it
	cascade.ResetResolutionStatistics();
	childless->AddStyleClass("Other");
	const u64 containerOnlyRequestCount = cascade.GetResolutionStatistics().RequestCount;

	cascade.ResetResolutionStatistics();
	plain->AddStyleClass("Other");
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().RequestCount == containerOnlyRequestCount)

	// Gaining the `Row` class makes the container match `container.Row`, but its children only see that once the container
	// is in a state the rule asks for
	cascade.ResetResolutionStatistics();
	plain->AddStyleClass("Row");
	B3D_TEST_ASSERT(cascade.GetResolutionStatistics().RequestCount == containerOnlyRequestCount)

	plain->AddStateFlags(GUIElementState::Hover);
	B3D_TEST_ASSERT(plainLabel->GetPadding().Left == 7)
	B3D_TEST_ASSERT(nestedLabel->GetPadding().Left == 7)

	topLevelLabel->Destroy();
	plain->Destroy();
	row->Destroy();
	childless->Destroy();
}

void GUIStyleSheetTestSuite::TestDescendantsFollowAncestorChanges()
{
	const TShared<GUIStyleSheet> styleSheet = ParseStyleSheet(kStyleClassSource);
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const TestWidgetScope widgetScope(styleSheet);

	TestContainer* const row = B3DNew<TestContainer>("Row");
	TestContainer* const plain = B3DNew<TestContainer>("Plain");
	GUILabel* const movedLabel = GUILabel::Create(GUIContent(HString("Moved")));
	plain->AddChild(movedLabel);

	widgetScope.Attach(row);
	widgetScope.Attach(plain);

	// The row has no children yet when it gets hovered, so a child added afterwards must still see the hover
	row->AddStateFlags(GUIElementState::Hover);

	GUILabel* const addedLabel = GUILabel::Create(GUIContent(HString("Added")));
	row->AddChild(addedLabel);
	B3D_TEST_ASSERT(addedLabel->GetPadding().Left == 7)

	// Moving a label into the hovered row and back out restyles it each time
	row->AddChild(movedLabel);
	B3D_TEST_ASSERT(movedLabel->GetPadding().Left == 7)

	plain->AddChild(movedLabel);
	B3D_TEST_ASSERT(movedLabel->GetPadding().Left == 0)

	// A disabled element cannot be hovered, so the elements below it lose the hovered look while the hover flag stays set
	row->AddStateFlags(GUIElementState::Disabled);
	B3D_TEST_ASSERT(addedLabel->GetPadding().Left == 0)

	row->RemoveStateFlags(GUIElementState::Disabled);
	B3D_TEST_ASSERT(addedLabel->GetPadding().Left == 7)

	row->Destroy();
	plain->Destroy();
}

void GUIStyleSheetTestSuite::TestParentOfCombinatorKeepsChildLevel()
{
	const TShared<GUIStyleSheet> styleSheet = CreateParentOfStyleSheet();
	B3D_TEST_ASSERT(styleSheet != nullptr)
	if(styleSheet == nullptr)
		return;

	const TestWidgetScope widgetScope(styleSheet);

	TestContainer* const row = B3DNew<TestContainer>("Row");
	GUILabel* const directLabel = GUILabel::Create(GUIContent(HString("Direct")));
	TestContainer* const plain = B3DNew<TestContainer>("Plain");
	GUILabel* const nestedLabel = GUILabel::Create(GUIContent(HString("Nested")));
	TestContainer* const innerPlain = B3DNew<TestContainer>("Plain");
	GUILabel* const deeperLabel = GUILabel::Create(GUIContent(HString("Deeper")));
	row->AddChild(directLabel);
	row->AddChild(plain);
	plain->AddChild(nestedLabel);
	plain->AddChild(innerPlain);
	innerPlain->AddChild(deeperLabel);

	GUILabel* const topLevelLabel = GUILabel::Create(GUIContent(HString("Top")));
	TestContainer* const outerPlain = B3DNew<TestContainer>("Plain");
	TestContainer* const outerInnerPlain = B3DNew<TestContainer>("Plain");
	GUILabel* const unrelatedLabel = GUILabel::Create(GUIContent(HString("Unrelated")));
	outerPlain->AddChild(outerInnerPlain);
	outerInnerPlain->AddChild(unrelatedLabel);

	widgetScope.Attach(row);
	widgetScope.Attach(topLevelLabel);
	widgetScope.Attach(outerPlain);
	row->AddStateFlags(GUIElementState::Hover);

	// `container.Row:hover > label` only reaches the label whose direct parent is the row
	B3D_TEST_ASSERT(directLabel->GetPadding().Left == 7)
	B3D_TEST_ASSERT(nestedLabel->GetPadding().Left == 0)

	// No selector matches the plain container in between, but it still decides that the row is not the nested label's
	// parent, so it cannot be left out of the style key
	B3D_TEST_ASSERT(directLabel->GetStyleKey() != nestedLabel->GetStyleKey())

	// Only the level right below the matched row has to be kept. Further levels, and containers away from any row, can
	// be left out as with any other combinator.
	B3D_TEST_ASSERT(deeperLabel->GetStyleKey() == nestedLabel->GetStyleKey())
	B3D_TEST_ASSERT(unrelatedLabel->GetStyleKey() == topLevelLabel->GetStyleKey())

	row->Destroy();
	topLevelLabel->Destroy();
	outerPlain->Destroy();
}
