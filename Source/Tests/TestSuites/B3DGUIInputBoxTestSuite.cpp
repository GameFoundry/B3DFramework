//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGUIInputBoxTestSuite.h"

#include "Components/B3DCamera.h"
#include "GUI/B3DGUICommandEvent.h"
#include "GUI/B3DGUIInputBox.h"
#include "GUI/B3DGUIMouseEvent.h"
#include "GUI/B3DGUIPanel.h"
#include "GUI/B3DGUITextInputEvent.h"
#include "GUI/B3DGUIVirtualButtonEvent.h"
#include "GUI/B3DGUIWidget.h"
#include "Input/B3DVirtualInput.h"
#include "Resources/B3DBuiltinResources.h"
#include "Scene/B3DSceneObject.h"

using namespace b3d;

namespace
{
	const char* const kThreeLineText = "Type in me...\nand some more\nand more";

	// Far enough to always be outside of the input box, wherever it ends up
	constexpr i32 kFarAway = 100000;

	/** Holds an input box in a widget that never renders, so that it resolves its font and bounds from the default styles. */
	struct InputBoxScope
	{
		explicit InputBoxScope(const String& text)
		{
			SceneObject = SceneObject::Create("GUIInputBoxTestWidget");

			Widget = SceneObject->AddComponent<GUIWidget>(HCamera());
			Widget->SetStyleSheetCascade(GetBuiltinResources().GetDefaultGUIStyleSheetCascade());

			InputBox = Widget->GetPanel()->AddNewElement<GUIInputBox>();
			InputBox->SetText(text);
			InputBox->SetPosition(0, 0);
			InputBox->SetWidth(400);
			InputBox->SetHeight(200);

			Widget->UpdateLayout();
		}

		~InputBoxScope()
		{
			if(SceneObject != nullptr)
				SceneObject->Destroy(true);
		}

		InputBoxScope(const InputBoxScope&) = delete;
		InputBoxScope& operator=(const InputBoxScope&) = delete;

		HSceneObject SceneObject;
		HGUIWidget Widget;
		GUIInputBox* InputBox = nullptr;
	};
} // namespace

GUIInputBoxTestSuite::GUIInputBoxTestSuite()
{
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestDragSelectsEverything)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestDragThenSelectKeepsAnchor)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestDragWithShiftExtendsSelection)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestMovingCollapsesSelection)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestTypingReplacesSelection)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestRejectedEditKeepsSelection)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestDeleteAtTextStartKeepsCaret)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestBackspaceAndDeleteAcrossNewline)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestCutWithoutSelection)
	B3D_ADD_TEST(GUIInputBoxTestSuite::TestLineEndingsAreNormalized)
}

void GUIInputBoxTestSuite::SendCommand(GUIInteractable* element, GUICommandEventType type)
{
	GUICommandEvent event;
	event.SetType(type);

	element->DoOnCommandEvent(event);
}

void GUIInputBoxTestSuite::TypeCharacter(GUIInteractable* element, u32 character)
{
	GUITextInputEvent event;
	event.SetData(character);

	element->DoOnTextInputEvent(event);
}

void GUIInputBoxTestSuite::Drag(GUIInteractable* element, i32 fromX, i32 fromY, i32 toX, i32 toY, bool isShiftDown)
{
	const GUIPhysicalPoint from(fromX, fromY);
	const GUIPhysicalPoint to(toX, toY);

	GUIMouseEvent event;
	event.mShift = isShiftDown;

	event.SetMouseDragStartData(from, from);
	element->DoOnMouseEvent(event);

	event.SetMouseDragData(to, to - from);
	element->DoOnMouseEvent(event);

	event.SetMouseDragEndData(to);
	element->DoOnMouseEvent(event);
}

void GUIInputBoxTestSuite::PressCut(GUIInteractable* element)
{
	GUIVirtualButtonEvent event;
	event.SetButton(VirtualInput::GetOrCreateVirtualButton("Cut"));

	element->DoOnVirtualButtonEvent(event);
}

void GUIInputBoxTestSuite::TestDragSelectsEverything()
{
	InputBoxScope scope(kThreeLineText);
	SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

	// From the bottom right of the text to its top left
	Drag(scope.InputBox, kFarAway, kFarAway, -kFarAway, -kFarAway);

	TypeCharacter(scope.InputBox, 'x');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "x")
}

