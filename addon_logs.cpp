///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// VREM logs function defintion 
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
#include "to_string.hpp"
#include "addon_objects.h"
#include "addon_functions.h"

//extern SharedState g_shared_state;

void log_addon_init() {
	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - initialization");
	}
}

void log_addon_cleanup_cloned() {
	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - cleaning up cloned pipelines");
	}
}

void log_addon_cleanup_filtered() {
	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - cleaning up filtered pipelines");
	}
}

void log_cleanup_texture() {
	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - cleaning up texture resources and view");
	}
}

void log_error_array_uniform(std::string effect_name, std::string uniform_name, uint32_t out_array_length) {
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "effect name=" << effect_name << ", uniform name = " << uniform_name;
		s << "array_length > 1 :" << out_array_length << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_uniform(std::string effect_name, std::string uniform_name, float uniform_value){
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "effect name=" << effect_name << ", uniform name = " << uniform_name << ", uniform value =" << uniform_value << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_effect_reloaded() {
	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - effect reloaded => update settings from uniforms");
	}
}

void log_pipeline_init(PipeLine_Definition pipeline)
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - on_init_pipeline : added pipeline, hash = " << std::hex << pipeline.hash << ", handle = " << std::hex << pipeline.handle << ", type = " << to_string(pipeline.type) << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_filtered_added(uint64_t handle) {
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		//filtered_pipeline

		s << "addon - added filtered pipeline, handle = " << std::hex << handle  << ", hash =" << std::hex << filtered_pipeline[handle].hash << ", action =" << to_string(filtered_pipeline[handle].action) << ", feature =" << to_string(filtered_pipeline[handle].feature) << ", filename =" << to_string(filtered_pipeline[handle].replace_filename) << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}

}

void log_pipeline_filtered_skipped(uint64_t handle) {
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		//filtered_pipeline

		s << "addon - filtered pipeline already added, handle = " << std::hex << handle << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}

}

void log_shader_code_readed(const wchar_t filename[], uint32_t hash, size_t code_size) {
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - load_shader_code() - Shader " << to_string(filename)  << ", hash =" << std::hex << hash  << ", size = " << code_size << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_shader_code_error(const wchar_t filename[], uint32_t hash)
{

	std::stringstream s;
	s << "ERROR : addon - load_shader_code() - Shader " << to_string(filename) << ", hash =" << std::hex << hash << " file not found";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_cloning_pipeline(reshade::api::pipeline pipeline, reshade::api::pipeline_layout layout, Shader_Definition* newShader, uint32_t subobjectCount) {

	if (g_shared_state->debug_log)
	{
		
		std::stringstream s;

		//log beginning of copy
		s << "addon - cloning pipeline ("
			<< std::hex << pipeline.handle
			<< ") Layout : " << std::hex << layout.handle
			<< ", Hash = " << std::hex << newShader->hash << ";"
			<< ", subobjects counts: " << (subobjectCount)
			<< " )";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	}
}

void  log_pipeline_clone_OK(uint64_t orig_handle, uint64_t cloned_handle) {

	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - pipeline  cloned"
			<< ", orig pipeline handle: " << std::hex << orig_handle
			<< ", cloned pipeline handle: " << std::hex << cloned_handle
			<< ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	}
}

void  log_pipeline_clone_error(uint64_t orig_handle) {

		std::stringstream s;
		s << "ERROR : addon - error in cloning pipeline"
			<< ", orig pipeline: " << std::hex << orig_handle
			<< ")";
		reshade::log::message(reshade::log::level::error, s.str().c_str());

}


void log_saved_pipelines_value(save_pipeline saved_pipeline) {
	if (g_shared_state->debug_log)
	{
		//uint32_t hash;
		std::stringstream s;
		s << "addon - saved pipeline : handle = " << std::hex << saved_pipeline.pipeline.handle << ", subobject_count = " << saved_pipeline.subobject_count << "; ";
		// reshade::log::message(reshade::log::level::info, s.str().c_str());
		// s.str("");
		// s.clear();

		// display sub_objects
		for (uint32_t i = 0; i < saved_pipeline.subobject_count; i++) {
			//hash = 0;
			const auto& sub = saved_pipeline.subobjects[i];

			s << "subobject[" << i << "] : hash = " << std::hex << saved_pipeline.hash[i] << ", type = " << to_string(sub.type) << "; ";
			//reshade::log::message(reshade::log::level::info, s.str().c_str());
			//s.str("");
			//s.clear();
		}
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_error_code_for_hash() {
	reshade::log::message(reshade::log::level::error, "ERROR : addon - can not get code for computing shader hash");
}

void log_shader(reshade::api::pipeline pipeline, Shader_Definition shader_def, bool status) {
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - shader identified in pipeline: " << std::hex << pipeline.handle << ", " << to_string(shader_def) << ", active : " << status << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_device_null() {
	reshade::log::message(reshade::log::level::error, "ERROR : addon - device is null");
}


void log_delete_cloned_pipeline(uint64_t handle) {
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - delete cloned pipeline : " << std::hex << handle << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_cleanup_shader_code(){

	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - cleanup shader code cache");
	}
}

void log_end_capture_frame() {
	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - onpresent :  -- End Frame -- ;");
	}
}

void log_start_capture_frame() {
	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - onpresent :  -- Start Frame -- ;");
	}
}

