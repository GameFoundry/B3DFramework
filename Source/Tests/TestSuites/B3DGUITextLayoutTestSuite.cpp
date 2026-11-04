//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGUITextLayoutTestSuite.h"

#include "GUI/B3DGUITextLayout.h"
#include "Math/B3DArea2.h"
#include "Resources/B3DBuiltinResources.h"
#include "String/B3DUnicode.h"
#include "Text/B3DFont.h"
#include "Text/B3DTextGeometry.h"

using namespace b3d;

namespace
{
	const char* const kThreeLineText = "Type in me...\nand some more\nand more";
	constexpr float kFontSize = 14.0f;

	/** Returns a description of @p text in the default font, with bounds of the provided size. */
	TextSpriteInformation CreateInformation(const String& text, i32 width = 0, i32 height = 0)
	{
		TextSpriteInformation information;
		information.Text = text;
		information.Font = GetBuiltinResources().GetDefaultFont();
		information.FontSize = kFontSize;
		information.Size = Size2I(width, height);

		return information;
	}

	/** Returns a point in the middle of the height of the line that @p position is on, at the horizontal coordinate @p x. */
	GUIPhysicalPoint CreatePointOnLineOf(const GUITextLayout& layout, const GUITextPosition& position, i32 x)
	{
		const GUIPhysicalArea caretBounds = layout.GetCaretBounds(position);
		return GUIPhysicalPoint(x, (i32)caretBounds.Y + (i32)caretBounds.Height / 2);
	}

	/** Returns the horizontal position of a caret located before the character with index @p characterIndex. */
	i32 GetCaretX(const GUITextLayout& layout, u32 characterIndex)
	{
		return (i32)layout.GetCaretBounds(GUITextPosition(characterIndex, false)).X;
	}
} // namespace

GUITextLayoutTestSuite::GUITextLayoutTestSuite()
{
	B3D_ADD_TEST(GUITextLayoutTestSuite::TestPositionsFollowTextOrderAcrossFontPages)
	B3D_ADD_TEST(GUITextLayoutTestSuite::TestSelectionAreas)
	B3D_ADD_TEST(GUITextLayoutTestSuite::TestStepsAcrossNewlines)
	B3D_ADD_TEST(GUITextLayoutTestSuite::TestWordWrap)
	B3D_ADD_TEST(GUITextLayoutTestSuite::TestAlignment)
	B3D_ADD_TEST(GUITextLayoutTestSuite::TestFractionalLineHeightLeavesNoGaps)
	B3D_ADD_TEST(GUITextLayoutTestSuite::TestPointsOutsideText)
	B3D_ADD_TEST(GUITextLayoutTestSuite::TestEmptyText)
}

void GUITextLayoutTestSuite::TestPositionsFollowTextOrderAcrossFontPages()
{
	const TextSpriteInformation information = CreateInformation(kThreeLineText);

	// Spaces are always on the first page, so this makes the text fail if positions are looked up by quad order
	{
		const U32String utf32Text = UTF8::ToUtF32(information.Text);
		const TTextGeometry<> textGeometry(utf32Text, information.Font, information.FontSize);
		B3D_TEST_ASSERT_MSG(textGeometry.GetPageCount() >= 2, "The test requires text that spans multiple font pages")
	}

	GUITextLayout layout;
	layout.Update(information);

	B3D_TEST_ASSERT(layout.GetLineCount() == 3)
	B3D_TEST_ASSERT(layout.GetCharacterCount() == 36)

	// Character 14 is the "a" that starts the second line, and character 28 the one that starts the third
	B3D_TEST_ASSERT(GetCaretX(layout, 0) == GetCaretX(layout, 14))
	B3D_TEST_ASSERT(GetCaretX(layout, 0) == GetCaretX(layout, 28))

	const u32 lineStartCharacterIndices[] = { 0, 14, 28 };
	const u32 lineEndCharacterIndices[] = { 13, 27, 36 };

	for(u32 lineIndex = 0; lineIndex < 3; lineIndex++)
	{
		for(u32 characterIndex = lineStartCharacterIndices[lineIndex]; characterIndex < lineEndCharacterIndices[lineIndex]; characterIndex++)
		{
			const GUITextPosition position(characterIndex, false);
			B3D_TEST_ASSERT(layout.GetLineIndex(position) == lineIndex)

			const i32 left = GetCaretX(layout, characterIndex);
			const i32 right = GetCaretX(layout, characterIndex + 1);
			B3D_TEST_ASSERT(right > left)

			// The left edge of a character is closest to the position before it, and the right edge to the one after it
			B3D_TEST_ASSERT(layout.FindPosition(CreatePointOnLineOf(layout, position, left)).CharacterIndex == characterIndex)
			B3D_TEST_ASSERT(layout.FindPosition(CreatePointOnLineOf(layout, position, right)).CharacterIndex == characterIndex + 1)
		}
	}
}

