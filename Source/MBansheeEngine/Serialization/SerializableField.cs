//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using System.Runtime.CompilerServices;

namespace BansheeEngine
{
    /** @addtogroup Serialization
     *  @{
     */

    /// <summary>
    /// Allows you to access meta-data about field in an object. Similar to Reflection but simpler and faster.
    /// </summary>
    public class SerializableField : ScriptObject
    {
        private SerializableObject parent;
        private SerializableProperty.FieldType type;
        private int flags;
        private Type internalType;
        private string name;

        /// <summary>
        /// Constructor for internal use by the runtime.
        /// </summary>
        /// <param name="parent">Object that conains the field.</param>
        /// <param name="name">Name of the field.</param>
        /// <param name="flags">Flags that control whether the field is inspectable or serializable.</param>
        /// <param name="internalType">Internal C# type of the field.</param>
        private SerializableField(SerializableObject parent, string name, int flags, Type internalType)
        {
            this.parent = parent;
            this.name = name;
            this.flags = flags;
            this.type = SerializableProperty.DetermineFieldType(internalType);
            this.internalType = internalType;
        }

        /// <summary>
        /// Returns the type of data contained in the field.
        /// </summary>
        public SerializableProperty.FieldType Type
        {
            get { return type; }
        }

        /// <summary>
        /// Returns the name of the field.
        /// </summary>
        public string Name
        {
            get { return name; }
        }

        /// <summary>
        /// Returns true if the field accepts a defined range.
        /// </summary>
        public bool Range
        {
            get { return (flags & 0x04) != 0; }
        }

        /// <summary>
        /// Returns the upper bound of the range
        /// </summary>
        public float RangeMaximum
        {
            get { return Range? Internal_GetRangeMaximum(mCachedPtr) : 0; }
        }

        /// <summary>
        /// Returns the lower bound of the range
        /// </summary>
        public float RangeMinimum
        {
            get { return Range? Internal_GetRangeMinimum(mCachedPtr) : 0; }
        }

        /// <summary>
        /// Returns the step of the range
        /// </summary>
        public float RangeStep
        {
            get { return Range ? Internal_GetRangeStep(mCachedPtr) : 0; }
        }

        /// <summary>
        /// Returns true if the field will be visible in the default inspector.
        /// </summary>
        public bool Inspectable
        {
            get { return (flags & 0x02) != 0; } // Flags as defined in native code in BsManagedSerializableObjectInfo.h
        }

        /// <summary>
        /// Returns true if the field will be automatically serialized.
        /// </summary>
        public bool Serializable
        {
            get { return (flags & 0x01) != 0; } // Flags as defined in native code in BsManagedSerializableObjectInfo.h
        }

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
                    return Internal_GetValue(mCachedPtr, parentObject);
                else
                    return null;
            };

            SerializableProperty.Setter setter = (object value) =>
            {
                object parentObject = parent.GetReferencedObject();

                if (parentObject != null)
                {
                    Internal_SetValue(mCachedPtr, parentObject, value);

                    // If value type we cannot just modify the parent object because it's just a copy
                    if (parentObject.GetType().IsValueType && parent.parentProperty != null)
                        parent.parentProperty.SetValue(parentObject);
                }
            };

            SerializableProperty newProperty = Internal_CreateProperty(mCachedPtr);
            newProperty.Construct(type, internalType, getter, setter);

            return newProperty;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SerializableProperty Internal_CreateProperty(IntPtr nativeInstance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern object Internal_GetValue(IntPtr nativeInstance, object instance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetValue(IntPtr nativeInstance, object instance, object value);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRangeMaximum(IntPtr field);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRangeMinimum(IntPtr field);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetRangeStep(IntPtr field);
    }

    /** @} */
}