void log_shader_binded(uint64_t handle, Shader_Definition shader_def) {
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - shader binded : " << std::hex << handle << ", shader :" << to_string(shader_def) << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_pipeline_replaced(uint64_t pipelineHandle, uint64_t cloned) {

	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - bind pipeline, replaced pipeline : " << std::hex << pipelineHandle << ", by cloned pipeline :" << std::hex << cloned << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_init_pipeline_layout(const uint32_t paramCount, const reshade::api::pipeline_layout_param* params, reshade::api::pipeline_layout layout)
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << " addon - init_pipeline_layout  = " << std::hex << layout.handle << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_init_pipeline_params(const uint32_t paramCount, const reshade::api::pipeline_layout_param* params, reshade::api::pipeline_layout layout, uint32_t paramIndex, reshade::api::pipeline_layout_param param)
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "    looping on  paramCount : param = " << to_string(paramIndex) << ", param.type = " << to_string(param.type) << ", param.push_descriptors.type = " << to_string(param.push_descriptors.type);
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	}
}

void log_create_CBlayout(std::string CBName, int CB_number)
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - new pipeline layout created, hash =" << reinterpret_cast<void*>(&a_shared.saved_pipeline_layout_CB[CB_number].handle) << " ).  for CB " << CBName << " injection, ";
		s << "  dx_register_index=" << CBINDEX << "; ";
		reshade::log::message(reshade::log::level::error, s.str().c_str());
	}
}


void log_error_creating_CBlayout(std::string CBName, int CB_number)
{
	std::stringstream s;

	s << "addon - pipeline_layout(" << reinterpret_cast<void*>(&a_shared.saved_pipeline_layout_CB[CB_number].handle) << " ). !!! Error in creating DX11 layout for CB " << CBName << "!!!;";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
	s.str("");
	s.clear();
}

void log_CB_injected(std::string CBName)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << " -> on_bind_pipeline: CB injected :" << CBName << " ;";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}


void log_destroy_CBlayout(uint64_t layout_handle)

{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - destroy pipeline layout for CB injection,  hash =" << std::hex << layout_handle  <<  ";";
		reshade::log::message(reshade::log::level::error, s.str().c_str());
	}
}

void log_increase_count_display()
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - on_bind_pipeline : update draw count : " << a_shared.count_display << ", mapMode = " << a_shared.cb_inject_values.mapMode << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_not_increase_draw_count()
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - on_bind_pipeline : depthstencil not copied => do not update draw count, texture not copied;";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_start_monitor(std::string texture_name)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - on_bind_pipeline : start monitor " << texture_name << ", draw : " << a_shared.count_display << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_stop_monitor(std::string texture_name)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - on_bind_pipeline : stop monitor " << texture_name << ", draw : " << a_shared.count_display << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_mirror_view()
{

	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - on_bind_pipeline() : count_display used for mirror view = " << a_shared.count_display << ", mirror VR = " << a_shared.mirror_VR << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}

}

