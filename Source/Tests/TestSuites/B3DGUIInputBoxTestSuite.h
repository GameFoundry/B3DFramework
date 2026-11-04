//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class GUIInputBox;
	class GUIInteractable;
	enum class GUICommandEventType;

	class GUIInputBoxTestSuite : public TestSuite
	{
	public:
		GUIInputBoxTestSuite();

	private:
		void TestDragSelectsEverything();
		void TestDragThenSelectKeepsAnchor();
		void TestDragWithShiftExtendsSelection();
		void TestMovingCollapsesSelection();
		void TestTypingReplacesSelection();
		void TestRejectedEditKeepsSelection();
		void TestDeleteAtTextStartKeepsCaret();
		void TestBackspaceAndDeleteAcrossNewline();
		void TestCutWithoutSelection();
		void TestLineEndingsAreNormalized();

		/** Delivers a command event to @p element. */
		void SendCommand(GUIInteractable* element, GUICommandEventType type);

		/** Types a single character into @p element. */
		void TypeCharacter(GUIInteractable* element, u32 character);

		/** Sends the events of a mouse drag from the point @p fromX, @p fromY to the point @p toX, @p toY. */
		void Drag(GUIInteractable* element, i32 fromX, i32 fromY, i32 toX, i32 toY, bool isShiftDown = false);

		/** Triggers the Cut virtual button on @p element. */
		void PressCut(GUIInteractable* element);
	};
} // namespace b3d
