///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  VREM mod "on present" and "on_reload" call => read settings from uniform
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
#include "addon_chrono.hpp"

using namespace reshade::api;

// *******************************************************************************************************
// Initialize counters 
//
void intialize_counters()
{
    // to know the current display : 2D, VR standard: left/right, VR quad view : left eye outer + inner + right eye outer+inner
    a_shared.count_display = 0;

    a_shared.cb_inject_values.mapMode = 1.0;
    do_not_draw = false;
    //a_shared.cb_inject_values.GUItodraw = 0.0;

    a_shared.render_technique = false;
    a_shared.track_for_render_target = false;

    a_shared.last_feature = Feature::Null;

	a_shared.draw_counter = 0;

    a_shared.last_pipeline_hash_PS = 0;

    a_shared.not_track_mask_anymore = false;

    a_shared.wait_for_technique = 0;

    a_shared.current_photo_number = 0;

    // initialize flags for texture copy
    current_DepthStencil_handle = 0;
    for (auto& [handle, ds_copy] : a_shared.copied_textures) {
        ds_copy.copied = false;
    }


}


// *******************************************************************************************************
// handle all key press outside imgui shortcut
//
void handle_keypress(effect_runtime* runtime)
{

    /*
    //example on handling "hold" feature => effect is triggered only when key is pressed
    if (runtime->is_key_down(VK_F1) && runtime->is_key_down(VK_SHIFT))
    {
        shared_data.cb_inject_values.testGlobal = 1.0;
    }
    else
    {
        shared_data.cb_inject_values.testGlobal = 0.0;
    }


    //example on toggling on/off 
    // default ALT+F6 toggle on/off video in IHADSS
    if (runtime->is_key_pressed(shared_data.vk_TADS_video) && runtime->is_key_down(shared_data.vk_TADS_video_mod))
    {
        // Toggle the value of disable_video_IHADSS between 0.0 and 1.0
        if (shared_data.cb_inject_values.disable_video_IHADSS == 1.0)
        {
            shared_data.cb_inject_values.disable_video_IHADSS = 0.0;
        }
        else if (shared_data.cb_inject_values.disable_video_IHADSS == 0.0)
        {
            shared_data.cb_inject_values.disable_video_IHADSS = 1.0;
        }
    }
*/



}


#ifdef _DEBUG
extern "C" {
#endif
    //*******************************************************************************
    VREM_EXPORT void vrem_on_reshade_present(reshade::api::effect_runtime* runtime)
    {
       
#if _DEBUG_CRASH 
        reshade::log::message(reshade::log::level::info, "addon - vrem_on_reshade_present started");
#endif
      
        //get device 
        reshade::api::device* device = runtime->get_device();
        g_shared_state->device = device;

        // initialize counter to identfiy what to do when in the next frame
        intialize_counters();

		// to get VREM settings => read from uniform values (if overlay is open or at first init)
        if (g_shared_state->overlay_is_open || addon_init == true) {
            //get_settings_from_uniforms(runtime);
            get_uniform_and_techniques(runtime);
            addon_init = false;
        }
  
        //initialize pipelines stuff once reshade has fully loaded
        if (a_shared.VREM_setting[SET_DEFAULT])
        { 
            // wait as much as possible to generate filtered shader list and clone pipelines if needed
            if (g_shared_state->filtered_pipeline_to_setup) {

                // create all pipeline_layouts for pushing dedicated CB (if not created)
                create_all_modified_CB_layout(device);
                
                // create all pipelines layput for pushing dedicated RV (if not created)
                create_RV_pipeline_layout(device);


                g_shared_state->filtered_pipeline_to_setup = setup_filtered_pipelines(g_shared_state->device, runtime);
            }

            //read texture from file
            if (a_shared.texture_to_read)
            {
                a_shared.texture_to_read = false;
                read_textures(device);
            }
        }
        else
        {
#if _DEBUG_LOGS
            log_waiting_setting();
#endif
        } 

        //------------------------------------
        // handle key press to toggle features 
        handle_keypress(runtime);

        //------------------------------------
        //handle stopwatch update

#ifdef _DEBUG

		// handle capture frame button press 
        if (flag_capture && frame_started)
        {
            flag_capture = false;
            frame_started = false;
#if _DEBUG_LOGS
            log_end_capture_frame();
#endif
        }

        // Détection front bouton
        static bool last_button = false;
        bool button = g_shared_state->button_capture;

        if (button && !last_button)
        {
            request_capture = true;
        }
        last_button = button;

        if (request_capture)
        {
            request_capture = false;
            flag_capture = true;
            frame_started = true;
            a_shared.flag_texture_dump = false;
            a_shared.flag_cb_dump = false;
#if _DEBUG_LOGS
            log_start_capture_frame();
            log_shader_def_list();
#endif


			// if shader hunter mode : clean list of PS
			if (g_shared_state->shader_hunter)  g_shared_state->PSshader_list.clear();

        } 
#endif

		//force capture for testing
        // flag_capture = true;

        reshade::log::message(reshade::log::level::info, "addon - vrem_on_reshade_present ending");

#if _DEBUG_CRASH  
        reshade::log::message(reshade::log::level::info, "addon - vrem_on_reshade_present ending");
#endif
    }

#ifdef _DEBUG
}
#endif