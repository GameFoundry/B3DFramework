//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

namespace BansheeEditor
{
    /// <summary>
    /// Contains boolean information about a field style.
    /// </summary>
    public enum InstectableFieldStyleFlags
    {
        None = 0,
        UseSlider = 1,
    }

    /// <summary>
    /// Contains all the information about a field style.
    /// </summary>
    public class InspectableFieldStyleInfo
    {
        /// <summary>
        /// Information about the field range.
        /// </summary>
        public InspectableFieldRangeStyle RangeStyle;

        /// <summary>
        /// Information about the field stepping.
        /// </summary>
        public InspectableFieldStepStyle StepStyle;

        /// <summary>
        /// Boolean information about the field.
        /// </summary>
        public InstectableFieldStyleFlags StyleFlags;

        /// <summary>
        /// Creates an empty set of information about a field style.
        /// </summary>
        public InspectableFieldStyleInfo()
        {
            
        }
    }
}
