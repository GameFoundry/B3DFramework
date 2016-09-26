//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

namespace BansheeEditor
{
    /// <summary>
    /// Contains information about the minimum change allowed to a field value.
    /// </summary>
    public sealed class InspectableFieldStepStyle : InspectableFieldStyle
    {
        public InspectableFieldStepStyle(float step)
        {
            this.Step = step;
        }

        /// <summary>
        /// Minimum change of the field. Every change will be rounded to a multiple of this value.
        /// </summary>
        public float Step { get; set; }
    }
}
