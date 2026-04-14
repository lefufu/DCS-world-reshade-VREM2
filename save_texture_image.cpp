///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  save texture for hunting mode
// ----------------------------------------------------------------------------------------
// 
// (c) Lefuneste.
//
// All rights reserved.
// https://github.com/xxx
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
//  * Redistributions of source code must retain the above copyright notice, this
//	  list of conditions and the following disclaimer.
//
//  * Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and / or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This software is using part of code or algorithms provided by
// * Crosire https://github.com/crosire/reshade  
// * FransBouma https://github.com/FransBouma/ShaderToggler
// * ShortFuse https://github.com/clshortfuse/renodx
// 
/////////////////////////////////////////////////////////////////////////

// file coming from reshade repo : examples/utils/save_texture_image.cpp
// with small changes
// 
// lefuneste

/*
 * Copyright (C) 2021 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <reshade.hpp>
#include "config.hpp"
#include "crc32_hash.hpp"
#include <vector>
#include <filesystem>

#include "stb_image_write.h"
#include "addon_objects.h"
#include "to_string.hpp"
#include "addon_logs.h"

#if RESHADE_ADDON_TEXTURE_SAVE_ENABLE_HASH_SET
#include <set>
#endif

using namespace reshade::api;

static std::filesystem::path make_texture_file_path(uint32_t texture_hash)
{
	// Prepend executable directory to image files
	/* wchar_t file_prefix[MAX_PATH] = L"";
	GetModuleFileNameW(nullptr, file_prefix, ARRAYSIZE(file_prefix));

	std::filesystem::path path = file_prefix;
	path = path.parent_path(); */

	std::filesystem::path  path = g_shared_state->g_vrem_base_path;
	path /= RESHADE_ADDON_TEXTURE_SAVE_DIR; 

	// Ensure target directory exists
	if (!std::filesystem::exists(path))
		std::filesystem::create_directory(path);

	wchar_t hash_string[11];
	swprintf_s(hash_string, L"0x%08X", texture_hash);

	path /= hash_string;
	path += RESHADE_ADDON_TEXTURE_SAVE_FORMAT;

	return path;
}