void GUITextLayoutTestSuite::TestSelectionAreas()
{
	GUITextLayout layout;
	layout.Update(CreateInformation(kThreeLineText));

	Vector<Area2I> areas;

	// Entire text gets one area per line, each covering the entire line
	layout.GetSelectionAreas(0, layout.GetCharacterCount(), areas);
	B3D_TEST_ASSERT(areas.size() == 3)
	if(areas.size() == 3)
	{
		B3D_TEST_ASSERT(areas[0].X == GetCaretX(layout, 0) && areas[0].X + areas[0].Width == GetCaretX(layout, 13))
		B3D_TEST_ASSERT(areas[1].X == GetCaretX(layout, 14) && areas[1].X + areas[1].Width == GetCaretX(layout, 27))
		B3D_TEST_ASSERT(areas[2].X == GetCaretX(layout, 28) && areas[2].X + areas[2].Width == GetCaretX(layout, 36))

		// Lines touch
		B3D_TEST_ASSERT(areas[0].Y + areas[0].Height == areas[1].Y)
		B3D_TEST_ASSERT(areas[1].Y + areas[1].Height == areas[2].Y)
	}

	// Part of a single line
	layout.GetSelectionAreas(16, 20, areas);
	B3D_TEST_ASSERT(areas.size() == 1)
	if(areas.size() == 1)
		B3D_TEST_ASSERT(areas[0].X == GetCaretX(layout, 16) && areas[0].X + areas[0].Width == GetCaretX(layout, 20))

	// Partial first and last line
	layout.GetSelectionAreas(5, 31, areas);
	B3D_TEST_ASSERT(areas.size() == 3)
	if(areas.size() == 3)
	{
		B3D_TEST_ASSERT(areas[0].X == GetCaretX(layout, 5))
		B3D_TEST_ASSERT(areas[2].X + areas[2].Width == GetCaretX(layout, 31))
	}

	// A newline on its own has nothing to display, and neither does an empty range
	layout.GetSelectionAreas(13, 14, areas);
	B3D_TEST_ASSERT(areas.empty())

	layout.GetSelectionAreas(5, 5, areas);
	B3D_TEST_ASSERT(areas.empty())

	// An empty line is skipped
	layout.Update(CreateInformation("a\n\nb"));
	layout.GetSelectionAreas(0, 4, areas);
	B3D_TEST_ASSERT(areas.size() == 2)
}

void GUITextLayoutTestSuite::TestStepsAcrossNewlines()
{
	GUITextLayout layout;
	layout.Update(CreateInformation(kThreeLineText));

	// Each step passes exactly one character, and a newline counts as one
	GUITextPosition position;
	for(u32 characterIndex = 1; characterIndex <= layout.GetCharacterCount(); characterIndex++)
	{
		position = layout.GetPositionRightOf(position);
		B3D_TEST_ASSERT(position.CharacterIndex == characterIndex)
	}

	position = layout.GetPositionRightOf(position);
	B3D_TEST_ASSERT(position.CharacterIndex == layout.GetCharacterCount())

	for(u32 characterIndex = layout.GetCharacterCount(); characterIndex > 0; characterIndex--)
	{
		position = layout.GetPositionLeftOf(position);
		B3D_TEST_ASSERT(position.CharacterIndex == characterIndex - 1)
	}

	position = layout.GetPositionLeftOf(position);
	B3D_TEST_ASSERT(position.CharacterIndex == 0)

	// The newline's own position is the end of its line, and the position after it the start of the next one
	B3D_TEST_ASSERT(layout.GetLineIndex(GUITextPosition(13, false)) == 0)
	B3D_TEST_ASSERT(layout.GetLineIndex(GUITextPosition(14, false)) == 1)
	B3D_TEST_ASSERT(layout.GetLineIndex(GUITextPosition(14, true)) == 1)
}

void GUITextLayoutTestSuite::TestWordWrap()
{
	const String text = "wrapped words on lines";

	// Find the width of the first word, and use something slightly wider to get one word per line
	GUITextLayout unwrappedLayout;
	unwrappedLayout.Update(CreateInformation(text));
	const i32 wrapWidth = GetCaretX(unwrappedLayout, 8) + 2;

	TextSpriteInformation information = CreateInformation(text, wrapWidth, 200);
	information.WordWrap = true;

	GUITextLayout layout;
	layout.Update(information);
	B3D_TEST_ASSERT(layout.GetLineCount() > 1)
	B3D_TEST_ASSERT(layout.GetCharacterCount() == (u32)text.size())

	// The first word and the space after it make up the first line, so character 8 is shared by the two lines
	const GUITextPosition endOfLine(8, true);
	const GUITextPosition startOfNextLine(8, false);
	B3D_TEST_ASSERT(layout.GetLineIndex(endOfLine) == 0)
	B3D_TEST_ASSERT(layout.GetLineIndex(startOfNextLine) == 1)
	B3D_TEST_ASSERT(layout.GetCaretBounds(endOfLine).X > layout.GetCaretBounds(startOfNextLine).X)

	// Pressing a key at the end of the line only moves the caret across the wrap
	GUITextPosition position = layout.GetPositionRightOf(GUITextPosition(7, false));
	B3D_TEST_ASSERT(position.CharacterIndex == 8 && layout.GetLineIndex(position) == 0)

	position = layout.GetPositionRightOf(position);
	B3D_TEST_ASSERT(position.CharacterIndex == 8 && layout.GetLineIndex(position) == 1)

	position = layout.GetPositionLeftOf(position);
	B3D_TEST_ASSERT(position.CharacterIndex == 8 && layout.GetLineIndex(position) == 0)

	position = layout.GetPositionLeftOf(position);
	B3D_TEST_ASSERT(position.CharacterIndex == 7)

	// Past the end of a wrapped line stays on it
	const GUITextPosition foundPosition = layout.FindPositionOnLine(0, 100000.0f);
	B3D_TEST_ASSERT(foundPosition.CharacterIndex == 8 && layout.GetLineIndex(foundPosition) == 0)

	// Both word-wrapped lines get a selection area of their own
	Vector<Area2I> areas;
	layout.GetSelectionAreas(0, 10, areas);
	B3D_TEST_ASSERT(areas.size() == 2)
}

