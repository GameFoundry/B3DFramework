//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Image/B3DColor.h"
#include "Allocators/B3DPoolAlloc.h"
#include "Utility/B3DBitwise.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Image-Internal
	 *  @{
	 */

	/** Helper used for implementing specializations of TColorGradient. */
	template <class COLOR>
	class TGradientHelper
	{};

	template <>
	class TGradientHelper<RGBA>
	{
	public:
		using INNER_TIME_TYPE = uint32_t;

		static uint32_t GetInternalTime(float t) { return Bitwise::UnormToUint<16>(t); }

		static float FromInternalTime(uint16_t t) { return Bitwise::UintToUnorm<16>(t); }

		static RGBA ToInternalColor(const Color& color) { return color.GetAsRgba(); }

		static Color FromInternalColor(RGBA color) { return Color::FromRgba(color); }

		static uint32_t InvLerp(uint32_t from, uint32_t to, uint32_t val) { return Bitwise::InvLerpWord(from, to, val) >> 8; }

		static uint32_t ToLerpFactor(float factor) { return Bitwise::UnormToUint<8>(factor); }
	};

	template <>
	class TGradientHelper<Color>
	{
	public:
		using INNER_TIME_TYPE = float;

		static float GetInternalTime(float t) { return t; }

		static float FromInternalTime(float t) { return t; }

		static Color ToInternalColor(const Color& color) { return color; }

		static Color FromInternalColor(const Color& color) { return color; }

		static float InvLerp(float from, float to, float val) { return Math::InvLerp(val, from, to); }

		static float ToLerpFactor(float factor) { return factor; }
	};

	/** @} */

	/** @addtogroup Image
	 *  @{
	 */

	/** Single key in a ColorGradient. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Image), ExportAsStruct(true)) ColorGradientKey
	{
		ColorGradientKey() = default;

		ColorGradientKey(const Color& color, float time)
			: Color(color), Time(time)
		{}

		Color Color;
		float Time = 0.0f;
	};

	/** @} */

	/** @addtogroup Image-Internal
	 *  @{
	 */

	/** Common templated class for different color gradient implementations. */
	template <class COLOR, class TIME>
	class B3D_EXPORT TColorGradient
	{
	public:
		using ColorType = COLOR;
		using TimeType = TIME;

		static constexpr u32 kMaxKeys = 8;

		B3D_SCRIPT_EXPORT()
		TColorGradient() = default;

		B3D_SCRIPT_EXPORT()
		TColorGradient(const Color& color);

		B3D_SCRIPT_EXPORT()
		TColorGradient(const Vector<ColorGradientKey>& keys);

		/** Evaluates a color at the specified @p t. */
		COLOR Evaluate(float t) const;

		/** Keys that control the gradient, sorted by time from first to last. Key times should be in range [0, 1]. */
		B3D_SCRIPT_EXPORT()
		void SetKeys(const Vector<ColorGradientKey>& keys, float duration = 1.0f);

		/** @copydoc SetKeys */
		B3D_SCRIPT_EXPORT()
		Vector<ColorGradientKey> GetKeys() const;

		/** Returns the number of color keys in the gradient. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(NumKeys))

		u32 GetNumKeys() const { return mNumKeys; }

		/** Returns the color key at the specified index. If out of range an empty key is returned. */
		B3D_SCRIPT_EXPORT()
		ColorGradientKey GetKey(u32 index) const;

		/** Specify a "gradient" that represents a single color value. */
		B3D_SCRIPT_EXPORT()
		void SetConstant(const Color& color);

		/**
		 * Returns the duration over which the gradient values are interpolated over. Corresponds to the time value of the
		 * final keyframe.
		 */
		float GetDuration() const { return mDuration; }

		/** Returns the time of the first and last keyframe in the gradient. */
		std::pair<float, float> GetTimeRange() const;

		bool operator==(const TColorGradient& rhs) const;

		bool operator!=(const TColorGradient& rhs) const { return !operator==(rhs); }

	protected:
		COLOR mColors[kMaxKeys];
		TIME mTimes[kMaxKeys];
		uint32_t mNumKeys = 0;
		float mDuration = 0.0f;
	};

	/** @} */

	/** @addtogroup Image-Internal
	 *  @{
	 */

	/**
	 * Represents a range of color values over some parameters, similar to a curve. Internally represented as a set of
	 * keys that get interpolated between. Stores colors as 32-bit integers, and is therefor unable to represent
	 * a color range outside of [0, 1] - see ColorGradientHDR for an alternative.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Image)) ColorGradient : public TColorGradient<RGBA, uint16_t>, public IScriptExportable
	{
		using TColorGradient::TColorGradient;

		friend struct RTTIPlainType<ColorGradient>;
	};

	/**
	 * Represents a range of color values over some parameters, similar to a curve. Internally represented as a set of
	 * keys that get interpolated between. Capable of representing HDR colors, unlike the normal ColorGradient.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Image)) ColorGradientHDR : public TColorGradient<Color, float>, public IScriptExportable
	{
		using TColorGradient::TColorGradient;

		friend struct RTTIPlainType<ColorGradientHDR>;
	};

	/* @} */

	B3D_IMPLEMENT_GLOBAL_POOL(ColorGradient, 32)
	B3D_IMPLEMENT_GLOBAL_POOL(ColorGradientHDR, 32)
} // namespace b3d