void log_push_descriptor(shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "on_push_descriptors(" << to_string(stages) << ", " << (void*)layout.handle << ", " << param_index << ", { " << to_string(update.type) << ", " << update.binding << ", " << update.count << " })";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		if (update.type == descriptor_type::shader_resource_view)
		{
			// add info on textures hash
			for (uint32_t i = 0; i < update.count; ++i)
			{
				auto item = static_cast<const reshade::api::resource_view*>(update.descriptors)[i];
				s << "=> on_push_descriptors(), resource_view[" << i << "],  handle = " << reinterpret_cast<void*>(item.handle) << " })";
				reshade::log::message(reshade::log::level::info, s.str().c_str());
				s.str("");
				s.clear();
			}
		}
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_creation_start(std::string texture_name)
{

	// if (g_shared_state->debug)
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << " create resources and resource views to copy " << texture_name << ", count_display = " << a_shared.count_display << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}


void log_resource_created(std::string texture_name, device* dev, resource_desc check_new_res, uint64_t handle)
{

	if (g_shared_state->debug_log)
	{

		// display resource info
		std::stringstream s;
		s << "addon - create resource for " << texture_name  << ":, type: " << to_string(check_new_res.type) << ", src handle =" << std::hex << handle << ", draw =" << a_shared.count_display;

		switch (check_new_res.type) {
		default:
		case reshade::api::resource_type::unknown:
			s << "!!! error: resource_type not texture* !!!";
			break;

		case reshade::api::resource_type::texture_1d:
		case reshade::api::resource_type::texture_2d:
		case reshade::api::resource_type::texture_3d:
			s << ", texture format: " << to_string(check_new_res.texture.format);
			s << ", texture width: " << std::dec << check_new_res.texture.width;
			s << ", texture height: " << std::dec << check_new_res.texture.height;
			s << ", texture depth: " << std::dec << check_new_res.texture.depth_or_layers;
			s << ", texture samples: " << std::dec << check_new_res.texture.samples;
			s << ", texture levels: " << std::dec << check_new_res.texture.levels;
			s << ", usage: " << to_string(check_new_res.usage);
			break;
		}
		s << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	}
}

void log_resource_view_created(std::string texture_name, device* dev, resource_view res_view, resource_view res_stencil_view, uint64_t handle)
{
	if (g_shared_state->debug_log)
	{

		resource_view_desc res_desc = dev->get_resource_view_desc(res_view);
		// display resource view infos
		std::stringstream s;
		s << "addon - create resource view for " << texture_name << ":, type: " << to_string(res_desc.type) << ", src handle =" << std::hex << handle << ", draw =" << a_shared.count_display;


		switch (res_desc.type) {
		default:
		case reshade::api::resource_view_type::unknown:
			s << "!!! error: resource_type not texture* !!!";
			break;

		case reshade::api::resource_view_type::texture_1d:
		case reshade::api::resource_view_type::texture_2d:
		case reshade::api::resource_view_type::texture_3d:
		case reshade::api::resource_view_type::texture_2d_multisample:
			s << ", res. view format: " << to_string(res_desc.format);
			break;
		}
		s << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		if (res_stencil_view.handle != 0)
		{
			s << "addon - create stencil resource view for " << texture_name << ":, type: " << to_string(res_desc.type) << ", src handle =" << std::hex << handle << ", draw =" << a_shared.count_display;

			switch (res_desc.type) {
			default:
			case reshade::api::resource_view_type::unknown:
				s << "!!! error: resource_type not texture* !!!";
				break;

			case reshade::api::resource_view_type::texture_1d:
			case reshade::api::resource_view_type::texture_2d:
			case reshade::api::resource_view_type::texture_3d:
			case reshade::api::resource_view_type::texture_2d_multisample:
				s << ", res. view format: " << to_string(res_desc.format);
				break;
			}
			s << ";";
			reshade::log::message(reshade::log::level::info, s.str().c_str());

		}
	}
}

void log_copy_texture(std::string texture_name, uint64_t handle)
{
	// if (debug_flag && shared_data.s_do_capture)
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << " = >  resource and view(s) copied for " << texture_name << ", src res.hande = " << std::hex << handle << ", for draw (" << a_shared.count_display << ");";
		reshade::log::message(reshade::log::level::info, s.str().c_str());

	}
}

