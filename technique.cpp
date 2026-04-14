///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// technique : all function to handle techniques
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

#include <cmath>
//#include <imgui.h>
#include <reshade.hpp>

#include "loader_addon_shared.h"
#include "addon_functions.h"
#include "addon_objects.h"
#include "addon_logs.h"
#include "CDataFile.h"

static size_t g_charBufferSize;
static char g_charBuffer[CHAR_BUFFER_SIZE];
static bool technique_status;

extern SharedState* g_shared_state;

CDataFile technique_iniFile;

//*******************************************************************************************************
/// <summary>
// roundToDecimal() : to avoid re writing preprocessor variable because rouding of value as string in reshade
/// </summary>

float roundToDecimal(float value, int decimals) {
    float factor = std::pow(10.0f, decimals);
    return std::round(value * factor) / factor;
}

//*******************************************************************************************************
/// <summary>
// default_preprocessor() : create preprocessor definition if different of value
/// </summary>
int default_preprocessor(effect_runtime* runtime, std::string name, float defaultValue, bool update, short int display_to_use)
{
    bool exists;
    char value[30];
    size_t  size = sizeof(value);
    float parsed = 0.0;

    int result = 0;

    exists = runtime->get_preprocessor_definition(name.c_str(), value, &size);



    if (exists)
    {
        // update preprocessor definition value only if requested (not needed to check existence)
        float parsed = std::strtof(value, nullptr);
        float rounded = roundToDecimal(defaultValue, 6);

        if ((parsed != rounded) && update)
        {
            runtime->set_preprocessor_definition(name.c_str(), std::to_string(rounded).c_str());
            result = 1;
#if _DEBUG_LOGS
            log_preprocessor(name, defaultValue, update, exists, parsed, false, 2, display_to_use);
#endif
        }
        else
        {
#if _DEBUG_LOGS
            log_preprocessor(name, defaultValue, update, exists, parsed, false, 3, display_to_use);
#endif
        }

    }
    else
    {
        // create preprocessor definition
        runtime->set_preprocessor_definition(name.c_str(), std::to_string(defaultValue).c_str());
        result = 1;
#if _DEBUG_LOGS
        log_preprocessor(name, defaultValue, update, exists, parsed, false, 1, display_to_use);
#endif
    }

    return result;

}

//*******************************************************************************************************
/// <summary>
// default_preprocessor() : initialize preprocess variable to avoid compil error when DCS launched for first time after mod install, not working as get_preprocessor_definition() does always return false...
/// </summary>

