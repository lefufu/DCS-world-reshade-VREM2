///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  read VREM settings from uniforms of VREM settings technique and 
//   * store them in a float array VREM_setting to handle shader to process
//   * fill the struct used to share data with shader using constant buffer
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

// #include <imgui.h>
#include <reshade.hpp>
#include <chrono>
#include <thread>
#include <filesystem>

#include "loader_addon_shared.h"
#include "addon_logs.h"
#include "addon_functions.h"
#include "CDataFile.h"			  

static PersistentPipelineData* g_persistent = nullptr;

extern CDataFile technique_iniFile;

//*******************************************************************************
// map uniform name/value in a array to make usage of settings faster and /or map them to CB injected to shaders
// mapping values are in VREM_settings.h
// mmaping for CB variable is at bottom of the function.

using namespace reshade::api;

//*******************************************************************************
// Structure to sture uniform information
struct UniformInfo {
    std::string name;
    effect_uniform_variable variable;
    std::vector<uint8_t> data;

    bool operator!=(const UniformInfo& other) const {
        return data.size() != other.data.size() ||
            std::memcmp(data.data(), other.data.data(), data.size()) != 0;
    }
};

//*******************************************************************************
// retrieve VREM settings from uniform of VREM_settings technique

void get_settings_from_uniforms(effect_runtime* runtime) {
    std::unordered_map<std::string, UniformInfo> uniforms;

    std::string name = VREM_SETTINGS_NAME;
    const char* effect_name = name.c_str();

#if _DEBUG_CRASH reshade::log::message(reshade::log::level::info, "addon - get_settings_from_uniforms started");
#endif

   // load the technique file, as status of technique are not relevant
    technique_iniFile.Load(technique_iniFileName);

    // read techhique settings
    // if flag is defined read it 

    if (!a_shared.technique_status_loaded)
    {
        std::string raw = technique_iniFile.GetString("technique_enabled", "technique");
        if (!raw.empty()) g_shared_state->technique_enabled = technique_iniFile.GetBool("technique_enabled", "technique");

        raw = technique_iniFile.GetString("technique_enabled", "technique");
        if (!raw.empty()) g_shared_state->no_double = technique_iniFile.GetBool("no_double", "technique");
		a_shared.technique_status_loaded = true;
    }
    runtime->enumerate_uniform_variables(effect_name,
        [&](effect_runtime* rt, effect_uniform_variable var) {
            char name_buffer[256];
            rt->get_uniform_variable_name(var, name_buffer);
            std::string uniform_name(name_buffer);

            rt->get_uniform_variable_effect_name(var, name_buffer);
            std::string effect_name(name_buffer);

            UniformInfo info;
            info.name = uniform_name;
            info.variable = var;
            reshade::api::format uniform_format;
            uint32_t out_row, out_colomns, out_array_length;
            float uniform_value = 0;

            rt->get_uniform_variable_type(var, &uniform_format, &out_row, &out_colomns, &out_array_length);
            if (out_array_length > 1)
            {

                log_error_array_uniform(effect_name, uniform_name, out_array_length);
            }
            else
            {
                switch (uniform_format)
                {
                    case format::r32_typeless:
                    {
                        bool value;
                        rt->get_uniform_value_bool(var, &value, 1, 0);
                        uniform_value = value;
                        break;
                    }

                    case format::r32_uint:
                    {
                        uint32_t value;
                        rt->get_uniform_value_uint(var, &value, 1, 0);
                        uniform_value = value;
                        break;
                    }

                    case format::r32_sint:
                    {
                        int32_t value;
                        rt->get_uniform_value_int(var, &value, 1, 0);
                        uniform_value = value;
                        break;
                    }

                    case format::r32_float:
                    {
                        float value;
                        rt->get_uniform_value_float(var, &value, 1, 0);
                        uniform_value = value;
                        break;

                    }
                }
            }
#if DEBUG_LOGS
            //log_uniform(effect_name, uniform_name, uniform_value);
#endif
            
            // update vrem settings if name is defined in settings_mapping
            if (settings_mapping.count(uniform_name) > 0)
            {
                a_shared.VREM_setting[settings_mapping[uniform_name]] = uniform_value;
            }
            
			//update CB variable if name is defined in variable_mapping
            auto it = var_mapping.find(uniform_name);
            if (it != var_mapping.end())
            {
                *(it->second) = uniform_value;
            }
        });

    // sync technique state with g_shared_state
	g_shared_state->technique_enabled = a_shared.VREM_setting[SET_TECHNIQUE];

    //save technique settings in file if requested
    if (g_shared_state->request_update_file)
    {
        g_shared_state->request_update_file = false;
        save_all_technique_status();
    }

#if _DEBUG_CRASH  reshade::log::message(reshade::log::level::info, "addon - get_settings_from_uniforms ended");
#endif
}

