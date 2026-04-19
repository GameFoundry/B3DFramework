//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup String
	 *  @{
	 */

	/** Represents a location (file, line, column) in source code that's being scanned or parsed. */
	class B3D_EXPORT SourceCodePosition
	{
	public:
		SourceCodePosition() = default;
		SourceCodePosition(u32 row, u32 column, const String& filename);
		SourceCodePosition(const SourceCodePosition& other);
		SourceCodePosition(SourceCodePosition&& other);

		SourceCodePosition& operator=(const SourceCodePosition& other);

		/** Returns the source position as string in the format "Row:Column", e.g. "75:10". */
		String ToString(bool printFilename = true) const;

		u32 GetRow() const { return mRow; }
		u32 GetColumn() const { return mColumn; }

		/** Increases the current row count by 1 and sets the column to 0. */
		void MoveToNextRow();

		/** Increases the current column by 1. */
		void MoveToNextColumn();

		/** Returns true if this is a valid source position. False if row and column are 0. */
		bool IsValid() const;

		/** Resets the source position to (0:0). */
		void Reset();

		bool operator<(const SourceCodePosition& rhs) const;

		operator bool() const { return IsValid(); }

		static const SourceCodePosition kInvalid;
	private:
		u32 mRow = ~0u;
		u32 mColumn = ~0u;
		String mFilename;
	};

	/** Wraps a source code represented as a text string, and provides helpers for movement through the code characters and lines. */
	class B3D_EXPORT SourceCode
	{
	public:
		SourceCode(const String& source);

		/** Returns true if this is a valid source code stream. */
		bool IsValid() const;

		/** Returns the next character from the source, and advances the cursor. */
		char GetNextCharacter();

		/** Ignores the next character and advances the cursor. */
		void SkipNextCharacter() { GetNextCharacter(); }

		/** Returns the current source position. */
		const SourceCodePosition& GetPosition() const { return mPosition; }

		/** Returns the current source line. */
		const String& GetLine() const { return mCurrentLine; }

	protected:
		SourceCode() = default;

		/** Returns the line (if it has already been read) by the zero-based line index. */
		String GetPreviouslyReadLine(u32 lineIndex) const;

		StringStream mStream;
		String mCurrentLine;
		Vector<String> mReadLines;
		SourceCodePosition mPosition;
	};

	/** @} */
} // namespace b3d
