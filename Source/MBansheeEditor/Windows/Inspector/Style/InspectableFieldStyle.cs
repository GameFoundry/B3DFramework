//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

using System;
using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Contains boolean information about a field style
    /// </summary>
    public enum InstectableFieldStyleFlags
    {
        None = 0,
        UseSlider = 1,   
    }

    /// <summary>
    /// Contains all the information about a field style
    /// </summary>
    public struct InspectableFieldStyleInfo
    {
        /// <summary>
        /// Information about the field range
        /// </summary>
        public InspectableFieldRangeStyle RangeStyle;

        /// <summary>
        /// Information about the field stepping
        /// </summary>
        public InspectableFieldStepStyle StepStyle;

        /// <summary>
        /// Boolean information about the field
        /// </summary>
        public InstectableFieldStyleFlags StyleFlags;
    }
    /// <summary>
    /// Contains style information about inspectable items
    /// </summary>
    public abstract class InspectableFieldStyle
    {
        /// <summary>
        /// Creates all the relevant style information for a SerializableField
        /// </summary>
        /// <param name="field">A SerializableField</param>
        /// <returns>Style information</returns>
        public static InspectableFieldStyleInfo Create(SerializableField field)
        {
            var styleInfo = new InspectableFieldStyleInfo();

            styleInfo.RangeStyle = field.Ranged? new InspectableFieldRangeStyle(field.RangeMinimum, field.RangeMaximum, field.IsSlider) : null;
            styleInfo.StepStyle = field.Stepped? new InspectableFieldStepStyle(field.Step) : null;
            return styleInfo;
        }
    }
}
