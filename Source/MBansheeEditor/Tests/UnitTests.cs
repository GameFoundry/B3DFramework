//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Text;
using BansheeEngine;
using DebugUnit = System.Diagnostics.Debug;

namespace BansheeEditor
{
    /** @addtogroup Tests
     *  @{
     */

    /// <summary>
    /// Contains various managed unit tests.
    /// </summary>
    class UnitTests
    {
        /// <summary>
        /// Triggers an exception when a unit test condition fails.
        /// </summary>
        /// <param name="success">True if the unit test condition succeeded, false otherwise.</param>
        static void Assert(bool success)
        {
            if (!success)
                throw new InvalidOperationException("Unit test failed.");
        }

        /// <summary>
        /// Tests managed object serialization and deserialization.
        /// </summary>
        static void UnitTest1_ManagedSerialization()
        {
            SceneObject otherSO = new SceneObject("OtherSO");
            UT1_Component2 dbgComponent2 = otherSO.AddComponent<UT1_Component2>();
            dbgComponent2.a2 = 33;

            SceneObject so = new SceneObject("TestSO");
            UT1_Component1 dbgComponent = so.AddComponent<UT1_Component1>();

            dbgComponent.a = 5;
            dbgComponent.b = "SomeTestVal";
            dbgComponent.complex.someValue = 19;
            dbgComponent.complex.anotherValue = "AnotherValue";
            dbgComponent.complex2.someValue2 = 21;
            dbgComponent.complex2.anotherValue2 = "AnotherValue2";

            dbgComponent.arrA = new int[5];
            dbgComponent.arrA[4] = 5;
            dbgComponent.arrB = new string[5];
            dbgComponent.arrB[4] = "ArrAnotherValue";
            dbgComponent.arrComplex = new UT1_SerzObj[5];
            dbgComponent.arrComplex[4].someValue = 99;
            dbgComponent.arrComplex[4].anotherValue = "ArrComplexAnotherValue";
            dbgComponent.arrComplex2 = new UT1_SerzCls[5];
            dbgComponent.arrComplex2[4] = new UT1_SerzCls();
            dbgComponent.arrComplex2[4].someValue2 = 101;
            dbgComponent.arrComplex2[4].anotherValue2 = "ArrComplex2AnotherValue";

            dbgComponent.listA = new List<int>();
            dbgComponent.listA.Add(5);
            dbgComponent.listB = new List<string>();
            dbgComponent.listB.Add("ListAnotherValue");
            dbgComponent.listB.Add(null);
            dbgComponent.listComplex = new List<UT1_SerzObj>();
            dbgComponent.listComplex.Add(new UT1_SerzObj());
            dbgComponent.listComplex.Add(new UT1_SerzObj(99, "ListComplexAnotherValue"));
            dbgComponent.listComplex2 = new List<UT1_SerzCls>();
            dbgComponent.listComplex2.Add(new UT1_SerzCls());
            dbgComponent.listComplex2[0].someValue2 = 101;
            dbgComponent.listComplex2[0].anotherValue2 = "ListComplexAnotherValue";
            dbgComponent.listComplex2.Add(null);

            dbgComponent.dictA = new Dictionary<int, string>();
            dbgComponent.dictA[5] = "value";
            dbgComponent.dictA[10] = "anotherValue";

            dbgComponent.dictB = new Dictionary<string, UT1_SerzObj>();
            dbgComponent.dictB["key1"] = new UT1_SerzObj(99, "DictComplexValue");

            dbgComponent.otherComponent = dbgComponent2;
            dbgComponent.otherSO = otherSO;

            Internal_UT1_GameObjectClone(so);

            System.Diagnostics.Debug.Assert(so.GetNumChildren() == 1);

            for (int i = 0; i < so.GetNumChildren(); i++)
            {
                SceneObject childSO = so.GetChild(i);

                UT1_Component1 otherComponent = childSO.GetComponent<UT1_Component1>();
                DebugUnit.Assert(otherComponent.a == 5);
                DebugUnit.Assert(otherComponent.b == "SomeTestVal");
                DebugUnit.Assert(otherComponent.complex.someValue == 19);
                DebugUnit.Assert(otherComponent.complex2.anotherValue2 == "AnotherValue2");

                DebugUnit.Assert(otherComponent.arrA[4] == 5);
                DebugUnit.Assert(otherComponent.arrB[4] == "ArrAnotherValue");
                DebugUnit.Assert(otherComponent.arrComplex[4].someValue == 99);
                DebugUnit.Assert(otherComponent.arrComplex2[4].anotherValue2 == "ArrComplex2AnotherValue");

                DebugUnit.Assert(otherComponent.listA[0] == 5);
                DebugUnit.Assert(otherComponent.listB[0] == "ListAnotherValue");
                DebugUnit.Assert(otherComponent.listComplex[1].someValue == 99);
                DebugUnit.Assert(otherComponent.listComplex2[0].anotherValue2 == "ListComplexAnotherValue");
            }

            so.Destroy();
            otherSO.Destroy();
        }

