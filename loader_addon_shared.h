///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// launcher and addon shared structures definition
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



#pragma once

#include <unordered_map>
#include <chrono>
#include <reshade.hpp>
#include <set>
#include <vector>
#include <string>		 

// name of addon .dll
#define VREM_ADDON_NAME "DCS_VREM2.dll"

#define MAX_OBJ_PER_PIPELINE 5

#ifdef _DEBUG
#define _DEBUG_LOGS 1
#else
#define _DEBUG_LOGS 0
#endif

#define _DEBUG_CRASH 0

using namespace reshade::api;

// handled pipeline types
constexpr  reshade::api::pipeline_stage ALLOWED_STAGES = pipeline_stage::pixel_shader | pipeline_stage::vertex_shader;
static const std::set<pipeline_subobject_type> ALLOWED_SHADERS = {
    pipeline_subobject_type::vertex_shader,
    pipeline_subobject_type::pixel_shader
};

//max number of objects in pipeline (to filter some case with invalid number
#define MAX_PIPELINE_OBJECTS 10


// Structure to store all pipeline infos
struct save_pipeline {
    reshade::api::device* device;
    reshade::api::pipeline_layout layout;
    reshade::api::pipeline pipeline;

    // subobjects copy
    std::vector<reshade::api::pipeline_subobject> subobjects;
    uint32_t subobject_count;

    // store data pointed by subobjects
    std::vector<uint8_t> vs_bytecode;
    std::vector<uint8_t> ps_bytecode;
    std::vector<uint8_t> gs_bytecode;
    std::vector<uint8_t> hs_bytecode;
    std::vector<uint8_t> ds_bytecode;

    reshade::api::shader_desc vs_desc;
    reshade::api::shader_desc ps_desc;
    reshade::api::shader_desc gs_desc;
    reshade::api::shader_desc hs_desc;
    reshade::api::shader_desc ds_desc;

    std::vector<reshade::api::input_element> input_elements;
    reshade::api::primitive_topology topology;
    reshade::api::rasterizer_desc rasterizer;
    reshade::api::blend_desc blend;
    reshade::api::depth_stencil_desc depth_stencil;
    std::vector<reshade::api::format> render_target_formats;
    reshade::api::format depth_stencil_format;

    // metadata for identification
    uint32_t vs_hash;
    uint32_t ps_hash;
    uint32_t hash[MAX_OBJ_PER_PIPELINE];
};


struct PipeLine_Definition {
    reshade::api::pipeline_subobject_type type;
    uint32_t hash;
    uint64_t handle;
};

// Structure for data that should be kept between mod reload
struct PersistentPipelineData {
    //std::unordered_map<uint32_t, PipeLine_Definition> pipeline_by_hash;
    //std::unordered_map<uint64_t, PipeLine_Definition> pipeline_by_handle;
    std::vector<save_pipeline> saved_pipelines;
};

// for technique uniform mapping
struct uniform_mapping {
    std::string name;
    float* vrem_variable;
	effect_uniform_variable unif_variable;
};


// for technique settings
struct technique_trace {
    effect_technique technique;
    std::string name;
    std::string eff_name;
    bool VR_technique_status;
	bool reshade_technique_status;
    std::vector<uniform_mapping> uniform;
    int quad_view_target; // 0 : all, 1 Outer, 2 Innner
};
// Structure to hold shared variables
struct SharedState {
    reshade::api::device* device = {};
    reshade::api::effect_runtime* runtime = nullptr;
    
    // for fps used to test the logic
    float last_fps_limit = 0;
    std::chrono::high_resolution_clock::time_point s_last_time_point;
    // to know when overlay is active and so update uniforms
    bool overlay_is_open = false;

    // debug flag
#ifdef _DEBUG
    bool debug = true;
#else
	bool debug = false;
#endif

#if _DEBUG_LOGS
    bool debug_log = true;
#else
    bool debug_log = false;
#endif

    // capture button
    bool button_capture = false;

	// flag to setup generation of filtered pipeline list and cloning
	bool filtered_pipeline_to_setup = true;
  
    PersistentPipelineData VREM_pipelines = {};

    reshade::api::pipeline_layout DX11_layout = {};

    // for hunting
	bool shader_hunter = false;
    // bool save_cso = false;
    //flag for shader marking (0=>hidden, 1=>color)
	int color_PS = 0;
	int number_of_pipelines = 0;
    int PSshader_index = 0;
    std::vector <uint64_t> PSshader_list;

	bool init_done = false;

    //for saving texture & CB
	bool save_texture_flag = false;
    wchar_t g_vrem_base_path[MAX_PATH] = {};
    bool  save_cb_flag = false;
	bool  save_rt_flag = false;

    //technique definition
    bool technique_enabled = false;
    std::vector<technique_trace> technique_vector;
    bool request_update_file = false;
    bool no_double = true;
	bool preprocessor_exported = false;
	bool is_VR = false;
    
};
