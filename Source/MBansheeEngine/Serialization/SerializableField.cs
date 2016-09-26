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
    /// Flags that are used to define properties of a single field in a managed object.
    /// </summary>
    internal enum SerializableFieldAttributes // Note: Must match C++ enum ScriptFieldFlags
    {
        Serializable = 0x01,
        Inspectable = 0x02,
        Ranged = 0x04,
        Stepped = 0x08,
        Animable = 0x10
    }

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
        /// Returns true if the field will be visible in the default inspector.
        /// </summary>
        public bool Inspectable
        {
            get { return (flags & (byte)SerializableFieldAttributes.Inspectable) != 0; }
        }

        /// <summary>
        /// Returns true if the field will be automatically serialized.
        /// </summary>
        public bool Serializable
        {
            get { return (flags & (byte)SerializableFieldAttributes.Serializable) != 0; }
        }

        /// <summary>
        /// Returns true if the field can be animated.
        /// </summary>
        public bool Animable
        {
            get { return (flags & (byte)SerializableFieldAttributes.Animable) != 0; }
        }

        /// <summary>
        /// Returns the requested style of the field, that may be used for controlling how the field is displayed and how
        /// is its input filtered.
        /// </summary>
        public SerializableFieldStyle Style
        {
            get
            {
                SerializableFieldStyle style;
                Internal_GetStyle(mCachedPtr, out style);
                return style;
            }
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
        private static extern void Internal_GetStyle(IntPtr nativeInstance, out SerializableFieldStyle style);
    }

    /// <summary>
    /// Contains information about a style of a serializable field.
    /// </summary>
    public struct SerializableFieldStyle // Note: Must match C++ struct SerializableMemberStyle
    {
        /// <summary>
        /// True if the range of the field is limited, false if unlimited.
        /// </summary>
        public bool HasRange;
        /// <summary>
        /// Returns the lower bound of the range. Only relevant if <see cref="HasRange"/> is true.
        /// </summary>
        public float RangeMin;
        /// <summary>
        /// Returns the upper bound of the range. Only relevant if <see cref="HasRange"/> is true.
        /// </summary>
        public float RangeMax;
        /// <summary>
        /// True if the field value can only be incremented in specific increments.
        /// </summary>
        public bool HasStep;
        /// <summary>
        /// Minimum increment the field value can be increment/decremented by. Only relevant if <see cref="HasStep"/> is true.
        /// </summary>
        public float StepIncrement;
        /// <summary>
        /// If true, number fields will be displayed as sliders instead of regular input boxes.
        /// </summary>
        public bool DisplayAsSlider;
    }

    /** @} */
}
