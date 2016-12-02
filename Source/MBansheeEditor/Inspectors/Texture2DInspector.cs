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
    /// Renders an inspector for the <see cref="Texture2D"/> resource.
    /// </summary>
    [CustomInspector(typeof(Texture2D))]
    internal class Texture2DInspector : Inspector
    {
        private GUIEnumField formatField = new GUIEnumField(typeof(PixelFormat), new LocEdString("Format"));
        private GUIToggleField generateMipsField = new GUIToggleField(new LocEdString("Generate mipmaps"));
        private GUIIntField maximumMipsField = new GUIIntField(new LocEdString("Maximum mipmap level"));
        private GUIToggleField srgbField = new GUIToggleField(new LocEdString("Gamma space"));
        private GUIToggleField cpuCachedField = new GUIToggleField(new LocEdString("CPU cached"));
        private GUIButton reimportButton = new GUIButton(new LocEdString("Reimport"));

        private TextureImportOptions importOptions;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            if (InspectedObject != null)
            {
                importOptions = GetImportOptions();

                formatField.OnSelectionChanged += x => importOptions.Format = (PixelFormat)x;
                generateMipsField.OnChanged += x => importOptions.GenerateMipmaps = x;
                maximumMipsField.OnChanged += x => importOptions.MaxMipmapLevel = x;
                srgbField.OnChanged += x => importOptions.IsSRGB = x;
                cpuCachedField.OnChanged += x => importOptions.CPUCached = x;

                reimportButton.OnClick += TriggerReimport;

                Layout.AddElement(formatField);
                Layout.AddElement(generateMipsField);
                Layout.AddElement(maximumMipsField);
                Layout.AddElement(srgbField);
                Layout.AddElement(cpuCachedField);
                Layout.AddSpace(10);

                GUILayout reimportButtonLayout = Layout.AddLayoutX();
                reimportButtonLayout.AddFlexibleSpace();
                reimportButtonLayout.AddElement(reimportButton);
            }
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            TextureImportOptions newImportOptions = GetImportOptions();

            formatField.Value = (ulong)newImportOptions.Format;
            generateMipsField.Value = newImportOptions.GenerateMipmaps;
            maximumMipsField.Value = newImportOptions.MaxMipmapLevel;
            srgbField.Value = newImportOptions.IsSRGB;
            cpuCachedField.Value = newImportOptions.CPUCached;

            importOptions = newImportOptions;

            return InspectableState.NotModified;
        }

        /// <summary>
        /// Retrieves import options for the texture we're currently inspecting.
        /// </summary>
        /// <returns>Texture import options object.</returns>
        private TextureImportOptions GetImportOptions()
        {
            Texture2D texture = InspectedObject as Texture2D;
            TextureImportOptions output = null;

            if (texture != null)
            {
                LibraryEntry texEntry = ProjectLibrary.GetEntry(ProjectLibrary.GetPath(texture));
                if (texEntry != null && texEntry.Type == LibraryEntryType.File)
                {
                    FileEntry texFileEntry = (FileEntry)texEntry;
                    output = texFileEntry.Options as TextureImportOptions;
                }
            }

            if (output == null)
            {
                if (importOptions == null)
                    output = new TextureImportOptions();
                else
                    output = importOptions;
            }

            return output;
        }

        /// <summary>
        /// Reimports the texture resource according to the currently set import options.
        /// </summary>
        private void TriggerReimport()
        {
            Texture2D texture = (Texture2D)InspectedObject;
            string resourcePath = ProjectLibrary.GetPath(texture);

            ProjectLibrary.Reimport(resourcePath, importOptions, true);
        }
    }

    /** @} */
}
