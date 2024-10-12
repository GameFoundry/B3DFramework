//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace bs
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Allows you to access meta-data about field in an object. Similar to Reflection but simpler and faster.
    /// </summary>
    public class SerializableField
    {
        private SerializableObject parent;
        private ManagedMemberInfo memberInfo;
        private SerializableProperty.FieldType type;
        private ManagedFieldMetaDataFlag flags;
        private Type internalType;
        private string name;

        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        /// <param name="parent">Object that conains the field.</param>
        /// <param name="memberInfo">Type information about the member.</param>
        public SerializableField(SerializableObject parent, ManagedMemberInfo memberInfo)
        {
            this.parent = parent;
            this.memberInfo = memberInfo;
            this.name = memberInfo.Name;
            this.flags = memberInfo.MetaDataFlags;
            this.internalType = memberInfo.TypeInfo.GetReflectionType();
            this.type = SerializableProperty.DetermineFieldType(this.internalType);
        }

        public ManagedFieldMetaDataFlag Flags
        {
            get { return flags; }
        }

        /// <summary>
        /// Returns the type of data contained in the field.
        /// </summary>
        public SerializableProperty.FieldType Type
        {
            get { return type; }
        }

        /// <summary>
        /// Returns the actual type of the object contained in the field.
        /// </summary>
        public Type InternalType
        {
            get { return internalType; }
        }

        /// <summary>
        /// Returns the name of the field.
        /// </summary>
        public string Name
        {
            get { return name; }
        }

        /// <summary>
        /// Returns the requested style of the field, that may be used for controlling how the field is displayed and how
        /// is its input filtered.
        /// </summary>
        public ManagedMemberStyle Style => memberInfo.ParseStyle();

        /// <summary>
        /// Returns a serializable property for the field.
        /// </summary>
        /// <returns>Serializable property that allows you to manipulate contents of the field.</returns>
        public SerializableProperty GetProperty()
        {
            SerializableProperty.Getter getter = () =>
            {
                object parentObject = parent.GetReferencedObject();

                if (parentObject != null)
                    return memberInfo.GetValue(parentObject);
                else
                    return null;
            };

            bool parentApplyOnChildChanges = parent.parentProperty?.ApplyOnChildChanges ?? false;

            SerializableProperty.Setter setter = (object value) =>
            {
                object parentObject = parent.GetReferencedObject();

                if (parentObject != null)
                {
                    memberInfo.SetValue(parentObject, value);

                    // If value type we cannot just modify the parent object because it's just a copy
                    if ((parentApplyOnChildChanges || parentObject.GetType().IsValueType) && parent.parentProperty != null)
                        parent.parentProperty.SetValue(parentObject);
                }
            };

            bool applyOnChildChanges = Flags.HasFlag(ManagedFieldMetaDataFlag.ApplyOnDirty) ||
                                       Flags.HasFlag(ManagedFieldMetaDataFlag.PassByCopy) ||
                                       parentApplyOnChildChanges;

            return new SerializableProperty(type, internalType, getter, setter, applyOnChildChanges);
        }
    }

    /** @} */
}