void log_texture_injected(std::string texture_name, uint64_t handle, bool depth_stencil, uint32_t slot)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << " => on_bind_pipeline : " << texture_name << ", src text. handle = " << std::hex << handle;
		if (!depth_stencil)
		{
			s << ", texture injected at slot " << slot << " for draw index : ";
		}
		else
		{
			s << ", depth texture and stencil injected at slots " << slot << ", " << slot+1 << " for draw index : ";
		}
		s << a_shared.count_display << ";";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_error_creating_view()
{

	reshade::log::message(reshade::log::level::error, "Error when creating resources or resources view");
}

void log_create_RVlayout()
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "on_init_pipeline_layout: new pipeline created, hash =" << reinterpret_cast<void*>(&a_shared.saved_pipeline_layout_RV.handle) << " ).  DX11 layout created for RV;";
		s << "dx_register_index=" << RVINDEX << "; ";
		reshade::log::message(reshade::log::level::warning, s.str().c_str());
	}
}

void log_error_creating_RVlayout()
{
	std::stringstream s;
	s << "on_init_pipeline_layout(" << reinterpret_cast<void*>(&a_shared.saved_pipeline_layout_RV.handle) << " ). !!! Error in creating DX11 layout for RV !!!;";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_reset_tracking()
{
	if (g_shared_state->debug_log && flag_capture)
	{
		reshade::log::message(reshade::log::level::info, " -> on_draw*: All tracking resetted");
	}
}

void log_ondraw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
{
	if ((g_shared_state->debug_log && flag_capture && (track_for_texture || a_shared.track_for_NS430 || a_shared.render_technique || g_shared_state->shader_hunter)) )
	{
		std::stringstream s;
		s << "draw(" << vertex_count << ", " << instance_count << ", " << first_vertex << ", " << first_instance << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		if (g_shared_state->shader_hunter) return;

		s << "track_for_depthStencil=" << track_for_texture << ", do_not_draw =" << do_not_draw << ", a_shared.render_effect =" << a_shared.render_technique << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		reshade::log::message(reshade::log::level::info, "Clear tracking flags");

	}
}

void log_on_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
{
	if ((g_shared_state->debug_log && flag_capture && (track_for_texture || a_shared.track_for_NS430 || a_shared.render_technique || g_shared_state->shader_hunter)))
	{
		std::stringstream s;
		s << "draw_indexed(" << index_count << ", " << instance_count << ", " << first_index << ", " << vertex_offset << ", " << first_instance << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());

		if (g_shared_state->shader_hunter) return;

		s << "track_for_depthStencil=" << track_for_texture << ", do_not_draw =" << do_not_draw << ", a_shared.render_effect =" << a_shared.render_technique << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		reshade::log::message(reshade::log::level::info, "Clear tracking flags");
	}
}

void log_on_drawOrDispatch_indirect(indirect_command type, resource buffer, uint64_t offset, uint32_t draw_count, uint32_t stride)
{
	if ((g_shared_state->debug_log && flag_capture && (track_for_texture || a_shared.track_for_NS430 || a_shared.render_technique || g_shared_state->shader_hunter)))
	{
		std::stringstream s;
		s << "draw_indexed_indirect(" << (void*)buffer.handle << ", " << offset << ", " << draw_count << ", " << stride << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());

		if (g_shared_state->shader_hunter) return;

		s << "track_for_depthStencil=" << track_for_texture << ", do_not_draw =" << do_not_draw << ", a_shared.render_effect =" << a_shared.render_technique << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		reshade::log::message(reshade::log::level::info, "Clear tracking flags");
	}
}

