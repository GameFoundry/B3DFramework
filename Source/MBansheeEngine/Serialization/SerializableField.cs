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
        Stepped = 0x08
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
        /// Returns true if the field accepts a defined range.
        /// </summary>
        public bool IsRanged
        {
            get { return (flags & (byte)SerializableFieldAttributes.Ranged) != 0; }
        }

        /// <summary>
        /// Returns the upper bound of the range.
        /// </summary>
        public float RangeMaximum
        {
            get { return IsRanged? Internal_GetRangeMaximum(mCachedPtr) : 0; }
        }

        /// <summary>
        /// Returns the lower bound of the range.
        /// </summary>
        public float RangeMinimum
        {
            get { return IsRanged? Internal_GetRangeMinimum(mCachedPtr) : 0; }
        }

        /// <summary>
        /// Whether the field is rendered as a slider.
        /// </summary>
        public bool IsSlider
        {
            get { return (IsRanged && Internal_RenderAsSlider(mCachedPtr)); }
        }

        /// <summary>
        /// Whether the field has an associated step value.
        /// </summary>
        public bool IsStepped
        {
            get { return (flags & (byte)SerializableFieldAttributes.Stepped) != 0; }
        }

        /// <summary>
        /// Returns the step of the range
        /// </summary>
        public float Step
        {
            get { return IsStepped? Internal_GetStep(mCachedPtr) : 0; }
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
        private static extern bool Internal_RenderAsSlider(IntPtr field);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float Internal_GetStep(IntPtr field);
    }

    /** @} */
}
