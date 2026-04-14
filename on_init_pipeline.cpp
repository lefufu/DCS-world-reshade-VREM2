// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// on_init_pipeline() :called once per pipeline, create a persistant list on pipeline, 
// that will be used at each reload of the addon.
// add the pipeline hash in the list of pipeline to be processed if relevant
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

static thread_local std::vector<std::vector<uint8_t>> shader_code;

#ifdef _DEBUG
extern "C" {
#endif

	/* void create_vrem_cb(reshade::api::device* device)
	{
		if (a_shared.res_CB13.handle != 0)
			return;  // Déjà créé

		reshade::api::resource_desc desc = {};
		desc.type = reshade::api::resource_type::buffer;
		desc.buffer.size = CBSIZE;
		desc.usage = reshade::api::resource_usage::copy_dest;
		desc.heap = reshade::api::memory_heap::gpu_to_cpu;

		bool result = device->create_resource(
			desc,
			nullptr,
			reshade::api::resource_usage::constant_buffer,
			&a_shared.res_CB13
		);

		if (result) {
			if (g_shared_state->debug)
			{
				std::stringstream s;
				s << "*** creation of cb13 resource " << std::hex << a_shared.res_CB13.handle;
				reshade::log::message(reshade::log::level::info, s.str().c_str());
			}
		}
		else {
			reshade::log::message(reshade::log::level::error, "Error in CB13 resource creation ");
		}
	}
	*/

	//*******************************************************************************
	VREM_EXPORT void vrem_on_init_pipeline(device* device, pipeline_layout layout, uint32_t subobjectCount, const pipeline_subobject* subobjects, pipeline pipelineHandle)
	{
		// save needed pipelines as init_pipeline is called once per game launch
		// only shader types defined in ALLOWED_SHADERS are saved

		save_pipeline_in_list(device, layout, subobjectCount, subobjects, pipelineHandle);

		// if init_pipeline after creation of filtered pipeline list, request an update
		if (!g_shared_state->filtered_pipeline_to_setup  && a_shared.VREM_setting[SET_DEFAULT]) {
			g_shared_state->filtered_pipeline_to_setup = true;
		}

		g_shared_state->DX11_layout = layout;
	}
#ifdef _DEBUG
}
#endif