void log_display_settings()
{
	if (g_shared_state->debug_log) 
	{
		std::stringstream s;
		reshade::log::message(reshade::log::level::info, "addon - first load or reload from 'home', options values :");
		s.str("");
		s.clear();
		for (size_t i = 0; i < SETTINGS_SIZE; i++)
		{
			//if (a_shared.VREM_setting[option])
			{
				s << " a_shared.VREM_setting[" << i << "] = " << a_shared.VREM_setting[i];
				reshade::log::message(reshade::log::level::info, s.str().c_str());
				s.str("");
				s.clear();
			}
		}
	}
}

void log_waiting_setting()
{
	if (g_shared_state->debug_log)
	{
		reshade::log::message(reshade::log::level::info, "addon - waiting uniforms available");
	}	
}

void log_renderTarget_depth(uint32_t count, const resource_view* rtvs, resource_view dsv, command_list* cmd_list, uint64_t RTV_handle)
{
	if (g_shared_state->debug_log && flag_capture && ( a_shared.track_for_render_target || g_shared_state->shader_hunter))
	{
		std::stringstream s;

		

		s << "addon - bind_render_targets_and_depth_stencil() parameters :";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		s << "    rt count = " << count << ", rtvs = { ";
		for (uint32_t i = 0; i < count; ++i)
			s << (void*)rtvs[i].handle << ", ";
		s << " }, dsv = " << (void*)dsv.handle << ")";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		if (count > 0)
		{

			if (rtvs[0].handle != 0)
			{

				reshade::api::device* dev = cmd_list->get_device();

				resource scr_resource = dev->get_resource_from_view(rtvs[0]);
				resource_desc src_resource_desc = dev->get_resource_desc(scr_resource);

				if (scr_resource.handle != 0)
				{
					s << "    width = " << std::dec << src_resource_desc.texture.width << ", height = " << src_resource_desc.texture.height << "; ";
					reshade::log::message(reshade::log::level::info, s.str().c_str());
					s.str("");
					s.clear();
				}
			}
		}
		
		if (g_shared_state->shader_hunter) return;

		//s << "    saved_RenderTargetViews[" << std::hex << RTV_handle << "].created, width = " << a_shared.saved_RenderTargetViews[RTV_handle].width <<", height = " << a_shared.saved_RenderTargetViews[RTV_handle].height << ";";
		s << "    last_RTV_saved updated for handle " << std::hex << last_RTV_saved.RV.handle << ", width = " << std::dec << last_RTV_saved.width << ", height = " << last_RTV_saved.height << "; ";
		extern saved_RenderTargetView last_RTV_saved;
		reshade::log::message(reshade::log::level::info, s.str().c_str());

	}
}

void log_effect_requested()
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - on_bind_pipeline(): set flag for engaging rendering request at next draw, ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}


void log_inject_preprocessor()
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "addon - on_push_descriptors() : inject pre_processor variables ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
}

}

void log_preprocessor(std::string name, float targetValue, bool update, bool status, float readedValue, bool inFrame, uint16_t step, short int display_to_use)
{

	std::string stepName;

	switch (step)
	{
	case 1:
		stepName = "CREATE";
		break;
	case 2:
		stepName = "UPDATE";
		break;
	case 3:
		stepName = "SKIP";
		break;
	default:
		stepName = "UNKNOWN";
		break;
	}


	// if (g_shared_state->debug && ((inFrame && flag_capture) || !inFrame)) 
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << stepName << " default_preprocessor, name = '" << name << "', target value = " << targetValue << ", exists = " << status << ", readed value = " << readedValue << " display = " << display_to_use << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}

}

