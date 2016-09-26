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
        /// <param name="field">Field to create the style info structure for.</param>
        /// <returns>Style information retrieved from the field.</returns>
        public static InspectableFieldStyleInfo Create(SerializableField field)
        {
            SerializableFieldStyle style = field.Style;

            var styleInfo = new InspectableFieldStyleInfo();
            styleInfo.RangeStyle = style.HasRange ? new InspectableFieldRangeStyle(style.RangeMin, style.RangeMax, style.DisplayAsSlider) : null;
            styleInfo.StepStyle = style.HasStep ? new InspectableFieldStepStyle(style.StepIncrement) : null;

            return styleInfo;
        }
    }
}
