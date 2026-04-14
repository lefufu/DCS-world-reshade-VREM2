///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  references of functions of VREM mod, including messages
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

#include <reshade.hpp>
#include "addon_objects.h"
#include <optional>

/* #ifdef _DEBUG
#define USE_HOT_RELOAD 1
#else
#define USE_HOT_RELOAD 0    
#endif */

#ifdef _DEBUG
#define VREM_EXPORT extern "C" __declspec(dllexport)
#else
#define VREM_EXPORT
#endif



extern void get_settings_from_uniforms(reshade::api::effect_runtime* runtime);
// extern uint32_t calculateShaderHash(void* shaderData);
extern uint32_t calculateShaderHash(const reshade::api::shader_desc& desc);
extern std::optional<Shader_Definition> is_in_mod_hash(uint32_t hash[], uint32_t subobject_count);
extern std::optional<Shader_Definition> is_in_mod_hash(uint32_t hash);
extern bool load_shader_code(std::unordered_map<uint32_t, std::vector<uint8_t>>& shader_cache, uint32_t hash, const wchar_t filename[]);
extern reshade::api::pipeline clone_pipeline(reshade::api::device* device, reshade::api::pipeline_layout layout, uint32_t subobjectCount, const reshade::api::pipeline_subobject* subobjects, reshade::api::pipeline pipeline, uint32_t hash[]);
extern void save_pipeline_in_list(reshade::api::device* device, reshade::api::pipeline_layout layout, uint32_t subobject_count, const reshade::api::pipeline_subobject* subobjects, reshade::api::pipeline pipeline);
extern save_pipeline* find__pipeline_per_handle(uint64_t handle);
extern std::vector<save_pipeline*> find__pipelines_per_hash(uint32_t hash);
extern size_t number_of_saved_pipeline();
extern void delete_saved_pipeline(save_pipeline& p);
extern void delete_all_saved_pipelines();
// extern bool hasMatch(const std::vector<uint32_t>& list, const uint32_t* array, size_t array_size);
extern bool setup_filtered_pipelines(reshade::api::device* device, reshade::api::effect_runtime* runtime);
extern void read_all_shader_code();
extern void delete_cloned_pipelines(reshade::api::device* dev);
extern void create_modified_CB_layout(reshade::api::device* device, int cbindex, std::string CB_name, int layout_number);
extern void create_all_modified_CB_layout(reshade::api::device* device);
//extern bool copy_plane_mask(reshade::api::command_list* cmd_list, reshade::api::shader_stage stages, reshade::api::pipeline_layout layout, uint32_t param_index, const reshade::api::descriptor_table_update& update);
extern void delete_texture_resources(reshade::api::device* device);
extern void create_RV_pipeline_layout(reshade::api::device* device);
extern void init_preprocess(effect_runtime* runtime);
extern void enumerateTechniques(effect_runtime* runtime);
extern void render_technique(short int display_to_use, command_list* cmd_list);
extern bool  get_uniform_and_techniques(effect_runtime* runtime);
extern bool read_constant_buffer(command_list* cmd_list, const descriptor_table_update& update, std::string CB_name, int descriptors_index, float* dest_array, int buffer_size);
extern void on_bind_pipeline_hunting(command_list* commandList, pipeline_stage stages, pipeline pipelineHandle);
extern void save_shader_code(device_api device_type, const shader_desc& desc);
extern bool save_texture_image(const resource_desc& desc, const subresource_data& data, const std::string& filepath);
extern bool save_render_target(command_list* cmd_list, uint32_t fisrthash);
extern uint64_t copy_texture_from_desc(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update, uint32_t dec_number, std::string textName);
extern void inject_texture(command_list* commandList, uint32_t stot, uint64_t text_handle, std::string text_name);
extern void intialize_counters();
extern void save_all_technique_status();
extern void pure_technique_vector();
extern void read_textures(reshade::api::device* device);
extern void delete_loaded_textures(reshade::api::device* device);
