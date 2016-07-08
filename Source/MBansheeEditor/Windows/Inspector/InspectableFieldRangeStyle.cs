using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BansheeEditor
{
    /// <summary>
    /// Contains info about the range of values a field can store
    /// </summary>
    public class InspectableFieldRangeStyle : InspectableFieldStyle
    {
        public float Max { get; set; }
        public float Min { get; set; }
        public float Step { get; set; }

        public InspectableFieldRangeStyle(float min, float max, float step = 0.0f)
        {
            this.Max = max;
            this.Min = min;
            this.Step = step;
        }
    }
}