void GUITextLayoutTestSuite::TestAlignment()
{
	constexpr i32 kWidth = 400;

	TextSpriteInformation information = CreateInformation("long line of text\nshort", kWidth, 100);
	information.HorzAlign = GUIHorizontalTextAlignment::Right;

	GUITextLayout layout;
	layout.Update(information);

	// Start of a line isn't always at zero, and the lines end at the right edge of the bounds
	B3D_TEST_ASSERT(GetCaretX(layout, 0) > 0)
	B3D_TEST_ASSERT(GetCaretX(layout, 18) > GetCaretX(layout, 0))
	B3D_TEST_ASSERT(Math::Abs(GetCaretX(layout, 17) - kWidth) <= 1)
	B3D_TEST_ASSERT(Math::Abs(GetCaretX(layout, 23) - kWidth) <= 1)
}

void GUITextLayoutTestSuite::TestFractionalLineHeightLeavesNoGaps()
{
	TextSpriteInformation information = CreateInformation("a\nb\nc\nd\ne\nf\ng");
	information.Metrics.LineHeight = 1.37f;

	GUITextLayout layout;
	layout.Update(information);

	const GUIPhysicalArea lastCaretBounds = layout.GetCaretBounds(GUITextPosition(layout.GetCharacterCount(), false));
	const i32 textHeight = (i32)lastCaretBounds.Y + (i32)lastCaretBounds.Height;

	// Every row belongs to the line above it or to the next one, and a pointer can't reach anything out of order
	u32 previousLineIndex = 0;
	for(i32 y = 0; y < textHeight; y++)
	{
		const u32 lineIndex = layout.GetLineIndex(layout.FindPosition(GUIPhysicalPoint(0, y)));
		B3D_TEST_ASSERT(lineIndex == previousLineIndex || lineIndex == previousLineIndex + 1)

		previousLineIndex = lineIndex;
	}

	B3D_TEST_ASSERT(previousLineIndex == layout.GetLineCount() - 1)
}

void GUITextLayoutTestSuite::TestPointsOutsideText()
{
	GUITextLayout layout;
	layout.Update(CreateInformation(kThreeLineText));

	B3D_TEST_ASSERT(layout.FindPosition(GUIPhysicalPoint(100, -1)).CharacterIndex == 0)
	B3D_TEST_ASSERT(layout.FindPosition(GUIPhysicalPoint(-100, 100000)).CharacterIndex == layout.GetCharacterCount())

	// To the side of a line resolves to the respective end of that line
	const GUITextPosition secondLine(14, false);
	B3D_TEST_ASSERT(layout.FindPosition(CreatePointOnLineOf(layout, secondLine, -100)).CharacterIndex == 14)
	B3D_TEST_ASSERT(layout.FindPosition(CreatePointOnLineOf(layout, secondLine, 100000)).CharacterIndex == 27)
}

void GUITextLayoutTestSuite::TestEmptyText()
{
	GUITextLayout layout;
	layout.Update(CreateInformation(""));

	B3D_TEST_ASSERT(layout.GetLineCount() == 1)
	B3D_TEST_ASSERT(layout.GetCharacterCount() == 0)
	B3D_TEST_ASSERT(layout.FindPosition(GUIPhysicalPoint(50, 50)).CharacterIndex == 0)
	B3D_TEST_ASSERT(layout.GetPositionRightOf(GUITextPosition()).CharacterIndex == 0)
	B3D_TEST_ASSERT(layout.GetPositionLeftOf(GUITextPosition()).CharacterIndex == 0)

	const GUIPhysicalArea caretBounds = layout.GetCaretBounds(GUITextPosition());
	B3D_TEST_ASSERT(caretBounds.X == 0 && (i32)caretBounds.Height > 0)

	Vector<Area2I> areas;
	layout.GetSelectionAreas(0, 0, areas);
	B3D_TEST_ASSERT(areas.empty())

	// A cleared layout answers the same questions without having any lines
	layout.Clear();
	B3D_TEST_ASSERT(layout.GetLineCount() == 0)
	B3D_TEST_ASSERT(layout.FindPosition(GUIPhysicalPoint(50, 50)).CharacterIndex == 0)
	B3D_TEST_ASSERT(layout.GetLineIndex(GUITextPosition()) == 0)
}
