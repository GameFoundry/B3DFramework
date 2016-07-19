//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspector
     *  @{
     */

    /// <summary>
    /// IsnpectableRangedField is a <see cref="InspectableField"/> that has a Range attribute and must be rendered as a slider.
    /// </summary>
    public abstract class InspectableRangedField : InspectableField
    {
        protected InspectableFieldStyleInfo style;
        /// <summary>
        /// Creates a new inspectable ranged field GUI for the specified property.
        /// </summary>
        /// <param name="parent">Parent Inspector this field belongs to.</param>
        /// <param name="title">Name of the property, or some other value to set as the title.</param>
        /// <param name="path">Full path to this property (includes name of this property and all parent properties).</param>
        /// <param name="type">Type of property this field will be used for displaying.</param>
        /// <param name="depth">Determines how deep within the inspector nesting hierarchy is this field. Some fields may
        ///                     contain other fields, in which case you should increase this value by one.</param>
        /// <param name="layout">Parent layout that all the field elements will be added to.</param>
        /// <param name="property">Serializable property referencing the array whose contents to display.</param>
        /// <param name="style">Contains information about the field style</param>
        public InspectableRangedField(Inspector parent, string title, string path, SerializableProperty.FieldType type,
            int depth, InspectableFieldLayout layout, SerializableProperty property, InspectableFieldStyleInfo style) : base(parent, title, path, type, depth, layout, property)
        {
            this.style = style;
        }
    }

    /** @} */
}
