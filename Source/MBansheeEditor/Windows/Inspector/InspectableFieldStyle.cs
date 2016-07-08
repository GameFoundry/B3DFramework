using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BansheeEditor;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Contains style information about inspectable items
    /// </summary>
    public class InspectableFieldStyle
    {
        public static InspectableFieldStyle[] Create(SerializableField field)
        {
            List<InspectableFieldStyle> styles = new List<InspectableFieldStyle>();
            if (field.Range)
                styles.Add(new InspectableFieldRangeStyle(field.RangeMinimum, field.RangeMaximum, field.RangeStep));
            return styles.ToArray();
        }

        public static T FindStyle<T>(InspectableFieldStyle[] styleInfo) where T : InspectableFieldRangeStyle
        {
            for  (int i = 0; i < styleInfo.Length; i++)
            {
                if (styleInfo[i].GetType() == typeof(T))
                    return (T)styleInfo[i];
            }
            return default(T);
        }
    }
}
