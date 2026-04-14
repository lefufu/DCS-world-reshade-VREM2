
///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// on_bind_render_targets_and_depth_stencil : store render target for technique rendering
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

#ifdef _DEBUG
extern "C" {
#endif
	//*******************************************************************************
	VREM_EXPORT void vrem_on_bind_render_targets_and_depth_stencil(command_list* cmd_list, uint32_t count, const resource_view* rtvs, resource_view dsv)
	{

		//reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_bind_render_targets_and_depth_stencil started");
			
#ifdef _DEBUG
		// Save the first render target view for technique rendering if not already done
		if (count > 0 && rtvs != nullptr && flag_capture)
		{
			a_shared.g_current_rtv = rtvs[0];
		}

#endif
		// copy render target if tracking	
		// if (a_shared.track_for_render_target && a_shared.count_display > -1 && !a_shared.cb_inject_values.mapMode && count > 0 && (a_shared.VREM_setting[SET_TECHNIQUE]))
		if (a_shared.track_for_render_target && a_shared.count_display > -1 &&  count > 0 && a_shared.VREM_setting[SET_TECHNIQUE])
		{
			//hanlde cases with rendere target null
			if (rtvs[0].handle != 0)
			//if (rtvs[0].handle != 0 && dsv != 0)
			{

				saved_RenderTargetView RTView;
				// only first render target view to get
				device* dev = cmd_list->get_device();
				resource scr_resource = dev->get_resource_from_view(rtvs[0]);
				resource_desc src_resource_desc = dev->get_resource_desc(scr_resource);

				// get information of render target
				last_RTV_saved.copied = true;
				last_RTV_saved.RV = rtvs[0];
				last_RTV_saved.width = src_resource_desc.texture.width;
				last_RTV_saved.height = src_resource_desc.texture.height;
#if _DEBUG_LOGS  			
				log_renderTarget_depth(count, rtvs, dsv, cmd_list, current_RTV_handle);
#endif			
			}
			else
				log_empy_render_target();
		}

		// log for shader hunting
		if (g_shared_state->shader_hunter)
		{
#if _DEBUG_LOGS  
			log_renderTarget_depth(count, rtvs, dsv, cmd_list, current_RTV_handle);
#endif
		}

		//reshade::log::message(reshade::log::level::info, "***** addon - vrem_on_bind_render_targets_and_depth_stencil ended");
	}
#ifdef _DEBUG
}
#endif