void log_technique_info(effect_runtime* runtime, effect_technique technique, std::string& name, std::string& eff_name, bool VR_technique_status, bool reshade_technique_status, int QV_target, bool has_texture, std::vector<uniform_mapping>& tech_uniforms)
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "init of technique in vector, Technique Name: " << name << ", Effect Name: " << eff_name << ", VR_technique_status : " << VR_technique_status <<  ", reshade_technique_status : " << reshade_technique_status << ", QV_target : " << QV_target << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		if (has_texture)
			reshade::log::message(reshade::log::level::info, "   Has DEPTH or STENCIL");
		else
			reshade::log::message(reshade::log::level::info, "   Do not have DEPTH or STENCIL");

		if (tech_uniforms.empty())
		{
			reshade::log::message(reshade::log::level::info, "   No VREM uniforms");
		}
		else
		{
			for (const auto& u : tech_uniforms)
			{
				s << "   VREM uniform: " << u.name << ", value: " << *u.vrem_variable << "; ";
				reshade::log::message(reshade::log::level::info, s.str().c_str());
				s.str("");
				s.clear();
			}
		}
	}
}

void log_export_texture(short int display_to_use)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << " => on_push_descriptors : export pre processor DEPTH and STENCIL texture for display_to_use = " << display_to_use << ";";

		reshade::log::message(reshade::log::level::warning, s.str().c_str());
	}
}

void log_texture_view(reshade::api::device* dev, std::string name, reshade::api::resource_view rview)
{

	if (g_shared_state->debug_log && flag_capture)
	{
		//display infos on resources views
		reshade::api::resource_view_desc descv = dev->get_resource_view_desc(rview);
		reshade::api::resource res = dev->get_resource_from_view(rview);
		reshade::api::resource_desc desc_res = dev->get_resource_desc(res);

		std::stringstream s;
		s << "  resource view " << name << ", handle = " << std::hex << rview.handle << " , type =" << to_string(descv.type) << " , format =" << (to_string)(descv.format) << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		s << "  associated resource handle " << std::hex << res.handle << " , type =" << to_string(desc_res.type) << " , usage =" << " 0x" << std::hex << (uint32_t)desc_res.usage;
		s << ", width: " << desc_res.texture.width << ", height: " << desc_res.texture.height << ", levels: " << desc_res.texture.levels << ", format: " << to_string(desc_res.texture.format);
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

	}
}

void log_effect(technique_trace tech, command_list* cmd_list, resource_view rv)
{
	if (g_shared_state->debug_log && flag_capture)
	{


		std::stringstream s;
		s << "=> on_push_descriptors(): engage render effects for technique = " << tech.name << " / " << tech.eff_name << ";";
		reshade::api::device* dev = cmd_list->get_device();
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();

		if (rv.handle != 0)
		{
			log_texture_view(dev, "render target for effect", rv);
		}
		else
		{
			s << "!!! resource view not catched for " << a_shared.count_display - 1 << "!!!";
			reshade::log::message(reshade::log::level::info, s.str().c_str());
		}
	}
}

void log_cbuffer_info(std::string CB_name, reshade::api::buffer_range cbuffer)
{

	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "--> cbuffer " << CB_name << " handle = " << std::hex << cbuffer.buffer.handle << ", size = " << cbuffer.size << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}

}

void log_constant_buffer_copy(std::string CB_name, float* dest_array, int buffer_size)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		for (int i = 0; i < buffer_size + 1; i++)
		{
			s << "--> cbuffer " << CB_name << "[" << i << "] = " << dest_array[i] << ";";
			reshade::log::message(reshade::log::level::info, s.str().c_str());
			s.str("");
			s.clear();
		}
	}
}

void log_constant_buffer_mapping_error(std::string CB_name)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		s << "**** map_buffer_region KO for " << CB_name << " !!! ***";
		reshade::log::message(reshade::log::level::error, s.str().c_str());
	}
}

