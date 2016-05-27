//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="AudioClip"/> resource.
    /// </summary>
    [CustomInspector(typeof(AudioClip))]
    internal class AudioClipInspector : Inspector
    {
        private GUIEnumField formatField = new GUIEnumField(typeof(AudioFormat), new LocEdString("Format"));
        private GUIEnumField readModeField = new GUIEnumField(typeof(AudioReadMode), new LocEdString("Read mode"));
        private GUIEnumField bitDepthField = new GUIEnumField(typeof(AudioBitDepth), new LocEdString("Bit depth"));
        private GUIToggleField is3DField = new GUIToggleField(new LocEdString("3D"));

        private GUIButton reimportButton = new GUIButton(new LocEdString("Reimport"));

        private AudioClipImportOptions importOptions;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            if (InspectedObject != null)
            {
                importOptions = GetImportOptions();

                formatField.OnSelectionChanged += x => importOptions.Format = (AudioFormat)x;
                readModeField.OnSelectionChanged += x => importOptions.ReadMode = (AudioReadMode)x;
                bitDepthField.OnSelectionChanged += x => importOptions.BitDepth = (AudioBitDepth)x;
                is3DField.OnChanged += x => importOptions.Is3D = x;

                reimportButton.OnClick += TriggerReimport;

                Layout.AddElement(formatField);
                Layout.AddElement(readModeField);
                Layout.AddElement(bitDepthField);
                Layout.AddElement(is3DField);
                Layout.AddSpace(10);

                GUILayout reimportButtonLayout = Layout.AddLayoutX();
                reimportButtonLayout.AddFlexibleSpace();
                reimportButtonLayout.AddElement(reimportButton);
            }
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            AudioClipImportOptions newImportOptions = GetImportOptions();

            formatField.Value = (ulong)newImportOptions.Format;
            readModeField.Value = (ulong)newImportOptions.ReadMode;
            bitDepthField.Value = (ulong)newImportOptions.BitDepth;
            is3DField.Value = newImportOptions.Is3D;

            importOptions = newImportOptions;

            return InspectableState.NotModified;
        }

        /// <summary>
        /// Retrieves import options for the audio clip we're currently inspecting.
        /// </summary>
        /// <returns>Audio clip import options object.</returns>
        private AudioClipImportOptions GetImportOptions()
        {
            AudioClip audioClip = InspectedObject as AudioClip;
            AudioClipImportOptions output = null;

            if (audioClip != null)
            {
                LibraryEntry meshEntry = ProjectLibrary.GetEntry(ProjectLibrary.GetPath(audioClip));
                if (meshEntry != null && meshEntry.Type == LibraryEntryType.File)
                {
                    FileEntry meshFileEntry = (FileEntry)meshEntry;
                    output = meshFileEntry.Options as AudioClipImportOptions;
                }
            }

            if (output == null)
            {
                if (importOptions == null)
                    output = new AudioClipImportOptions();
                else
                    output = importOptions;
            }

            return output;
        }

        /// <summary>
        /// Reimports the resource according to the currently set import options.
        /// </summary>
        private void TriggerReimport()
        {
            AudioClip audioClip = (AudioClip)InspectedObject;
            string resourcePath = ProjectLibrary.GetPath(audioClip);

            ProjectLibrary.Reimport(resourcePath, importOptions, true);
        }
    }

    /** @} */
}
