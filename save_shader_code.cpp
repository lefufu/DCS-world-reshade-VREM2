///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// save shader code to file, reshade file from Crosire
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

#pragma once



#include <reshade.hpp>
#include "config.hpp"
#include "crc32_hash.hpp"
#include <cstring>
#include <fstream>
#include <filesystem>

using namespace reshade::api;

constexpr uint32_t SPIRV_MAGIC = 0x07230203;

static std::filesystem::path make_shader_file_path(uint32_t shader_hash, const wchar_t* extension)
{
	// Prepend executable directory to image files
	wchar_t file_prefix[MAX_PATH] = L"";
	GetModuleFileNameW(nullptr, file_prefix, ARRAYSIZE(file_prefix));

	std::filesystem::path path = file_prefix;
	path = path.parent_path();
	path /= RESHADE_ADDON_SHADER_SAVE_DIR;

	// Ensure target directory exists
	if (!std::filesystem::exists(path))
		std::filesystem::create_directory(path);

	wchar_t hash_string[11];
	swprintf_s(hash_string, L"0x%08X", shader_hash);

	path /= hash_string;
	path += extension;

	return path;
}

void save_shader_code(device_api device_type, const shader_desc& desc)
{

	if (desc.code_size == 0)
		return;

	uint32_t shader_hash = compute_crc32(static_cast<const uint8_t*>(desc.code), desc.code_size);

	const wchar_t* extension = L".cso";
	if (device_type == device_api::vulkan || (device_type == device_api::opengl && desc.code_size > sizeof(uint32_t) && *static_cast<const uint32_t*>(desc.code) == SPIRV_MAGIC))
		extension = L".spv"; // Vulkan uses SPIR-V (and sometimes OpenGL does too)
	else if (device_type == device_api::opengl)
		extension = desc.code_size > 5 && std::strncmp(static_cast<const char*>(desc.code), "!!ARB", 5) == 0 ? L".txt" : L".glsl"; // OpenGL otherwise uses plain text ARB assembly language or GLSL

	const std::filesystem::path file_path = make_shader_file_path(shader_hash, extension);

	std::ofstream file(file_path, std::ios::binary);
	file.write(static_cast<const char*>(desc.code), desc.code_size);
}
