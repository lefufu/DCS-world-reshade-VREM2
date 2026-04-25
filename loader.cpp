///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// Reshade addon launcher 
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

#include <imgui.h>
#include "addon_objects.h"
#include "CDataFile.h"
#include "loader_addon_shared.h"
#include "loader_on_event.hpp"



//local variables
VREMHotReloader* g_reloader = nullptr;
static bool g_reload_in_progress = false;
static bool g_reload_requested = false;

SharedState g_shared_state_l;
#ifdef _DEBUG
//not used, just ot avoid link issue
struct addon_shared a_shared; 
#endif 

//****************************************************************
// addon infos
extern "C" __declspec(dllexport) const char* NAME = "DCS VREM2";
extern "C" __declspec(dllexport) const char* DESCRIPTION = 
#if _DEBUG
"VR Enhancer Mod2 for DCS 1.0 (DEBUG - Hot Reload Enabled).";
#else
"VR Enhancer Mod2 for DCS v 0.5 beta (RELEASE).";
#endif

#ifndef _DEBUG
extern void vrem_on_reshade_present(reshade::api::effect_runtime* runtime);
extern void vrem_init(reshade::api::device* device,reshade::api::command_queue* queue, reshade::api::swapchain* swapchain,PersistentPipelineData* persistent_data,SharedState* shared_state);
extern void vrem_cleanup(PersistentPipelineData* persistent_data);
#endif


//****************************************************************
// GUI for reshade addon
static void draw_settings(reshade::api::effect_runtime* runtime)
{
#ifdef _DEBUG
    // reload button
    // deactivate button during reloard
    if (g_reload_in_progress) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button("Reload dll"))
    {
        g_reload_requested = true;
        g_reload_in_progress = true;
    }

    if (g_reload_in_progress) {
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Reloading...");
    }
    ImGui::Separator();

	// debug flag (unused for now)
    ImGui::Checkbox("Debug messages", &g_shared_state_l.debug);

    // save pictures
    ImGui::Checkbox("Save textures in capture frame", &g_shared_state_l.save_texture_flag);
    ImGui::Checkbox("Save constant buffer in capture frame", &g_shared_state_l.save_cb_flag);
    ImGui::Checkbox("Save render target in capture frame", &g_shared_state_l.save_rt_flag);
    

    // shader hunter mode : display full framelogs and filter 1 PS
    ImGui::Checkbox("Shader hunter", &g_shared_state_l.shader_hunter);

	//shader hunter mode
    ImGui::RadioButton("Disable", &g_shared_state_l.color_PS, 0); ImGui::SameLine();
    ImGui::RadioButton("Color", &g_shared_state_l.color_PS, 1);

    //capture a fame 
    if (ImGui::Button("Capture frame"))
    {
        g_shared_state_l.button_capture = true;
		g_shared_state_l.PSshader_index = 0;									
    }
    else
    {
        g_shared_state_l.button_capture = false;
    }


    ImGui::Text("Number of pipeline: %d", g_shared_state_l.PSshader_list.size() - 1);
    ImGui::Text("current hunted pipeline index");
    if (ImGui::Button("-"))
    {
        g_shared_state_l.PSshader_index--;
        if (g_shared_state_l.PSshader_index < 0)  g_shared_state_l.PSshader_index = g_shared_state_l.PSshader_list.size() - 1;
    }
    ImGui::SameLine();
    ImGui::InputInt("", &g_shared_state_l.PSshader_index, 0, static_cast<int>(g_shared_state_l.PSshader_list.size()) - 1);
    ImGui::SameLine();
    if (ImGui::Button("+"))
    {
        g_shared_state_l.PSshader_index++;
        if (g_shared_state_l.PSshader_index > g_shared_state_l.PSshader_list.size() - 1) g_shared_state_l.PSshader_index = 0;
    }

    // display info on pipeline
    if (g_shared_state_l.PSshader_list.size() > 0)
    {
        uint64_t handle = g_shared_state_l.PSshader_list[g_shared_state_l.PSshader_index];
        ImGui::Text("Hunted pipeline handle: 0x%llx", handle);
        uint32_t hash = 0;
		save_pipeline* p_found = nullptr;
        if (handle != 0)
        {

            for (auto& p : g_shared_state_l.VREM_pipelines.saved_pipelines) {
                if (p.pipeline.handle == handle) {
                    hash = p.hash[0];
                    p_found = &p;
                }
            }
        }
        ImGui::SameLine();
        ImGui::Text("    first hash = 0x%llx", hash);
        //ImGui::Text("    first hash = 0x%llx", p_found->hash[0]);
        static char buffer[256] = "marking...";
        ImGui::InputText("Comment", buffer, sizeof(buffer));

        if (ImGui::Button("write shader in log"))
        {
            log_shader_marked(p_found, buffer);
        }
    }
