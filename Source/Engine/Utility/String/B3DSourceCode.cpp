//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "String/B3DSourceCode.h"

using namespace b3d;

const SourceCodePosition SourceCodePosition::kInvalid{};

SourceCodePosition::SourceCodePosition(u32 row, u32 column, const String& filename)
	: mRow(row), mColumn(column), mFilename(filename)
{ }

SourceCodePosition::SourceCodePosition(const SourceCodePosition& other)
{
	mRow = other.mRow;
	mColumn = other.mColumn;
	mFilename = other.mFilename;
}

SourceCodePosition::SourceCodePosition(SourceCodePosition&& other)
{
	mRow = other.mRow;
	mColumn = other.mColumn;
	mFilename = std::move(other.mFilename);
}

SourceCodePosition& SourceCodePosition::operator=(const SourceCodePosition& other)
{
	mRow = other.mRow;
	mColumn = other.mColumn;
	mFilename = other.mFilename;

	return *this;
}

String SourceCodePosition::ToString(bool printFilename) const
{
	StringStream stringStream;

	if(printFilename && !mFilename.empty())
		stringStream << "File:" << mFilename << " | ";

	stringStream << "Line: " << (mRow + 1) << " | Column: " << (mColumn + 1);
	return stringStream.str();
}

void SourceCodePosition::MoveToNextRow()
{
	++mRow;
	mColumn = 0;
}

void SourceCodePosition::MoveToNextColumn()
{
	++mColumn;
}

bool SourceCodePosition::IsValid() const
{
	return mRow != ~0u && mColumn != ~0u;
}

void SourceCodePosition::Reset()
{
	mRow = ~0u;
	mColumn = ~0u;
}

bool SourceCodePosition::operator<(const SourceCodePosition& rhs) const
{
	if(mFilename.data() < rhs.mFilename.data())
		return true;
	else if(mFilename.data() > rhs.mFilename.data())
		return false;

	if(mRow < rhs.mRow)
		return true;

	if(mRow > rhs.mRow)
		return false;

	return mColumn < rhs.mColumn;
}

SourceCode::SourceCode(const String& source)
{
	mStream << source;
	mStream.seekg(0, std::ios::beg);
}

bool SourceCode::IsValid() const
{
	return mStream.good();
}

char SourceCode::GetNextCharacter()
{
	// Check if reader is at end-of-line
	while(mPosition.GetColumn() >= mCurrentLine.size())
	{
		// Check if end-of-file is reached.
		if(!IsValid() || mStream.eof())
			return 0;

		// Read new line in source file
		std::getline(mStream, mCurrentLine);
		mCurrentLine += '\n';
		mPosition.MoveToNextRow();

		// Store current line for later reports
		mReadLines.push_back(mCurrentLine);
	}

	// Increment column and return current character
	const char character = mCurrentLine[mPosition.GetColumn()];
	mPosition.MoveToNextColumn();

	return character;
}

String SourceCode::GetPreviouslyReadLine(u32 lineIndex) const
{
	return lineIndex < mReadLines.size() ? mReadLines[lineIndex] : "";
}
