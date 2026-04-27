///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  on push_descriptors : used 
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
#include <thread>

#include "loader_addon_shared.h"
#include "addon_functions.h"
#include "addon_logs.h"

#include "export_texture.hpp"
#include "export_CB.hpp"

using namespace reshade::api;

//*******************************************************************************************************
// for hunting
void dump_text_cb(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
#ifdef _DEBUG
	// log for shader hunting
	if (g_shared_state->shader_hunter)
	{
#if _DEBUG_LOGS
		log_hunting_push_descriptor(cmd_list, stages, layout, param_index, update);
#endif

	}
	
	//export textures for hunting if requested
	if (flag_capture && a_shared.flag_texture_dump && g_shared_state->save_texture_flag && update.type == descriptor_type::shader_resource_view)
	{
		TextureExporter g_exporter;
		g_exporter.export_descriptors(
			cmd_list,
			stages,
			layout,
			param_index,
			update,
			a_shared.ps_hash_for_text_dump,
			a_shared.count_display
		);


	}

	if (flag_capture && a_shared.flag_cb_dump && g_shared_state->save_cb_flag && update.type == descriptor_type::constant_buffer)
	{
		ConstantBufferExporter cb_exporter;
		cb_exporter.export_constant_buffers(
			cmd_list,
			stages,
			layout,
			param_index,
			update,
			a_shared.ps_hash_for_cb_dump,
			a_shared.count_display,
			true // true = export en .txt (lisible), false = .bin (binaire)
		);
		
	}

#endif
}

//*******************************************************************************************************
// injection of texture 
void get_texture(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
#if _DEBUG_LOGS
	//log infos
	log_push_descriptor(stages, layout, param_index, update);
#endif


	device* dev = cmd_list->get_device();

	// get mask from ext plane  PS, filter by the number of resource
	if (a_shared.last_feature == Feature::GetStencil && update.count == 6)
	{
		// in some case the resource view handle is null, skip these cases
		// as this is a depth stencil texture, 2 resource view will be created
		if (reinterpret_cast<const reshade::api::resource_view*>(update.descriptors)[3].handle != 0)
		{

			// to retrieve infos for pushing texture in bind_pipeline
			current_DepthStencil_handle = copy_texture_from_desc(cmd_list, stages, layout, param_index, update, 3, "DepthStencil");
		}

	}


	// stop tracking
	track_for_texture = false;
}


#ifdef _DEBUG
extern "C" {
#endif
	// *******************************************************************************************************
	// on_push_descriptors() : to be monitored in order to copy texture and engage effect
	// called a lot !
	VREM_EXPORT  void vrem_on_push_descriptors(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
	{

#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_push_descriptors started");
#endif
#ifdef _DEBUG
		// for hunting and texture / CB dump
		dump_text_cb(cmd_list, stages, layout, param_index, update);
#endif
		// to limit processing only when a tracking is setup n;
		if (!track_for_texture && !a_shared.render_technique && (((a_shared.cb_inject_values.hazeReduction == 1.0 && a_shared.cb_inject_values.gCockpitIBL == 1.0) && a_shared.VREM_setting[SET_MISC]) || !a_shared.VREM_setting[SET_MISC]) )
		{
#if _DEBUG_CRASH 
			reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_push_descriptors ended (no track)");
#endif
			return;
		}

		// read CB to get original values for reflection control using cperframe CB
		if ((a_shared.cb_inject_values.hazeReduction != 1.0 || a_shared.cb_inject_values.gCockpitIBL != 1.0) && a_shared.VREM_setting[SET_MISC])
		{
			if (update.type == descriptor_type::constant_buffer && update.binding == CPERFRAME_INDEX && update.count == 1 && stages == shader_stage::pixel)
			{

				bool error = read_constant_buffer(cmd_list, update, "CPerFrame", 0, a_shared.dest_CB_array[CPERFRAME_CB_NB], CPERFRAME_SIZE);
				if (!error)
				{

					// copy original value for gAtmIntensity
					a_shared.orig_values[CPERFRAME_CB_NB][GATMINTENSITY_SAVE] = a_shared.dest_CB_array[CPERFRAME_CB_NB][FOG_INDEX];

					// copy original value for gCockpitIBL.xy
					a_shared.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_X_SAVE] = a_shared.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_X];
					a_shared.orig_values[CPERFRAME_CB_NB][GCOCKPITIBL_Y_SAVE] = a_shared.dest_CB_array[CPERFRAME_CB_NB][GCOCKPITIBL_INDEX_Y];


					a_shared.CB_copied[CPERFRAME_CB_NB] = true;
				}

			}
		}

		// display_to_use = 0 => outer left, 1 = outer right, 2 = Inner left, 3 = inner right.
		short int display_to_use = a_shared.count_display - 1;

		// render effect part 
		
		// do not engage effect if option not selected 
		if (a_shared.render_technique && a_shared.draw_passed && a_shared.VREM_setting[SET_TECHNIQUE])
		{
			render_technique(display_to_use, cmd_list);
			a_shared.render_technique = false;
		}

		// copy texture part
		if (track_for_texture && update.type == descriptor_type::shader_resource_view && stages == shader_stage::pixel)
		{
			get_texture(cmd_list, stages, layout, param_index, update);
		}

#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_push_descriptors ended");
#endif

	}
#ifdef _DEBUG
}
#endif

