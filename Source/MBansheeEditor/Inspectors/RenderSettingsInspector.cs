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
    /// Draws GUI elements for inspecting an <see cref="AutoExposureSettings"/> object.
    /// </summary>
    internal class AutoExposureSettingsGUI
    {
        private AutoExposureSettings settings;

        private GUISliderField histogramLog2MinField = new GUISliderField(-16.0f, 0.0f, new LocEdString("Histogram min."));
        private GUISliderField histogramLog2MaxField = new GUISliderField(0.0f, 16.0f, new LocEdString("Histogram max."));
        private GUISliderField histogramPctLowField = new GUISliderField(0.0f, 1.0f, new LocEdString("Histogram % low"));
        private GUISliderField histogramPctHighField = new GUISliderField(0.0f, 1.0f, new LocEdString("Histogram % high"));
        private GUISliderField minEyeAdaptationField = new GUISliderField(0.0f, 10.0f, new LocEdString("Min. eye adapatation"));
        private GUISliderField maxEyeAdaptationField = new GUISliderField(0.0f, 10.0f, new LocEdString("Max. eye adapatation"));
        private GUISliderField eyeAdaptationSpeedUpField = new GUISliderField(0.01f, 20.0f, new LocEdString("Eye adaptation speed up"));
        private GUISliderField eyeAdaptationSpeedDownField = new GUISliderField(0.01f, 20.0f, new LocEdString("Eye adaptation speed down"));

        public Action<AutoExposureSettings> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current value of the settings object.
        /// </summary>
        public AutoExposureSettings Settings
        {
            get { return settings; }
            set
            {
                settings = value;

                histogramLog2MinField.Value = value.HistogramLog2Min;
                histogramLog2MaxField.Value = value.HistogramLog2Max;

                histogramPctLowField.Value = value.HistogramPctLow;
                histogramPctHighField.Value = value.HistogramPctHigh;

                minEyeAdaptationField.Value = value.MinEyeAdaptation;
                maxEyeAdaptationField.Value = value.MaxEyeAdaptation;

                eyeAdaptationSpeedUpField.Value = value.EyeAdaptationSpeedUp;
                eyeAdaptationSpeedDownField.Value = value.EyeAdaptationSpeedDown;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the auto exposure settings object.
        /// </summary>
        /// <param name="settings">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        public AutoExposureSettingsGUI(AutoExposureSettings settings, GUILayout layout)
        {
            this.settings = settings;

            histogramLog2MinField.OnChanged += x => { this.settings.HistogramLog2Min = x; MarkAsModified(); ConfirmModify(); };
            histogramLog2MaxField.OnChanged += x => { this.settings.HistogramLog2Max = x; MarkAsModified(); ConfirmModify(); };
            histogramPctLowField.OnChanged += x => { this.settings.HistogramPctLow = x; MarkAsModified(); ConfirmModify(); };
            histogramPctHighField.OnChanged += x => { this.settings.HistogramPctHigh = x; MarkAsModified(); ConfirmModify(); };
            minEyeAdaptationField.OnChanged += x => { this.settings.MinEyeAdaptation = x; MarkAsModified(); ConfirmModify(); };
            maxEyeAdaptationField.OnChanged += x => { this.settings.MaxEyeAdaptation = x; MarkAsModified(); ConfirmModify(); };
            eyeAdaptationSpeedUpField.OnChanged += x => { this.settings.EyeAdaptationSpeedUp = x; MarkAsModified(); ConfirmModify(); };
            eyeAdaptationSpeedDownField.OnChanged += x => { this.settings.EyeAdaptationSpeedDown = x; MarkAsModified(); ConfirmModify(); };

            layout.AddElement(histogramLog2MinField);
            layout.AddElement(histogramLog2MaxField);
            layout.AddElement(histogramPctLowField);
            layout.AddElement(histogramPctHighField);
            layout.AddElement(minEyeAdaptationField);
            layout.AddElement(maxEyeAdaptationField);
            layout.AddElement(eyeAdaptationSpeedUpField);
            layout.AddElement(eyeAdaptationSpeedDownField);
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(settings);
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
    /// Draws GUI elements for inspecting an <see cref="TonemappingSettings"/> object.
    /// </summary>
    internal class TonemappingSettingsGUI
    {
        private TonemappingSettings settings;

        private GUIFloatField shoulderStrengthField = new GUIFloatField(new LocEdString("Shoulder strength"));
        private GUIFloatField linearStrengthField = new GUIFloatField(new LocEdString("Linear strength"));
        private GUIFloatField linearAngleField = new GUIFloatField(new LocEdString("Linear angle"));
        private GUIFloatField toeStrengthField = new GUIFloatField(new LocEdString("Toe strength"));
        private GUIFloatField toeNumeratorField = new GUIFloatField(new LocEdString("Toe numerator"));
        private GUIFloatField toeDenominatorField = new GUIFloatField(new LocEdString("Toe denominator"));
        private GUIFloatField whitePointField = new GUIFloatField(new LocEdString("White point"));

        public Action<TonemappingSettings> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current value of the settings object.
        /// </summary>
        public TonemappingSettings Settings
        {
            get { return settings; }
            set
            {
                settings = value;

                shoulderStrengthField.Value = value.FilmicCurveShoulderStrength;
                linearStrengthField.Value = value.FilmicCurveLinearStrength;

                linearAngleField.Value = value.FilmicCurveLinearAngle;
                toeStrengthField.Value = value.FilmicCurveToeStrength;

                toeNumeratorField.Value = value.FilmicCurveToeNumerator;
                toeDenominatorField.Value = value.FilmicCurveToeDenominator;

                whitePointField.Value = value.FilmicCurveLinearWhitePoint;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the tone mapping settings object.
        /// </summary>
        /// <param name="settings">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        public TonemappingSettingsGUI(TonemappingSettings settings, GUILayout layout)
        {
            this.settings = settings;

            shoulderStrengthField.OnChanged += x => { this.settings.FilmicCurveShoulderStrength = x; MarkAsModified(); };
            shoulderStrengthField.OnFocusLost += ConfirmModify;
            shoulderStrengthField.OnConfirmed += ConfirmModify;

            linearStrengthField.OnChanged += x => { this.settings.FilmicCurveLinearStrength = x; MarkAsModified(); };
            linearStrengthField.OnFocusLost += ConfirmModify;
            linearStrengthField.OnConfirmed += ConfirmModify;

            linearAngleField.OnChanged += x => { this.settings.FilmicCurveLinearAngle = x; MarkAsModified(); };
            linearAngleField.OnFocusLost += ConfirmModify;
            linearAngleField.OnConfirmed += ConfirmModify;

            toeStrengthField.OnChanged += x => { this.settings.FilmicCurveToeStrength = x; MarkAsModified(); };
            toeStrengthField.OnFocusLost += ConfirmModify;
            toeStrengthField.OnConfirmed += ConfirmModify;

            toeNumeratorField.OnChanged += x => { this.settings.FilmicCurveToeNumerator = x; MarkAsModified(); };
            toeNumeratorField.OnFocusLost += ConfirmModify;
            toeNumeratorField.OnConfirmed += ConfirmModify;

            toeDenominatorField.OnChanged += x => { this.settings.FilmicCurveToeDenominator = x; MarkAsModified(); };
            toeDenominatorField.OnFocusLost += ConfirmModify;
            toeDenominatorField.OnConfirmed += ConfirmModify;

            whitePointField.OnChanged += x => { this.settings.FilmicCurveLinearWhitePoint = x; MarkAsModified(); };
            whitePointField.OnFocusLost += ConfirmModify;
            whitePointField.OnConfirmed += ConfirmModify;

            layout.AddElement(shoulderStrengthField);
            layout.AddElement(linearStrengthField);
            layout.AddElement(linearAngleField);
            layout.AddElement(toeStrengthField);
            layout.AddElement(toeNumeratorField);
            layout.AddElement(toeDenominatorField);
            layout.AddElement(whitePointField);
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(settings);
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
    /// Draws GUI elements for inspecting an <see cref="ColorGradingSettings"/> object.
    /// </summary>
    internal class ColorGradingSettingsGUI
    {
        private ColorGradingSettings settings;

        private GUIVector3Field saturationField = new GUIVector3Field(new LocEdString("Saturation"));
        private GUIVector3Field contrastField = new GUIVector3Field(new LocEdString("Contrast"));
        private GUIVector3Field gainField = new GUIVector3Field(new LocEdString("Gain"));
        private GUIVector3Field offsetField = new GUIVector3Field(new LocEdString("Offset"));

        public Action<ColorGradingSettings> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current value of the settings object.
        /// </summary>
        public ColorGradingSettings Settings
        {
            get { return settings; }
            set
            {
                settings = value;

                saturationField.Value = value.Saturation;
                contrastField.Value = value.Contrast;
                gainField.Value = value.Gain;
                offsetField.Value = value.Offset;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the color grading settings object.
        /// </summary>
        /// <param name="settings">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        public ColorGradingSettingsGUI(ColorGradingSettings settings, GUILayout layout)
        {
            this.settings = settings;

            saturationField.OnChanged += x => { this.settings.Saturation = x; MarkAsModified(); };
            saturationField.OnFocusLost += ConfirmModify;
            saturationField.OnConfirmed += ConfirmModify;

            contrastField.OnChanged += x => { this.settings.Contrast = x; MarkAsModified(); };
            contrastField.OnFocusLost += ConfirmModify;
            contrastField.OnConfirmed += ConfirmModify;

            gainField.OnChanged += x => { this.settings.Gain = x; MarkAsModified(); };
            gainField.OnFocusLost += ConfirmModify;
            gainField.OnConfirmed += ConfirmModify;

            offsetField.OnChanged += x => { this.settings.Offset = x; MarkAsModified(); };
            offsetField.OnFocusLost += ConfirmModify;
            offsetField.OnConfirmed += ConfirmModify;

            layout.AddElement(saturationField);
            layout.AddElement(contrastField);
            layout.AddElement(gainField);
            layout.AddElement(offsetField);
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(settings);
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
    /// Draws GUI elements for inspecting an <see cref="WhiteBalanceSettings"/> object.
    /// </summary>
    internal class WhiteBalanceSettingsGUI
    {
        private WhiteBalanceSettings settings;

        private GUISliderField temperatureField = new GUISliderField(1500.0f, 15000.0f, new LocEdString("Temperature"));
        private GUISliderField tintField = new GUISliderField(-1.0f, 1.0f, new LocEdString("Tint"));

        public Action<WhiteBalanceSettings> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current value of the settings object.
        /// </summary>
        public WhiteBalanceSettings Settings
        {
            get { return settings; }
            set
            {
                settings = value;

                temperatureField.Value = value.Temperature;
                tintField.Value = value.Tint;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the white balance settings object.
        /// </summary>
        /// <param name="settings">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        public WhiteBalanceSettingsGUI(WhiteBalanceSettings settings, GUILayout layout)
        {
            this.settings = settings;

            temperatureField.OnChanged += x => { this.settings.Temperature = x; MarkAsModified(); ConfirmModify(); };
            tintField.OnChanged += x => { this.settings.Tint = x; MarkAsModified(); ConfirmModify(); };

            layout.AddElement(temperatureField);
            layout.AddElement(tintField);
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(settings);
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
    /// Draws GUI elements for inspecting an <see cref="RenderSettings"/> object.
    /// </summary>
    internal class RenderSettingsGUI
    {
        private RenderSettings settings;
        private SerializableProperties properties;

        private GUIToggleField enableAutoExposureField = new GUIToggleField(new LocEdString("Enable auto exposure"));
        private GUIToggle autoExposureFoldout = new GUIToggle(new LocEdString("Auto exposure"), EditorStyles.Foldout);
        private AutoExposureSettingsGUI autoExposureGUI;

        private GUIToggleField enableToneMappingField = new GUIToggleField(new LocEdString("Enable tone mapping"));
        private GUIToggle toneMappingFoldout = new GUIToggle(new LocEdString("Tone mapping"), EditorStyles.Foldout);
        private TonemappingSettingsGUI toneMappingGUI;
        private GUIToggle whiteBalanceFoldout = new GUIToggle(new LocEdString("White balance"), EditorStyles.Foldout);
        private WhiteBalanceSettingsGUI whiteBalanceGUI;
        private GUIToggle colorGradingFoldout = new GUIToggle(new LocEdString("Color grading"), EditorStyles.Foldout);
        private ColorGradingSettingsGUI colorGradingGUI;

        private GUISliderField gammaField = new GUISliderField(1.0f, 3.0f, new LocEdString("Gamma"));
        private GUISliderField exposureScaleField = new GUISliderField(-8.0f, 8.0f, new LocEdString("Exposure scale"));

        private GUILayout autoExposureLayout;
        private GUILayout toneMappingLayout;
        private GUILayout whiteBalanceLayout;
        private GUILayout colorGradingLayout;

        public Action<RenderSettings> OnChanged;
        public Action OnConfirmed;

        /// <summary>
        /// Current value of the settings object.
        /// </summary>
        public RenderSettings Settings
        {
            get { return settings; }
            set
            {
                settings = value;

                enableAutoExposureField.Value = value.EnableAutoExposure;
                autoExposureGUI.Settings = value.AutoExposure;
                enableToneMappingField.Value = value.EnableTonemapping;
                toneMappingGUI.Settings = value.Tonemapping;
                whiteBalanceGUI.Settings = value.WhiteBalance;
                colorGradingGUI.Settings = value.ColorGrading;
                gammaField.Value = value.Gamma;
                exposureScaleField.Value = value.ExposureScale;
            }
        }

        /// <summary>
        /// Constructs a new set of GUI elements for inspecting the post process settings object.
        /// </summary>
        /// <param name="settings">Initial values to assign to the GUI elements.</param>
        /// <param name="layout">Layout to append the GUI elements to.</param>
        /// <param name="properties">A set of properties that are persisted by the parent inspector. Used for saving state.
        ///                          </param>
        public RenderSettingsGUI(RenderSettings settings, GUILayout layout, SerializableProperties properties)
        {
            this.settings = settings;
            this.properties = properties;

            // Auto exposure
            enableAutoExposureField.OnChanged += x => { this.settings.EnableAutoExposure = x; MarkAsModified(); ConfirmModify(); };
            layout.AddElement(enableAutoExposureField);

            autoExposureFoldout.OnToggled += x =>
            {
                properties.SetBool("autoExposure_Expanded", x);
                ToggleFoldoutFields();
            };
            layout.AddElement(autoExposureFoldout);

            autoExposureLayout = layout.AddLayoutX();
            {
                autoExposureLayout.AddSpace(10);

                GUILayoutY contentsLayout = autoExposureLayout.AddLayoutY();
                autoExposureGUI = new AutoExposureSettingsGUI(settings.AutoExposure, contentsLayout);
                autoExposureGUI.OnChanged += x => { this.settings.AutoExposure = x; MarkAsModified(); };
                autoExposureGUI.OnConfirmed += ConfirmModify;
            }

            // Tonemapping
            enableToneMappingField.OnChanged += x => { this.settings.EnableTonemapping = x; MarkAsModified(); ConfirmModify(); };
            layout.AddElement(enableToneMappingField);

            //// Tonemapping settings
            toneMappingFoldout.OnToggled += x =>
            {
                properties.SetBool("toneMapping_Expanded", x);
                ToggleFoldoutFields();
            };
            layout.AddElement(toneMappingFoldout);

            toneMappingLayout = layout.AddLayoutX();
            {
                toneMappingLayout.AddSpace(10);

                GUILayoutY contentsLayout = toneMappingLayout.AddLayoutY();
                toneMappingGUI = new TonemappingSettingsGUI(settings.Tonemapping, contentsLayout);
                toneMappingGUI.OnChanged += x => { this.settings.Tonemapping = x; MarkAsModified(); };
                toneMappingGUI.OnConfirmed += ConfirmModify;
            }

            //// White balance settings
            whiteBalanceFoldout.OnToggled += x =>
            {
                properties.SetBool("whiteBalance_Expanded", x);
                ToggleFoldoutFields();
            };
            layout.AddElement(whiteBalanceFoldout);

            whiteBalanceLayout = layout.AddLayoutX();
            {
                whiteBalanceLayout.AddSpace(10);

                GUILayoutY contentsLayout = whiteBalanceLayout.AddLayoutY();
                whiteBalanceGUI = new WhiteBalanceSettingsGUI(settings.WhiteBalance, contentsLayout);
                whiteBalanceGUI.OnChanged += x => { this.settings.WhiteBalance = x; MarkAsModified(); };
                whiteBalanceGUI.OnConfirmed += ConfirmModify;
            }

            //// Color grading settings
            colorGradingFoldout.OnToggled += x =>
            {
                properties.SetBool("colorGrading_Expanded", x);
                ToggleFoldoutFields();
            };
            layout.AddElement(colorGradingFoldout);

            colorGradingLayout = layout.AddLayoutX();
            {
                colorGradingLayout.AddSpace(10);

                GUILayoutY contentsLayout = colorGradingLayout.AddLayoutY();
                colorGradingGUI = new ColorGradingSettingsGUI(settings.ColorGrading, contentsLayout);
                colorGradingGUI.OnChanged += x => { this.settings.ColorGrading = x; MarkAsModified(); };
                colorGradingGUI.OnConfirmed += ConfirmModify;
            }

            // Gamma
            gammaField.OnChanged += x => { this.settings.Gamma = x; MarkAsModified(); ConfirmModify(); };
            layout.AddElement(gammaField);

            // Exposure scale
            exposureScaleField.OnChanged += x => { this.settings.ExposureScale = x; MarkAsModified(); ConfirmModify(); };
            layout.AddElement(exposureScaleField);

            ToggleFoldoutFields();
        }

        /// <summary>
        /// Marks the contents of the inspector as modified.
        /// </summary>
        private void MarkAsModified()
        {
            if (OnChanged != null)
                OnChanged(settings);
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
        /// Hides or shows settings property GUI elements depending on set values.
        /// </summary>
        private void ToggleFoldoutFields()
        {
            autoExposureLayout.Active = properties.GetBool("autoExposure_Expanded");
            toneMappingLayout.Active = properties.GetBool("toneMapping_Expanded");
            whiteBalanceLayout.Active = properties.GetBool("whiteBalance_Expanded");
            colorGradingLayout.Active = properties.GetBool("colorGrading_Expanded");
        }
    }

    /** @} */
}