void GUIInputBoxTestSuite::TestDragThenSelectKeepsAnchor()
{
	InputBoxScope scope("abcd");
	SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

	// Dragging left leaves the caret at the moving end, at the start of the text
	Drag(scope.InputBox, kFarAway, kFarAway, -kFarAway, -kFarAway);

	// The selection shrinks from the moving end, so the fixed one is still at the end of the text
	SendCommand(scope.InputBox, GUICommandEventType::SelectRight);

	TypeCharacter(scope.InputBox, 'x');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "ax")
}

void GUIInputBoxTestSuite::TestDragWithShiftExtendsSelection()
{
	InputBoxScope scope("abcd");
	SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

	// Leave the caret at the start of the text with nothing selected
	SendCommand(scope.InputBox, GUICommandEventType::MoveLeft);

	// With Shift held the press location is ignored, and the existing caret location stays as the fixed end
	Drag(scope.InputBox, kFarAway, kFarAway, kFarAway, kFarAway, true);

	TypeCharacter(scope.InputBox, 'x');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "x")
}

void GUIInputBoxTestSuite::TestMovingCollapsesSelection()
{
	{
		InputBoxScope scope("abcd");
		SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

		SendCommand(scope.InputBox, GUICommandEventType::MoveLeft);
		TypeCharacter(scope.InputBox, 'x');
		B3D_TEST_ASSERT(scope.InputBox->GetText() == "xabcd")
	}

	{
		InputBoxScope scope("abcd");
		SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

		SendCommand(scope.InputBox, GUICommandEventType::MoveRight);
		TypeCharacter(scope.InputBox, 'x');
		B3D_TEST_ASSERT(scope.InputBox->GetText() == "abcdx")
	}
}

void GUIInputBoxTestSuite::TestTypingReplacesSelection()
{
	InputBoxScope scope("abcd");
	SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

	// Gaining focus selects everything
	TypeCharacter(scope.InputBox, 'x');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "x")

	// Further characters go after the previous one, since nothing is selected anymore
	TypeCharacter(scope.InputBox, 'y');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "xy")

	// Keyboard selection
	SendCommand(scope.InputBox, GUICommandEventType::SelectLeft);
	TypeCharacter(scope.InputBox, 'z');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "xz")
}

void GUIInputBoxTestSuite::TestRejectedEditKeepsSelection()
{
	InputBoxScope scope("abcd");
	scope.InputBox->SetFilter([](const String& text) { return text.find('1') == String::npos; });

	SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

	// Rejected by the filter, so the selected text stays and it is still selected
	TypeCharacter(scope.InputBox, '1');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "abcd")

	TypeCharacter(scope.InputBox, 'x');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "x")
}

void GUIInputBoxTestSuite::TestDeleteAtTextStartKeepsCaret()
{
	InputBoxScope scope("abcd");
	SendCommand(scope.InputBox, GUICommandEventType::FocusGained);
	SendCommand(scope.InputBox, GUICommandEventType::MoveLeft);

	SendCommand(scope.InputBox, GUICommandEventType::Delete);
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "bcd")

	TypeCharacter(scope.InputBox, 'x');
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "xbcd")
}

void GUIInputBoxTestSuite::TestBackspaceAndDeleteAcrossNewline()
{
	InputBoxScope scope("ab\ncd");
	SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

	// Collapse to the start of the text, then step to the end of the first line
	SendCommand(scope.InputBox, GUICommandEventType::MoveLeft);
	SendCommand(scope.InputBox, GUICommandEventType::MoveRight);
	SendCommand(scope.InputBox, GUICommandEventType::MoveRight);

	SendCommand(scope.InputBox, GUICommandEventType::Delete);
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "abcd")

	SendCommand(scope.InputBox, GUICommandEventType::Backspace);
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "acd")

	// Nothing before the start of the text
	SendCommand(scope.InputBox, GUICommandEventType::MoveLeft);
	SendCommand(scope.InputBox, GUICommandEventType::Backspace);
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "acd")
}

void GUIInputBoxTestSuite::TestCutWithoutSelection()
{
	InputBoxScope scope("abcd");
	SendCommand(scope.InputBox, GUICommandEventType::FocusGained);

	// Everything was selected on focus, but moving the caret ended that selection
	SendCommand(scope.InputBox, GUICommandEventType::MoveRight);

	PressCut(scope.InputBox);
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "abcd")
}

void GUIInputBoxTestSuite::TestLineEndingsAreNormalized()
{
	InputBoxScope scope("a\r\nb\rc\n");
	B3D_TEST_ASSERT(scope.InputBox->GetText() == "a\nb\nc\n")
}
