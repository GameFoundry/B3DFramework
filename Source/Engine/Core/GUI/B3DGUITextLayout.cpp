//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUITextLayout.h"
#include "Allocators/B3DFrameAllocator.h"
#include "Math/B3DMath.h"
#include "Math/B3DVector2.h"
#include "String/B3DUnicode.h"
#include "Text/B3DFont.h"
#include "Text/B3DTextGeometry.h"

using namespace b3d;

void GUITextLayout::Update(const TextSpriteInformation& information)
{
	// An empty layout is always rebuilt, because the font might not have been available the last time
	const bool isUnchanged = !mLines.empty() &&
		mInformation.Text == information.Text &&
		mInformation.Font == information.Font &&
		mInformation.FontSize == information.FontSize &&
		mInformation.Metrics == information.Metrics &&
		mInformation.Size == information.Size &&
		mInformation.HorzAlign == information.HorzAlign &&
		mInformation.VertAlign == information.VertAlign &&
		mInformation.WordWrap == information.WordWrap &&
		mInformation.WordBreak == information.WordBreak;

	if(isUnchanged)
		return;

	mInformation = information;

	mLines.clear();
	mCharacterEdges.clear();
	mCharacterCount = 0;
	mLastLineHeight = 0;

	FrameAllocatorScope frameScope;

	const U32String utf32Text = UTF8::ToUtF32(information.Text);
	const u32 width = (u32)information.Size.Width;
	const u32 height = (u32)information.Size.Height;
	const TTextGeometry<FrameAllocatorTag> textGeometry(utf32Text, information.Font, information.FontSize, width, height, information.WordWrap, information.WordBreak, information.Metrics);

	const u32 lineCount = textGeometry.GetLineCount();
	if(lineCount == 0)
		return;

	FrameVector<Vector2> alignmentOffsets(lineCount);
	TextSprite::GetAlignmentOffsets(textGeometry, width, height, information.HorzAlign, information.VertAlign, alignmentOffsets.data());

	mLines.reserve(lineCount);

	for(u32 lineIndex = 0; lineIndex < lineCount; lineIndex++)
	{
		const TextGeometry::Line& geometryLine = textGeometry.GetLine(lineIndex);

		const bool endsWithNewline = geometryLine.HasNewlineChar() && lineIndex != (lineCount - 1);
		const Line line(mCharacterCount, geometryLine.GetCharacterCount(), (u32)mCharacterEdges.size(), Math::RoundToI32(alignmentOffsets[lineIndex].Y), endsWithNewline);
		mLines.push_back(line);

		mCharacterEdges.resize(mCharacterEdges.size() + line.CharacterCount + 1);
		geometryLine.FillCharacterEdges(&mCharacterEdges[line.FirstEdgeIndex], alignmentOffsets[lineIndex].X);

		mCharacterCount += line.CharacterCount + (line.EndsWithNewline ? 1 : 0);
	}

	mLastLineHeight = Math::RoundToI32(textGeometry.GetLineHeight());
}

void GUITextLayout::Clear()
{
	mInformation = TextSpriteInformation();
	mLines.clear();
	mCharacterEdges.clear();
	mCharacterCount = 0;
	mLastLineHeight = 0;
}

u32 GUITextLayout::GetLineIndex(const GUITextPosition& position) const
{
	if(mLines.empty())
		return 0;

	// Find the last line that starts at or before the position
	const auto nextLine = std::upper_bound(mLines.begin(), mLines.end(), position.CharacterIndex,
		[](u32 characterIndex, const Line& line) { return characterIndex < line.StartCharacterIndex; });

	const u32 lineIndex = (u32)(nextLine - mLines.begin()) - 1;
	if(position.IsEndOfWrappedLine && lineIndex > 0 && mLines[lineIndex].StartCharacterIndex == position.CharacterIndex && IsLineWrapped(lineIndex - 1))
		return lineIndex - 1;

	return lineIndex;
}

GUITextPosition GUITextLayout::FindPosition(const GUIPhysicalPoint& point) const
{
	if(mLines.empty())
		return GUITextPosition();

	const i32 y = (i32)point.Y;
	if(y < mLines[0].Top)
		return GUITextPosition();

	const u32 lastLineIndex = GetLineCount() - 1;
	if(y >= mLines[lastLineIndex].Top + mLastLineHeight)
		return GUITextPosition(mCharacterCount, false);

	// Find the last line that starts at or above the point
	const auto nextLine = std::upper_bound(mLines.begin(), mLines.end(), y,
		[](i32 value, const Line& line) { return value < line.Top; });

	return FindPositionOnLine((u32)(nextLine - mLines.begin()) - 1, (float)point.X);
}