void init_preprocess(effect_runtime* runtime)
{
    // if (!shared_data.init_preprocessor && shared_data.effects_feature)
    if (!a_shared.init_preprocessor)
    {

        a_shared.init_preprocessor = true;
        int check = 0;
        g_shared_state->runtime = runtime;

        check += default_preprocessor(runtime, "MSAAX", 1.0, false, -1);
        check += default_preprocessor(runtime, "MSAAY", 1.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_WIDTH", 1920.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_HEIGHT", 1080.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_RCP_WIDTH", 1.0/1920, false, -1);
        check += default_preprocessor(runtime, "BUFFER_RCP_HEIGHT", 1.0 / 1080, false, -1);
        /*
        check += default_preprocessor(runtime, "BUFFER_WIDTH_QVIN", 1920.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_HEIGHT_QVIN", 1080.0, false, -1);
        check += default_preprocessor(runtime, "BUFFER_RCP_WIDTH_QVIN", 1.0 / 1920, false, -1);
        check += default_preprocessor(runtime, "BUFFER_RCP_HEIGHT_QVIN", 1.0 / 1080, false, -1);
        */

        // set one technique to activate pre process


        //save check done
        // saveShaderTogglerIniFile();
    }
}



// *******************************************************************************************************
/// <summary>
/// save technique status in the ini file
/// </summary>
/// 
void save_technique_status(std::string technique_name, std::string effect_name, bool VR_technique_status, int quad_view_target)
{
    technique_iniFile.SetBool(technique_name, VR_technique_status, "", "technique");
}

// *******************************************************************************************************
/// <summary>
/// save all technique status in the ini file
/// </summary>
/// 
void save_all_technique_status()
{
        
    for (auto& entry : g_shared_state->technique_vector)
    {
        save_technique_status(entry.name, entry.eff_name, entry.VR_technique_status, 0);

    }

    technique_iniFile.SetBool("technique_enabled", g_shared_state->technique_enabled, "", "technique");
    technique_iniFile.SetBool("no_double", g_shared_state->no_double, "", "technique");

    technique_iniFile.SetFileName(technique_iniFileName);

    technique_iniFile.Save();
}
// *******************************************************************************************************
/// <summary>
/// read the technique status in the .ini file
/// </summary>
bool read_technique_status_from_file(std::string name)
{
 
    bool status = false;

    /*if (name == VR_ONLY_EFFECT)
        // do not use this effect for VR
        status = false;
    else*/
        status = technique_iniFile.GetBool(name, "technique");

    return status;

}

// *******************************************************************************************************
/// <summary>
/// purge technique vector
/// </summary>
void pure_technique_vector()
{
    // no need to clear the std::vector<uniform_mapping>
    g_shared_state->technique_vector.clear();
}

// *******************************************************************************************************
/// <summary>
/// enumerate technique : call a function for all techniques
/// </summary>
void enumerateTechniques(effect_runtime* runtime)
{
    
    //if (a_shared.VREM_setting[SET_TECHNIQUE])
	if (g_shared_state->technique_enabled)
    {
        //purge the technique vector
        pure_technique_vector();

        // init flags for texture or uniform injection
        a_shared.uniform_needed = false;
        //shared_data.texture_needed = false;

        // load the technique file, as status of technique are not relevant
        technique_iniFile.Load(technique_iniFileName);


        // Pass the logging function as the callback
        runtime->enumerate_techniques(nullptr, [](effect_runtime* rt, effect_technique technique) {

            // Buffer size definition
            g_charBufferSize = CHAR_BUFFER_SIZE;

            // Get technique name
            rt->get_technique_name(technique, g_charBuffer, &g_charBufferSize);
            std::string name(g_charBuffer);

            technique_status = rt->get_technique_state(technique);

            //do not handle the technique used for VREM settings
            if ((name + ".fx") != VREM_SETTINGS_NAME)
            {
                // Get effect name
                g_charBufferSize = CHAR_BUFFER_SIZE;
                rt->get_technique_effect_name(technique, g_charBuffer, &g_charBufferSize);
                std::string eff_name(g_charBuffer);
                //technique_status = rt->get_technique_state(technique);

                // add technique in vector if active
                //if (technique_status)
                {

                    //check if shader is containing a VREM texture (¨DEPTH' or 'STENCIL') or other options in GUI that need stencil
                    bool has_depth_or_stencil = false;
                    if (rt->find_texture_variable(g_charBuffer, DEPTH_NAME) != 0 || rt->find_texture_variable(g_charBuffer, STENCIL_NAME) != 0)
                    {
                        has_depth_or_stencil = true;
                        a_shared.texture_needed = true;
                    }

                    std::vector<uniform_mapping> tech_uniforms;

                    //check if shader is containing VREM uniform
                    for (const auto& [unif_name, value] : var_mapping)
                    {
                        reshade::api::effect_uniform_variable unif = rt->find_uniform_variable(g_charBuffer, unif_name.c_str());
                        if (unif != 0)
                        {
							//the uniform exist in the shader, we need to add the info in the vector to update it later
                            tech_uniforms.push_back({ unif_name, value, unif });
                        }
                    }

                    // add the technique in the vector
                    bool VRtechnique_status = read_technique_status_from_file(name);
                    //for support of QV in other mod...
					int QV_target = 0;

                    g_shared_state->technique_vector.push_back({ technique, name, eff_name, VRtechnique_status, technique_status, tech_uniforms, QV_target });
                    //g_shared_state->technique_vector.push_back({ technique, name, eff_name , VRtechnique_status, technique_status, QV_target });
#if _DEBUG_LOGS
                    //log 
                    //log_technique_info(rt, technique, name, eff_name, VRtechnique_status, technique_status, QV_target, has_depth_or_stencil, tech_uniforms);
#endif

                }
            }
            else if (!technique_status)
            {
				// ensure the settings technique is enabled
                rt->set_technique_state(technique, true);

            }
  
            });
    }
}

// *******************************************************************************************************
/// <summary>
/// Called for every technique change => set refresh of technique
/// </summary>
/// 

/*
bool onReshadeSetTechniqueState(effect_runtime* runtime, effect_technique technique, bool enabled) {

    // request update of shader if not VR only
    if (!shared_data.VRonly_technique)
        shared_data.button_technique = true;

    // let things as requested
    return false;
}
*/

// *******************************************************************************************************
/// <summary>
/// Disable all techniques
/// </summary>
/// 
/*
void disableAllTechnique(bool save_flag) {

    // disable all active techniques
    for (int i = 0; i < shared_data.technique_vector.size(); ++i)
        shared_data.runtime->set_technique_state(shared_data.technique_vector[i].technique, false);

    //enable VR only technique
    if (shared_data.VR_only_technique_handle != 0)
        shared_data.runtime->set_technique_state(shared_data.VR_only_technique_handle, true);
    
}
*/
// *******************************************************************************************************
/// <summary>
/// Re enable all techniques
/// </summary>
/// 
/*
void reEnableAllTechnique(bool save_flag) {

    // enable all active techniques
    for (int i = 0; i < shared_data.technique_vector.size(); ++i)
        shared_data.runtime->set_technique_state(shared_data.technique_vector[i].technique, true);

    //disable VR only technique
    if (shared_data.VR_only_technique_handle != 0 )
        shared_data.runtime->set_technique_state(shared_data.VR_only_technique_handle, false);

}
*/

// *******************************************************************************************************
/// <summary>
/// Render effect 
/// </summary>
void render_technique(short int display_to_use, command_list* cmd_list) {

    // do not engage effect if render target view is not identified and either in 2D or in VR with enough time to get texture
//if (flag_capture)
{ 
    if (last_RTV_saved.copied && !a_shared.cb_inject_values.mapMode  && ((a_shared.wait_for_technique > FRAME_BEFORE_TECHNIQUE && a_shared.cb_inject_values.max_display > 0) || a_shared.cb_inject_values.max_display == 0))
    {
        
        //texture needed defined if at least 1 shader is using DEPTH or STENCIL, computed when reading technique list
        if (a_shared.texture_needed)
        {
            
            // export DEPTH and STENCIL once for all effects (must be done in 2D too !!)
            // update DEPTH texture
            
            if (a_shared.copied_textures[current_DepthStencil_handle].texresource_view.handle!=0)
                g_shared_state->runtime->update_texture_bindings("DEPTH", a_shared.copied_textures[current_DepthStencil_handle].texresource_view, a_shared.copied_textures[current_DepthStencil_handle].texresource_view);
            // update STENCIL texture
            if (a_shared.copied_textures[current_DepthStencil_handle].texresource_view_stencil.handle != 0)
                g_shared_state->runtime->update_texture_bindings("STENCIL", a_shared.copied_textures[current_DepthStencil_handle].texresource_view_stencil, a_shared.copied_textures[current_DepthStencil_handle].texresource_view_stencil);
               
           
#if _DEBUG_LOGS
            log_export_texture(display_to_use);

#endif
        }


        //export preprocessor variables (once) 
        if (display_to_use <= 1 && !g_shared_state->preprocessor_exported)
        {
            int check = 0;
            
            check += default_preprocessor(g_shared_state->runtime, "BUFFER_WIDTH", last_RTV_saved.width, true, display_to_use);
            check += default_preprocessor(g_shared_state->runtime, "BUFFER_HEIGHT", last_RTV_saved.height, true, display_to_use);
            check += default_preprocessor(g_shared_state->runtime, "BUFFER_RCP_WIDTH", 1.0 / last_RTV_saved.width, true, display_to_use);
            check += default_preprocessor(g_shared_state->runtime, "BUFFER_RCP_HEIGHT", 1.0 / last_RTV_saved.height, true, display_to_use);
            g_shared_state->preprocessor_exported = true;
            
            
#if _DEBUG_LOGS
            log_inject_preprocessor();
#endif
        }

        // render all activated techniques if not 2D mirror or in 2D (reshade is already rendering the effect) 
        // if (!g_shared_state->no_double)
        {

            // render all activated techniques if not 2D mirror or in 2D (reshade is already rendering the effect) 
            
                for (int i = 0; i < g_shared_state->technique_vector.size(); ++i)
                {
                
                    if (g_shared_state->technique_vector[i].VR_technique_status && (!g_shared_state->no_double || (g_shared_state->no_double && !g_shared_state->technique_vector[i].reshade_technique_status)))
                    {
                        //set uniform for technique if needed
                        if (g_shared_state->technique_vector[i].uniform.size() > 0)
                        {
                    
                            for (const auto& u : g_shared_state->technique_vector[i].uniform)
                            {
                                g_shared_state->runtime->set_uniform_value_float(u.unif_variable, *u.vrem_variable);
                            }
                    
                        }
                
                        // engage effect (will be compiled at the first launch)
                        g_shared_state->runtime->render_technique(g_shared_state->technique_vector[i].technique, cmd_list, last_RTV_saved.RV, last_RTV_saved.RV);
    #if _DEBUG_LOGS
                        log_effect(g_shared_state->technique_vector[i], cmd_list, last_RTV_saved.RV);
    #endif
                    }
                }
        }
    }
}
}
