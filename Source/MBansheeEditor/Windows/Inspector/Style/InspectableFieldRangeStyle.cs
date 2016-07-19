//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

namespace BansheeEditor
{
    /// <summary>
    /// Contains info about the range of values a field can store.
    /// </summary>
    public sealed class InspectableFieldRangeStyle : InspectableFieldStyle
    {
        public InspectableFieldRangeStyle(float min, float max, bool slider)
        {
            this.Max = max;
            this.Min = min;
            this.Slider = slider;
        }

        /// <summary>
        /// The maximum value the field can be assigned.
        /// </summary>
        public float Max { get; set; }

        /// <summary>
        /// The minimum value the field can be assigned.
        /// </summary>
        public float Min { get; set; }

        /// <summary>
        /// Whether the field is rendered as a slider.
        /// </summary>
        public bool Slider { get; set; }
    }
}