GUITextPosition GUITextLayout::FindPositionOnLine(u32 lineIndex, float x) const
{
	if(lineIndex >= GetLineCount())
		return GUITextPosition(mCharacterCount, false);

	const Line& line = mLines[lineIndex];
	const float* const edges = &mCharacterEdges[line.FirstEdgeIndex];

	for(u32 characterOffset = 0; characterOffset < line.CharacterCount; characterOffset++)
	{
		const float center = (edges[characterOffset] + edges[characterOffset + 1]) * 0.5f;
		if(x <= center)
			return GUITextPosition(line.StartCharacterIndex + characterOffset, false);
	}

	return GUITextPosition(line.StartCharacterIndex + line.CharacterCount, true);
}

GUITextPosition GUITextLayout::GetPositionLeftOf(const GUITextPosition& position) const
{
	if(position.CharacterIndex == 0 || mLines.empty())
		return GUITextPosition();

	// Crossing a word wrap takes a step of its own, since it moves the caret without passing a character
	const u32 lineIndex = GetLineIndex(position);
	if(lineIndex > 0 && position.CharacterIndex == mLines[lineIndex].StartCharacterIndex && IsLineWrapped(lineIndex - 1))
		return GUITextPosition(position.CharacterIndex, true);

	return GUITextPosition(position.CharacterIndex - 1, false);
}

GUITextPosition GUITextLayout::GetPositionRightOf(const GUITextPosition& position) const
{
	if(position.CharacterIndex >= mCharacterCount || mLines.empty())
		return GUITextPosition(mCharacterCount, false);

	// Crossing a word wrap takes a step of its own, since it moves the caret without passing a character
	const u32 lineIndex = GetLineIndex(position);
	if(IsLineWrapped(lineIndex) && position.CharacterIndex == mLines[lineIndex + 1].StartCharacterIndex)
		return GUITextPosition(position.CharacterIndex, false);

	return GUITextPosition(position.CharacterIndex + 1, true);
}

GUIPhysicalArea GUITextLayout::GetCaretBounds(const GUITextPosition& position) const
{
	if(mLines.empty())
		return GUIPhysicalArea();

	const u32 lineIndex = GetLineIndex(position);
	const Line& line = mLines[lineIndex];

	const u32 characterOffset = std::min(position.CharacterIndex - line.StartCharacterIndex, line.CharacterCount);
	const i32 x = Math::RoundToI32(mCharacterEdges[line.FirstEdgeIndex + characterOffset]);

	return GUIPhysicalArea(x, line.Top, 1, GetLineHeight(lineIndex));
}

void GUITextLayout::GetSelectionAreas(u32 startCharacterIndex, u32 endCharacterIndex, Vector<Area2I>& outAreas) const
{
	outAreas.clear();

	for(u32 lineIndex = 0; lineIndex < GetLineCount(); lineIndex++)
	{
		const Line& line = mLines[lineIndex];

		const u32 lineEndCharacterIndex = line.StartCharacterIndex + line.CharacterCount;
		const u32 rangeStart = std::max(startCharacterIndex, line.StartCharacterIndex);
		const u32 rangeEnd = std::min(endCharacterIndex, lineEndCharacterIndex);
		if(rangeStart >= rangeEnd)
			continue;

		const float* const edges = &mCharacterEdges[line.FirstEdgeIndex];
		const i32 left = Math::RoundToI32(edges[rangeStart - line.StartCharacterIndex]);
		const i32 right = Math::RoundToI32(edges[rangeEnd - line.StartCharacterIndex]);

		outAreas.push_back(Area2I(left, line.Top, right - left, GetLineHeight(lineIndex)));
	}
}

i32 GUITextLayout::GetLineHeight(u32 lineIndex) const
{
	if(lineIndex + 1 < GetLineCount())
		return mLines[lineIndex + 1].Top - mLines[lineIndex].Top;

	return mLastLineHeight;
}

bool GUITextLayout::IsLineWrapped(u32 lineIndex) const
{
	return lineIndex + 1 < GetLineCount() && !mLines[lineIndex].EndsWithNewline;
}
