
///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  draw* : reset tracking flags and skip draw if needed
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
#include <unordered_map>


#include "loader_addon_shared.h"
#include "addon_functions.h"
#include "addon_objects.h"
#include "addon_logs.h"

#include "to_string.hpp"


using namespace reshade::api;

//*******************************************************************************************************
// clear tracking flags to avoid tracking resources if push_descriptor did not detect it...

static void clear_tracking_flags()
{

	// if (a_shared.track_for_texture || a_shared.track_for_NS430 || a_shared.do_not_draw)
	if (track_for_texture || do_not_draw)
	{
#if _DEBUG_LOGS  
		log_reset_tracking();
#endif
		track_for_texture = false;
		//a_shared.track_for_NS430 = false;
		do_not_draw = false;
	}

	// shared_data.track_for_CB[DEF_UNIFORMS_CB_NB] = false;
	a_shared.draw_passed = true;

	//if (a_shared.render_technique && flag_capture) reshade::log::message(reshade::log::level::info, "***** addon - ondraw*() => draw_passed = true");

#ifdef _DEBUG
	//to stop texture and buffer dump
	a_shared.flag_texture_dump = false;
	a_shared.flag_cb_dump = false;

#endif

}

#ifdef _DEBUG
extern "C" {
#endif

	// *******************************************************************************************************
	// On draw* : skip draw
	//
	VREM_EXPORT bool vrem_on_draw(command_list* commandList, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
	{
/*
#ifdef _DEBUG
		//dump render target if capture button is active, to help shader hunting and technique setup
		if (g_shared_state->save_rt_flag && flag_capture && a_shared.track_for_render_target)
		{
			save_render_target(commandList);
		}
#endif
*/
#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_draw started");
#endif

#if _DEBUG_LOGS
		// log
		if (g_shared_state->shader_hunter && flag_capture) log_ondraw(vertex_count, instance_count, first_vertex, first_instance);
#endif		
		if (!track_for_texture && !do_not_draw && !a_shared.render_technique && !a_shared.flag_texture_dump && !a_shared.flag_cb_dump) return false;
		
		bool skip = false;
		if (do_not_draw) skip = true;
#if _DEBUG_LOGS
		// log
		log_ondraw(vertex_count, instance_count, first_vertex,first_instance);
#endif	
		// clear tracking flags
		clear_tracking_flags();

#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_draw ended");
#endif
		return skip;
	}

	// *******************************************************************************************************
	// On draw* : skip draw
	VREM_EXPORT bool vrem_on_draw_indexed(command_list* commandList, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
	{
/*
#ifdef _DEBUG
		if (g_shared_state->save_rt_flag && flag_capture)
		{
			save_render_target(commandList);
		}
#endif
*/
#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_draw_indexed started");
#endif

#if _DEBUG_LOGS
		// log
		if (g_shared_state->shader_hunter && flag_capture) log_on_draw_indexed(index_count, instance_count, first_index, vertex_offset, first_instance);
#endif		
		if (!track_for_texture && !do_not_draw && !a_shared.render_technique && !a_shared.flag_texture_dump && !a_shared.flag_cb_dump) return false;

		bool skip = false;
		if (do_not_draw) skip = true;
#if _DEBUG_LOGS
		// log
		log_on_draw_indexed(index_count, instance_count, first_index, vertex_offset, first_instance);
#endif
		// clear trackign flags
		clear_tracking_flags();

#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_draw_indexed ended");
#endif

		return skip;
	}

	// *******************************************************************************************************
	// On draw* : skip draw
	VREM_EXPORT bool vrem_on_drawOrDispatch_indirect(command_list* commandList, indirect_command type, resource buffer, uint64_t offset, uint32_t draw_count, uint32_t stride)
	{

#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_draw_indexed started");
#endif

#if _DEBUG_LOGS
		// log
		if (g_shared_state->shader_hunter && flag_capture) log_on_drawOrDispatch_indirect(type, buffer, offset, draw_count, stride);
#endif
		if (!track_for_texture && !do_not_draw && !a_shared.render_technique && !a_shared.flag_texture_dump && !a_shared.flag_cb_dump) return false;
		
		bool skip = false;
		if (do_not_draw) skip = true;
#if _DEBUG_LOGS
		// log
		log_on_drawOrDispatch_indirect(type, buffer, offset, draw_count, stride);
#endif
		// clear trackign flags
		clear_tracking_flags();

#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_draw_indexed ended");
#endif

		return skip;

	}
#ifdef _DEBUG
}
#endif