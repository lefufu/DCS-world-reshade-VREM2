///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  on_create_pipeline : save shader code
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

#include <reshade.hpp>

#include "loader_addon_shared.h"
#include "addon_functions.h"
#include "addon_objects.h"
#include "addon_logs.h"

extern std::unordered_map<uint32_t, std::vector<uint8_t>> shader_code_cache;

using namespace reshade::api;

// *******************************************************************************************************
// load_shader_code_static() : replace shader code once for all at creation as done by crosire for usage in create_pipeline
// update pipeline_by_hash with new hash
// 
bool load_shader_code_static(device_api device_type, shader_desc& orig_desc)
{
	
	if (orig_desc.code_size == 0)
	{
		log_error_loading_shader_code(" on create pipeline - code size zero");
		return false;
	}

	//do static replacement of shader code only if not in debug mode, otherwise dynamic replacement is forced in on_bind_pipeline
	if (!g_shared_state->debug)
	{
		uint32_t shader_hash = calculateShaderHash(orig_desc);
		//check if hash is in shader_by_hash
		auto it = shader_by_hash.find(shader_hash);
		if (it != shader_by_hash.end()) {

			if (it->second.action & action_replace)
			{

				auto it2 = shader_code_cache.find(shader_hash);
				//if hash found in cache replace code in the cloned desc
				if (!(it2 == shader_code_cache.end()))
				{

					// replace code by the one from cso
					const std::vector<uint8_t>& shader_code = it2->second;

					orig_desc.code = shader_code.data();
					orig_desc.code_size = shader_code.size();

					uint32_t new_shader_hash = calculateShaderHash(orig_desc);

					// add a new entry in shader_by_hash with new hash if not existing
					auto it3 = shader_by_hash.find(new_shader_hash);
					if (it3 == shader_by_hash.end()) {
						Shader_Definition new_shader_def = it->second;
						new_shader_def.hash = new_shader_hash;
						shader_by_hash.emplace(new_shader_hash, new_shader_def);
					}
#if _DEBUG_LOGS  
					log_replaced_shader_code(shader_hash, it, new_shader_hash);
#endif
					return true;
				}
			}
		}
	}
	return false;
}



#ifdef _DEBUG
extern "C" {
#endif


	//*******************************************************************************
	// Save shaders code and replace statically shader code (if option setup)
	VREM_EXPORT  bool vrem_on_create_pipeline(device* device, pipeline_layout, uint32_t subobject_count, const pipeline_subobject* subobjects) {

		const device_api device_type = device->get_api();

		bool replaced_stages = false;
#if _DEBUG_CRASH  reshade::log::message(reshade::log::level::info, "addon - vrem_on_create_pipeline starting");
#endif
		for (uint32_t i = 0; i < subobject_count; ++i)
		{
			
			const auto& sub = subobjects[i];
			if (ALLOWED_SHADERS.count(sub.type) > 0)
			{
				// save shader code for debug
				if (g_shared_state->debug) {
					save_shader_code(device_type, *static_cast<const shader_desc*>(subobjects[i].data));
				}
				// handle replacement of code if option setup for this shader
				replaced_stages |= load_shader_code_static(device_type, *static_cast<shader_desc*>(subobjects[i].data));
				break;
			}
		}

#if _DEBUG_CRASH  reshade::log::message(reshade::log::level::info, "addon - vrem_on_create_pipeline ending");
#endif
	// Return whether any shader code was replaced
	return replaced_stages;

	}
#ifdef _DEBUG
}
#endif