static void unpack_r5g6b5(uint16_t data, uint8_t rgb[3])
{
	uint32_t temp;
	temp =  (data           >> 11) * 255 + 16;
	rgb[0] = static_cast<uint8_t>((temp / 32 + temp) / 32);
	temp = ((data & 0x07E0) >>  5) * 255 + 32;
	rgb[1] = static_cast<uint8_t>((temp / 64 + temp) / 64);
	temp =  (data & 0x001F)        * 255 + 16;
	rgb[2] = static_cast<uint8_t>((temp / 32 + temp) / 32);
}
static void unpack_bc1_value(const uint8_t color_0[3], const uint8_t color_1[3], uint32_t color_index, uint8_t result[4], bool not_degenerate = true)
{
	switch (color_index)
	{
	case 0:
		for (int c = 0; c < 3; ++c)
			result[c] = color_0[c];
		result[3] = 255;
		break;
	case 1:
		for (int c = 0; c < 3; ++c)
			result[c] = color_1[c];
		result[3] = 255;
		break;
	case 2:
		for (int c = 0; c < 3; ++c)
			result[c] = not_degenerate ? (2 * color_0[c] + color_1[c]) / 3 : (color_0[c] + color_1[c]) / 2;
		result[3] = 255;
		break;
	case 3:
		for (int c = 0; c < 3; ++c)
			result[c] = not_degenerate ? (color_0[c] + 2 * color_1[c]) / 3 : 0;
		result[3] = not_degenerate ? 255 : 0;
		break;
	}
}
static void unpack_bc4_value(uint8_t alpha_0, uint8_t alpha_1, uint32_t alpha_index, uint8_t *result)
{
	const bool interpolation_type = alpha_0 > alpha_1;

	switch (alpha_index)
	{
	case 0:
		*result = alpha_0;
		break;
	case 1:
		*result = alpha_1;
		break;
	case 2:
		*result = interpolation_type ? (6 * alpha_0 + 1 * alpha_1) / 7 : (4 * alpha_0 + 1 * alpha_1) / 5;
		break;
	case 3:
		*result = interpolation_type ? (5 * alpha_0 + 2 * alpha_1) / 7 : (3 * alpha_0 + 2 * alpha_1) / 5;
		break;
	case 4:
		*result = interpolation_type ? (4 * alpha_0 + 3 * alpha_1) / 7 : (2 * alpha_0 + 3 * alpha_1) / 5;
		break;
	case 5:
		*result = interpolation_type ? (3 * alpha_0 + 4 * alpha_1) / 7 : (1 * alpha_0 + 4 * alpha_1) / 5;
		break;
	case 6:
		*result = interpolation_type ? (2 * alpha_0 + 5 * alpha_1) / 7 : 0;
		break;
	case 7:
		*result = interpolation_type ? (1 * alpha_0 + 6 * alpha_1) / 7 : 255;
		break;
	}
}
// ************************************************************************************************************
bool save_texture_image(const resource_desc &desc, const subresource_data &data, const std::string& filepath)
{


#if RESHADE_ADDON_TEXTURE_SAVE_HASH_TEXMOD
	// Behavior of the original TexMod (see https://github.com/codemasher/texmod/blob/master/uMod_DX9/uMod_TextureFunction.cpp#L41)
	const uint32_t hash = ~compute_crc32(
		static_cast<const uint8_t *>(data.data),
		desc.texture.height * static_cast<size_t>(
			(desc.texture.format >= format::bc1_typeless && desc.texture.format <= format::bc1_unorm_srgb) || (desc.texture.format >= format::bc4_typeless && desc.texture.format <= format::bc4_snorm) ? (desc.texture.width * 4) / 8 :
			(desc.texture.format >= format::bc2_typeless && desc.texture.format <= format::bc2_unorm_srgb) || (desc.texture.format >= format::bc3_typeless && desc.texture.format <= format::bc3_unorm_srgb) || (desc.texture.format >= format::bc5_typeless && desc.texture.format <= format::bc7_unorm_srgb) ? desc.texture.width :
			format_row_pitch(desc.texture.format, desc.texture.width)));
#else
	// Correct hash calculation using entire resource data
	/*const uint32_t hash = compute_crc32(
		static_cast<const uint8_t *>(data.data),
		format_slice_pitch(desc.texture.format, data.row_pitch, desc.texture.height));
		*/
#endif

#if RESHADE_ADDON_TEXTURE_SAVE_ENABLE_HASH_SET
	static std::set<uint32_t> hash_set;
	if (hash_set.find(hash) != hash_set.end())
	{
		reshade::log::message(reshade::log::level::error, "Skipped texture that was already dumped.");
		return true;
	}
	else
	{
		hash_set.insert(hash);
	}
#endif

	const uint32_t block_count_x = (desc.texture.width + 3) / 4;
	const uint32_t block_count_y = (desc.texture.height + 3) / 4;

	uint8_t *data_p = static_cast<uint8_t *>(data.data);
	// Add sufficient padding for block compressed textures that are not a multiple of 4 in all dimensions
	std::vector<uint8_t> rgba_pixel_data((block_count_x * 4) * (block_count_y * 4) * 4);

	switch (desc.texture.format)
	{
	case format::l8_unorm:
		for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
		{
			for (size_t x = 0; x < desc.texture.width; ++x)
			{
				const uint8_t *const src = data_p + x;
				uint8_t *const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

				dst[0] = src[0];
				dst[1] = src[0];
				dst[2] = src[0];
				dst[3] = 255;
			}
		}
		break;
	case format::a8_unorm:
		for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
		{
			for (size_t x = 0; x < desc.texture.width; ++x)
			{
				const uint8_t *const src = data_p + x;
				uint8_t *const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

				dst[0] = 0;
				dst[1] = 0;
				dst[2] = 0;
				dst[3] = src[0];
			}
		}
		break;
	case format::r8_typeless:
	case format::r8_unorm:
	case format::r8_snorm:
		for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
		{
			for (size_t x = 0; x < desc.texture.width; ++x)
			{
				const uint8_t *const src = data_p + x;
				uint8_t *const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

				dst[0] = src[0];
				dst[1] = 0;
				dst[2] = 0;
				dst[3] = 255;
			}
		}
		break;
	case format::l8a8_unorm:
		for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
		{
			for (size_t x = 0; x < desc.texture.width; ++x)
			{
				const uint8_t *const src = data_p + x * 2;
				uint8_t *const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

				dst[0] = src[0];
				dst[1] = src[0];
				dst[2] = src[0];
				dst[3] = src[1];
			}
		}
		break;
	case format::r8g8_typeless:
	case format::r8g8_unorm:
	case format::r8g8_snorm:
		for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
		{
			for (size_t x = 0; x < desc.texture.width; ++x)
			{
				const uint8_t *const src = data_p + x * 2;
				uint8_t *const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = 0;
				dst[3] = 255;
			}
		}
		break;
	case format::r8g8b8a8_typeless:
	case format::r8g8b8a8_unorm:
	case format::r8g8b8a8_unorm_srgb:
	case format::r8g8b8x8_unorm:
	case format::r8g8b8x8_unorm_srgb:
		for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
		{
			for (size_t x = 0; x < desc.texture.width; ++x)
			{
				const uint8_t *const src = data_p + x * 4;
				uint8_t *const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = src[3];
			}
		}
		break;
	case format::b8g8r8a8_typeless:
	case format::b8g8r8a8_unorm:
	case format::b8g8r8a8_unorm_srgb:
	case format::b8g8r8x8_typeless:
	case format::b8g8r8x8_unorm:
	case format::b8g8r8x8_unorm_srgb:
		for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
		{
			for (size_t x = 0; x < desc.texture.width; ++x)
			{
				const uint8_t *const src = data_p + x * 4;
				uint8_t *const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

				// Swap red and blue channel
				dst[0] = src[2];
				dst[1] = src[1];
				dst[2] = src[0];
				dst[3] = src[3];
			}
		}
		break;
	case format::bc1_typeless:
	case format::bc1_unorm:
	case format::bc1_unorm_srgb:
		// See https://docs.microsoft.com/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc1
		for (size_t block_y = 0; block_y < block_count_y; ++block_y, data_p += data.row_pitch)
		{
			for (size_t block_x = 0; block_x < block_count_x; ++block_x)
			{
				const uint8_t *const src = data_p + block_x * 8;

				const uint16_t color_0 = *reinterpret_cast<const uint16_t *>(src);
				const uint16_t color_1 = *reinterpret_cast<const uint16_t *>(src + 2);
				const uint32_t color_i = *reinterpret_cast<const uint32_t *>(src + 4);

				uint8_t color_0_rgb[3];
				unpack_r5g6b5(color_0, color_0_rgb);
				uint8_t color_1_rgb[3];
				unpack_r5g6b5(color_1, color_1_rgb);
				const bool degenerate = color_0 > color_1;

				for (int y = 0; y < 4; ++y)
				{
					for (int x = 0; x < 4; ++x)
					{
						uint8_t *const dst = rgba_pixel_data.data() + ((block_y * 4 + y) * desc.texture.width + (block_x * 4 + x)) * 4;

						unpack_bc1_value(color_0_rgb, color_1_rgb, (color_i >> (2 * (y * 4 + x))) & 0x3, dst, degenerate);
					}
				}
			}
		}
		break;
	case format::bc3_typeless:
	case format::bc3_unorm:
	case format::bc3_unorm_srgb:
		// See https://docs.microsoft.com/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc3
		for (size_t block_y = 0; block_y < block_count_y; ++block_y, data_p += data.row_pitch)
		{
			for (size_t block_x = 0; block_x < block_count_x; ++block_x)
			{
				const uint8_t *const src = data_p + block_x * 16;

				const uint8_t  alpha_0 = src[0];
				const uint8_t  alpha_1 = src[1];
				const uint64_t alpha_i =
					(static_cast<uint64_t>(src[2])      ) |
					(static_cast<uint64_t>(src[3]) <<  8) |
					(static_cast<uint64_t>(src[4]) << 16) |
					(static_cast<uint64_t>(src[5]) << 24) |
					(static_cast<uint64_t>(src[6]) << 32) |
					(static_cast<uint64_t>(src[7]) << 40);

				const uint16_t color_0 = *reinterpret_cast<const uint16_t *>(src + 8);
				const uint16_t color_1 = *reinterpret_cast<const uint16_t *>(src + 10);
				const uint32_t color_i = *reinterpret_cast<const uint32_t *>(src + 12);

				uint8_t color_0_rgb[3];
				unpack_r5g6b5(color_0, color_0_rgb);
				uint8_t color_1_rgb[3];
				unpack_r5g6b5(color_1, color_1_rgb);

				for (int y = 0; y < 4; ++y)
				{
					for (int x = 0; x < 4; ++x)
					{
						uint8_t *const dst = rgba_pixel_data.data() + ((block_y * 4 + y) * desc.texture.width + (block_x * 4 + x)) * 4;

						unpack_bc1_value(color_0_rgb, color_1_rgb, (color_i >> (2 * (y * 4 + x))) & 0x3, dst);
						unpack_bc4_value(alpha_0, alpha_1, (alpha_i >> (3 * (y * 4 + x))) & 0x7, dst + 3);
					}
				}
			}
		}
		break;
	case format::bc4_typeless:
	case format::bc4_unorm:
	case format::bc4_snorm:
		// See https://docs.microsoft.com/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc4
		for (size_t block_y = 0; block_y < block_count_y; ++block_y, data_p += data.row_pitch)
		{
			for (size_t block_x = 0; block_x < block_count_x; ++block_x)
			{
				const uint8_t *const src = data_p + block_x * 8;

				const uint8_t  red_0 = src[0];
				const uint8_t  red_1 = src[1];
				const uint64_t red_i =
					(static_cast<uint64_t>(src[2])      ) |
					(static_cast<uint64_t>(src[3]) <<  8) |
					(static_cast<uint64_t>(src[4]) << 16) |
					(static_cast<uint64_t>(src[5]) << 24) |
					(static_cast<uint64_t>(src[6]) << 32) |
					(static_cast<uint64_t>(src[7]) << 40);

				for (int y = 0; y < 4; ++y)
				{
					for (int x = 0; x < 4; ++x)
					{
						uint8_t *const dst = rgba_pixel_data.data() + ((block_y * 4 + y) * desc.texture.width + (block_x * 4 + x)) * 4;

						unpack_bc4_value(red_0, red_1, (red_i >> (3 * (y * 4 + x))) & 0x7, dst);
						dst[1] = dst[0];
						dst[2] = dst[0];
						dst[3] = 255;
					}
				}
			}
		}
		break;
	case format::bc5_typeless:
	case format::bc5_unorm:
	case format::bc5_snorm:
		// See https://docs.microsoft.com/windows/win32/direct3d10/d3d10-graphics-programming-guide-resources-block-compression#bc5
		for (size_t block_y = 0; block_y < block_count_y; ++block_y, data_p += data.row_pitch)
		{
			for (size_t block_x = 0; block_x < block_count_x; ++block_x)
			{
				const uint8_t *const src = data_p + block_x * 16;

				const uint8_t  red_0 = src[0];
				const uint8_t  red_1 = src[1];
				const uint64_t red_i =
					(static_cast<uint64_t>(src[2])      ) |
					(static_cast<uint64_t>(src[3]) <<  8) |
					(static_cast<uint64_t>(src[4]) << 16) |
					(static_cast<uint64_t>(src[5]) << 24) |
					(static_cast<uint64_t>(src[6]) << 32) |
					(static_cast<uint64_t>(src[7]) << 40);

				const uint8_t  green_0 = src[8];
				const uint8_t  green_1 = src[9];
				const uint64_t green_i =
					(static_cast<uint64_t>(src[10])      ) |
					(static_cast<uint64_t>(src[11]) <<  8) |
					(static_cast<uint64_t>(src[12]) << 16) |
					(static_cast<uint64_t>(src[13]) << 24) |
					(static_cast<uint64_t>(src[14]) << 32) |
					(static_cast<uint64_t>(src[15]) << 40);

				for (int y = 0; y < 4; ++y)
				{
					for (int x = 0; x < 4; ++x)
					{
						uint8_t *const dst = rgba_pixel_data.data() + ((block_y * 4 + y) * desc.texture.width + (block_x * 4 + x)) * 4;

						unpack_bc4_value(red_0, red_1, (red_i >> (3 * (y * 4 + x))) & 0x7, dst);
						unpack_bc4_value(green_0, green_1, (green_i >> (3 * (y * 4 + x))) & 0x7, dst + 1);
						dst[2] = 0;
						dst[3] = 255;
					}
				}
			}
		}
		break;
		//added from claude
		case format::r10g10b10a2_typeless:
		case format::r10g10b10a2_unorm:
		case format::r10g10b10a2_uint:
			for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
			{
				for (size_t x = 0; x < desc.texture.width; ++x)
				{
					const uint32_t* const src = reinterpret_cast<const uint32_t*>(data_p + x * 4);
					uint8_t* const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

					// Extraction des composants (10 bits R, 10 bits G, 10 bits B, 2 bits A)
					const uint32_t r10 = (*src) & 0x3FF;  // 10 premiers bits
					const uint32_t g10 = (*src >> 10) & 0x3FF;  // 10 bits suivants
					const uint32_t b10 = (*src >> 20) & 0x3FF;  // 10 bits suivants
					const uint32_t a2 = (*src >> 30) & 0x3;    // 2 derniers bits

					// Conversion de 10 bits vers 8 bits (0-1023 -> 0-255)
					// On divise par 1023 et multiplie par 255, ce qui équivaut à >> 2
					dst[0] = static_cast<uint8_t>((r10 * 255 + 511) / 1023);
					dst[1] = static_cast<uint8_t>((g10 * 255 + 511) / 1023);
					dst[2] = static_cast<uint8_t>((b10 * 255 + 511) / 1023);

					// Conversion de 2 bits vers 8 bits (0-3 -> 0-255)
					dst[3] = static_cast<uint8_t>(a2 * 85);  // 85 = 255/3
				}
			}
			break;

		case format::r16_typeless:
		case format::r16_unorm:
		case format::r16_uint:
		case format::r16_snorm:
		case format::r16_sint:
			for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
			{
				for (size_t x = 0; x < desc.texture.width; ++x)
				{
					const uint16_t* const src = reinterpret_cast<const uint16_t*>(data_p + x * 2);
					uint8_t* const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

					// Conversion de 16 bits vers 8 bits (0-65535 -> 0-255)
					dst[0] = static_cast<uint8_t>((*src * 255 + 32767) / 65535);
					dst[1] = 0;
					dst[2] = 0;
					dst[3] = 255;
				}
			}
			break;

		case format::r16g16_typeless:
		case format::r16g16_unorm:
		case format::r16g16_uint:
		case format::r16g16_snorm:
		case format::r16g16_sint:
			for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
			{
				for (size_t x = 0; x < desc.texture.width; ++x)
				{
					const uint16_t* const src = reinterpret_cast<const uint16_t*>(data_p + x * 4);
					uint8_t* const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

					// Conversion de 16 bits vers 8 bits pour R et G
					dst[0] = static_cast<uint8_t>((src[0] * 255 + 32767) / 65535);
					dst[1] = static_cast<uint8_t>((src[1] * 255 + 32767) / 65535);
					dst[2] = 0;
					dst[3] = 255;
				}
			}
			break;

			case format::r32_g8_typeless:
				for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
				{
					for (size_t x = 0; x < desc.texture.width; ++x)
					{
						const uint8_t* const src = data_p + x * 8; // 32 bits R + 8 bits padding + 24 bits unused
						uint8_t* const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

						// Pour R32_FLOAT_X8_TYPELESS, le premier composant est un float
						// On le convertit en uint8 en le clampant entre 0 et 1
						const float r_float = *reinterpret_cast<const float*>(src);
						//const float clamped = std::max(0.0f, std::min(1.0f, r_float));
						const float clamped = std::clamp(r_float, 0.0f, 1.0f);

						dst[0] = static_cast<uint8_t>(clamped * 255.0f);
						dst[1] = src[4]; // Le composant G8 est à l'offset 4
						dst[2] = 0;
						dst[3] = 255;
					}
				}
				break;

				case format::r16g16b16a16_typeless:
				case format::r16g16b16a16_unorm:
				case format::r16g16b16a16_uint:
				case format::r16g16b16a16_snorm:
				case format::r16g16b16a16_sint:
				case format::r16g16b16a16_float:
					for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
					{
						for (size_t x = 0; x < desc.texture.width; ++x)
						{
							const uint16_t* const src = reinterpret_cast<const uint16_t*>(data_p + x * 8);
							uint8_t* const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

							// Conversion de 16 bits vers 8 bits pour chaque canal (RGBA)
							dst[0] = static_cast<uint8_t>((src[0] * 255 + 32767) / 65535);
							dst[1] = static_cast<uint8_t>((src[1] * 255 + 32767) / 65535);
							dst[2] = static_cast<uint8_t>((src[2] * 255 + 32767) / 65535);
							dst[3] = static_cast<uint8_t>((src[3] * 255 + 32767) / 65535);
						}
					}
					break;

				case format::r24_g8_typeless:
				case format::d24_unorm_s8_uint:
					for (size_t y = 0; y < desc.texture.height; ++y, data_p += data.row_pitch)
					{
						for (size_t x = 0; x < desc.texture.width; ++x)
						{
							const uint32_t* const src = reinterpret_cast<const uint32_t*>(data_p + x * 4);
							uint8_t* const dst = rgba_pixel_data.data() + (y * desc.texture.width + x) * 4;

							// Extraction des composants (24 bits R, 8 bits G)
							const uint32_t r24 = (*src) & 0xFFFFFF;  // 24 premiers bits
							const uint32_t g8 = (*src >> 24) & 0xFF;      // 8 derniers bits

							// Conversion de 24 bits vers 8 bits (0-16777215 -> 0-255)
							dst[0] = static_cast<uint8_t>((r24 * 255 + 8388607) / 16777215);
							dst[1] = static_cast<uint8_t>(g8);
							dst[2] = 0;
							dst[3] = 255;
						}
					}
					break;

				case format::bc2_typeless:
				case format::bc2_unorm:
				case format::bc2_unorm_srgb:
					for (size_t block_y = 0; block_y < block_count_y; ++block_y, data_p += data.row_pitch)
					{
						for (size_t block_x = 0; block_x < block_count_x; ++block_x)
						{
							const uint8_t* const src = data_p + block_x * 16;

							// BC2 : 8 octets d'alpha explicite (4 bits par pixel) + 8 octets de couleur BC1
							// Les 8 premiers octets contiennent 16 valeurs alpha de 4 bits chacune
							const uint64_t alpha_data = *reinterpret_cast<const uint64_t*>(src);

							// Les 8 octets suivants sont identiques à BC1
							const uint16_t color_0 = *reinterpret_cast<const uint16_t*>(src + 8);
							const uint16_t color_1 = *reinterpret_cast<const uint16_t*>(src + 10);
							const uint32_t color_i = *reinterpret_cast<const uint32_t*>(src + 12);

							uint8_t color_0_rgb[3];
							unpack_r5g6b5(color_0, color_0_rgb);
							uint8_t color_1_rgb[3];
							unpack_r5g6b5(color_1, color_1_rgb);

							for (int y = 0; y < 4; ++y)
							{
								for (int x = 0; x < 4; ++x)
								{
									uint8_t* const dst = rgba_pixel_data.data() + ((block_y * 4 + y) * desc.texture.width + (block_x * 4 + x)) * 4;

									// Décodage de la couleur RGB (comme BC1, mais toujours non-dégénéré)
									unpack_bc1_value(color_0_rgb, color_1_rgb, (color_i >> (2 * (y * 4 + x))) & 0x3, dst, true);

									// Décodage de l'alpha (4 bits par pixel)
									const int pixel_index = y * 4 + x;
									const uint8_t alpha_4bit = static_cast<uint8_t>((alpha_data >> (4 * pixel_index)) & 0xF);

									// Conversion de 4 bits vers 8 bits (0-15 -> 0-255)
									dst[3] = static_cast<uint8_t>((alpha_4bit * 255 + 7) / 15);
								}
							}
						}
					}
					break;

		default:
		// Unsupported format
		log_error_unsuported_format(desc, filepath);
		return false;
	}

	//const std::filesystem::path file_path = make_texture_file_path(hash);
	const std::filesystem::path file_path = filepath;

	if (file_path.extension() == L".bmp")
		return stbi_write_bmp(reinterpret_cast<const char*>(file_path.u8string().c_str()), desc.texture.width, desc.texture.height, 4, rgba_pixel_data.data()) != 0;
	else if (file_path.extension() == L".png")
		return stbi_write_png(reinterpret_cast<const char*>(file_path.u8string().c_str()), desc.texture.width, desc.texture.height, 4, rgba_pixel_data.data(), desc.texture.width * 4) != 0;
	else
		return false;
}
