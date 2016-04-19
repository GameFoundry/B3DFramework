//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Inspectors
     *  @{
     */

    /// <summary>
    /// Draws GUI elements for inspecting an <see cref="Spring"/> object.
    /// </summary>
    internal class SpringGUI
    {
        private Spring spring;

        private GUIFloatField stiffnessField = new GUIFloatField(new LocEdString("Stiffness"));
        private GUIFloatField dampingField = new GUIFloatField(new LocEdString("Damping"));

        public Action<Spring> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current value of the spring object.
        /// </summary>
        public Spring Spring
        {
            get { return spring; }
            set
            {
                spring = value;

                stiffnessField.Value = value.stiffness;
                dampingField.Value = value.damping;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the spring object.
        /// </summary>
        /// <param name="spring">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        public SpringGUI(Spring spring, GUILayout layout)
        {
            this.spring = spring;

            stiffnessField.OnChanged += x => { spring.stiffness = x; MarkAsModified(); };
            stiffnessField.OnFocusLost += ConfirmModify;
            stiffnessField.OnConfirmed += ConfirmModify;

            dampingField.OnChanged += x => { spring.damping = x; MarkAsModified(); };
            dampingField.OnFocusLost += ConfirmModify;
            dampingField.OnConfirmed += ConfirmModify;

            layout.AddElement(stiffnessField);
            layout.AddElement(dampingField);
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(spring);
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        private void ConfirmModify()
        {
            if (OnConfirmed != null)
                OnConfirmed();
        }
    }

    /// <summary>
    /// Draws GUI elements for inspecting an <see cref="LimitCommon"/> object.
    /// </summary>
    internal class LimitCommonGUI
    {
        private LimitCommonData limitData;
        private SerializableProperties properties;
        private string prefix;

        private GUIToggle hardFoldout = new GUIToggle(new LocEdString("Hard"), EditorStyles.Foldout);
        private GUIFloatField contactDistanceField = new GUIFloatField(new LocEdString("Contact distance"));
        private GUIToggle softFoldout = new GUIToggle(new LocEdString("Soft"), EditorStyles.Foldout);
        private GUISliderField restitutionField = new GUISliderField(0, 1, new LocEdString("Restitution"));
        private GUIToggle springFoldout = new GUIToggle(new LocEdString("Spring"), EditorStyles.Foldout);
        private SpringGUI springGUI;

        private GUILayoutX hardLimitLayout;
        private GUILayoutX softLimitLayout;
        private GUILayoutX springLayout;

        public Action<LimitCommonData> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current limit properties.
        /// </summary>
        public LimitCommonData LimitData
        {
            get { return limitData; }
            set
            {
                limitData = value;

                contactDistanceField.Value = value.contactDist;
                restitutionField.Value = value.restitution;
                springGUI.Spring = value.spring;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the limit object.
        /// </summary>
        /// <param name="prefix">Prefix that identifies the exact type of the limit type.</param>
        /// <param name="limitData">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        /// <param name="properties">A set of properties that are persisted by the parent inspector. Used for saving state.
        ///                          </param>
        public LimitCommonGUI(string prefix, LimitCommonData limitData, GUILayout layout, SerializableProperties properties)
        {
            this.limitData = limitData;
            this.properties = properties;
            this.prefix = prefix;

            hardFoldout.OnToggled += x =>
            {
                properties.SetBool(prefix + "_hardLimit_Expanded", x);
                ToggleLimitFields();
            };

            contactDistanceField.OnChanged += x => { this.limitData.contactDist = x; MarkAsModified(); };
            contactDistanceField.OnFocusLost += ConfirmModify;
            contactDistanceField.OnConfirmed += ConfirmModify;

            softFoldout.OnToggled += x =>
            {
                properties.SetBool(prefix + "_softLimit_Expanded", x);
                ToggleLimitFields();
            };

            restitutionField.OnChanged += x => { this.limitData.restitution = x; MarkAsModified(); };
            restitutionField.OnFocusLost += ConfirmModify;

            springFoldout.OnToggled += x =>
            {
                properties.SetBool(prefix + "_spring_Expanded", x);
                ToggleLimitFields();
            };

            hardLimitLayout = layout.AddLayoutX();
            {
                hardLimitLayout.AddSpace(10);

                GUILayoutY hardLimitContentsLayout = hardLimitLayout.AddLayoutY();
                hardLimitContentsLayout.AddElement(contactDistanceField);
            }

            softLimitLayout = layout.AddLayoutX();
            layout.AddElement(softFoldout);
            {
                softLimitLayout.AddSpace(10);

                GUILayoutY softLimitContentsLayout = softLimitLayout.AddLayoutY();
                softLimitContentsLayout.AddElement(restitutionField);
                softLimitContentsLayout.AddElement(springFoldout);
                springLayout = softLimitContentsLayout.AddLayoutX();
                {
                    springLayout.AddSpace(10);

                    GUILayoutY springContentsLayout = springLayout.AddLayoutY();
                    springGUI = new SpringGUI(limitData.spring, springContentsLayout);
                    springGUI.OnChanged += x => { this.limitData.spring = x; MarkAsModified(); };
                    springGUI.OnConfirmed += ConfirmModify;
                }
            }
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(limitData);
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        private void ConfirmModify()
        {
            if (OnConfirmed != null)
                OnConfirmed();
        }

        /// <summary>
        /// Hides or shows limit property GUI elements depending on set properties.
        /// </summary>
        private void ToggleLimitFields()
        {
            hardLimitLayout.Active = properties.GetBool(prefix + "_hardLimit_Expanded");
            softLimitLayout.Active = properties.GetBool(prefix + "_softLimit_Expanded");
            springLayout.Active = properties.GetBool(prefix + "_spring_Expanded");
        }
    }

    /// <summary>
    /// Draws GUI elements for inspecting an <see cref="LimitLinear"/> object.
    /// </summary>
    internal class LimitLinearGUI
    {
        private LimitLinearData limitData;

        private GUIFloatField limitExtentField = new GUIFloatField(new LocEdString("Extent"));
        private LimitCommonGUI limitCommonGUI;

        public Action<LimitLinearData, LimitCommonData> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current limit properties.
        /// </summary>
        public LimitLinear Limit
        {
            set
            {
                limitData = value.Data;

                limitExtentField.Value = limitData.extent;
                limitCommonGUI.LimitData = value.CommonData;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the limit object.
        /// </summary>
        /// <param name="limit">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        /// <param name="properties">A set of properties that are persisted by the parent inspector. Used for saving state.
        ///                          </param>
        public LimitLinearGUI(LimitLinear limit, GUILayout layout, SerializableProperties properties)
        {
            limitData = limit.Data;

            limitExtentField.OnChanged += x => { limitData.extent = x; MarkAsModified(); };
            limitExtentField.OnFocusLost += ConfirmModify;

            layout.AddElement(limitExtentField);
            limitCommonGUI = new LimitCommonGUI("linear", limit.CommonData, layout, properties);
            limitCommonGUI.OnChanged += x => MarkAsModified();
            limitCommonGUI.OnConfirmed += ConfirmModify;
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(limitData, limitCommonGUI.LimitData);
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        private void ConfirmModify()
        {
            if (OnConfirmed != null)
                OnConfirmed();
        }
    }

    /// <summary>
    /// Draws GUI elements for inspecting an <see cref="LimitLinearRange"/> object.
    /// </summary>
    internal class LimitLinearRangeGUI
    {
        private LimitLinearRangeData limitData;

        private GUIFloatField limitLowerField = new GUIFloatField(new LocEdString("Lower"));
        private GUIFloatField limitUpperField = new GUIFloatField(new LocEdString("Upper"));
        private LimitCommonGUI limitCommonGUI;

        public Action<LimitLinearRangeData, LimitCommonData> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current limit properties.
        /// </summary>
        public LimitLinearRange Limit
        {
            set
            {
                limitData = value.Data;

                limitLowerField.Value = limitData.lower;
                limitUpperField.Value = limitData.upper;
                limitCommonGUI.LimitData = value.CommonData;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the limit object.
        /// </summary>
        /// <param name="limit">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        /// <param name="properties">A set of properties that are persisted by the parent inspector. Used for saving state.
        ///                          </param>
        public LimitLinearRangeGUI(LimitLinearRange limit, GUILayout layout, SerializableProperties properties)
        {
            this.limitData = limit.Data;

            limitLowerField.OnChanged += x => { limitData.lower = x; MarkAsModified(); };
            limitLowerField.OnFocusLost += ConfirmModify;

            limitUpperField.OnChanged += x => { limitData.upper = x; MarkAsModified(); };
            limitUpperField.OnFocusLost += ConfirmModify;

            layout.AddElement(limitLowerField);
            layout.AddElement(limitUpperField);
            limitCommonGUI = new LimitCommonGUI("linearRange", limit.CommonData, layout, properties);
            limitCommonGUI.OnChanged += x => MarkAsModified();
            limitCommonGUI.OnConfirmed += ConfirmModify;
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(limitData, limitCommonGUI.LimitData);
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        private void ConfirmModify()
        {
            if (OnConfirmed != null)
                OnConfirmed();
        }
    }

    /// <summary>
    /// Draws GUI elements for inspecting an <see cref="LimitAngularRange"/> object.
    /// </summary>
    internal class LimitAngularRangeGUI
    {
        private LimitAngularRangeData limitData;

        private GUISliderField limitLowerField = new GUISliderField(0, 359, new LocEdString("Lower"));
        private GUISliderField limitUpperField = new GUISliderField(0, 359, new LocEdString("Upper"));
        private LimitCommonGUI limitCommonGUI;

        public Action<LimitAngularRangeData, LimitCommonData> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current limit properties.
        /// </summary>
        public LimitAngularRange Limit
        {
            set
            {
                limitData = value.Data;

                limitLowerField.Value = limitData.lower.Degrees;
                limitUpperField.Value = limitData.upper.Degrees;
                limitCommonGUI.LimitData = value.CommonData;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the limit object.
        /// </summary>
        /// <param name="limit">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        /// <param name="properties">A set of properties that are persisted by the parent inspector. Used for saving state.
        ///                          </param>
        public LimitAngularRangeGUI(LimitAngularRange limit, GUILayout layout, SerializableProperties properties)
        {
            this.limitData = limit.Data;

            limitLowerField.OnChanged += x => { limitData.lower = new Degree(x); MarkAsModified(); };
            limitLowerField.OnFocusLost += ConfirmModify;

            limitUpperField.OnChanged += x => { limitData.upper = new Degree(x); MarkAsModified(); };
            limitUpperField.OnFocusLost += ConfirmModify;

            layout.AddElement(limitLowerField);
            layout.AddElement(limitUpperField);
            limitCommonGUI = new LimitCommonGUI("angularRange", limit.CommonData, layout, properties);
            limitCommonGUI.OnChanged += x => MarkAsModified();
            limitCommonGUI.OnConfirmed += ConfirmModify;
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(limitData, limitCommonGUI.LimitData);
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        private void ConfirmModify()
        {
            if (OnConfirmed != null)
                OnConfirmed();
        }
    }

    /// <summary>
    /// Draws GUI elements for inspecting an <see cref="LimitConeRange"/> object.
    /// </summary>
    internal class LimitConeRangeGUI
    {
        private LimitConeRangeData limitData;

        private GUISliderField yLimitAngleField = new GUISliderField(0, 180, new LocEdString("Y limit"));
        private GUISliderField zLimitAngleField = new GUISliderField(0, 180, new LocEdString("Z limit"));
        private LimitCommonGUI limitCommonGUI;

        public Action<LimitConeRangeData, LimitCommonData> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current limit properties.
        /// </summary>
        public LimitConeRange Limit
        {
            set
            {
                limitData = value.Data;

                yLimitAngleField.Value = limitData.yLimitAngle.Degrees;
                zLimitAngleField.Value = limitData.zLimitAngle.Degrees;
                limitCommonGUI.LimitData = value.CommonData;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the limit object.
        /// </summary>
        /// <param name="limit">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        /// <param name="properties">A set of properties that are persisted by the parent inspector. Used for saving state.
        ///                          </param>
        public LimitConeRangeGUI(LimitConeRange limit, GUILayout layout, SerializableProperties properties)
        {
            this.limitData = limit.Data;

            yLimitAngleField.OnChanged += x => { limitData.yLimitAngle = new Degree(x); MarkAsModified(); };
            yLimitAngleField.OnFocusLost += ConfirmModify;

            zLimitAngleField.OnChanged += x => { limitData.zLimitAngle = new Degree(x); MarkAsModified(); };
            zLimitAngleField.OnFocusLost += ConfirmModify;

            layout.AddElement(yLimitAngleField);
            layout.AddElement(zLimitAngleField);
            limitCommonGUI = new LimitCommonGUI("coneRange", limit.CommonData, layout, properties);
            limitCommonGUI.OnChanged += x => MarkAsModified();
            limitCommonGUI.OnConfirmed += ConfirmModify;
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(limitData, limitCommonGUI.LimitData);
        }

        /// <summary>
        /// Confirms any queued modifications.
        /// </summary>
        private void ConfirmModify()
        {
            if (OnConfirmed != null)
                OnConfirmed();
        }
    }

    /** @} */
}