//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System;
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// A color picker window that allows the user to select from a gamut of colors.
    /// </summary>
    public class ColorPicker : ModalWindow
    {
        private const int SliderIndividualWidth = 150;
        private const int SliderIndividualHeight = 15;

        private const int ColorBoxWidth = 200;
        private const int ColorBoxHeight = 200;

        private const int SliderSideWidth = 40;
        private const int SliderSideHeight = 200;

        private float colRed, colGreen, colBlue;
        private float colHue, colSaturation, colValue;
        private float colAlpha = 1.0f;

        private ColorSlider1DHorz sliderR, sliderG, sliderB, sliderA;
        private ColorSlider2D colorBox;
        private ColorSlider1DVert sideSlider;

        private ColorBoxMode colorBoxMode = ColorBoxMode.BG_R;
        private SliderMode sliderMode = SliderMode.HSV;

        private GUIColorField guiColor;
        private GUITexture guiSlider2DTex;
        private GUITexture guiSliderVertTex;
        private GUITexture guiSliderRHorzTex;
        private GUITexture guiSliderGHorzTex;
        private GUITexture guiSliderBHorzTex;
        private GUITexture guiSliderAHorzTex;

        private GUIButton guiColorBoxBtn;
        private GUIButton guiColorModeBtn;

        private GUISliderV guiSliderVert;
        private GUISliderH guiSliderRHorz;
        private GUISliderH guiSliderGHorz;
        private GUISliderH guiSliderBHorz;
        private GUISliderH guiSliderAHorz;
        private GUITexture guiSlider2DHandle;

        private GUILabel guiLabelR;
        private GUILabel guiLabelG;
        private GUILabel guiLabelB;
        private GUILabel guiLabelA;

        private GUIIntField guiInputR;
        private GUIIntField guiInputG;
        private GUIIntField guiInputB;
        private GUIIntField guiInputA;

        private GUIButton guiOK;
        private GUIButton guiCancel;

        private Action<bool, Color> closedCallback;

        /// <summary>
        /// Determines color gamut shown in the color box.
        /// </summary>
        public enum ColorBoxMode
        {
            BG_R,
            BR_G,
            RG_B,
            SV_H,
            HV_S,
            HS_V
        }

        /// <summary>
        /// Determines color gamut shown in the horizontal sliders.
        /// </summary>
        public enum SliderMode
        {
            RGB,
            HSV
        }

        /// <summary>
        /// Returns currently selected color as RGBA values.
        /// </summary>
        public Color SelectedColor
        {
            get
            {
                return new Color(colRed, colGreen, colBlue, colAlpha);
            }
        }

        /// <summary>
        /// Shows the color picker window.
        /// </summary>
        /// <param name="closedCallback">Optional callback to trigger when the user selects a color or cancels out
        ///                              of the dialog.</param>
        /// <returns>A. instance of the color picker window.</returns>
        public static ColorPicker Show(Action<bool, Color> closedCallback = null)
        {
            ColorPicker picker = new ColorPicker(Color.Black, closedCallback);
            return picker;
        }

        /// <summary>
        /// Shows the color picker window and sets the initial color to show.
        /// </summary>
        /// <param name="color">Initial color to display.</param>
        /// <param name="closedCallback">Optional callback to trigger when the user selects a color or cancels out
        ///                              of the dialog.</param>
        /// <returns>A. instance of the color picker window.</returns>
        public static ColorPicker Show(Color color, Action<bool, Color> closedCallback = null)
        {
            ColorPicker picker = new ColorPicker(color, closedCallback);
            return picker;
        }

        /// <summary>
        /// Constructs a new color picker window.
        /// </summary>
        /// <param name="color">Initial color to display.</param>
        /// <param name="closedCallback">Optional callback to trigger when the user selects a color or cancels out
        ///                              of the dialog.</param>
        protected ColorPicker(Color color, Action<bool, Color> closedCallback = null)
            : base(false)
        {
            Title = new LocEdString("Color Picker");
            Width = 270;
            Height = 400;

            colRed = color.r;
            colGreen = color.g;
            colBlue = color.b;
            colAlpha = color.a;

            RGBToHSV();

            this.closedCallback = closedCallback;
        }

        private void OnInitialize()
        {
            guiColor = new GUIColorField("", GUIOption.FixedWidth(100));
            guiSlider2DTex = new GUITexture(null, GUIOption.FixedHeight(ColorBoxHeight), GUIOption.FixedWidth(ColorBoxWidth));
            guiSliderVertTex = new GUITexture(null, GUIOption.FixedHeight(SliderSideHeight), GUIOption.FixedWidth(SliderSideWidth));
            guiSliderRHorzTex = new GUITexture(null, GUIOption.FixedHeight(SliderIndividualHeight));
            guiSliderGHorzTex = new GUITexture(null, GUIOption.FixedHeight(SliderIndividualHeight));
            guiSliderBHorzTex = new GUITexture(null, GUIOption.FixedHeight(SliderIndividualHeight));
            guiSliderAHorzTex = new GUITexture(null, GUIOption.FixedHeight(SliderIndividualHeight));

            guiColorBoxBtn = new GUIButton(colorBoxMode.ToString());
            guiColorModeBtn = new GUIButton(sliderMode.ToString());

            guiSliderVert = new GUISliderV(EditorStylesInternal.ColorSliderVert);
            guiSliderRHorz = new GUISliderH(EditorStylesInternal.ColorSliderHorz);
            guiSliderGHorz = new GUISliderH(EditorStylesInternal.ColorSliderHorz);
            guiSliderBHorz = new GUISliderH(EditorStylesInternal.ColorSliderHorz);
            guiSliderAHorz = new GUISliderH(EditorStylesInternal.ColorSliderHorz);
            guiSlider2DHandle = new GUITexture(null, EditorStylesInternal.ColorSlider2DHandle);

            guiLabelR = new GUILabel(new LocEdString("R"));
            guiLabelG = new GUILabel(new LocEdString("G"));
            guiLabelB = new GUILabel(new LocEdString("B"));
            guiLabelA = new GUILabel(new LocEdString("A"));

            guiInputR = new GUIIntField();
            guiInputG = new GUIIntField();
            guiInputB = new GUIIntField();
            guiInputA = new GUIIntField();

            guiOK = new GUIButton(new LocEdString("OK"));
            guiCancel = new GUIButton(new LocEdString("Cancel"));

            guiColorBoxBtn.OnClick += OnColorBoxModeChanged;
            guiColorModeBtn.OnClick += OnSliderModeChanged;

            guiSliderVert.OnChanged += OnSliderVertChanged;
            guiSliderRHorz.OnChanged += OnSliderRHorzChanged;
            guiSliderGHorz.OnChanged += OnSliderGHorzChanged;
            guiSliderBHorz.OnChanged += OnSliderBHorzChanged;
            guiSliderAHorz.OnChanged += OnSliderAHorzChanged;

            guiInputR.OnChanged += OnInputRChanged;
            guiInputG.OnChanged += OnInputGChanged;
            guiInputB.OnChanged += OnInputBChanged;
            guiInputA.OnChanged += OnInputAChanged;

            guiOK.OnClick += OnOK;
            guiCancel.OnClick += OnCancel;

            GUIPanel mainPanel = GUI.AddPanel(0);
            GUILayout v0 = mainPanel.AddLayoutY();

            v0.AddSpace(5);

            GUILayout h0 = v0.AddLayoutX();
            h0.AddSpace(10);
            h0.AddElement(guiColor);
            h0.AddFlexibleSpace();
            h0.AddElement(guiColorBoxBtn);
            h0.AddElement(guiColorModeBtn);
            h0.AddSpace(10);

            v0.AddSpace(10);

            GUILayout h1 = v0.AddLayoutX();
            h1.AddSpace(10);
            h1.AddElement(guiSlider2DTex);
            h1.AddFlexibleSpace();
            h1.AddElement(guiSliderVertTex);
            h1.AddSpace(10);

            v0.AddSpace(10);

            GUILayout h2 = v0.AddLayoutX();
            h2.AddSpace(10);
            h2.AddElement(guiLabelR);
            h2.AddFlexibleSpace();
            h2.AddElement(guiSliderRHorzTex);
            h2.AddFlexibleSpace();
            h2.AddElement(guiInputR);
            h2.AddSpace(10);

            v0.AddSpace(5);

            GUILayout h3 = v0.AddLayoutX();
            h3.AddSpace(10);
            h3.AddElement(guiLabelG);
            h3.AddFlexibleSpace();
            h3.AddElement(guiSliderGHorzTex);
            h3.AddFlexibleSpace();
            h3.AddElement(guiInputG);
            h3.AddSpace(10);

            v0.AddSpace(5);

            GUILayout h4 = v0.AddLayoutX();
            h4.AddSpace(10);
            h4.AddElement(guiLabelB);
            h4.AddFlexibleSpace();
            h4.AddElement(guiSliderBHorzTex);
            h4.AddFlexibleSpace();
            h4.AddElement(guiInputB);
            h4.AddSpace(10);

            v0.AddSpace(5);

            GUILayout h5 = v0.AddLayoutX();
            h5.AddSpace(10);
            h5.AddElement(guiLabelA);
            h5.AddFlexibleSpace();
            h5.AddElement(guiSliderAHorzTex);
            h5.AddFlexibleSpace();
            h5.AddElement(guiInputA);
            h5.AddSpace(10);

            v0.AddSpace(10);

            GUILayout h6 = v0.AddLayoutX();
            h6.AddFlexibleSpace();
            h6.AddElement(guiOK);
            h6.AddSpace(10);
            h6.AddElement(guiCancel);
            h6.AddFlexibleSpace();

            v0.AddSpace(5);

            GUIPanel overlay = GUI.AddPanel(-1);
            overlay.SetWidth(Width);
            overlay.SetHeight(Height);

            overlay.AddElement(guiSliderVert);
            overlay.AddElement(guiSliderRHorz);
            overlay.AddElement(guiSliderGHorz);
            overlay.AddElement(guiSliderBHorz);
            overlay.AddElement(guiSliderAHorz);
            overlay.AddElement(guiSlider2DHandle);

            colorBox = new ColorSlider2D(guiSlider2DTex, guiSlider2DHandle, ColorBoxWidth, ColorBoxHeight);
            sideSlider = new ColorSlider1DVert(guiSliderVertTex, guiSliderVert, SliderSideWidth, SliderSideHeight);

            sliderR = new ColorSlider1DHorz(guiSliderRHorzTex, guiSliderRHorz, SliderIndividualWidth, SliderIndividualHeight);
            sliderG = new ColorSlider1DHorz(guiSliderGHorzTex, guiSliderGHorz, SliderIndividualWidth, SliderIndividualHeight);
            sliderB = new ColorSlider1DHorz(guiSliderBHorzTex, guiSliderBHorz, SliderIndividualWidth, SliderIndividualHeight);
            sliderA = new ColorSlider1DHorz(guiSliderAHorzTex, guiSliderAHorz, SliderIndividualWidth, SliderIndividualHeight);

            colorBox.OnValueChanged += OnColorBoxValueChanged;

            Color startA = new Color(0, 0, 0, 1);
            Color stepA = new Color(1, 1, 1, 0);
            sliderA.UpdateTexture(startA, stepA, false);
            guiInputA.SetRange(0, 255);
            guiInputA.Value = 255;
            guiSliderAHorz.Percent = 1.0f;

            guiColor.Value = SelectedColor;
            UpdateInputBoxValues();
            Update2DSliderValues();
            Update1DSliderValues();
            UpdateSliderMode();
            Update2DSliderTextures();
            Update1DSliderTextures();
        }

        private void OnEditorUpdate()
        {
            Vector2I windowPos = ScreenToWindowPos(Input.PointerPosition);

            colorBox.UpdateInput(windowPos);
        }

        /// <summary>
        /// Fills a 2D area with colors using the provided starting point and gradients.
        /// </summary>
        /// <param name="width">Width of the area to fill.</param>
        /// <param name="height">Height of the area to fill.</param>
        /// <param name="colors">Array to contain the output. Must be of 
        ///                      <paramref name="width"/>*<paramref name="height"/> size.</param>
        /// <param name="start">Initial color in the top-left corner of the area.</param>
        /// <param name="rightGradient">Gradient towards which the colors increase to the right of the area.</param>
        /// <param name="downGradient">Gradient towards which the colors increase to the bottom of the area.</param>
        private static void FillArea(int width, int height, Color[] colors, Color start, Color rightGradient, Color downGradient)
        {
            Color rightDelta = new Color(0, 0, 0, 0);
            if (width > 1)
                rightDelta = rightGradient / (width - 1);

            Color downDelta = new Color(0, 0, 0, 0);
            if (height > 1)
                downDelta = downGradient / (height - 1);

            Color verticalColor = start;
            for (int y = 0; y < height; y++)
            {
                int rowIdx = (height - y - 1) * width;

                Color currentColor = verticalColor;
                for (int x = 0; x < width; x++)
                {
                    colors[rowIdx + x] = currentColor;
                    currentColor += rightDelta;
                }
                verticalColor += downDelta;
            }
        }

        /// <summary>
        /// Converts the currently selected color from HSV to RGB color space.
        /// </summary>
        void HSVToRGB()
        {
            Color hsv = new Color(colHue, colSaturation, colValue);
            Color rgb = Color.HSV2RGB(hsv);

            colRed = rgb.r;
            colGreen = rgb.g;
            colBlue = rgb.b;
        }

        /// <summary>
        /// Converts the currently selected color from RGB to HSV color space.
        /// </summary>
        void RGBToHSV()
        {
            Color rgb = new Color(colRed, colGreen, colBlue);
            Color hsv = Color.RGB2HSV(rgb);

            colHue = hsv.r;
            colSaturation = hsv.g;
            colValue = hsv.b;
        }

        /// <summary>
        /// Triggered when the user selects a new mode for the color box, changing the gamut of colors displayed in the box.
        /// </summary>
        void OnColorBoxModeChanged()
        {
            int maxModes = Enum.GetNames(colorBoxMode.GetType()).Length;
            colorBoxMode = (ColorBoxMode)(((int)colorBoxMode + 1) % maxModes);

            guiColorBoxBtn.SetContent(colorBoxMode.ToString());
            Update2DSliderTextures();
            Update2DSliderValues();
        }

        /// <summary>
        /// Triggered when the user selects a new mode for the side slider, changing the gamut of colors displayed in the 
        /// slider.
        /// </summary>
        void OnSliderModeChanged()
        {
            int maxModes = Enum.GetNames(sliderMode.GetType()).Length;
            sliderMode = (SliderMode)(((int)sliderMode + 1) % maxModes);

            UpdateSliderMode();

            guiColorModeBtn.SetContent(sliderMode.ToString());
            UpdateInputBoxValues();
            Update1DSliderTextures();
            Update1DSliderValues();
        }

        /// <summary>
        /// Triggered when the user selects a color in the color box.
        /// </summary>
        /// <param name="value">Location on the color box that was selected.</param>
        void OnColorBoxValueChanged(Vector2 value)
        {
            switch (colorBoxMode)
            {
                case ColorBoxMode.BG_R:
                    colBlue = value.x;
                    colGreen = value.y;
                    RGBToHSV();
                    break;
                case ColorBoxMode.BR_G:
                    colBlue = value.x;
                    colRed = value.y;
                    RGBToHSV();
                    break;
                case ColorBoxMode.RG_B:
                    colRed = value.x;
                    colGreen = value.y;
                    RGBToHSV();
                    break;
                case ColorBoxMode.SV_H:
                    colSaturation = value.x;
                    colValue = value.y;
                    HSVToRGB();
                    break;
                case ColorBoxMode.HV_S:
                    colHue = value.x;
                    colValue = value.y;
                    HSVToRGB();
                    break;
                case ColorBoxMode.HS_V:
                    colHue = value.x;
                    colSaturation = value.y;
                    HSVToRGB();
                    break;
            }

            guiColor.Value = SelectedColor;
            UpdateInputBoxValues();
            Update1DSliderTextures();
            Update1DSliderValues();
            UpdateSideSliderTexture();

            Vector2 xy;
            float z;

            GetColorBoxValues(out xy, out z);
            guiSliderVert.Percent = 1.0f - z;
        }

        /// <summary>
        /// Triggered when the user moves the side slider.
        /// </summary>
        /// <param name="percent">New value of the slider.</param>
        void OnSliderVertChanged(float percent)
        {
            percent = 1.0f - percent;

            switch (colorBoxMode)
            {
                case ColorBoxMode.BG_R:
                    colRed = percent;
                    RGBToHSV();
                    break;
                case ColorBoxMode.BR_G:
                    colGreen = percent;
                    RGBToHSV();
                    break;
                case ColorBoxMode.RG_B:
                    colBlue = percent;
                    RGBToHSV();
                    break;
                case ColorBoxMode.SV_H:
                    colHue = percent;
                    HSVToRGB();
                    break;
                case ColorBoxMode.HV_S:
                    colSaturation = percent;
                    HSVToRGB();
                    break;
                case ColorBoxMode.HS_V:
                    colValue = percent;
                    HSVToRGB();
                    break;
            }

            guiColor.Value = SelectedColor;
            UpdateInputBoxValues();
            Update1DSliderTextures();
            Update1DSliderValues();
        }

        /// <summary>
        /// Triggered when the user moves the horizontal Red/Hue slider.
        /// </summary>
        /// <param name="percent">New value of the slider.</param>
        void OnSliderRHorzChanged(float percent)
        {
            bool isHSV = sliderMode == SliderMode.HSV;
            if (isHSV)
            {
                colHue = percent;
                HSVToRGB();
            }
            else
            {
                colRed = percent;
                RGBToHSV();
            }

            guiColor.Value = SelectedColor;
            UpdateInputBoxValues();
            Update2DSliderTextures();
            Update2DSliderValues();
        }

        /// <summary>
        /// Triggered when the user moves the horizontal Green/Saturation slider.
        /// </summary>
        /// <param name="percent">New value of the slider.</param>
        void OnSliderGHorzChanged(float percent)
        {
            bool isHSV = sliderMode == SliderMode.HSV;
            if (isHSV)
            {
                colSaturation = percent;
                HSVToRGB();
            }
            else
            {
                colGreen = percent;
                RGBToHSV();
            }

            guiColor.Value = SelectedColor;
            UpdateInputBoxValues();
            Update2DSliderTextures();
            Update2DSliderValues();
        }

        /// <summary>
        /// Triggered when the user moves the horizontal Blue/Value slider.
        /// </summary>
        /// <param name="percent">New value of the slider.</param>
        void OnSliderBHorzChanged(float percent)
        {
            bool isHSV = sliderMode == SliderMode.HSV;
            if (isHSV)
            {
                colValue = percent;
                HSVToRGB();
            }
            else
            {
                colBlue = percent;
                RGBToHSV();
            }

            guiColor.Value = SelectedColor;
            UpdateInputBoxValues();
            Update2DSliderTextures();
            Update2DSliderValues();
        }

        /// <summary>
        /// Triggered when the user moves the horizontal alpha slider.
        /// </summary>
        /// <param name="percent">New value of the slider.</param>
        void OnSliderAHorzChanged(float percent)
        {
            colAlpha = percent;

            guiColor.Value = SelectedColor;
            guiInputA.Value = MathEx.RoundToInt(colAlpha * 255.0f);
        }

        /// <summary>
        /// Triggered when the user inputs new value in the Red/Hue input box.
        /// </summary>
        /// <param name="value">New value in the input box.</param>
        void OnInputRChanged(int value)
        {
            bool isHSV = sliderMode == SliderMode.HSV;
            if (isHSV)
            {
                colHue = value/359.0f;
                HSVToRGB();
            }
            else
            {
                colRed = value/255.0f;
                RGBToHSV();
            }

            guiColor.Value = SelectedColor;
            Update1DSliderValues();
            Update2DSliderTextures();
            Update2DSliderValues();
        }

        /// <summary>
        /// Triggered when the user inputs new value in the Green/Saturation input box.
        /// </summary>
        /// <param name="value">New value in the input box.</param>
        void OnInputGChanged(int value)
        {
            bool isHSV = sliderMode == SliderMode.HSV;
            if (isHSV)
            {
                colSaturation = value / 255.0f;
                HSVToRGB();
            }
            else
            {
                colGreen = value / 255.0f;
                RGBToHSV();
            }

            guiColor.Value = SelectedColor;
            Update1DSliderValues();
            Update2DSliderTextures();
            Update2DSliderValues();
        }

        /// <summary>
        /// Triggered when the user inputs new value in the Blue/Value input box.
        /// </summary>
        /// <param name="value">New value in the input box.</param>
        void OnInputBChanged(int value)
        {
            bool isHSV = sliderMode == SliderMode.HSV;
            if (isHSV)
            {
                colValue = value / 255.0f;
                HSVToRGB();
            }
            else
            {
                colBlue = value / 255.0f;
                RGBToHSV();
            }

            guiColor.Value = SelectedColor;
            Update1DSliderValues();
            Update2DSliderTextures();
            Update2DSliderValues();
        }

        /// <summary>
        /// Triggered when the user inputs new value in the alpha input box.
        /// </summary>
        /// <param name="value">New value in the input box.</param>
        void OnInputAChanged(int value)
        {
            colAlpha = value/255.0f;

            guiColor.Value = SelectedColor;
            guiSliderAHorz.Percent = colAlpha;
        }

        /// <summary>
        /// Triggered when the user selects a color and closes the dialog.
        /// </summary>
        void OnOK()
        {
            if (closedCallback != null)
                closedCallback(true, SelectedColor);

            Close();
        }

        /// <summary>
        /// Triggered when the user cancels color selection and closes the dialog.
        /// </summary>
        void OnCancel()
        {
            if (closedCallback != null)
                closedCallback(false, SelectedColor);

            Close();
        }

        /// <summary>
        /// Updates Red/Green/Blue or Hue/Saturation/Value labels and input box ranges depending on currently active mode.
        /// </summary>
        void UpdateSliderMode()
        {
            if (sliderMode == SliderMode.RGB)
            {
                guiLabelR.SetContent(new LocEdString("R"));
                guiLabelG.SetContent(new LocEdString("G"));
                guiLabelB.SetContent(new LocEdString("B"));

                guiInputR.SetRange(0, 255);
                guiInputG.SetRange(0, 255);
                guiInputB.SetRange(0, 255);
            }
            else
            {
                guiLabelR.SetContent(new LocEdString("H"));
                guiLabelG.SetContent(new LocEdString("S"));
                guiLabelB.SetContent(new LocEdString("V"));

                guiInputR.SetRange(0, 359);
                guiInputG.SetRange(0, 255);
                guiInputB.SetRange(0, 255);
            }
        }

        /// <summary>
        /// Updates Red/Green/Blue or Hue/Saturation/Value input boxes with currently selected color.
        /// </summary>
        void UpdateInputBoxValues()
        {
            bool isHSV = sliderMode == SliderMode.HSV;
            if (isHSV)
            {
                guiInputR.Value = MathEx.RoundToInt(colHue * 359.0f);
                guiInputG.Value = MathEx.RoundToInt(colSaturation * 255.0f);
                guiInputB.Value = MathEx.RoundToInt(colValue * 255.0f);
            }
            else
            {
                guiInputR.Value = MathEx.RoundToInt(colRed * 255.0f);
                guiInputG.Value = MathEx.RoundToInt(colGreen * 255.0f);
                guiInputB.Value = MathEx.RoundToInt(colBlue * 255.0f);
            }
        }

        /// <summary>
        /// Updates Red/Green/Blue or Hue/Saturation/Value sliders with currently selected color.
        /// </summary>
        void Update1DSliderValues()
        {
            bool isHSV = sliderMode == SliderMode.HSV;
            if (isHSV)
            {
                guiSliderRHorz.Percent = colHue;
                guiSliderGHorz.Percent = colSaturation;
                guiSliderBHorz.Percent = colValue;
            }
            else
            {
                guiSliderRHorz.Percent = colRed;
                guiSliderGHorz.Percent = colGreen;
                guiSliderBHorz.Percent = colBlue;
            }
        }

        /// <summary>
        /// Returns the current color in the form of color box and side slider coordinates.
        /// </summary>
        /// <param name="xy">Coordinates on the color box the current color is located on.</param>
        /// <param name="z">Coordinates on the side slider the current color is located on.</param>
        void GetColorBoxValues(out Vector2 xy, out float z)
        {
            xy = Vector2.Zero;
            z = 0.0f;

            switch (colorBoxMode)
            {
                case ColorBoxMode.BG_R:
                    xy.x = colBlue;
                    xy.y = colGreen;
                    z = colRed;
                    break;
                case ColorBoxMode.BR_G:
                    xy.x = colBlue;
                    xy.y = colRed;
                    z = colGreen;
                    break;
                case ColorBoxMode.RG_B:
                    xy.x = colRed;
                    xy.y = colGreen;
                    z = colBlue;
                    break;
                case ColorBoxMode.SV_H:
                    xy.x = colSaturation;
                    xy.y = colValue;
                    z = colHue;
                    break;
                case ColorBoxMode.HV_S:
                    xy.x = colHue;
                    xy.y = colValue;
                    z = colSaturation;
                    break;
                case ColorBoxMode.HS_V:
                    xy.x = colHue;
                    xy.y = colSaturation;
                    z = colValue;
                    break;
            }
        }

        /// <summary>
        /// Updates values of the color box and side slider according to the current color.
        /// </summary>
        void Update2DSliderValues()
        {
            Vector2 xy = Vector2.Zero;
            float z = 0.0f;

            GetColorBoxValues(out xy, out z);

            colorBox.SetValue(xy);
            guiSliderVert.Percent = z;
        }

        /// <summary>
        /// Generates textures to display for all horizontal (RGB/HSV) sliders depending on active slider mode.
        /// </summary>
        void Update1DSliderTextures()
        {
            bool isHSV = sliderMode == SliderMode.HSV;

            if (isHSV)
            {
                Color startH = new Color(0, 1, 1);
                Color stepH = new Color(1, 0, 0, 0);

                sliderR.UpdateTexture(startH, stepH, true);

                Color startS = new Color(colHue, 0, MathEx.Max(colValue, 0.2f));
                Color stepS = new Color(0, 1, 0, 0);

                sliderG.UpdateTexture(startS, stepS, true);

                Color startV = new Color(colHue, colSaturation, 0);
                Color stepV = new Color(0, 0, 1, 0);

                sliderB.UpdateTexture(startV, stepV, true);
            }
            else
            {
                Color startR = new Color(0, colGreen, colBlue);
                Color stepR = new Color(1, 0, 0, 0);

                sliderR.UpdateTexture(startR, stepR, false);

                Color startG = new Color(colRed, 0, colBlue);
                Color stepG = new Color(0, 1, 0, 0);

                sliderG.UpdateTexture(startG, stepG, false);

                Color startB = new Color(colRed, colGreen, 0);
                Color stepB = new Color(0, 0, 1, 0);

                sliderB.UpdateTexture(startB, stepB, false);
            }
        }

        /// <summary>
        /// Generates a texture for the side slider depending on active color box mode.
        /// </summary>
        void UpdateSideSliderTexture()
        {
            switch (colorBoxMode)
            {
                case ColorBoxMode.BG_R:
                    sideSlider.UpdateTexture(new Color(0, colGreen, colBlue, 1), new Color(1, 0, 0, 0), false);
                    break;
                case ColorBoxMode.BR_G:
                    sideSlider.UpdateTexture(new Color(colRed, 0, colBlue, 1), new Color(0, 1, 0, 0), false);
                    break;
                case ColorBoxMode.RG_B:
                    sideSlider.UpdateTexture(new Color(colRed, colGreen, 0, 1), new Color(0, 0, 1, 0), false);
                    break;
                case ColorBoxMode.SV_H:
                    sideSlider.UpdateTexture(new Color(0, 1, 1, 1), new Color(1, 0, 0, 0), true);
                    break;
                case ColorBoxMode.HV_S:
                    sideSlider.UpdateTexture(new Color(colHue, 0, MathEx.Max(colValue, 0.2f), 1), new Color(0, 1, 0, 0), true);
                    break;
                case ColorBoxMode.HS_V:
                    sideSlider.UpdateTexture(new Color(colHue, colSaturation, 0, 1), new Color(0, 0, 1, 0), true);
                    break;
            }
        }

        /// <summary>
        /// Generates textures for the color box and the side slider depending on active color box mode.
        /// </summary>
        void Update2DSliderTextures()
        {
            UpdateSideSliderTexture();

            float[] valueLookup = new float[] { colRed, colGreen, colBlue, colHue, colSaturation, colValue };
            colorBox.UpdateTexture(colorBoxMode, valueLookup[(int)colorBoxMode]);
        }

        /// <summary>
        /// Manages GUI for a 1D horizontal slider (used RGB/HSV display).
        /// </summary>
        public class ColorSlider1DHorz
        {
            private const int SLIDER_X_OFFSET = 3;
            private const int SLIDER_Y_OFFSET = 5;

            private int width, height;
            private Texture2D texture;
            private SpriteTexture spriteTexture;

            private GUITexture guiTexture;
            private GUISliderH guiSlider;

            /// <summary>
            /// Creates a new horizontal slider.
            /// </summary>
            /// <param name="guiTexture">GUI element to display the slider color range on.</param>
            /// <param name="guiSlider">Slider rendered on top of the texture that may be moved by the user to select a 
            ///                         color.</param>
            /// <param name="width">Width of the slider in pixels.</param>
            /// <param name="height">Height of the slider in pixels.</param>
            public ColorSlider1DHorz(GUITexture guiTexture, GUISliderH guiSlider, int width, int height)
            {
                this.width = width;
                this.height = height;
                this.guiTexture = guiTexture;
                this.guiSlider = guiSlider;

                texture = new Texture2D(width, height);
                spriteTexture = new SpriteTexture(texture);
            }

            /// <summary>
            /// Updates the displayed texture with specified color information.
            /// </summary>
            /// <param name="start">Initial color on the left of the slider.</param>
            /// <param name="step">Final color to the right of the slider.</param>
            /// <param name="isHSV">Determines are the provided colors in RGB or HSV space.</param>
            public void UpdateTexture(Color start, Color step, bool isHSV)
            {
                Color[] colors = new Color[width * height];
                FillArea(width, height, colors, start, step, new Color(0, 0, 0, 0));

                if (isHSV)
                {
                    for (int i = 0; i < colors.Length; i++)
                        colors[i] = Color.HSV2RGB(colors[i]);
                }

                texture.SetPixels(colors);
                guiTexture.SetTexture(spriteTexture);

                Rect2I sliderBounds = guiTexture.Bounds;
                sliderBounds.x -= SLIDER_X_OFFSET;
                sliderBounds.width += SLIDER_X_OFFSET * 2;
                sliderBounds.y -= SLIDER_Y_OFFSET;
                sliderBounds.height += SLIDER_Y_OFFSET;

                guiSlider.Bounds = sliderBounds;
            }
        }

        /// <summary>
        /// Manages GUI for a 1D vertical slider (side slider along with the color box).
        /// </summary>
        public class ColorSlider1DVert
        {
            private const int SLIDER_X_OFFSET = 5;
            private const int SLIDER_Y_OFFSET = 3;

            private int width, height;
            private Texture2D texture;
            private SpriteTexture spriteTexture;

            private GUITexture guiTexture;
            private GUISliderV guiSlider;

            /// <summary>
            /// Creates a new vertical slider.
            /// </summary>
            /// <param name="guiTexture">GUI element to display the slider color range on.</param>
            /// <param name="guiSlider">Slider rendered on top of the texture that may be moved by the user to select a 
            ///                         color.</param>
            /// <param name="width">Width of the slider in pixels.</param>
            /// <param name="height">Height of the slider in pixels.</param>
            public ColorSlider1DVert(GUITexture guiTexture, GUISliderV guiSlider, int width, int height)
            {
                this.width = width;
                this.height = height;
                this.guiTexture = guiTexture;
                this.guiSlider = guiSlider;

                texture = new Texture2D(width, height);
                spriteTexture = new SpriteTexture(texture);
            }

            /// <summary>
            /// Updates the displayed texture with specified color information.
            /// </summary>
            /// <param name="start">Initial color on the top of the slider.</param>
            /// <param name="step">Final color to the bottom of the slider.</param>
            /// <param name="isHSV">Determines are the provided colors in RGB or HSV space.</param>
            public void UpdateTexture(Color start, Color step, bool isHSV)
            {
                Color[] colors = new Color[width * height];
                FillArea(width, height, colors, start, new Color(0, 0, 0, 0), step);

                if (isHSV)
                {
                    for (int i = 0; i < colors.Length; i++)
                        colors[i] = Color.HSV2RGB(colors[i]);
                }

                texture.SetPixels(colors);
                guiTexture.SetTexture(spriteTexture);

                Rect2I sliderBounds = guiTexture.Bounds;
                sliderBounds.x -= SLIDER_X_OFFSET;
                sliderBounds.width += SLIDER_X_OFFSET;
                sliderBounds.y -= SLIDER_Y_OFFSET;
                sliderBounds.height += SLIDER_Y_OFFSET * 2;

                guiSlider.Bounds = sliderBounds;
            }
        }

        /// <summary>
        /// Manages GUI for a 2D color box, as well as manually handling color box input. Color box serves as a 2D sliders 
        /// as you can portion of it to select a color.
        /// </summary>
        public class ColorSlider2D
        {
            private int width, height;
            private Texture2D texture;
            private SpriteTexture spriteTexture;

            private GUITexture guiTexture;
            private GUITexture guiSliderHandle;

            private Vector2 oldValue = new Vector2(-1, -1);

            public delegate void OnValueChangedDelegate(Vector2 value);
            public event OnValueChangedDelegate OnValueChanged;

            /// <summary>
            /// Creates a new color box.
            /// </summary>
            /// <param name="guiTexture">GUI element to display the 2D color range on.</param>
            /// <param name="guiSliderHandle">Texture to be used for displaying the position of the currently selected 
            ///                               color.</param>
            /// <param name="width">Width of the slider in pixels.</param>
            /// <param name="height">Height of the slider in pixels.</param>
            public ColorSlider2D(GUITexture guiTexture, GUITexture guiSliderHandle, int width, int height)
            {
                this.width = width;
                this.height = height;

                this.guiTexture = guiTexture;
                this.guiSliderHandle = guiSliderHandle;

                texture = new Texture2D(width, height);
                spriteTexture = new SpriteTexture(texture);
            }

            /// <summary>
            /// Updates the texture displayed on the color box.
            /// </summary>
            /// <param name="mode">Mode determining the color gamut shown in the color box.</param>
            /// <param name="value">Value of the third component (normally retrieved from the separate side slider).</param>
            public void UpdateTexture(ColorBoxMode mode, float value)
            {
                Color[] colors = new Color[width * height];

                switch (mode)
                {
                    case ColorBoxMode.BG_R:
                        FillArea(width, height, colors, new Color(value, 0, 0, 1), new Color(0, 0, 1, 0), new Color(0, 1, 0, 0));
                        break;
                    case ColorBoxMode.BR_G:
                        FillArea(width, height, colors, new Color(0, value, 0, 1), new Color(0, 0, 1, 0), new Color(1, 0, 0, 0));
                        break;
                    case ColorBoxMode.RG_B:
                        FillArea(width, height, colors, new Color(0, 0, value, 1), new Color(1, 0, 0, 0), new Color(0, 1, 0, 0));
                        break;
                    case ColorBoxMode.SV_H:
                        FillArea(width, height, colors, new Color(value, 0, 0, 1), new Color(0, 1, 0, 0), new Color(0, 0, 1, 0));
                        for (int i = 0; i < colors.Length; i++)
                            colors[i] = Color.HSV2RGB(colors[i]);
                        break;
                    case ColorBoxMode.HV_S:
                        FillArea(width, height, colors, new Color(0, value, 0, 1), new Color(1, 0, 0, 0), new Color(0, 0, 1, 0));
                        for (int i = 0; i < colors.Length; i++)
                            colors[i] = Color.HSV2RGB(colors[i]);
                        break;
                    case ColorBoxMode.HS_V:
                        FillArea(width, height, colors, new Color(0, 0, value, 1), new Color(1, 0, 0, 0), new Color(0, 1, 0, 0));
                        for (int i = 0; i < colors.Length; i++)
                            colors[i] = Color.HSV2RGB(colors[i]);
                        break;
                }

                texture.SetPixels(colors);
                guiTexture.SetTexture(spriteTexture);
            }

            /// <summary>
            /// Handles input over the color box, moving the handle as needed.
            /// </summary>
            /// <param name="windowPos">Position of the pointer relative to the color picker window.</param>
            public void UpdateInput(Vector2I windowPos)
            {
                if (Input.IsPointerButtonHeld(PointerButton.Left))
                {
                    Rect2I bounds = guiTexture.Bounds;

                    if (bounds.Contains(windowPos))
                    {
                        Vector2 newValue = Vector2.Zero;
                        newValue.x = (windowPos.x - bounds.x) / (float)bounds.width;
                        newValue.y = 1.0f - (windowPos.y - bounds.y) / (float)bounds.height;

                        SetValue(newValue);
                    }
                }
            }

            /// <summary>
            /// Moves the handle to a specific location on the color box and selects that color.
            /// </summary>
            /// <param name="value">Coordinates relative to the color box.</param>
            public void SetValue(Vector2 value)
            {
                Vector2 pos = value;
                pos.y = 1.0f - pos.y;

                if (oldValue == value)
                    return;

                Rect2I handleBounds = guiSliderHandle.Bounds;
                Rect2I boxBounds = guiTexture.Bounds;

                handleBounds.x = boxBounds.x + MathEx.RoundToInt(pos.x * boxBounds.width) - handleBounds.width / 2;
                handleBounds.y = boxBounds.y + MathEx.RoundToInt(pos.y * boxBounds.height) - handleBounds.height / 2;

                guiSliderHandle.Bounds = handleBounds;
                oldValue = value;

                if (OnValueChanged != null)
                    OnValueChanged(value);
            }
        }
    }

    /** @} */
}
