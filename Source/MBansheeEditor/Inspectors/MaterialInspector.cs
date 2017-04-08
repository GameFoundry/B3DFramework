//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="Material"/> resource.
    /// </summary>
    [CustomInspector(typeof (Material))]
    internal class MaterialInspector : Inspector
    {
        private MaterialParamGUI[] guiParams;
        private GUIResourceField shaderField;
        private GUIEnumField builtinShaderField;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            Material material = InspectedObject as Material;
            if (material == null)
                return;

            Shader activeShader = material.Shader;
            BuiltinShader builtinType = ShaderToBuiltin(activeShader);

            builtinShaderField = new GUIEnumField(typeof(BuiltinShader), new LocEdString("Shader"));
            builtinShaderField.Value = (ulong) builtinType;
            builtinShaderField.OnSelectionChanged += x =>
            {
                BuiltinShader newBuiltinType = (BuiltinShader) x;

                material.Shader = Builtin.GetShader(newBuiltinType);
                EditorApplication.SetDirty(material);
                RebuildParamGUI(material);

                bool newIsCustom = newBuiltinType == BuiltinShader.Custom;
                shaderField.Active = newIsCustom;
            };

            shaderField = new GUIResourceField(typeof(Shader), new LocEdString("Shader file"));
            shaderField.Value = material.Shader;
            shaderField.OnChanged += (x) =>
            {
                Shader shader = Resources.Load<Shader>(x);

                material.Shader = shader;
                EditorApplication.SetDirty(material);
                RebuildParamGUI(material);
            };

            bool isCustom = builtinType == BuiltinShader.Custom;
            shaderField.Active = isCustom;

            Layout.AddElement(builtinShaderField);
            Layout.AddElement(shaderField);

            RebuildParamGUI(material);
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            Material material = InspectedObject as Material;
            if (material == null)
                return InspectableState.NotModified;

            if (material.Shader != shaderField.Value)
            {
                shaderField.Value = material.Shader;
                RebuildParamGUI(material);
            }

            if (guiParams != null)
            {
                foreach (var param in guiParams)
                    param.Refresh(material);
            }

            return InspectableState.NotModified;
        }

        /// <summary>
        /// Recreates GUI elements for all material parameters.
        /// </summary>
        /// <param name="material">Material to create parameters for</param>
        private void RebuildParamGUI(Material material)
        {
            if (guiParams != null)
            {
                foreach (var param in guiParams)
                    param.Destroy();

                guiParams = null;
            }

            if (material != null && material.Shader != null)
                guiParams = CreateMaterialGUI(material, Layout);
        }

        /// <summary>
        /// Converts a shader resource into a builtin shader.
        /// </summary>
        /// <param name="shader">Shader resource to convert.</param>
        /// <returns>Type of builtin shader, if any.</returns>
        private BuiltinShader ShaderToBuiltin(Shader shader)
        {
            // Note: Need a better way to detect the builtin shader perhaps (store it in Material?)
            Shader standardShader = Builtin.GetShader(BuiltinShader.Standard);
            Shader transparentShader = Builtin.GetShader(BuiltinShader.Transparent);
;            if(standardShader == shader)
                return BuiltinShader.Standard;
            else if(transparentShader == shader)
                return BuiltinShader.Transparent;
            
            return BuiltinShader.Custom;
        }

        /// <summary>
        /// Creates a set of objects in which each object represents a GUI for a material parameter.
        /// </summary>
        /// <param name="mat">Material for whose parameters to create GUI for.</param>
        /// <param name="layout">Layout to add the parameter GUI elements to.</param>
        /// <returns>A material parameter GUI object for each supported material parameter.</returns>
        static internal MaterialParamGUI[] CreateMaterialGUI(Material mat, GUILayout layout)
        {
            Shader shader = mat.Shader;
            if (shader == null)
                return new MaterialParamGUI[0];

            List<MaterialParamGUI> guiParams = new List<MaterialParamGUI>();
            ShaderParameter[] shaderParams = shader.Parameters;

            foreach (var param in shaderParams)
            {
                if (param.Internal)
                    continue;

                switch (param.Type)
                {
                    case ShaderParameterType.Float:
                        layout.AddSpace(5);
                        guiParams.Add(new MaterialParamFloatGUI(param, mat, layout));
                        break;
                    case ShaderParameterType.Vector2:
                        layout.AddSpace(5);
                        guiParams.Add(new MaterialParamVec2GUI(param, mat, layout));
                        break;
                    case ShaderParameterType.Vector3:
                        layout.AddSpace(5);
                        guiParams.Add(new MaterialParamVec3GUI(param, mat, layout));
                        break;
                    case ShaderParameterType.Vector4:
                        layout.AddSpace(5);
                        guiParams.Add(new MaterialParamVec4GUI(param, mat, layout));
                        break;
                    case ShaderParameterType.Matrix3:
                        layout.AddSpace(5);
                        guiParams.Add(new MaterialParamMat3GUI(param, mat, layout));
                        break;
                    case ShaderParameterType.Matrix4:
                        layout.AddSpace(5);
                        guiParams.Add(new MaterialParamMat4GUI(param, mat, layout));
                        break;
                    case ShaderParameterType.Color:
                        layout.AddSpace(5);
                        guiParams.Add(new MaterialParamColorGUI(param, mat, layout));
                        break;
                    case ShaderParameterType.Texture2D:
                    case ShaderParameterType.Texture3D:
                    case ShaderParameterType.TextureCube:
                        layout.AddSpace(5);
                        guiParams.Add(new MaterialParamTextureGUI(param, mat, layout));
                        break;
                }
            }

            return guiParams.ToArray();
        }
    }

    /// <summary>
    /// Contains GUI element(s) for a single parameter in a <see cref="Material"/>.
    /// </summary>
    internal abstract class MaterialParamGUI
    {
        protected ShaderParameter shaderParam;

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for.</param>
        protected MaterialParamGUI(ShaderParameter shaderParam)
        {
            this.shaderParam = shaderParam;
        }

        /// <summary>
        /// Checks if the data stored in GUI and in the material matches, and updates the GUI if it doesn't.
        /// </summary>
        /// <param name="material">Material whose data to check.</param>
        internal abstract void Refresh(Material material);

        /// <summary> 
        /// Destroys the internal GUI elements.
        /// </summary>
        internal abstract void Destroy();
    }

    /// <summary>
    /// Contains GUI element(s) for a single floating point parameter in a <see cref="Material"/>.
    /// </summary>
    internal class MaterialParamFloatGUI : MaterialParamGUI
    {
        private GUIFloatField guiElem;

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for. Must be of floating point type.</param>
        /// <param name="material">Material the parameter is a part of.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        internal MaterialParamFloatGUI(ShaderParameter shaderParam, Material material, GUILayout layout)
            : base(shaderParam)
        {
            LocString title = new LocEdString(shaderParam.Name);
            guiElem = new GUIFloatField(title);
            guiElem.OnChanged += (x) =>
            {
                material.SetFloat(shaderParam.Name, x);
                EditorApplication.SetDirty(material);
            };

            layout.AddElement(guiElem);
        }

        /// <inheritdoc/>
        internal override void Refresh(Material material)
        {
            guiElem.Value = material.GetFloat(shaderParam.Name);
        }

        /// <inheritdoc/>
        internal override void Destroy()
        {
            guiElem.Destroy();
        }
    }

    /// <summary>
    /// Contains GUI element(s) for a single 2D vector parameter in a <see cref="Material"/>.
    /// </summary>
    internal class MaterialParamVec2GUI : MaterialParamGUI
    {
        private GUIVector2Field guiElem;

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for. Must be of 2D vector type.</param>
        /// <param name="material">Material the parameter is a part of.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        internal MaterialParamVec2GUI(ShaderParameter shaderParam, Material material, GUILayout layout)
            : base(shaderParam)
        {
            LocString title = new LocEdString(shaderParam.Name);
            guiElem = new GUIVector2Field(title);
            guiElem.OnChanged += (x) =>
            {
                material.SetVector2(shaderParam.Name, x);
                EditorApplication.SetDirty(material);
            };

            layout.AddElement(guiElem);
        }

        /// <inheritdoc/>
        internal override void Refresh(Material material)
        {
            guiElem.Value = material.GetVector2(shaderParam.Name);
        }

        /// <inheritdoc/>
        internal override void Destroy()
        {
            guiElem.Destroy();
        }
    }

    /// <summary>
    /// Contains GUI element(s) for a single 3D vector parameter in a <see cref="Material"/>.
    /// </summary>
    internal class MaterialParamVec3GUI : MaterialParamGUI
    {
        private GUIVector3Field guiElem;

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for. Must be of 3D vector type.</param>
        /// <param name="material">Material the parameter is a part of.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        internal MaterialParamVec3GUI(ShaderParameter shaderParam, Material material, GUILayout layout)
            : base(shaderParam)
        {
            LocString title = new LocEdString(shaderParam.Name);
            guiElem = new GUIVector3Field(title);
            guiElem.OnChanged += (x) =>
            {
                material.SetVector3(shaderParam.Name, x);
                EditorApplication.SetDirty(material);
            };

            layout.AddElement(guiElem);
        }

        /// <inheritdoc/>
        internal override void Refresh(Material material)
        {
            guiElem.Value = material.GetVector3(shaderParam.Name);
        }

        /// <inheritdoc/>
        internal override void Destroy()
        {
            guiElem.Destroy();
        }
    }

    /// <summary>
    /// Contains GUI element(s) for a single 4D vector parameter in a <see cref="Material"/>.
    /// </summary>
    internal class MaterialParamVec4GUI : MaterialParamGUI
    {
        private GUIVector4Field guiElem;

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for. Must be of 4D vector type.</param>
        /// <param name="material">Material the parameter is a part of.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        internal MaterialParamVec4GUI(ShaderParameter shaderParam, Material material, GUILayout layout)
            : base(shaderParam)
        {
            LocString title = new LocEdString(shaderParam.Name);
            guiElem = new GUIVector4Field(title);
            guiElem.OnChanged += (x) =>
            {
                material.SetVector4(shaderParam.Name, x);
                EditorApplication.SetDirty(material);
            };

            layout.AddElement(guiElem);
        }

        /// <inheritdoc/>
        internal override void Refresh(Material material)
        {
            guiElem.Value = material.GetVector4(shaderParam.Name);
        }

        /// <inheritdoc/>
        internal override void Destroy()
        {
            guiElem.Destroy();
        }
    }

    /// <summary>
    /// Contains GUI element(s) for a single 3x3 matrix parameter in a <see cref="Material"/>.
    /// </summary>
    internal class MaterialParamMat3GUI : MaterialParamGUI
    {
        private const int MAT_SIZE = 3;

        private GUILayout mainLayout;
        private GUIFloatField[] guiMatFields = new GUIFloatField[MAT_SIZE * MAT_SIZE];

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for. Must be of 3x3 matrix type.</param>
        /// <param name="material">Material the parameter is a part of.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        internal MaterialParamMat3GUI(ShaderParameter shaderParam, Material material, GUILayout layout)
            : base(shaderParam)
        {
            LocString title = new LocEdString(shaderParam.Name);
            GUILabel guiTitle = new GUILabel(title, GUIOption.FixedWidth(100));

            mainLayout = layout.AddLayoutY();
            GUILayoutX titleLayout = mainLayout.AddLayoutX();
            titleLayout.AddElement(guiTitle);
            titleLayout.AddFlexibleSpace();

            GUILayoutY contentLayout = mainLayout.AddLayoutY();

            GUILayoutX[] rows = new GUILayoutX[MAT_SIZE];
            for (int i = 0; i < rows.Length; i++)
                rows[i] = contentLayout.AddLayoutX();

            for (int row = 0; row < MAT_SIZE; row++)
            {
                for (int col = 0; col < MAT_SIZE; col++)
                {
                    int index = row * MAT_SIZE + col;
                    guiMatFields[index] = new GUIFloatField(row + "," + col, 20, "", GUIOption.FixedWidth(80));

                    GUIFloatField field = guiMatFields[index];
                    rows[row].AddElement(field);
                    rows[row].AddSpace(5);

                    int hoistedRow = row;
                    int hoistedCol = col;
                    field.OnChanged += (x) =>
                    {
                        Matrix3 value = material.GetMatrix3(shaderParam.Name);
                        value[hoistedRow, hoistedCol] = x;
                        material.SetMatrix3(shaderParam.Name, value);
                        EditorApplication.SetDirty(material);
                    };
                }
            }
        }

        /// <inheritdoc/>
        internal override void Refresh(Material material)
        {
            Matrix3 value = material.GetMatrix3(shaderParam.Name);

            for (int row = 0; row < MAT_SIZE; row++)
            {
                for (int col = 0; col < MAT_SIZE; col++)
                {
                    int index = row * MAT_SIZE + col;
                    guiMatFields[index].Value = value[row, col];
                }
            }
        }

        /// <inheritdoc/>
        internal override void Destroy()
        {
            mainLayout.Destroy();
        }
    }

    /// <summary>
    /// Contains GUI element(s) for a single 4x4 matrix parameter in a <see cref="Material"/>.
    /// </summary>
    internal class MaterialParamMat4GUI : MaterialParamGUI
    {
        private const int MAT_SIZE = 4;

        private GUILayout mainLayout;
        private GUIFloatField[] guiMatFields = new GUIFloatField[MAT_SIZE * MAT_SIZE];

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for. Must be of 4x4 matrix type.</param>
        /// <param name="material">Material the parameter is a part of.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        internal MaterialParamMat4GUI(ShaderParameter shaderParam, Material material, GUILayout layout)
            : base(shaderParam)
        {
            LocString title = new LocEdString(shaderParam.Name);
            GUILabel guiTitle = new GUILabel(title, GUIOption.FixedWidth(100));

            mainLayout = layout.AddLayoutY();
            GUILayoutX titleLayout = mainLayout.AddLayoutX();
            titleLayout.AddElement(guiTitle);
            titleLayout.AddFlexibleSpace();

            GUILayoutY contentLayout = mainLayout.AddLayoutY();

            GUILayoutX[] rows = new GUILayoutX[MAT_SIZE];
            for (int i = 0; i < rows.Length; i++)
                rows[i] = contentLayout.AddLayoutX();

            for (int row = 0; row < MAT_SIZE; row++)
            {
                for (int col = 0; col < MAT_SIZE; col++)
                {
                    int index = row * MAT_SIZE + col;
                    guiMatFields[index] = new GUIFloatField(row + "," + col, 20, "", GUIOption.FixedWidth(80));

                    GUIFloatField field = guiMatFields[index];
                    rows[row].AddElement(field);
                    rows[row].AddSpace(5);

                    int hoistedRow = row;
                    int hoistedCol = col;
                    field.OnChanged += (x) =>
                    {
                        Matrix4 value = material.GetMatrix4(shaderParam.Name);
                        value[hoistedRow, hoistedCol] = x;
                        material.SetMatrix4(shaderParam.Name, value);
                        EditorApplication.SetDirty(material);
                    };
                }
            }
        }

        /// <inheritdoc/>
        internal override void Refresh(Material material)
        {
            Matrix4 value = material.GetMatrix4(shaderParam.Name);

            for (int row = 0; row < MAT_SIZE; row++)
            {
                for (int col = 0; col < MAT_SIZE; col++)
                {
                    int index = row * MAT_SIZE + col;
                    guiMatFields[index].Value = value[row, col];
                }
            }
        }

        /// <inheritdoc/>
        internal override void Destroy()
        {
            mainLayout.Destroy();
        }
    }

    /// <summary>
    /// Contains GUI element(s) for a single color parameter in a <see cref="Material"/>.
    /// </summary>
    internal class MaterialParamColorGUI : MaterialParamGUI
    {
        private GUIColorField guiElem;

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for. Must be of color type.</param>
        /// <param name="material">Material the parameter is a part of.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        internal MaterialParamColorGUI(ShaderParameter shaderParam, Material material, GUILayout layout)
            : base(shaderParam)
        {
            LocString title = new LocEdString(shaderParam.Name);
            guiElem = new GUIColorField(title);
            guiElem.OnChanged += (x) =>
            {
                material.SetColor(shaderParam.Name, x);
                EditorApplication.SetDirty(material);
            };

            layout.AddElement(guiElem);
        }

        /// <inheritdoc/>
        internal override void Refresh(Material material)
        {
            guiElem.Value = material.GetColor(shaderParam.Name);
        }

        /// <inheritdoc/>
        internal override void Destroy()
        {
            guiElem.Destroy();
        }
    }

    /// <summary>
    /// Contains GUI element(s) for a single texture parameter in a <see cref="Material"/>.
    /// </summary>
    internal class MaterialParamTextureGUI : MaterialParamGUI
    {
        private GUITextureField guiElem;

        /// <summary>
        /// Creates a new material parameter GUI.
        /// </summary>
        /// <param name="shaderParam">Shader parameter to create the GUI for. Must be of texture type.</param>
        /// <param name="material">Material the parameter is a part of.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        internal MaterialParamTextureGUI(ShaderParameter shaderParam, Material material, GUILayout layout)
            : base(shaderParam)
        {
            LocString title = new LocEdString(shaderParam.Name);
            guiElem = new GUITextureField(title);

            switch (shaderParam.Type)
            {
                case ShaderParameterType.Texture2D:
                case ShaderParameterType.Texture3D:
                case ShaderParameterType.TextureCube:
                    guiElem.OnChanged += (x) =>
                    {
                        Texture texture = Resources.Load<Texture>(x);

                        material.SetTexture(shaderParam.Name, texture);
                        EditorApplication.SetDirty(material);
                    };
                    break;
            }

            layout.AddElement(guiElem);
        }

        /// <inheritdoc/>
        internal override void Refresh(Material material)
        {
            Texture value = null;
            switch (shaderParam.Type)
            {
                case ShaderParameterType.Texture2D:
                case ShaderParameterType.Texture3D:
                case ShaderParameterType.TextureCube:
                    value = material.GetTexture(shaderParam.Name);
                    break;
            }

            guiElem.Value = value;
        }

        /// <inheritdoc/>
        internal override void Destroy()
        {
            guiElem.Destroy();
        }
    }

    /** @} */
}
