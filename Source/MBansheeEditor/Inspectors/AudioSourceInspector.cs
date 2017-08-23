//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Renders an inspector for the <see cref="AudioSource"/> component.
    /// </summary>
    [CustomInspector(typeof(AudioSource))]
    internal class AudioSourceInspector : Inspector
    {
        private GUIResourceField audioClipField = new GUIResourceField(typeof(AudioClip), new LocEdString("Clip"));
        private GUISliderField volumeField = new GUISliderField(0.0f, 1.0f, new LocEdString("Volume"));
        private GUIFloatField pitchField = new GUIFloatField(new LocEdString("Pitch"));
        private GUIToggleField loopField = new GUIToggleField(new LocEdString("Loop"));
        private GUIIntField priorityField = new GUIIntField(new LocEdString("Priority"));
        private GUIFloatField minDistanceField = new GUIFloatField(new LocEdString("Min. distance"));
        private GUIFloatField attenuationField = new GUIFloatField(new LocEdString("Attenuation"));
        private GUIToggleField playOnStartField = new GUIToggleField(new LocEdString("Play on start"));

        private InspectableState modifyState;
        
        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            BuildGUI();
        }

        /// <inheritdoc/>
        protected internal override InspectableState Refresh()
        {
            AudioSource source = InspectedObject as AudioSource;
            if (source == null)
                return InspectableState.NotModified;

            audioClipField.Value = source.Clip;
            volumeField.Value = source.Volume;
            pitchField.Value = source.Pitch;
            loopField.Value = source.Loop;
            priorityField.Value = (int)source.Priority;
            minDistanceField.Value = source.MinDistance;
            attenuationField.Value = source.Attenuation;
            playOnStartField.Value = source.PlayOnStart;

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

            AudioSource source = InspectedObject as AudioSource;
            if (source == null)
                return;

            audioClipField.OnChanged += x =>
            {
                AudioClip clip = Resources.Load<AudioClip>(x);

                source.Clip = clip;
                MarkAsModified();
                ConfirmModify();
            };

            volumeField.OnChanged += x =>
            {
                source.Volume = x;
                MarkAsModified();
                ConfirmModify();
            };

            pitchField.OnChanged += x => { source.Pitch = x; MarkAsModified(); };
            pitchField.OnConfirmed += ConfirmModify;
            pitchField.OnFocusLost += ConfirmModify;

            loopField.OnChanged += x =>
            {
                source.Loop = x;
                MarkAsModified();
                ConfirmModify();
            };

            priorityField.OnChanged += x => { source.Priority = x; MarkAsModified(); };
            priorityField.OnConfirmed += ConfirmModify;
            priorityField.OnFocusLost += ConfirmModify;

            minDistanceField.OnChanged += x => { source.MinDistance = x; MarkAsModified(); };
            minDistanceField.OnConfirmed += ConfirmModify;
            minDistanceField.OnFocusLost += ConfirmModify;

            attenuationField.OnChanged += x => { source.Attenuation = x; MarkAsModified(); };
            attenuationField.OnConfirmed += ConfirmModify;
            attenuationField.OnFocusLost += ConfirmModify;

            playOnStartField.OnChanged += x =>
            {
                source.PlayOnStart = x;
                MarkAsModified();
                ConfirmModify();
            };

            Layout.AddElement(audioClipField);
            Layout.AddElement(volumeField);
            Layout.AddElement(pitchField);
            Layout.AddElement(loopField);
            Layout.AddElement(priorityField);
            Layout.AddElement(minDistanceField);
            Layout.AddElement(attenuationField);
            Layout.AddElement(playOnStartField);
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
    }

    /** @} */
}