#else
    // Mode Release : indiquer que le hot reload est désactivé
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "RELEASE MODE - Hot reload & debug messages disabled");
    ImGui::Separator();
#endif
 
    // to be used only if 2D, otherwise technique control is to be applied
    if (g_shared_state_l.technique_enabled && !g_shared_state_l.is_VR)
    {
        // display technique options
        ImGui::Separator();
        ImGui::Text("!! To be used in 2D only !! ");
        ImGui::Text("Integrate the technique into the game's render pipeline and not on the last draw");
        ImGui::Text("(to avoid processing of GUI elements or mask displayed on GUI elements : map,..) ");
        ImGui::Text("tick option 'Technique into the game's render pipeline' in VREM Mod settings ");
        ImGui::Separator();
    /*if (ImGui::Checkbox("Enable non global technique ", &g_shared_state_l.technique_enabled))
    {
        // save technique status in file (in get_settings_from_uniform)
        g_shared_state_l.request_update_file = true;
        reshade::log::message(reshade::log::level::info, "****** loader - change flag for technique => request save *******");
    } */


        // handle double injection issue : if tehcnique is active for end of draw it will not be displayed into the game's render pipeline
        if (ImGui::Checkbox("Disable technique injection if effect is active (when configuring technique)", &g_shared_state_l.no_double))
        {
            // save technique status in file (in get_settings_from_uniform)
            g_shared_state_l.request_update_file = true;
        }
        
        for (auto& entry : g_shared_state_l.technique_vector)
        {

            // bool current_status = runtime->get_technique_state(entry.technique);

            ImGui::BeginDisabled(entry.reshade_technique_status);

            // Affiche "effect_name :: technique_name"
            std::string label = entry.eff_name + " :: " + entry.name;

            if (ImGui::Checkbox(label.c_str(), &entry.VR_technique_status))
            {
                // save technique status in file (in get_settings_from_uniform)
                g_shared_state_l.request_update_file = true;
                reshade::log::message(reshade::log::level::info, "****** loader - change status of technique => request save *******");
            }
            ImGui::EndDisabled();
        }
    }
    else if (g_shared_state_l.technique_enabled && g_shared_state_l.is_VR)
    {
        ImGui::Separator();
        ImGui::Text("'Technique into the game's render pipeline' in VREM Mod settings activated");
        ImGui::Text("activate the technique from 'home' menu for VR to see them in VR and in mirror window");
    }
}

// *******************************************************************************************************
/// Called for every technique change => set refresh of technique
/// 
bool on_reshade_set_technique_state(effect_runtime* runtime, effect_technique technique, bool enabled) {

    // request update of shader if not VR only
    if (g_shared_state_l.technique_enabled)
        // shared_data.button_technique = true;
        //SHOULD SET STATE OF TECHNIQUE HERE => NEED TO HAVE A MAP INSTEAD OF A VECTOR TO FIND THE TECHNIQUE IN THE VECTOR AND UPDATE IT
         for (auto& entry : g_shared_state_l.technique_vector)
         {
             if (entry.technique == technique)
             {
                 entry.reshade_technique_status = enabled;
             }
		 }

    // let things as requested
    return false;
}
//****************************************************************
// to trace overalay is opened
static bool reshade_open_overlay(reshade::api::effect_runtime* runtime, bool open, reshade::api::input_source source)
{

    //trace reshade GUI is active => refresh mod settings with uniforms of VREM_settings

    g_shared_state_l.overlay_is_open = open;

    return false;
}

//*******************************************************************************
// handle button press plus call vrem "on_present" function

static void on_reshade_present(effect_runtime* runtime) {
#ifdef _DEBUG
    if (g_reloader) {
        // Vérifier demande GUI
        if (g_reload_requested) {
            g_reload_requested = false;
            g_reloader->manual_reload();
            g_reload_in_progress = false;  // Réinitialiser après le rechargement
        }
        
        if (g_reloader->get_functions().on_reshade_present) {
            typedef void (*Func)(effect_runtime*);
            ((Func)g_reloader->get_functions().on_reshade_present)(runtime);
        }
        
    }
#else
    vrem_on_reshade_present(runtime);
#endif
}

//*******************************************************************************
// declaration of addon 

