///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// VREM misc function : calculateShaderHash, 
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
#include "crc32_hash.hpp"
#include "addon_objects.h"
#include <optional>
#include "addon_logs.h"


using namespace reshade::api;

// *******************************************************************************************************
/// <summary>
/// Calculates a crc32 hash from the passed in shader bytecode. The hash is used to identity the shader in future runs.
/// </summary>
/// <param name="shaderData"></param>
/// <returns></returns>

uint32_t calculateShaderHash(const reshade::api::shader_desc& desc)
{
	
	if (desc.code_size == 0)
	{
		log_error_code_for_hash();
		return 0;
	}

	uint32_t shader_hash = compute_crc32(static_cast<const uint8_t*>(desc.code), desc.code_size);

	/*
	bool load_shader_code_crosire(device_api device_type, shader_desc & desc, std::vector<std::vector<uint8_t>>&data_to_delete)
	{
		if (desc.code_size == 0)
		{
			log_error_loading_shader_code("code size zero");
			return false;
		}

		uint32_t shader_hash = compute_crc32(static_cast<const uint8_t*>(desc.code), desc.code_size);
		std::unordered_map<uint32_t, Shader_Definition>::iterator it = pipeline_by_hash.find(shader_hash);
	*/

	return shader_hash;
}

// *******************************************************************************************************
/// <summary>
/// check if a hash is in the mod list
/// 
/// 
std::optional<Shader_Definition> is_in_mod_hash(uint32_t hash[], uint32_t subobject_count) {
	
	bool found = false;
	Shader_Definition foundShader;

	for (uint32_t i = 0; i < subobject_count; i++)
	{
		auto it = shader_by_hash.find(hash[i]);
		if (it != shader_by_hash.end()) {
			foundShader = it->second;
			found = true;
		}
	}
	if (found)
	{
		return foundShader;
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<Shader_Definition> is_in_mod_hash(uint32_t hash) {

	bool found = false;
	Shader_Definition foundShader;

	auto it = shader_by_hash.find(hash);
	if (it != shader_by_hash.end()) {
		foundShader = it->second;
		found = true;
	}

	if (found)
	{
		return foundShader;
	}
	else
	{
		return std::nullopt;
	}
}