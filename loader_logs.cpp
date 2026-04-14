///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// launcher logs function defintion 
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
#include "to_string.hpp"

#include "loader_addon_shared.h"
//#include "addon_objects.h"

extern SharedState g_shared_state_l;
// extern SharedState* g_shared_state;

void log_manual_reload() {
	if (g_shared_state_l.debug_log)
	{
		reshade::log::message(reshade::log::level::info, "VREM launcher : manual reloading...");
	}
}

void log_dll_copy_error() {
	if (g_shared_state_l.debug_log)
	{
		reshade::log::message(reshade::log::level::error,
			"VREM launcher: Impossible to copy DLL");
	}
}

void  log_dll_copy_error_code(DWORD error)
{
	if (g_shared_state_l.debug_log)
	{
		reshade::log::message(reshade::log::level::error,
			("VREM launcher: LoadLibrary failed, code: " + std::to_string(error)).c_str());
	}
}

void   log_success_load() {
	if (g_shared_state_l.debug_log)
	{
		reshade::log::message(reshade::log::level::info, "VREM launcher: Load Library success");
	}

}

void log_unloaded() {
	if (g_shared_state_l.debug_log)
	{
		reshade::log::message(reshade::log::level::info, "VREM launcher: Unload library success");
	}
}

void log_delete_saved_pipelines() {
	if (g_shared_state_l.debug_log)
	{
		reshade::log::message(reshade::log::level::info, "VREM launcher: deleting saved pipelines :");
	}
}

void log_saved_pipelines_value_l(save_pipeline saved_pipeline) {
	if (g_shared_state_l.debug_log)
	{
		//uint32_t hash;
		std::stringstream s;
		s << "VREM launcher: pipeline handle = " << std::hex << saved_pipeline.pipeline.handle << ", subobject_count = " << saved_pipeline.subobject_count << "; ";
		//reshade::log::message(reshade::log::level::info, s.str().c_str());
		// s.str("");
		// s.clear();

		// display sub_objects
		for (uint32_t i = 0; i < saved_pipeline.subobject_count; i++) {
			//hash = 0;
			const auto& sub = saved_pipeline.subobjects[i];
			/*
			switch (sub.type) {
			case reshade::api::pipeline_subobject_type::vertex_shader: {
				hash = saved_pipeline.vs_hash;
				break;
			}

			case reshade::api::pipeline_subobject_type::pixel_shader: {
				hash = saved_pipeline.ps_hash;
				break;
			}
			} */
			
			s << "subobject[" << i << "] : hash = " << std::hex << saved_pipeline.hash[i] << ", type = " << to_string(sub.type) << "; ";
			//reshade::log::message(reshade::log::level::info, s.str().c_str());
			//s.str("");
			//s.clear();
		}
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_shader_marked(save_pipeline* p_found, char comment[256])
{

	if (g_shared_state_l.shader_hunter)
	{
		std::stringstream s;

		if (p_found->pipeline.handle != 0)
		{
			s << "loader: hunting ===>  pipeline marked : "<< comment << ", handle = " << std::hex << p_found->pipeline.handle;
			for (int i = 0; i < p_found->subobject_count; i++)
			{
				s << ", hash[" << i << "] = " << std::hex << p_found->hash[i];
			}
		}
		else
		{
			s << "loader: hunting ===>  pipeline marked : handle = " << std::hex << p_found->pipeline.handle;
		}
		s << " <====";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}
