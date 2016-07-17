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

            styleInfo.RangeStyle = field.Range? new InspectableFieldRangeStyle(field.RangeMinimum, field.RangeMaximum) : null;
            styleInfo.StepStyle = field.Step != 0? new InspectableFieldStepStyle(field.Step) : null;
            if (styleInfo.StepStyle != null)
            {
                Debug.Log(styleInfo.StepStyle.Step);
            }
            else
            {
                Debug.Log("Step null");
            }
            if (styleInfo.RangeStyle != null)
            {
                Debug.Log(styleInfo.RangeStyle.Max);
                Debug.Log(styleInfo.RangeStyle.Min);
            }
            else
            {
                Debug.Log("Range null");
            }
            return styleInfo;
        }
    }
}
