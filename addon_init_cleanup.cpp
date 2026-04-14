///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  initialization & cleanup of VREM mod
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
#include <filesystem>

#include "loader_addon_shared.h"
#include "addon_logs.h"
#include "addon_objects.h"
#include "addon_functions.h"

//*******************************************************************************
// variables shared 
// with the launcher 
SharedState* g_shared_state = nullptr;
// only for this addon
addon_shared a_shared;

bool addon_init = false;

bool request_capture = false;   // demande utilisateur
bool flag_capture = false;      // capture ACTIVE (ex-capturing)
bool frame_started = false;     // au moins un bind_pipeline vu


// for logging shader_resource_view in push_descriptors() to get depthStencil 
// bool track_for_texture = false;
// current Depth Stencil handle
// uint64_t current_DepthStencil_handle = 0;
// uint64_t current_depth_handle = 0;

// track render target
// bool track_for_render_target = false;
// uint64_t current_RTV_handle;
// saved_RenderTargetView last_RTV_saved;

// to skip draw call if some shader are to be skipped
bool do_not_draw = false;


//
// structure to contain shaders/pipeline to process, regarding mod option selected
std::unordered_map<uint64_t, Shader_Definition> filtered_pipeline;

std::unordered_map<uint32_t, std::vector<uint8_t>> shader_code_cache;

#ifdef _DEBUG
extern "C" {
#endif
    //*******************************************************************************
	VREM_EXPORT void vrem_init(
        reshade::api::device* device,
        reshade::api::command_queue* queue,
        reshade::api::swapchain* swapchain,
        PersistentPipelineData* persistent_data,
        SharedState* shared_state
    )
    {
        
#if _DEBUG_CRASH		 reshade::log::message(reshade::log::level::info, "addon - vrem_init started"); 
#endif
		
		// set the g_shared_state vaiable of the addon to the variable shared by the launcher
        g_shared_state = shared_state;

		// device is null when called
		// g_shared_state->device = device;
#ifdef USE_LOGS  
        log_addon_init();
#endif
        // reshade::log::message(reshade::log::level::info, "** DCS VREM: Initialisation de l'addon...");

        // Code du DLL_PROCESS_ATTACH (not used ?)
        WCHAR buf[MAX_PATH];
        const std::filesystem::path dllPath = GetModuleFileNameW(nullptr, buf, ARRAYSIZE(buf)) ?
            buf : std::filesystem::path();
        const std::filesystem::path basePath = dllPath.parent_path();
#if _DEBUG_LOGS      
		g_shared_state->debug_log = 1;
#endif       
        // initialize addon variables 

        addon_init = true;

		// initialize hash in shader definition (for display purpose)
		for (auto& [hash, shader_def] : shader_by_hash)
		{
			shader_def.hash = hash;
		}

		// to avoid doing things before 3D rendering started
		bool cockpit_rendering_started = false;

		//display the shader list
		// display_shader_by_hash();
		
		// to request import of .cso
		a_shared.cso_imported = false;

		// to request generation of filtered pipelines
		g_shared_state->filtered_pipeline_to_setup = true;

		//to trace compilation of technique
		a_shared.technique_compiled = false;

		// some init to do once per library launch
		a_shared.flag_texture_dump = false;
		a_shared.flag_cb_dump = false;

		a_shared.VREM_setting[SET_DEFAULT] = 0;
		
		a_shared.cb_inject_values.max_display = 0;

		a_shared.technique_status_loaded = false;

		a_shared.first_PS_pipeline_handle = 0;

		a_shared.wait_for_technique = 0;

		a_shared.current_photo_number = 0;
		a_shared.max_photo_number = 0;
		a_shared.target_photo_number = 0;
		a_shared.default_photo_number = true;

		// parse the shader list to load all shader codes and store codes in shader_code_cache (if not done)
		read_all_shader_code();

		//intialize the counters
		intialize_counters();

#if _DEBUG_CRASH reshade::log::message(reshade::log::level::info, "addon - vrem_init ended");
#endif
        // reshade::log::message(reshade::log::level::info,"DCS VREM: register done...");
    }

    //*******************************************************************************
	// called when the addon is unloaded
	//*******************************************************************************
	VREM_EXPORT void vrem_cleanup(PersistentPipelineData* persistent_data)
    {
		// delete cloned pipelines => if reload .cso may have changed

#ifdef _DEBUG  		
		log_addon_cleanup_cloned();
#endif

		if (g_shared_state->device == nullptr)
		{
			log_device_null();
		}
		if (g_shared_state != nullptr)
		{
			delete_cloned_pipelines(g_shared_state->device);

			// clean the filtered shader/pipeline list => if reload shader list may have changed
#ifdef _DEBUG  
			log_addon_cleanup_filtered();
#endif 
			filtered_pipeline.clear();

			// clean the shader code cache
#ifdef _DEBUG  
			log_cleanup_shader_code();
#endif
			shader_code_cache.clear();

			// to reload filtered and cloned
			g_shared_state->filtered_pipeline_to_setup = true;

			// delete texture resources created for mod
#ifdef _DEBUG  
			log_cleanup_texture();
#endif
			delete_texture_resources(g_shared_state->device);
			a_shared.copied_textures.clear();
			pure_technique_vector();

			g_shared_state->preprocessor_exported = false;

			g_shared_state->PSshader_index = 0;						  
			g_shared_state->PSshader_list.clear();

			a_shared.texture_to_read = true;

		}

    }
#ifdef _DEBUG
}
#endif
