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
    /// Renders an inspector for the <see cref="ScriptCode"/> resource.
    /// </summary>
    [CustomInspector(typeof (ScriptCode))]
    internal class ScriptCodeInspector : Inspector
    {
        private const int MAX_SHOWN_CHARACTERS = 3000;

        private GUILabel textLabel = new GUILabel("", EditorStyles.MultiLineLabel, GUIOption.FixedHeight(500));
        private GUITexture textBg = new GUITexture(null, EditorStylesInternal.ScrollAreaBg);
        private GUIToggleField isEditorField = new GUIToggleField(new LocEdString("Is editor script"));

        private string shownText = "";
        private ScriptCodeImportOptions importOptions;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            ScriptCode scriptCode = InspectedObject as ScriptCode;
            if (scriptCode == null)
                return;

            importOptions = GetImportOptions();

            isEditorField.OnChanged += x =>
            {
                importOptions.EditorScript = x;
            };

            GUIPanel textPanel = Layout.AddPanel();
            GUILayout textLayoutY = textPanel.AddLayoutY();
            textLayoutY.AddSpace(5);
            GUILayout textLayoutX = textLayoutY.AddLayoutX();
            textLayoutX.AddSpace(5);
            textLayoutX.AddElement(textLabel);
            textLayoutX.AddSpace(5);
            textLayoutY.AddSpace(5);

            GUIPanel textBgPanel = textPanel.AddPanel(1);
            textBgPanel.AddElement(textBg);

            Layout.AddElement(isEditorField);

            GUIButton reimportButton = new GUIButton(new LocEdString("Reimport"));
            reimportButton.OnClick += TriggerReimport;

            GUILayout reimportButtonLayout = Layout.AddLayoutX();
            reimportButtonLayout.AddElement(reimportButton);
            reimportButtonLayout.AddFlexibleSpace();
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            ScriptCode scriptCode = InspectedObject as ScriptCode;
            if (scriptCode == null)
                return InspectableState.NotModified;

            isEditorField.Value = importOptions.EditorScript;

            string newText = scriptCode.Text;
            string newShownText = scriptCode.Text.Substring(0, MathEx.Min(newText.Length, MAX_SHOWN_CHARACTERS));

            if (newShownText != shownText)
            {
                textLabel.SetContent(newShownText);
                shownText = newShownText;
            }

            return InspectableState.NotModified;
        }

        /// <summary>
        /// Retrieves import options for the resource we're currently inspecting.
        /// </summary>
        /// <returns>Script code import options object.</returns>
        private ScriptCodeImportOptions GetImportOptions()
        {
            ScriptCode scriptCode = InspectedObject as ScriptCode;
            ScriptCodeImportOptions output = null;

            if (scriptCode != null)
            {
                LibraryEntry libEntry = ProjectLibrary.GetEntry(ProjectLibrary.GetPath(scriptCode));
                if (libEntry != null && libEntry.Type == LibraryEntryType.File)
                {
                    FileEntry fileEntry = (FileEntry)libEntry;
                    output = fileEntry.Options as ScriptCodeImportOptions;
                }
            }

            if (output == null)
            {
                if (importOptions == null)
                    output = new ScriptCodeImportOptions();
                else
                    output = importOptions;
            }

            return output;
        }

        /// <summary>
        /// Reimports the script code resource according to the currently set import options.
        /// </summary>
        private void TriggerReimport()
        {
            ScriptCode scriptCode = (ScriptCode)InspectedObject;
            string resourcePath = ProjectLibrary.GetPath(scriptCode);

            ProjectLibrary.Reimport(resourcePath, importOptions, true);
        }
    }

    /** @} */
}