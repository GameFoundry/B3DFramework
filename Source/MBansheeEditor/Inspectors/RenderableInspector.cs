//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="Renderable"/> component.
    /// </summary>
    [CustomInspector(typeof (Renderable))]
    internal class RenderableInspector : Inspector
    {
        private GUIResourceField meshField;
        private GUIListBoxField layersField;
        private GUIArrayField<Material, MaterialArrayRow> materialsField;
        private List<MaterialParamGUI[]> materialParams = new List<MaterialParamGUI[]>();

        private ulong layersValue = 0;
        private InspectableState modifyState;

        private Material[] materials;
        private GUILayout materialsLayout;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            BuildGUI();
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            Renderable renderable = InspectedObject as Renderable;
            if (renderable == null)
                return InspectableState.NotModified;

            bool rebuildMaterialsGUI = false;

            Material[] newMaterials = renderable.Materials;
            if (newMaterials == null)
                rebuildMaterialsGUI = materials != null;
            else
            {
                if (materials == null)
                    rebuildMaterialsGUI = true;
                else
                {
                    if (materials.Length != newMaterials.Length)
                        rebuildMaterialsGUI = true;
                    else
                    {
                        for (int i = 0; i < materials.Length; i++)
                        {
                            if (materials[i] != newMaterials[i])
                            {
                                rebuildMaterialsGUI = true;
                                break;
                            }
                        }
                    }
                }
            }

            if (rebuildMaterialsGUI)
                BuildMaterialsGUI();

            meshField.Value = renderable.Mesh;

            if (layersValue != renderable.Layers)
            {
                bool[] states = new bool[64];
                for (int i = 0; i < states.Length; i++)
                    states[i] = (renderable.Layers & Layers.Values[i]) == Layers.Values[i];

                layersField.States = states;
                layersValue = renderable.Layers;
            }

            InspectableState materialsModified = materialsField.Refresh();
            if (materialsModified == InspectableState.Modified)
                renderable.Materials = materials;

            modifyState |= materialsModified;

            if (materials != null)
            {
                for (int i = 0; i < materialParams.Count; i++)
                {
                    if (materialParams[i] != null)
                    {
                        foreach (var param in materialParams[i])
                            param.Refresh(materials[i]);
                    }
                }
            }

            InspectableState oldState = modifyState;
            if (modifyState.HasFlag(InspectableState.Modified))
                modifyState = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Recreates all the GUI elements used by this inspector.
        /// </summary>
        private void BuildGUI()
        {
            Layout.Clear();

            Renderable renderable = InspectedObject as Renderable;
            if (renderable == null)
                return;

            meshField = new GUIResourceField(typeof(Mesh), new LocEdString("Mesh"));
            layersField = new GUIListBoxField(Layers.Names, false, new LocEdString("Layer"));

            Layout.AddElement(meshField);
            Layout.AddElement(layersField);

            layersValue = 0;
            materials = renderable.Materials;
            materialsField = GUIArrayField<Material, MaterialArrayRow>.Create(new LocEdString("Materials"), materials, Layout);
            materialsField.OnChanged += x => { materials = x; };
            materialsField.IsExpanded = Persistent.GetBool("materialsField_Expanded");
            materialsField.OnExpand += x => Persistent.SetBool("materialsField_Expanded", x);

            meshField.OnChanged += x =>
            {
                Mesh mesh = Resources.Load<Mesh>(x);
                renderable.Mesh = mesh;

                MarkAsModified();
                ConfirmModify();
            };

            layersField.OnSelectionChanged += x =>
            {
                ulong layers = 0;
                bool[] states = layersField.States;
                for (int i = 0; i < states.Length; i++)
                    layers |= states[i] ? Layers.Values[i] : 0;

                layersValue = layers;
                renderable.Layers = layers;

                MarkAsModified();
                ConfirmModify();
            };

            materialsLayout = Layout.AddLayoutY();
            BuildMaterialsGUI();
        }

        /// <summary>
        /// Builds the portion of the GUI that displays details about individual materials.
        /// </summary>
        private void BuildMaterialsGUI()
        {
            materialsLayout.Clear();

            materialParams.Clear();
            if (materials != null)
            {
                for (int i = 0; i < materials.Length; i++)
                {
                    if (materials[i] == null)
                    {
                        materialParams.Add(new MaterialParamGUI[0]);
                        continue;
                    }

                    materialsLayout.AddSpace(10);

                    MaterialParamGUI[] matParams = MaterialInspector.CreateMaterialGUI(materials[i], materialsLayout);
                    materialParams.Add(matParams);
                }
            }
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        protected void MarkAsModified()
        {
            modifyState |= InspectableState.ModifyInProgress;
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        protected void ConfirmModify()
        {
            if (modifyState.HasFlag(InspectableState.ModifyInProgress))
                modifyState |= InspectableState.Modified;
        }

        /// <summary>
        /// Row element used for displaying GUI for material array elements.
        /// </summary>
        private class MaterialArrayRow : GUIListFieldRow
        {
            private GUIResourceField materialField;

            /// <inheritdoc/>
            protected override GUILayoutX CreateGUI(GUILayoutY layout)
            {
                GUILayoutX titleLayout = layout.AddLayoutX();
                materialField = new GUIResourceField(typeof(Material), new LocEdString(SeqIndex + ". "), 20);
                titleLayout.AddElement(materialField);

                materialField.OnChanged += x =>
                {
                    Material material = Resources.Load<Material>(x);

                    SetValue(material);
                    MarkAsModified();
                    ConfirmModify();
                };

                return titleLayout;
            }

            /// <inheritdoc/>
            internal protected override InspectableState Refresh()
            {
                materialField.Value = GetValue<Material>();

                return base.Refresh();
            }
        }
    }

    /** @} */
}