        /// <summary>
        /// Tests serializable properties used for inspection.
        /// </summary>
        static void UnitTest2_SerializableProperties()
        {
            SerializableObject obj = new SerializableObject(typeof(UT1_SerzCls), new UT1_SerzCls());

            SerializableProperty prop = obj.Fields[0].GetProperty();
            prop.SetValue(33);
            DebugUnit.Assert(prop.GetValue<int>() == 33);

            SerializableProperty prop2 = obj.Fields[2].GetProperty();

            UT1_SerzCls child = new UT1_SerzCls();
            child.anotherValue2 = "potato";
            prop2.SetValue<UT1_SerzCls>(child);

            DebugUnit.Assert(prop2.GetValue<UT1_SerzCls>() != null);
            DebugUnit.Assert(prop2.GetValue<UT1_SerzCls>().anotherValue2 == "potato");
        }

        /// <summary>
        /// Tests managed diff creation used by prefabs.
        /// </summary>
        static void UnitTest3_ManagedDiff()
        {
            UT_DiffObj original = new UT_DiffObj();
            UT_DiffObj modified = new UT_DiffObj();

            modified.plain2 = "banana";
            modified.complex = new UT_DiffChildObj();
            modified.complex2 = null;
            modified.complex3.plain2 = "tomato";

            modified.arrPlain1 = new[] {-1, -2, -3, -4};
            modified.arrPlain2[2] = "cherry";
            modified.arrComplex = new UT_DiffChildObj[3];
            modified.arrComplex2[0].plain1 = -10;

            modified.listPlain1[0] = -20;
            modified.listPlain2 = new List<string>();
            modified.listComplex = new List<UT_DiffChildObj>();
            modified.listComplex.Add(new UT_DiffChildObj());
            modified.listComplex2[1].plain2 = "orange";

            modified.dictPlain1.Remove(20);
            modified.dictPlain1[-30] = -30;
            modified.dictComplex = new Dictionary<int, UT_DiffChildObj>();
            modified.dictComplex[-40] = new UT_DiffChildObj();
            modified.dictComplex2[31].plain1 = -50;

            Internal_UT3_GenerateDiff(original, modified);
            Internal_UT3_ApplyDiff(original);

            DebugUnit.Assert(original.plain1 == modified.plain1);
            DebugUnit.Assert(original.plain2 == modified.plain2);
            DebugUnit.Assert(original.complex.plain2 == modified.complex.plain2);
            DebugUnit.Assert(original.complex2 == modified.complex2);
            DebugUnit.Assert(original.complex3.plain2 == modified.complex3.plain2);

            DebugUnit.Assert(original.arrPlain1.Length == modified.arrPlain1.Length);
            for (int i = 0; i < original.arrPlain1.Length; i++)
                DebugUnit.Assert(original.arrPlain1[i] == modified.arrPlain1[i]);

            for (int i = 0; i < original.arrPlain2.Length; i++)
                DebugUnit.Assert(original.arrPlain2[i] == modified.arrPlain2[i]);

            for (int i = 0; i < original.arrComplex.Length; i++)
                DebugUnit.Assert(original.arrComplex[i] == modified.arrComplex[i]);

            DebugUnit.Assert(original.arrComplex2[0].plain1 == modified.arrComplex2[0].plain1);

            for (int i = 0; i < original.listPlain1.Count; i++)
                DebugUnit.Assert(original.listPlain1[i] == modified.listPlain1[i]);

            DebugUnit.Assert(original.listPlain2.Count == modified.listPlain2.Count);

            for (int i = 0; i < original.listComplex.Count; i++)
                DebugUnit.Assert(original.listComplex[i].plain1 == modified.listComplex[i].plain1);

            DebugUnit.Assert(original.listComplex2[1].plain2 == modified.listComplex2[1].plain2);

            foreach (var entry in modified.dictPlain1)
            {
                if (!original.dictPlain1.ContainsKey(entry.Key))
                    DebugUnit.Assert(false);

                DebugUnit.Assert(entry.Value == original.dictPlain1[entry.Key]);
            }

            foreach (var entry in modified.dictPlain2)
            {
                if (!original.dictPlain2.ContainsKey(entry.Key))
                    DebugUnit.Assert(false);

                DebugUnit.Assert(entry.Value == original.dictPlain2[entry.Key]);
            }

            foreach (var entry in modified.dictComplex)
            {
                if (!original.dictComplex.ContainsKey(entry.Key))
                    DebugUnit.Assert(false);

                DebugUnit.Assert(entry.Value.plain1 == original.dictComplex[entry.Key].plain1);
            }

            foreach (var entry in modified.dictComplex2)
            {
                if (!original.dictComplex2.ContainsKey(entry.Key))
                    DebugUnit.Assert(false);

                DebugUnit.Assert(entry.Value.plain1 == original.dictComplex2[entry.Key].plain1);
            }
        }