BOOL APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID) 
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
    {
        
        if (!reshade::register_addon(hModule)) {
            return FALSE;
        }

        // Initialize addon
        WCHAR buf[MAX_PATH];
        const std::filesystem::path dllPath = GetModuleFileNameW(hModule, buf, ARRAYSIZE(buf)) ?
            buf : std::filesystem::path();
        const std::filesystem::path basePath = dllPath.parent_path();
        const std::filesystem::path addonPath = basePath / VREM_ADDON_NAME;

        

        // intialization is doing mapping between exported VREM function and the function used in "register_event"
        // if fuction are not exported in addon there will be no call
//#ifdef _DEBUG

        g_reloader = new VREMHotReloader(addonPath.string());
        //copy path for saving textures or CB in addon
        wcscpy_s(g_shared_state_l.g_vrem_base_path, basePath.wstring().c_str());
//#endif
#ifdef _DEBUG
        reshade::log::message(reshade::log::level::info, "VREM Loader: DEBUG MODE - Hot reload enabled");
        // register event : local mapping
        reshade::register_overlay(nullptr, draw_settings);
        reshade::register_event<reshade::addon_event::reshade_present>(on_reshade_present);
        reshade::register_event<reshade::addon_event::reshade_open_overlay>(reshade_open_overlay);
        reshade::register_event<reshade::addon_event::init_pipeline>(on_init_pipeline);
        // !!! warning !!! to optimize performance there is a filtering setup in on_bind_pipeline() to limit processing to ALLOWED_STAGES
        reshade::register_event<reshade::addon_event::bind_pipeline>(on_bind_pipeline);
        reshade::register_event<reshade::addon_event::init_pipeline_layout>(on_init_pipeline_layout);
        reshade::register_event<reshade::addon_event::draw>(on_draw);
        reshade::register_event<reshade::addon_event::draw_indexed>(on_draw_indexed);
        reshade::register_event<reshade::addon_event::draw_or_dispatch_indirect>(on_draw_indirect);
        reshade::register_event<reshade::addon_event::push_descriptors>(on_push_descriptors);
        reshade::register_event<reshade::addon_event::create_pipeline>(on_create_pipeline);
        reshade::register_event<reshade::addon_event::bind_render_targets_and_depth_stencil>(on_bind_render_targets);
        reshade::register_event<reshade::addon_event::reshade_overlay>(on_reshade_overlay);
        reshade::register_event<reshade::addon_event::reshade_reloaded_effects>(on_reshade_reloaded_effects);
        reshade::register_event<reshade::addon_event::init_swapchain>(on_init_swapchain);

#else
        // Mode Release : pas de reloader
        reshade::log::message(reshade::log::level::info, "VREM Loader: RELEASE MODE - Direct calls");

        vrem_init(nullptr, nullptr, nullptr, nullptr, &g_shared_state_l);

        // register event : local mapping
        reshade::register_overlay(nullptr, draw_settings);
        reshade::register_event<reshade::addon_event::reshade_present>(vrem_on_reshade_present);
        reshade::register_event<reshade::addon_event::reshade_open_overlay>(reshade_open_overlay);
        reshade::register_event<reshade::addon_event::init_pipeline>(vrem_on_init_pipeline);
        // !!! warning !!! to optimize performance there is a filtering setup in on_bind_pipeline() to limit processing to ALLOWED_STAGES
        reshade::register_event<reshade::addon_event::bind_pipeline>(vrem_on_bind_pipeline);
        reshade::register_event<reshade::addon_event::init_pipeline_layout>(vrem_on_init_pipeline_layout);
        reshade::register_event<reshade::addon_event::draw>(vrem_on_draw);
        reshade::register_event<reshade::addon_event::draw_indexed>(vrem_on_draw_indexed);
        reshade::register_event<reshade::addon_event::draw_or_dispatch_indirect>(vrem_on_drawOrDispatch_indirect);
        reshade::register_event<reshade::addon_event::push_descriptors>(vrem_on_push_descriptors);
        reshade::register_event<reshade::addon_event::create_pipeline>(vrem_on_create_pipeline);
        reshade::register_event<reshade::addon_event::bind_render_targets_and_depth_stencil>(vrem_on_bind_render_targets_and_depth_stencil);
        reshade::register_event<reshade::addon_event::reshade_overlay>(vrem_on_reshade_overlay); 
        reshade::register_event<reshade::addon_event::reshade_reloaded_effects>(vrem_on_reshade_reloaded_effects); 
#endif
		reshade::register_event<reshade::addon_event::reshade_set_technique_state>(on_reshade_set_technique_state);																										   
        //reshade::register_event<reshade::addon_event::destroy_pipeline>(on_destroy_pipeline); 
        // register event : call of addon exported functions (see loader_on_event.hpp)
        // reshade::register_event<reshade::addon_event::init_swapchain>(on_init_swapchain);
        // // reshade::register_event<reshade::addon_event::reshade_present>(on_reshade_present);
        // // reshade::register_event<reshade::addon_event::init_pipeline>(on_after_create_pipeline);
        //reshade::register_event<reshade::addon_event::reshade_set_technique_state>(on_reshade_set_technique_state);
 
        break;
        return TRUE;
    }
    case DLL_PROCESS_DETACH:
    {

#ifdef _DEBUG
        // unregister event : local mapping
        reshade::unregister_overlay(nullptr, draw_settings);
        reshade::unregister_event<reshade::addon_event::reshade_present>(on_reshade_present);
        reshade::unregister_event<reshade::addon_event::reshade_open_overlay>(reshade_open_overlay);
        reshade::unregister_event<reshade::addon_event::init_pipeline>(on_init_pipeline);
        reshade::unregister_event<reshade::addon_event::bind_pipeline>(on_bind_pipeline);
        reshade::unregister_event<reshade::addon_event::init_pipeline_layout>(on_init_pipeline_layout);
        reshade::unregister_event<reshade::addon_event::draw>(on_draw);
        reshade::unregister_event<reshade::addon_event::draw_indexed>(on_draw_indexed);
        reshade::unregister_event<reshade::addon_event::draw_or_dispatch_indirect>(on_draw_indirect);
        reshade::unregister_event<reshade::addon_event::push_descriptors>(on_push_descriptors);
        reshade::unregister_event<reshade::addon_event::create_pipeline>(on_create_pipeline);
        reshade::unregister_event<reshade::addon_event::bind_render_targets_and_depth_stencil>(on_bind_render_targets);
        reshade::unregister_event<reshade::addon_event::reshade_overlay>(on_reshade_overlay);
        reshade::unregister_event<reshade::addon_event::reshade_reloaded_effects>(on_reshade_reloaded_effects);
        reshade::unregister_event<reshade::addon_event::init_swapchain>(on_init_swapchain);
#else
        //cleaning of addon variables if no hot reload
        vrem_cleanup(nullptr);
        // register event : local mapping
        reshade::register_overlay(nullptr, draw_settings);
        reshade::unregister_event<reshade::addon_event::reshade_present>(vrem_on_reshade_present);
        reshade::unregister_event<reshade::addon_event::reshade_open_overlay>(reshade_open_overlay);
        reshade::unregister_event<reshade::addon_event::init_pipeline>(vrem_on_init_pipeline);
        reshade::unregister_event<reshade::addon_event::bind_pipeline>(vrem_on_bind_pipeline);
        reshade::unregister_event<reshade::addon_event::init_pipeline_layout>(vrem_on_init_pipeline_layout);
        reshade::unregister_event<reshade::addon_event::draw>(vrem_on_draw);
        reshade::unregister_event<reshade::addon_event::draw_indexed>(vrem_on_draw_indexed);
        reshade::unregister_event<reshade::addon_event::draw_or_dispatch_indirect>(vrem_on_drawOrDispatch_indirect);
        reshade::unregister_event<reshade::addon_event::push_descriptors>(vrem_on_push_descriptors);
        reshade::unregister_event<reshade::addon_event::create_pipeline>(vrem_on_create_pipeline);
        reshade::unregister_event<reshade::addon_event::bind_render_targets_and_depth_stencil>(vrem_on_bind_render_targets_and_depth_stencil);
        reshade::unregister_event<reshade::addon_event::reshade_overlay>(vrem_on_reshade_overlay);
        reshade::unregister_event<reshade::addon_event::reshade_reloaded_effects>(vrem_on_reshade_reloaded_effects);
#endif
        reshade::unregister_event<reshade::addon_event::reshade_set_technique_state>(on_reshade_set_technique_state);
		//reshade::unregister_event<reshade::addon_event::destroy_pipeline>(on_destroy_pipeline);
        // unregister event : call of addon exported functions (see loader_on_event.hpp)
        // reshade::unregister_event<reshade::addon_event::init_swapchain>(on_init_swapchain);
        // reshade::unregister_event<reshade::addon_event::reshade_present>(on_reshade_present);
        //reshade::unregister_event<reshade::addon_event::reshade_set_technique_state>(on_reshade_set_technique_state);
        // reshade::unregister_event<reshade::addon_event::init_pipeline>(on_after_create_pipeline);


#ifdef _DEBUG
        if (g_reloader) {
            delete g_reloader;
            g_reloader = nullptr;
        }
#endif
        // clean remanents objects 
        delete_persistant_objects();

        reshade::unregister_addon(hModule);
        break;
    }
    }
    return TRUE;
}