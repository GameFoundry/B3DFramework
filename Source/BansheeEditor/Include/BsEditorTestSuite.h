//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsTestSuite.h"
#include "BsComponent.h"

namespace BansheeEngine
{
	/** @addtogroup Testing-Editor
	 *  @{
	 */
	/** @cond TEST */

	class TestComponentA : public Component
	{
	public:
		HSceneObject ref1;
		HComponent ref2;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;

		TestComponentA(const HSceneObject& parent);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class TestComponentARTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		TestComponentA() {} // Serialization only
	};

	class TestComponentB : public Component
	{
	public:
		HSceneObject ref1;
		String val1;

		/************************************************************************/
		/* 							COMPONENT OVERRIDES                    		*/
		/************************************************************************/

	protected:
		friend class SceneObject;

		TestComponentB(const HSceneObject& parent);

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class TestComponentBRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		TestComponentB() {} // Serialization only
	};

	/** @endcond */

	/**	Contains a set of unit tests for the editor. */
	class EditorTestSuite : public TestSuite
	{
	public:
		EditorTestSuite();

	private:
		/**	Tests SceneObject record undo/redo operation. */
		void SceneObjectRecord_UndoRedo();

		/**	Tests SceneObject delete undo/redo operation. */
		void SceneObjectDelete_UndoRedo();

		/** Tests native diff by modifiying an object, generating a diff and re-applying the modifications. */
		void BinaryDiff();

		/** Tests prefab diff by modifiying a prefab, generating a diff and re-applying the modifications. */
		void TestPrefabDiff();

		/** Tests a complex set of operations on a prefab. */
		void TestPrefabComplex();

		/**	Tests the frame allocator. */
		void TestFrameAlloc();
	};

	/** @} */
}