        /// <summary>
        /// Tests saving, loading and updating of prefabs.
        /// </summary>
        private static void UnitTest4_Prefabs()
        {
            if (!EditorApplication.IsProjectLoaded)
            {
                Debug.LogWarning("Skipping unit test as no project is loaded.");
                return;
            }

            if (EditorApplication.IsSceneModified())
            {
                Debug.LogWarning("Cannot perform unit test as the current scene is modified.");
                return;
            }

            Action PrintSceneState = () =>
            {
                SceneObject root = Scene.Root;

                Stack<SceneObject> todo = new Stack<SceneObject>();
                todo.Push(root);

                StringBuilder output = new StringBuilder();
                while (todo.Count > 0)
                {
                    SceneObject so = todo.Pop();

                    int numChildren = so.GetNumChildren();
                    for (int i = numChildren - 1; i >= 0; i--)
                    {
                        SceneObject child = so.GetChild(i);

                        output.AppendLine(child.Name);
                        todo.Push(child);
                    }  
                }

                Debug.Log(output);
            };

            // Disabled because it's a slow test, enable only when relevant (or when a build machine is set up)
            /*
            string oldScene = Scene.ActiveSceneUUID;
            Scene.Clear();

            try
            {
                // Simple scene save & load
                {
                    {
                        // unitTest4Scene_0.prefab:
                        // so0 (Comp1)
                        //  - so0_0
                        //  - so0_1 (Comp1)
                        //    - so0_1_0 (Comp1)
                        // so1 (Comp2)
                        //  - so1_0

                        SceneObject so0 = new SceneObject("so0");
                        SceneObject so1 = new SceneObject("so1");
                        SceneObject so0_0 = new SceneObject("so0_0");
                        SceneObject so0_1 = new SceneObject("so0_1");
                        SceneObject so1_0 = new SceneObject("so1_0");
                        SceneObject so0_1_0 = new SceneObject("so0_1_0");

                        so0_0.Parent = so0;
                        so0_1.Parent = so0;
                        so1_0.Parent = so1;
                        so0_1_0.Parent = so0_1;

                        so0_1_0.LocalPosition = new Vector3(10.0f, 15.0f, 20.0f);
                        so0_1.LocalPosition = new Vector3(1.0f, 2.0f, 3.0f);
                        so1_0.LocalPosition = new Vector3(0, 123.0f, 0.0f);

                        UT1_Component1 comp0 = so0.AddComponent<UT1_Component1>();
                        UT1_Component2 comp1 = so1.AddComponent<UT1_Component2>();
                        UT1_Component1 comp1_1 = so0_1.AddComponent<UT1_Component1>();
                        UT1_Component1 comp0_1_0 = so0_1_0.AddComponent<UT1_Component1>();

                        comp0.otherSO = so0_1_0;
                        comp0.otherComponent = comp1;

                        comp1_1.b = "originalValue2";

                        comp0_1_0.b = "testValue";
                        comp0_1_0.otherSO = so0;
                        comp0_1_0.otherComponent2 = comp0;

                        EditorApplication.SaveScene("unitTest4Scene_0.prefab");
                    }
                    {
                        EditorApplication.LoadScene("unitTest4Scene_0.prefab");

                        SceneObject sceneRoot = Scene.Root;
                        SceneObject so0 = sceneRoot.FindChild("so0", false);
                        SceneObject so1 = sceneRoot.FindChild("so1", false);
                        SceneObject so0_0 = so0.FindChild("so0_0", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

                        Assert(so0_0 != null);
                        Assert(so0_1 != null);
                        Assert(so0_1_0 != null);

                        UT1_Component1 comp0 = so0.GetComponent<UT1_Component1>();
                        UT1_Component2 comp1 = so1.GetComponent<UT1_Component2>();
                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();

                        Assert(comp0 != null);
                        Assert(comp1 != null);
                        Assert(comp0_1_0 != null);
                        Assert(comp0_1_0.b == "testValue");
                        Assert(comp0.otherSO == so0_1_0);
                        Assert(comp0.otherComponent == comp1);
                        Assert(comp0_1_0.otherSO == so0);
                        Assert(comp0_1_0.otherComponent2 == comp0);
                    }
                }

                Debug.Log("Passed stage 1");

                // Load & save a scene that contains a prefab and references its objects
                {
                    {
                        // unitTest4Scene_1.prefab:
                        // parentSO0
                        //  - [unitTest4Scene_0.prefab]
                        // parentSO1
                        //  - parentSO1_0 (Comp1)

                        Scene.Clear();

                        SceneObject parentSO0 = new SceneObject("parentSO0", false);
                        SceneObject parentSO1 = new SceneObject("parentSO1", false);
                        SceneObject parentSO1_0 = new SceneObject("parentSO1_0", false);

                        parentSO1_0.Parent = parentSO1;
                        parentSO0.LocalPosition = new Vector3(50.0f, 50.0f, 50.0f);

                        UT1_Component1 parentComp1_0 = parentSO1_0.AddComponent<UT1_Component1>();

                        Prefab scene0Prefab = ProjectLibrary.Load<Prefab>("unitTest4Scene_0.prefab");
                        SceneObject prefabInstance = scene0Prefab.Instantiate();
                        prefabInstance.Parent = parentSO0;
                        prefabInstance.LocalPosition = Vector3.Zero;

                        SceneObject so0 = prefabInstance.FindChild("so0", false);
                        SceneObject so1 = prefabInstance.FindChild("so1", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();

                        parentComp1_0.otherSO = so1_0;
                        parentComp1_0.otherComponent2 = comp0_1_0;

                        EditorApplication.SaveScene("unitTest4Scene_1.prefab");
                    }
                    {
                        EditorApplication.LoadScene("unitTest4Scene_1.prefab");

                        SceneObject parentSO0 = Scene.Root.FindChild("parentSO0", false);
                        SceneObject parentSO1 = Scene.Root.FindChild("parentSO1", false);
                        SceneObject parentSO1_0 = parentSO1.FindChild("parentSO1_0", false);

                        UT1_Component1 parentComp1_0 = parentSO1_0.GetComponent<UT1_Component1>();

                        SceneObject prefabInstance = parentSO0.GetChild(0);
                        SceneObject so0 = prefabInstance.FindChild("so0", false);
                        SceneObject so1 = prefabInstance.FindChild("so1", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();

                        Assert(parentComp1_0.otherSO == so1_0);
                        Assert(parentComp1_0.otherComponent2 == comp0_1_0);
                    }
                }

                Debug.Log("Passed stage 2");

                // Modify prefab, reload the scene and ensure it is updated with modified prefab
                {
                    {
                        // unitTest4Scene_0.prefab:
                        // so0
                        //  - so0_1 (Comp1)
                        //    - so0_1_0 (Comp1)
                        // so1 (Comp1, Comp2)
                        //  - so1_0
                        //  - so1_1

                        Scene.Load("unitTest4Scene_0.prefab");

                        SceneObject sceneRoot = Scene.Root;
                        SceneObject so0 = sceneRoot.FindChild("so0", false);
                        SceneObject so0_0 = so0.FindChild("so0_0", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1 = sceneRoot.FindChild("so1", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

                        SceneObject so1_1 = new SceneObject("so1_1");
                        so1_1.Parent = so1;

                        so0.RemoveComponent<UT1_Component1>();
                        UT1_Component1 comp1 = so1.AddComponent<UT1_Component1>();
                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();

                        so0_0.Destroy();

                        comp1.otherSO = so1_0;
                        comp1.otherComponent2 = comp0_1_0;

                        comp0_1_0.otherSO = so1_1;
                        comp0_1_0.otherComponent2 = comp1;
                        comp0_1_0.a = 123;
                        comp0_1_0.b = "modifiedValue";

                        so1.Name = "so1_modified";
                        so1.LocalPosition = new Vector3(0, 999.0f, 0.0f);

                        EditorApplication.SaveScene("unitTest4Scene_0.prefab");
                    }

                    {
                        EditorApplication.LoadScene("unitTest4Scene_1.prefab");

                        SceneObject parentSO0 = Scene.Root.FindChild("parentSO0", false);
                        SceneObject parentSO1 = Scene.Root.FindChild("parentSO1", false);
                        SceneObject parentSO1_0 = parentSO1.FindChild("parentSO1_0", false);

                        UT1_Component1 parentComp1_0 = parentSO1_0.GetComponent<UT1_Component1>();

                        SceneObject prefabInstance = parentSO0.GetChild(0);
                        SceneObject so0 = prefabInstance.FindChild("so0", false);
                        SceneObject so1 = prefabInstance.FindChild("so1_modified", false);
                        SceneObject so0_0 = so0.FindChild("so0_0", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);
                        SceneObject so1_1 = so1.FindChild("so1_1", false);

                        UT1_Component1 comp0 = so0.GetComponent<UT1_Component1>();
                        UT1_Component1 comp1 = so1.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();

                        Assert(parentComp1_0.otherSO == so1_0);
                        Assert(parentComp1_0.otherComponent2 == comp0_1_0);
                        Assert(so1_1 != null);
                        Assert(so0_0 == null);
                        Assert(comp0 == null);
                        Assert(comp0_1_0.otherSO == so1_1);
                        Assert(comp0_1_0.otherComponent2 == comp1);
                        Assert(comp0_1_0.a == 123);
                        Assert(comp0_1_0.b == "modifiedValue");
                        Assert(comp1.otherSO == so1_0);
                        Assert(comp1.otherComponent2 == comp0_1_0);
                        Assert(MathEx.ApproxEquals(so1.LocalPosition.y, 999.0f));
                    }
                }

                Debug.Log("Passed stage 3");

                // Make instance specific changes to the prefab, modify the prefab itself and ensure
                // both changes persist
                {
                    // Create new scene referencing the prefab and make instance modifications
                    {
                        // unitTest4Scene_2.prefab:
                        // parent2SO0
                        //  - [unitTest4Scene_0.prefab]
                        // parent2SO1
                        //  - parent2SO1_0 (Comp1)

                        // unitTest4Scene_0.prefab (unitTest4Scene_2.prefab instance):
                        // so0 (Comp1(INSTANCE))
                        //  - so0_0 (INSTANCE)
                        //  - so0_1 (Comp1)
                        //    - so0_1_0 (Comp1)
                        // so1 (Comp2)
                        //  - so1_0

                        Scene.Clear();

                        SceneObject parent2SO0 = new SceneObject("parent2SO0");
                        SceneObject parent2SO1 = new SceneObject("parent2SO1");
                        SceneObject parent2SO1_0 = new SceneObject("parent2SO1_0");

                        parent2SO1_0.Parent = parent2SO1;

                        UT1_Component1 parentComp1_0 = parent2SO1_0.AddComponent<UT1_Component1>();

                        Prefab scene0Prefab = ProjectLibrary.Load<Prefab>("unitTest4Scene_0.prefab");
                        SceneObject prefabInstance = scene0Prefab.Instantiate();
                        prefabInstance.Parent = parent2SO0;

                        SceneObject so0 = prefabInstance.FindChild("so0", false);
                        SceneObject so1 = prefabInstance.FindChild("so1_modified", false);

                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so1_1 = so1.FindChild("so1_1", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

                        UT1_Component2 comp1 = so1.GetComponent<UT1_Component2>();
                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0_1 = so0_1.GetComponent<UT1_Component1>();

                        SceneObject so0_0 = new SceneObject("so0_0");
                        so0_0.Parent = so0;
                        UT1_Component1 comp0 = so0.AddComponent<UT1_Component1>();

                        so1.RemoveComponent<UT1_Component1>();
                        so1_1.Destroy();

                        comp0.otherSO = so0_1_0;
                        comp0.otherComponent = comp1;

                        parentComp1_0.otherSO = so1_0;
                        parentComp1_0.otherComponent2 = comp0_1_0;

                        comp0_1_0.otherSO = parent2SO1_0;
                        comp0_1_0.otherComponent2 = parentComp1_0;
                        comp0_1_0.b = "instanceValue";

                        comp0_1.b = "instanceValue2";

                        EditorApplication.SaveScene("unitTest4Scene_2.prefab");
                    }

                    Debug.Log("Passed stage 4.1");

                    // Reload the scene and ensure instance modifications remain
                    {
                        EditorApplication.LoadScene("unitTest4Scene_2.prefab");

                        SceneObject root = Scene.Root;
                        SceneObject parent2SO0 = root.FindChild("parent2SO0", false);
                        SceneObject parent2SO1 = root.FindChild("parent2SO1", false);
                        SceneObject parent2SO1_0 = parent2SO1.FindChild("parent2SO1_0", false);

                        SceneObject prefabInstance = parent2SO0.GetChild(0);

                        SceneObject so0 = prefabInstance.FindChild("so0", false);
                        SceneObject so1 = prefabInstance.FindChild("so1_modified", false);
                        SceneObject so0_0 = so0.FindChild("so0_0", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so1_1 = so1.FindChild("so1_1", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

                        UT1_Component1 parentComp1_0 = parent2SO1_0.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0 = so0.GetComponent<UT1_Component1>();
                        UT1_Component2 comp1 = so1.GetComponent<UT1_Component2>();
                        UT1_Component1 comp11 = so1.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0_1 = so0_1.GetComponent<UT1_Component1>();

                        Assert(so0_0 != null);
                        Assert(comp0 != null);
                        Assert(so1_1 == null);
                        Assert(comp11 == null);

                        Assert(comp0.otherSO == so0_1_0);
                        Assert(comp0.otherComponent == comp1);

                        Assert(parentComp1_0.otherSO == so1_0);
                        Assert(parentComp1_0.otherComponent2 == comp0_1_0);

                        Assert(comp0_1_0.otherSO == parent2SO1_0);
                        Assert(comp0_1_0.otherComponent2 == parentComp1_0);
                        Assert(comp0_1_0.b == "instanceValue");

                        Assert(comp0_1.b == "instanceValue2");
                    }

                    Debug.Log("Passed stage 4.2");

                    // Load original scene and ensure instance modifications didn't influence it
                    {
                        EditorApplication.LoadScene("unitTest4Scene_1.prefab");

                        SceneObject parentSO0 = Scene.Root.FindChild("parentSO0", false);
                        SceneObject parentSO1 = Scene.Root.FindChild("parentSO1", false);
                        SceneObject parentSO1_0 = parentSO1.FindChild("parentSO1_0", false);

                        UT1_Component1 parentComp1_0 = parentSO1_0.GetComponent<UT1_Component1>();

                        SceneObject prefabInstance = parentSO0.GetChild(0);
                        SceneObject so0 = prefabInstance.FindChild("so0", false);
                        SceneObject so1 = prefabInstance.FindChild("so1_modified", false);
                        SceneObject so0_0 = so0.FindChild("so0_0", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);
                        SceneObject so1_1 = so1.FindChild("so1_1", false);

                        UT1_Component1 comp0 = so0.GetComponent<UT1_Component1>();
                        UT1_Component1 comp1 = so1.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0_1 = so0_1.GetComponent<UT1_Component1>();

                        Assert(parentComp1_0.otherSO == so1_0);
                        Assert(parentComp1_0.otherComponent2 == comp0_1_0);
                        Assert(so1_1 != null);
                        Assert(so0_0 == null);
                        Assert(comp0 == null);
                        Assert(comp0_1_0.otherSO == so1_1);
                        Assert(comp0_1_0.otherComponent2 == comp1);
                        Assert(comp0_1_0.a == 123);
                        Assert(comp0_1_0.b == "modifiedValue");
                        Assert(comp1.otherSO == so1_0);
                        Assert(comp1.otherComponent2 == comp0_1_0);
                        Assert(comp0_1.b == "originalValue2");
                        Assert(MathEx.ApproxEquals(so1.LocalPosition.y, 999.0f));
                    }

                    Debug.Log("Passed stage 4.3");

                    // Modify prefab and ensure both prefab and instance modifications remain
                    {
                        // unitTest4Scene_0.prefab:
                        // so0 (Comp2)
                        //  - so0_1
                        //    - so0_1_0 (Comp1)
                        // so1 (Comp1, Comp2)
                        //  - so1_1
                        //  - so1_2 (Comp1)

                        // unitTest4Scene_0.prefab (unitTest4Scene_2.prefab instance):
                        // so0 (Comp1)
                        //  - so0_0
                        //  - so0_1 (Comp1)
                        //    - so0_1_0 (Comp1)
                        // so1 (Comp2)
                        //  - so1_2 (Comp1)

                        Scene.Load("unitTest4Scene_0.prefab");

                        SceneObject sceneRoot = Scene.Root;
                        SceneObject so0 = sceneRoot.FindChild("so0", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1 = sceneRoot.FindChild("so1_modified", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

                        SceneObject so1_2 = new SceneObject("so1_2");
                        so1_2.Parent = so1;

                        so0.AddComponent<UT1_Component2>();
                        so0_1.RemoveComponent<UT1_Component1>();
                        so1_0.Destroy();

                        UT1_Component1 comp3 = so1_2.AddComponent<UT1_Component1>();
                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();
                        comp0_1_0.b = "modifiedValueAgain";
                        so1.Name = "so1_modifiedAgain";

                        comp3.otherSO = so0_1;
                        comp3.otherComponent2 = comp0_1_0;

                        EditorApplication.SaveScene("unitTest4Scene_0.prefab");
                    }

                    Debug.Log("Passed stage 4.4");

                    // Reload the scene and ensure both instance and prefab modifications remain
                    {
                        EditorApplication.LoadScene("unitTest4Scene_2.prefab");

                        SceneObject root = Scene.Root;
                        SceneObject parent2SO0 = root.FindChild("parent2SO0", false);
                        SceneObject parent2SO1 = root.FindChild("parent2SO1", false);
                        SceneObject parent2SO1_0 = parent2SO1.FindChild("parent2SO1_0", false);

                        SceneObject prefabInstance = parent2SO0.GetChild(0);

                        SceneObject so0 = prefabInstance.FindChild("so0", false);
                        SceneObject so1 = prefabInstance.FindChild("so1_modifiedAgain", false);
                        SceneObject so0_0 = so0.FindChild("so0_0", false);
                        SceneObject so0_1 = so0.FindChild("so0_1", false);
                        SceneObject so1_0 = so1.FindChild("so1_0", false);
                        SceneObject so1_1 = so1.FindChild("so1_1", false);
                        SceneObject so1_2 = so1.FindChild("so1_2", false);
                        SceneObject so0_1_0 = so0_1.FindChild("so0_1_0", false);

                        UT1_Component1 parentComp1_0 = parent2SO1_0.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0 = so0.GetComponent<UT1_Component1>();
                        UT1_Component2 comp1 = so1.GetComponent<UT1_Component2>();
                        UT1_Component1 comp11 = so1.GetComponent<UT1_Component1>();
                        UT1_Component1 comp0_1_0 = so0_1_0.GetComponent<UT1_Component1>();
                        UT1_Component1 comp3 = so1_2.GetComponent<UT1_Component1>();

                        // Check instance modifications (they should override any prefab modifications)
                        Assert(so0_0 != null);
                        Assert(comp0 != null);
                        Assert(so1_1 == null);
                        Assert(comp11 == null);

                        Assert(comp0.otherSO == so0_1_0);
                        Assert(comp0.otherComponent == comp1);

                        Assert(parentComp1_0.otherSO == so1_0);
                        Assert(parentComp1_0.otherComponent2 == comp0_1_0);

                        Assert(comp0_1_0.otherSO == parent2SO1_0);
                        Assert(comp0_1_0.otherComponent2 == parentComp1_0);
                        Assert(comp0_1_0.b == "instanceValue");

                        // Check prefab modifications
                        Assert(so1_0 == null);
                        Assert(so1.Name == "so1_modifiedAgain");

                        Assert(comp3.otherSO == so0_1);
                        Assert(comp3.otherComponent2 == comp0_1_0);
                    }

                    Debug.Log("Passed stage 4.5");
                }
            }
            catch
            {
                PrintSceneState();

                throw;
            }
            finally
            {
                

                if (!string.IsNullOrEmpty(oldScene))
                    Scene.Load(ProjectLibrary.GetPath(oldScene));
                else
                    Scene.Clear();

                ProjectLibrary.Delete("unitTest4Scene_0.prefab");
                ProjectLibrary.Delete("unitTest4Scene_1.prefab");
                ProjectLibrary.Delete("unitTest4Scene_2.prefab");
            }

            */
        }

        /// <summary>
        /// Runs all tests.
        /// </summary>
        static void RunTests()
        {
            UnitTest1_ManagedSerialization();
            UnitTest2_SerializableProperties();
            UnitTest3_ManagedDiff();
            UnitTest4_Prefabs();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UT1_GameObjectClone(SceneObject so);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UT3_GenerateDiff(UT_DiffObj oldObj, UT_DiffObj newObj);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_UT3_ApplyDiff(UT_DiffObj obj);
    }

    /** @} */
}
