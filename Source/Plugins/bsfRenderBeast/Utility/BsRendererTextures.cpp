//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererTextures.h"
#include "Math/BsVector2.h"
#include "Math/BsVector3.h"
#include "Image/BsColor.h"
#include "Math/BsMath.h"
#include "Image/BsTexture.h"
#include "Image/BsPixelData.h"
#include "Renderer/BsIBLUtility.h"
#include "Image/BsColorGradient.h"

namespace bs {
namespace ct {

SPtr<ct::Texture> generate4x4RandomizationTexture()
{
	u32 mapping[16] = { 13, 5, 1, 9, 14, 3, 7, 11, 15, 2, 6, 12, 4, 8, 0, 10 };
	Vector2 bases[16];
	for(u32 i = 0; i < 16; ++i)
	{
		float angle = (mapping[i] / 16.0f) * Math::kPi;
		bases[i].X = cos(angle);
		bases[i].Y = sin(angle);
	}

	SPtr<PixelData> pixelData = PixelData::Create(4, 4, 1, PF_RG8);
	for(u32 y = 0; y < 4; ++y)
		for(u32 x = 0; x < 4; ++x)
		{
			u32 base = (y * 4) + x;

			Color color;
			color.R = bases[base].X * 0.5f + 0.5f;
			color.G = bases[base].Y * 0.5f + 0.5f;

			pixelData->SetColorAt(color, x, y);
		}

	return ct::Texture::Create(pixelData);
}

// Reverse bits functions used for Hammersley sequence
float reverseBits(u32 bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);

