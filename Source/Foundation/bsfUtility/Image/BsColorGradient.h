//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Image/BsColor.h"
#include "Allocators/BsPoolAlloc.h"
#include "Utility/BsBitwise.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Helper used for implementing specializations of TColorGradient. */
	namespace impl
	{
		template<class COLOR>
		class TGradientHelper
		{ };

		template<>
		class TGradientHelper<RGBA>
		{
		public:
			using INNER_TIME_TYPE = uint32_t;

			static uint32_t getInternalTime(float t) { return Bitwise::unormToUint<16>(t); }
			static float fromInternalTime(uint16_t t) { return Bitwise::uintToUnorm<16>(t); }
			static RGBA toInternalColor(const Color& color) { return color.getAsRGBA(); }
			static Color fromInternalColor(RGBA color) { return Color::fromRGBA(color); }
			static uint32_t invLerp(uint32_t from, uint32_t to, uint32_t val) { return Bitwise::invLerpWord(from, to, val) >> 8; }
			static uint32_t toLerpFactor(float factor) { return Bitwise::unormToUint<8>(factor); }
		};

		template<>
		class TGradientHelper<Color>
		{
		public:
			using INNER_TIME_TYPE = float;

			static float getInternalTime(float t) { return t; }
			static float fromInternalTime(float t) { return t; }
			static Color toInternalColor(const Color& color) { return color; }
			static Color fromInternalColor(const Color& color) { return color; }
			static float invLerp(float from, float to, float val) { return Math::invLerp(val, from, to); }
			static float toLerpFactor(float factor) { return factor; }
		};
	}

	/** @} */

	/** @addtogroup Image
	 *  @{
	 */

	/** Single key in a ColorGradient. */
	struct BS_SCRIPT_EXPORT(m:Image,pl:true) ColorGradientKey
	{
		ColorGradientKey() = default;
		ColorGradientKey(const Color& color, float time)
			:color(color), time(time)
		{ }

		Color color;
		float time = 0.0f;
	};

	/** Common templated class for different color gradient implementations. */
	template<class COLOR, class TIME>
	class BS_UTILITY_EXPORT TColorGradient
	{
	public:
		using ColorType = COLOR;
		using TimeType = TIME;

		static constexpr UINT32 MAX_KEYS = 8;

		BS_SCRIPT_EXPORT()
		TColorGradient() = default;

		BS_SCRIPT_EXPORT()
		TColorGradient(const Color& color);

		BS_SCRIPT_EXPORT()
		TColorGradient(const Vector<ColorGradientKey>& keys);

		/** Evaluates a color at the specified @p t. */
		COLOR evaluate(float t) const;

		/** Keys that control the gradient, sorted by time from first to last. Key times should be in range [0, 1]. */
		BS_SCRIPT_EXPORT()
		void setKeys(const Vector<ColorGradientKey>& keys, float duration = 1.0f);

		/** @copydoc setKeys */
		BS_SCRIPT_EXPORT()
		Vector<ColorGradientKey> getKeys() const;

		/** Returns the number of color keys in the gradient. */
		BS_SCRIPT_EXPORT(pr:getter,n:NumKeys)
		UINT32 getNumKeys() const { return mNumKeys; }

		/** Returns the color key at the specified index. If out of range an empty key is returned. */
		BS_SCRIPT_EXPORT()
		ColorGradientKey getKey(UINT32 idx) const;

		/** Specify a "gradient" that represents a single color value. */
		BS_SCRIPT_EXPORT()
		void setConstant(const Color& color);

		/**
		 * Returns the duration over which the gradient values are interpolated over. Corresponds to the time value of the
		 * final keyframe.
		 */
		float getDuration() const { return mDuration; }

		/** Returns the time of the first and last keyframe in the gradient. */
		std::pair<float, float> getTimeRange() const;

		bool operator== (const TColorGradient& rhs) const;
		bool operator!= (const TColorGradient& rhs) const { return !operator==(rhs); }
	protected:
		COLOR mColors[MAX_KEYS];
		TIME mTimes[MAX_KEYS];
		uint32_t mNumKeys = 0;
		float mDuration = 0.0f;
	};

	/**
	 * Represents a range of color values over some parameters, similar to a curve. Internally represented as a set of
	 * keys that get interpolated between. Stores colors as 32-bit integers, and is therefor unable to represent
	 * a color range outside of [0, 1] - see ColorGradientHDR for an alternative.
	 */
	class BS_UTILITY_EXPORT BS_SCRIPT_EXPORT(m:Image) ColorGradient : public TColorGradient<RGBA, uint16_t>
	{
		using TColorGradient::TColorGradient;

		friend struct RTTIPlainType<ColorGradient>;
	};

	/**
	 * Represents a range of color values over some parameters, similar to a curve. Internally represented as a set of
	 * keys that get interpolated between. Capable of representing HDR colors, unlike the normal ColorGradient.
	 */
	class BS_UTILITY_EXPORT BS_SCRIPT_EXPORT(m:Image) ColorGradientHDR : public TColorGradient<Color, float>
	{
		using TColorGradient::TColorGradient;

		friend struct RTTIPlainType<ColorGradientHDR>;
	};

	/* @} */

	IMPLEMENT_GLOBAL_POOL(ColorGradient, 32)
	IMPLEMENT_GLOBAL_POOL(ColorGradientHDR, 32)
}
