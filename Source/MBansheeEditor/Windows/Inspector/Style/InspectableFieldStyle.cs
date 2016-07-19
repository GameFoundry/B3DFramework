//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Contains style information about inspectable items.
    /// </summary>
    public abstract class InspectableFieldStyle
    {
        /// <summary>
        /// Creates all the relevant style information for a SerializableField.
        /// </summary>
        /// <param name="field">A SerializableField.</param>
        /// <returns>Style information.</returns>
        public static InspectableFieldStyleInfo Create(SerializableField field)
        {
            var styleInfo = new InspectableFieldStyleInfo();

            styleInfo.RangeStyle = field.IsRanged? new InspectableFieldRangeStyle(field.RangeMinimum, field.RangeMaximum, field.IsSlider) : null;
            styleInfo.StepStyle = field.IsStepped? new InspectableFieldStepStyle(field.Step) : null;
            return styleInfo;
        }
    }
}
