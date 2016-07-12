//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspector
     *  @{
     */

    /// <summary>
    /// Displays GUI for a serializable property containing a floating point value.
    /// </summary>
    public class InspectableRangedInt : InspectableRangedField
    {
        private GUISliderField guiIntField;
        private InspectableState state;

        /// <summary>
        /// Creates a new inspectable float GUI for the specified property with a range.
        /// </summary>
        /// <param name="parent">Parent Inspector this field belongs to.</param>
        /// <param name="title">Name of the property, or some other value to set as the title.</param>
        /// <param name="path">Full path to this property (includes name of this property and all parent properties).</param>
        /// <param name="depth">Determines how deep within the inspector nesting hierarchy is this field. Some fields may
        ///                     contain other fields, in which case you should increase this value by one.</param>
        /// <param name="layout">Parent layout that all the field elements will be added to.</param>
        /// <param name="property">Serializable property referencing the array whose contents to display.</param>
        /// <param name="rangeStyle">Information about the range of the field</param>
        public InspectableRangedInt(Inspector parent, string title, string path, int depth, InspectableFieldLayout layout,
            SerializableProperty property, InspectableFieldRangeStyle rangeStyle)
            : base(parent, title, path, SerializableProperty.FieldType.Float, depth, layout, property, rangeStyle)
        {

        }

        /// <inheritoc/>
        protected internal override void Initialize(int layoutIndex)
        {
            if (property != null)
            {
                guiIntField = new GUISliderField(rangeStyle.Min, rangeStyle.Max, new GUIContent(title));
                if (rangeStyle.Step != 0)
                    guiIntField.Step = rangeStyle.Step;
                guiIntField.OnChanged += OnFieldValueChanged;
                guiIntField.OnFocusLost += OnFieldValueConfirm;

                layout.AddElement(layoutIndex, guiIntField);
            }
        }

        /// <inheritdoc/>
        public override InspectableState Refresh(int layoutIndex)
        {
            if (guiIntField != null && !guiIntField.HasInputFocus)
                guiIntField.Value = property.GetValue<float>();

            InspectableState oldState = state;
            if (state.HasFlag(InspectableState.Modified))
                state = InspectableState.NotModified;

            return oldState;
        }

        /// <summary>
        /// Triggered when the user inputs a new floating point value.
        /// </summary>
        /// <param name="newValue">New value of the float field.</param>
        private void OnFieldValueChanged(float newValue)
        {
            property.SetValue(newValue);
            state |= InspectableState.ModifyInProgress;
        }

        /// <summary>
        /// Triggered when the user confirms input in the float field.
        /// </summary>
        private void OnFieldValueConfirm()
        {
            if (state.HasFlag(InspectableState.ModifyInProgress))
                state |= InspectableState.Modified;
        }
    }

    /** @} */
}