void log_hunting_bind_pipeline(command_list* commandList, pipeline_stage stages, pipeline pipelineHandle)
{
	if (g_shared_state->debug_log && flag_capture)
	{
		std::stringstream s;
		if (pipelineHandle.handle != 0)
		{
			save_pipeline* pipe = find__pipeline_per_handle(pipelineHandle.handle);
			//only first hash displayed
			s << "adddon: hunting - BIND_PIPELINE(" << to_string(stages) << " 1st hash = " << std::hex << pipe->hash[0] << ", pipelineHandle: " << std::hex << pipelineHandle.handle << ")";
		}
		else
			s << "adddon: hunting - BIND_PIPELINE(" << to_string(stages) << ", pipelineHandle: " << std::hex << pipelineHandle.handle << ")";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_shader_marked()
{
	
	if (g_shared_state->shader_hunter)
	{
		std::stringstream s;

		uint64_t handle = g_shared_state->PSshader_list[g_shared_state->PSshader_index];
		if (handle != 0)
		{
			save_pipeline* pipe = find__pipeline_per_handle(handle);
			s << "adddon: hunting ===>  pipeline marked : handle =" << std::hex << pipe->pipeline.handle;
			for (int i = 0; i < pipe->subobject_count; i++)
			{
				s << ", hash[" << i << "] = " << std::hex << pipe->hash[i];
			}
		}
		else
		{
			s << "adddon: hunting ===>  pipeline marked : handle =" << std::hex << handle;
		}
		s << " <====";
			
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}


void log_hunting_push_descriptor(command_list* cmd_list, shader_stage stages, pipeline_layout layout, uint32_t param_index, const descriptor_table_update& update)
{
	
	if (flag_capture)
	{
		std::stringstream s;
		s << "adddon: hunting - push_descriptors(stage =" << to_string(stages) << ", layout.handle=" << (void*)layout.handle << ", param_index=" << param_index << ", update = { type:" << to_string(update.type) << ", binding:" << update.binding << ", count:" << update.count << " })";

		if (update.type == descriptor_type::shader_resource_view)
		{
			// add info on textures hash
			for (uint32_t i = 0; i < update.count; ++i)
			{
				auto item = static_cast<const reshade::api::resource_view*>(update.descriptors)[i];
				s << "--> resource_view[" << i << "],  handle = " << reinterpret_cast<void*>(item.handle) << " })";
			}
			reshade::log::message(reshade::log::level::info, s.str().c_str());
		}


		// test to handle cbuffer cPerFrame : register(b6)
		if (update.type == descriptor_type::constant_buffer)
		{

			// add info on textures hash
			for (uint32_t i = 0; i < update.count; ++i)
			{
				auto item = static_cast<const reshade::api::resource_view*>(update.descriptors)[i];
				s << "--> constant_buffer[" << i << "],  handle = " << reinterpret_cast<void*>(item.handle) << " })";
			}

			reshade::log::message(reshade::log::level::info, s.str().c_str());
		}
	}
}

void log_error_loading_shader_code(std::string message)
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon **** Error when loading shader code : " << message << " !!! ***";
		reshade::log::message(reshade::log::level::error, s.str().c_str());
	}
}

void log_replaced_shader_code(uint32_t hash, std::unordered_map<uint32_t, Shader_Definition>::iterator it, uint32_t newHash)
{

	if (g_shared_state->debug_log)
	{
		std::stringstream s;

		s << "addon - onCreatePipeline, hash to handle = " << std::hex << hash << " ;";
		s << "shader code statically replaced by :" << to_string(it->second.replace_filename) << ", new hash =" << newHash << ";";

		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_shader_def_list()
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		for (const auto& [hash, shader] : shader_by_hash) {
			//{0xC0CC8D69, Shader_Definition(action_replace_bind, Feature::Rotor, L"AH64_rotorPS.cso", 0, {SET_ROTOR})},
			// s << "hash = " << std::hex << hash << ", action = " << to_string(shader.action) << ", feature = " << to_string(shader.feature) << ", replace_filename = " << to_string(shader.replace_filename) <<  ", replace_filename = " << shader.VREM_options << ";";
			s << "hash = " << std::hex << hash << ", " << to_string(shader);
			reshade::log::message(reshade::log::level::info, s.str().c_str());
			s.str("");
			s.clear();
		}
	}	
}

void log_error_staging()
{
	reshade::log::message(reshade::log::level::error, "addon - Failed to create staging texture/CB");
}

void log_error_map_staging()
{
	reshade::log::message(reshade::log::level::error, "addon - Failed to map staging texture/CB");
}

void log_error_map_data()
{
	reshade::log::message(reshade::log::level::error, "addon - Mapped data is null");
}


void log_exported_texture(std::string text_path, device* dev, resource tex, resource_view srv)
{
	
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - Exported texture: " << text_path;
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		reshade::api::resource_desc desc = dev->get_resource_desc(tex);
		s << "addon - width =" << desc.texture.width << ", height =" << desc.texture.height << ", depth =" << desc.texture.depth_or_layers << ", mips =" << desc.texture.levels << ", samples =" << desc.texture.samples << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
		//A finir !
		reshade::api::resource_view_desc view_desc = dev->get_resource_view_desc(srv);

		s << "addon - resource view format =" << to_string(view_desc.format) << ", first_mip =" << view_desc.texture.first_level << ", mip_count =" << view_desc.texture.level_count << ", first_layer =" << view_desc.texture.first_layer << ", layer_count =" << view_desc.texture.layer_count << ";";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
		s.str("");
		s.clear();
	
	}

}

