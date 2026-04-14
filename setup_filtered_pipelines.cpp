///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  setup list of pipeline to process, regarding mod option. Clone pipelines if needed
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

#include <unordered_map>
#include "loader_addon_shared.h"
#include "addon_functions.h"
#include "addon_logs.h"
#include "to_string.hpp"

//extern SharedState g_shared_state;

// static thread_local std::vector<std::vector<uint8_t>> shader_code;
// extern std::unordered_map<uint32_t, std::vector<uint8_t>> shader_code_cache;

// *******************************************************************************************************
/// <summary>
/// Chck if the shader definition is related to an active option in the mod settings
/// </summary>

bool check_if_active_option(Shader_Definition shader_def)
{
	for (uint32_t index : shader_def.VREM_options)
	{
		if (a_shared.VREM_setting[index] != 0)
		{
			return 1;
		}
	}
	return 0;
}

// *******************************************************************************************************
/// <summary>
/// setup filetered pipelines regarding mod settings
/// </summary>

bool setup_filtered_pipelines(reshade::api::device* device, reshade::api::effect_runtime* runtime)
{
	/*
	// parse the shader list to load all shader codes and store codes in shader_code_cache
	read_all_shader_code();
	
	// ensure settings are updated from uniforms to be sure shader will be loaded regarding current mod settings
	get_settings_from_uniforms(runtime);
	*/

	uint64_t last_handle;

	filtered_pipeline.reserve(g_shared_state->VREM_pipelines.saved_pipelines.size());
	std::optional<Shader_Definition> shader_def_opt;

	/*
	std::stringstream s;
	s << "g_shared_state->VREM_pipelines.saved_pipelines.size() =" << g_shared_state->VREM_pipelines.saved_pipelines.size() << ";";
	reshade::log::message(reshade::log::level::info, s.str().c_str());
	*/

	int i = 0;
	// parse the list of saved pipelines to identify which one to keep regarding mod settings
	for (auto& p : g_shared_state->VREM_pipelines.saved_pipelines)
	{
		
		/* std::stringstream s;
		s << "p.hash =" << p.hash << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.clear();
		*/

		// Validate pipeline handle
		if (p.pipeline.handle == 0) {
			// reshade::log::message(reshade::log::level::info, "*** addon - handle null");
			continue;
		}
		
		// is this pipeline to be processed regarding mod settings ?
		bool to_be_filtered = false;
		// check if the shader hash is in the mod list


		// reshade::log::message(reshade::log::level::info, "*** addon - check if not already added");

		// check if not already added
		auto filtered_pipe = filtered_pipeline.find(p.pipeline.handle);

		// reshade::log::message(reshade::log::level::info, "*** addon - after find");
		if (filtered_pipe != filtered_pipeline.end()) {
#if _DEBUG_LOGS
			// pipeline is already in the list, do nothing
			// ***************************************************************                   
			// removed because too verbose !
			// remove crash in release before moving shader_def_opt out of te loop....
			// log_pipeline_filtered_skipped(p.pipeline.handle);
#endif
		}
		else {
			// add pipeline in the filtered list

			// reshade::log::message(reshade::log::level::info, "*** addon - add pipeline in the filtered list");

			shader_def_opt = is_in_mod_hash(p.hash, p.subobject_count);

			i++;

			if (shader_def_opt.has_value())
			{

				//reshade::log::message(reshade::log::level::info, "*** addon - process pipeline");

				bool active = check_if_active_option(shader_def_opt.value());
#if _DEBUG_LOGS
				// ***************************************************************                   
				// removed because too verbose !
				// log_shader(p.pipeline, shader_def_opt.value(), active);
#endif
				// if the shader is active, add it to the filtered pipeline list for later processing
				if (active)
				{
					// cloned pipeline if needed
					if ((shader_def_opt.value().action & action_replace_bind) || (shader_def_opt.value().action & action_replace))
					{

						//clone the pipeline with the new shader code
						pipeline cloned_pipeline = clone_pipeline(p.device, p.layout, p.subobject_count, p.subobjects.data(), p.pipeline, p.hash);
						if (cloned_pipeline.handle == 0)
						{
							log_pipeline_clone_error(p.pipeline.handle);

						}
						else
						{
							// store the cloned & modified pipeline for later usage in bind_pipeline
							shader_def_opt.value().substitute_pipeline = cloned_pipeline;
							//add cloned pipeline in cloned_pipeline to suppress them if addon is reloaded
							cloned_pipeline_list.emplace(cloned_pipeline.handle, cloned_pipeline);
							last_handle = cloned_pipeline.handle;
						}

					}
					filtered_pipeline.emplace(p.pipeline.handle, shader_def_opt.value());
#if _DEBUG_LOGS
					// ***************************************************************                   
					// removed because too verbose !
					// log_filtered_added(p.pipeline.handle);
#endif
				}
			}
		}

		//create constant color pipeline for hunting by using the first PS pipeline and the has corresponding to constant color .cso
		if (p.pipeline.handle == a_shared.first_PS_pipeline_handle)
		{
	
			uint32_t hash_color[1] = { CONSTANT_HASH };
			a_shared.cloned_constant_color_pipeline = clone_pipeline(p.device, p.layout, p.subobject_count, p.subobjects.data(), p.pipeline, hash_color);
		}

	}

	return false;
}