	return (float)(double(bits) / (double)0x100000000LL);
}

void hammersleySequence(u32 i, u32 count, float& e0, float& e1)
{
	e0 = i / (float)count;
	e1 = reverseBits(i);
}

Vector3 sphericalToCartesian(float cosTheta, float sinTheta, float phi)
{
	Vector3 output;
	output.X = sinTheta * cos(phi);
	output.Y = sinTheta * sin(phi);
	output.Z = cosTheta;

	return output;
}

// Generates an angle in spherical coordinates, importance sampled for the specified roughness based on some uniformly
// distributed random variables in range [0, 1].
void importanceSampleGGX(float e0, float e1, float roughness4, float& cosTheta, float& phi)
{
	// See GGXImportanceSample.nb for derivation (essentially, take base GGX, normalize it, generate PDF, split PDF into
	// marginal probability for theta and conditional probability for phi. Plug those into the CDF, invert it.)
	cosTheta = sqrt((1.0f - e0) / (1.0f + (roughness4 - 1.0f) * e0));
	phi = 2.0f * Math::kPi * e1;
}

float calcMicrofacetShadowingSmithGGX(float roughness4, float NoV, float NoL)
{
	// Note: See lighting shader for derivation. Includes microfacet BRDF divisor.
	float g1V = NoV + sqrt(NoV * (NoV - NoV * roughness4) + roughness4);
	float g1L = NoL + sqrt(NoL * (NoL - NoL * roughness4) + roughness4);
	return 1.0f / (g1V * g1L);
}

SPtr<ct::Texture> generatePreintegratedEnvBRDF()
{
	TEXTURE_DESC desc;
	desc.Type = TEX_TYPE_2D;
	desc.Format = PF_RG16F;
	desc.Width = 128;
	desc.Height = 32;

	SPtr<ct::Texture> texture = ct::Texture::Create(desc);
	PixelData pixelData = texture->Lock(GBL_WRITE_ONLY_DISCARD);

	for(u32 y = 0; y < desc.Height; y++)
	{
		float roughness = (float)(y + 0.5f) / desc.Height;
		float m = roughness * roughness;
		float m2 = m * m;

		for(u32 x = 0; x < desc.Width; x++)
		{
			float NoV = (float)(x + 0.5f) / desc.Width;

			Vector3 V;
			V.X = sqrt(1.0f - NoV * NoV); // sine
			V.Y = 0.0f;
			V.Z = NoV;

			// These are the two integrals resulting from the second part of the split-sum approximation. Described in
			// Epic's 2013 paper:
			//    http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
			float scale = 0.0f;
			float offset = 0.0f;

			// We use the same importance sampling function we use for reflection cube importance sampling, only we
			// sample G and F, instead of D factors of the microfactet BRDF. See GGXImportanceSample.nb for derivation.
			constexpr u32 NumSamples = 128;
			for(u32 i = 0; i < NumSamples; i++)
			{
				float e0, e1;
				hammersleySequence(i, NumSamples, e0, e1);

				float cosTheta, phi;
				importanceSampleGGX(e0, e1, m2, cosTheta, phi);

				float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
				Vector3 H = sphericalToCartesian(cosTheta, sinTheta, phi);
				Vector3 L = 2.0f * Vector3::Dot(V, H) * H - V;

				float VoH = std::max(Vector3::Dot(V, H), 0.0f);
				float NoL = std::max(L.Z, 0.0f); // N assumed (0, 0, 1)
				float NoH = std::max(H.Z, 0.0f); // N assumed (0, 0, 1)

				// Set second part of the split sum integral is split into two parts:
				//   F0*I[G * (1 - (1 - v.h)^5) * cos(theta)] + I[G * (1 - v.h)^5 * cos(theta)] (F0 * scale + bias)

				// We calculate the fresnel scale (1 - (1 - v.h)^5) and bias ((1 - v.h)^5) parts
				float fc = pow(1.0f - VoH, 5.0f);
				float fresnelScale = 1.0f - fc;
				float fresnelOffset = fc;

				// We calculate the G part
				float G = calcMicrofacetShadowingSmithGGX(m2, NoV, NoL);

				// When we factor out G and F, then divide D by PDF, this is what's left
				// Note: This is based on PDF: D * NoH / (4 * VoH). (4 * VoH) factor comes from the Jacobian of the
				// transformation from half vector to light vector
				float pdfFactor = 4.0f * VoH / NoH;

				if(NoL > 0.0f)
				{
					scale += NoL * pdfFactor * G * fresnelScale;
					offset += NoL * pdfFactor * G * fresnelOffset;
				}
			}

			scale /= NumSamples;
			offset /= NumSamples;

			Color color;
			color.R = Math::Clamp01(scale);
			color.G = Math::Clamp01(offset);

			pixelData.SetColorAt(color, x, y);
		}
	}

	texture->Unlock();

	return texture;
}

SPtr<ct::Texture> generateDefaultIndirect()
{
	TEXTURE_DESC dummySkyDesc;
	dummySkyDesc.Type = TEX_TYPE_CUBE_MAP;
	dummySkyDesc.Format = PF_RG11B10F;
	dummySkyDesc.Width = 2;
	dummySkyDesc.Height = 2;

	// Note: Eventually replace this with a time of day model
	float intensity = 1.0f;
	Color skyColor = Color::kWhite * intensity;
	SPtr<ct::Texture> skyTexture = ct::Texture::Create(dummySkyDesc);

	u32 sides[] = { CF_PositiveX, CF_NegativeX, CF_PositiveZ, CF_NegativeZ };
	for(u32 i = 0; i < 4; ++i)
	{
		PixelData data = skyTexture->Lock(GBL_WRITE_ONLY_DISCARD, 0, sides[i]);

		data.SetColorAt(skyColor, 0, 0);
		data.SetColorAt(skyColor, 1, 0);
		data.SetColorAt(Color::kBlack, 0, 1);
		data.SetColorAt(Color::kBlack, 1, 1);

		skyTexture->Unlock();
	}

	{
		PixelData data = skyTexture->Lock(GBL_WRITE_ONLY_DISCARD, 0, CF_PositiveY);

		data.SetColorAt(skyColor, 0, 0);
		data.SetColorAt(skyColor, 1, 0);
		data.SetColorAt(skyColor, 0, 1);
		data.SetColorAt(skyColor, 1, 1);

		skyTexture->Unlock();
	}

	{
		PixelData data = skyTexture->Lock(GBL_WRITE_ONLY_DISCARD, 0, CF_NegativeY);

		data.SetColorAt(Color::kBlack, 0, 0);
		data.SetColorAt(Color::kBlack, 1, 0);
		data.SetColorAt(Color::kBlack, 0, 1);
		data.SetColorAt(Color::kBlack, 1, 1);

		skyTexture->Unlock();
	}

	TEXTURE_DESC irradianceCubemapDesc;
	irradianceCubemapDesc.Type = TEX_TYPE_CUBE_MAP;
	irradianceCubemapDesc.Format = PF_RG11B10F;
	irradianceCubemapDesc.Width = IBLUtility::kIrradianceCubemapSize;
	irradianceCubemapDesc.Height = IBLUtility::kIrradianceCubemapSize;
	irradianceCubemapDesc.NumMips = 0;
	irradianceCubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

	SPtr<ct::Texture> irradiance = ct::Texture::Create(irradianceCubemapDesc);
	gIBLUtility().FilterCubemapForIrradiance(skyTexture, irradiance);

	return irradiance;
}

SPtr<ct::Texture> generateLensFlareGradientTint()
{
	Vector<ColorGradientKey> keys = {
		ColorGradientKey(Color(1.0f, 1.0f, 1.0f), 0.0f),
		ColorGradientKey(Color(0.631f, 1.0f, 0.792f), 0.066f),
		ColorGradientKey(Color(0.203f, 0.913f, 0.266f), 0.2f),
		ColorGradientKey(Color(0.65f, 0.76f, 0.07f), 0.266f),
		ColorGradientKey(Color(0.843f, 0.56f, 0.0f), 0.333f),
		ColorGradientKey(Color(0.631f, 0.082f, 0.058f), 0.533f),
		ColorGradientKey(Color(0.0f, 0.0f, 0.0f), 1.0f)
	};

	ColorGradient gradient(keys);

	SPtr<PixelData> pixels = PixelData::Create(32, 1, 1, PF_RGBA8);
	for(u32 i = 0; i < 16; i++)
		pixels->SetColorAt(Color::FromRgba(gradient.Evaluate(i / 16.0f)), i, 0);

	// We keep the second half of the texture empty, to avoid a mul in shader
	for(u32 i = 16; i < 32; i++)
		pixels->SetColorAt(Color::kBlack, i, 0);

	return ct::Texture::Create(pixels);
}

SPtr<ct::Texture> generateChromaticAberrationFringe()
{
	SPtr<PixelData> pixels = PixelData::Create(3, 1, 1, PF_RGBA8);
	pixels->SetColorAt(Color(1.0f, 0.0f, 0.0f, 1.0f), 0, 0);
	pixels->SetColorAt(Color(0.0f, 1.0f, 0.0f, 1.0f), 1, 0);
	pixels->SetColorAt(Color(0.0f, 0.0f, 1.0f, 1.0f), 2, 0);

	return ct::Texture::Create(pixels);
}

SPtr<ct::Texture> RendererTextures::preintegratedEnvGF;
SPtr<ct::Texture> RendererTextures::ssaoRandomization4x4;
SPtr<ct::Texture> RendererTextures::defaultIndirect;
SPtr<ct::Texture> RendererTextures::lensFlareGradient;
SPtr<ct::Texture> RendererTextures::bokehFlare;
SPtr<ct::Texture> RendererTextures::chromaticAberrationFringe;

void RendererTextures::StartUp(const LoadedRendererTextures& textures)
{
	preintegratedEnvGF = generatePreintegratedEnvBRDF();
	ssaoRandomization4x4 = generate4x4RandomizationTexture();
	defaultIndirect = generateDefaultIndirect();
	lensFlareGradient = generateLensFlareGradientTint();
	bokehFlare = textures.BokehFlare;
	chromaticAberrationFringe = generateChromaticAberrationFringe();
}

void RendererTextures::ShutDown()
{
	preintegratedEnvGF = nullptr;
	ssaoRandomization4x4 = nullptr;
	defaultIndirect = nullptr;
	lensFlareGradient = nullptr;
	bokehFlare = nullptr;
	chromaticAberrationFringe = nullptr;
}
}} // namespace bs::ct