void log_error_not_buffer()
{
	reshade::log::message(reshade::log::level::error, "Addon - Resource is not a buffer");
}

void log_error_CB_size()
{
	reshade::log::message(reshade::log::level::error, "Addon - Constant buffer has zero size");
}

void log_error_CB_tooLarge()
{
	reshade::log::message(reshade::log::level::error, "Addon - Constant buffer too large, skipping");
}

void log_error_create_file(std::string path)
{
	std::stringstream s;
	s << "Addon - failed to create file for exporting CB/Texture : " << path;
	reshade::log::message(reshade::log::level::info, s.str().c_str());
}

void log_error_create_ressource()
{
	reshade::log::message(reshade::log::level::error, "Addon - create ressource KO");
}

void log_exported_CB(std::string CB_path)
{

	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - Exported CB: " << CB_path;
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}

}

void log_error_unsuported_format(const resource_desc& desc, const std::string& filepath)
{
	std::stringstream s;
	s << "addon - unsupported format: " << to_string(desc.texture.format) << " for texture " << filepath << ";";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_error_srv_handle_null(const std::string& filepath)
{
	std::stringstream s;
	s << "addon - resourve view handle null for texture: " << filepath << ";";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_error_txt_handle_null(const std::string& filepath)
{
	std::stringstream s;
	s << "addon - texture view hanlde null for texture: " << filepath << ";";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_error_too_many_objectsl(reshade::api::pipeline pipeline, uint32_t subobject_count)
{
	std::stringstream s;
	s << "addon - Error, pipeline not saved in list, pipeline handle =" << pipeline.handle << "subobject count = " << subobject_count << ";";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_empy_render_target()
{
	if (g_shared_state->debug_log && flag_capture) 	reshade::log::message(reshade::log::level::info, "addon - empty render target ignored");
}

void log_error_png_not_found(std::filesystem::path dds_path)
{
	std::stringstream s;
	s << "addon - png loader : error, png texture file not found, name =" << dds_path.string() << ";";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_error_dds_format(std::filesystem::path dds_path, std::string error)
{
	std::stringstream s;
	s << "addon - dds loader : error, dds format issue, name =" << dds_path.string() << ", error = " << error << ";";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_error_png_resource(std::filesystem::path dds_path, std::string error)
{
	std::stringstream s;
	s << "addon - dds loader : error, creating " << error <<", name = " << dds_path.string() << "; ";
	reshade::log::message(reshade::log::level::error, s.str().c_str());
}

void log_png_loaded(std::filesystem::path dds_path)
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - png loader :  png texture loaded : " << dds_path.string() << "; ";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}

void log_init_swapchain(swapchain* swapchain)
{
	if (g_shared_state->debug_log)
	{
		std::stringstream s;
		s << "addon - init_swapchain;";
		reshade::log::message(reshade::log::level::info, s.str().c_str());
	}
}