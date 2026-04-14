///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// addon logs function reference 
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
#include <filesystem>

#include "loader_addon_shared.h"
#include "addon_objects.h"

using namespace reshade::api;

extern void log_error_array_uniform(std::string effect_name, std::string uniform_name, uint32_t out_array_length);
extern void log_pipeline_clone_error(uint64_t orig_handle);
extern void log_error_code_for_hash();
extern void log_device_null();
extern void log_error_creating_CBlayout(std::string CBName, int CB_number);
extern void log_error_creating_view();
extern void log_error_creating_RVlayout();
extern void log_error_loading_shader_code(std::string message);
extern void log_shader_code_error(const wchar_t filename[], uint32_t hash);
extern void log_constant_buffer_mapping_error(std::string CB_name);
extern void log_error_unsuported_format(const resource_desc& desc, const std::string& filepath);
extern void log_error_srv_handle_null(const std::string& filepath);
extern void log_error_txt_handle_null(const std::string& filepath);
extern void log_error_too_many_objectsl(reshade::api::pipeline pipeline, uint32_t subobject_count);
extern void log_empy_render_target();
extern void log_error_png_not_found(std::filesystem::path dds_path);
extern void log_error_dds_format(std::filesystem::path dds_path, std::string error);
extern void log_error_png_resource(std::filesystem::path dds_path, std::string error);


#if _DEBUG_LOGS 
extern void log_addon_init();
extern void log_addon_cleanup_cloned();
extern void log_addon_cleanup_filtered();
extern void log_cleanup_texture();
extern void log_uniform(std::string effect_name, std::string uniform_name, float uniform_value);
extern void log_effect_reloaded();
extern void log_pipeline_init(PipeLine_Definition pipeline);
extern void log_filtered_added(uint64_t handle);
extern void log_shader_code_readed(const wchar_t filename[], uint32_t hash, size_t code_size);
extern void log_cloning_pipeline(reshade::api::pipeline pipeline, reshade::api::pipeline_layout layout, Shader_Definition* newShader, uint32_t subobjectCount);
extern void log_pipeline_clone_OK(uint64_t orig_handle, uint64_t cloned_handle);
extern void log_saved_pipelines_value(save_pipeline saved_pipeline);
extern void log_shader(reshade::api::pipeline pipeline, Shader_Definition shader_def, bool status);
extern void log_delete_cloned_pipeline(uint64_t handle);
extern void log_cleanup_shader_code();
extern void log_end_capture_frame();
extern void log_start_capture_frame();
extern void log_shader_binded(uint64_t handle, Shader_Definition shader_def);
extern void log_pipeline_replaced(uint64_t pipelineHandle, uint64_t cloned);
extern void log_init_pipeline_layout(const uint32_t paramCount, const pipeline_layout_param* params, reshade::api::pipeline_layout layout);
extern void log_init_pipeline_params(const uint32_t paramCount, const pipeline_layout_param* params, reshade::api::pipeline_layout layout, uint32_t paramIndex, reshade::api::pipeline_layout_param param);
extern void log_create_CBlayout(std::string CBName, int CB_number);
extern void log_CB_injected(std::string CBName);
extern void log_destroy_CBlayout( uint64_t layout_handle);
extern void log_increase_count_display();
extern void log_not_increase_draw_count();
extern void log_start_monitor(std::string texture_name);
extern void log_stop_monitor(std::string texture_name);
extern void log_mirror_view();
extern void log_push_descriptor(shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update);
extern void log_creation_start(std::string texture_name);
extern void log_resource_created(std::string texture_name, device* dev, resource_desc check_new_res, uint64_t handle);
extern void log_copy_texture(std::string texture_name, uint64_t handle);
extern void log_resource_view_created(std::string texture_name, device* dev, resource_view res_view, resource_view res_stencil_view, uint64_t handle);
extern void log_texture_injected(std::string texture_name, uint64_t handle, bool depth_stencil, uint32_t slot);
extern void log_create_RVlayout();
extern void log_reset_tracking();
extern void log_ondraw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance);
extern void log_on_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance);
extern void log_on_drawOrDispatch_indirect(indirect_command type, resource buffer, uint64_t offset, uint32_t draw_count, uint32_t stride);
extern void log_display_settings();
extern void log_waiting_setting();
extern void log_renderTarget_depth(uint32_t count, const resource_view* rtvs, resource_view dsv, command_list* cmd_list, uint64_t RTV_handle);
extern void log_effect_requested();
extern void log_preprocessor(std::string name, float targetValue, bool update, bool status, float readedValue, bool inFrame, uint16_t step, short int display_to_use);
extern void log_technique_info(effect_runtime* runtime, effect_technique technique, std::string& name, std::string& eff_name, bool VR_technique_status, bool reshade_technique_status, int QV_target, bool has_texture, std::vector<uniform_mapping>& tech_uniforms);
extern void log_export_texture(short int display_to_use);
extern void log_effect(technique_trace tech, command_list* cmd_list, resource_view rv);
extern void log_texture_view(reshade::api::device* dev, std::string name, reshade::api::resource_view rview);
extern void log_cbuffer_info(std::string CB_name, reshade::api::buffer_range cbuffer);
extern void log_constant_buffer_copy(std::string CB_name, float* dest_array, int buffer_size);
extern void log_pipeline_filtered_skipped(uint64_t handle);
extern void log_hunting_bind_pipeline(command_list* commandList, pipeline_stage stages, pipeline pipelineHandle);
extern void log_shader_marked();
extern void log_hunting_push_descriptor(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update);
extern void log_replaced_shader_code(uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it, uint32_t newHash);
extern void log_shader_def_list();
extern void log_error_staging();
extern void log_error_map_staging();
extern void log_error_map_data();
extern void log_exported_texture(std::string text_path, device* dev, resource tex, resource_view srv);
extern void log_error_not_buffer();
extern void log_error_CB_size();
extern void log_error_CB_tooLarge();
extern void log_error_create_file(std::string path);
extern void log_error_create_ressource();
extern void log_exported_CB(std::string path);
extern void log_inject_preprocessor();
extern void log_png_loaded(std::filesystem::path dds_path);
extern void log_init_swapchain(swapchain* swapchain);
#endif