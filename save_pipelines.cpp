///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// Save pipelines info (to clone them) functions
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
#include <vector>
#include <unordered_map>
#include "addon_functions.h"
#include "addon_logs.h"
#include "to_string.hpp"

//----------------------------------------------------------------------------------------
// save a pipeline info in a save_pipeline object
void save_code_and_desc(
    const reshade::api::pipeline_subobject& subobject,
    std::vector<uint8_t>& bytecode_dest,
    reshade::api::shader_desc& desc_dest) {

    auto* desc_src = static_cast<const reshade::api::shader_desc*>(subobject.data);
    // Copy bytecode
    bytecode_dest.assign(
        static_cast<const uint8_t*>(desc_src->code),
        static_cast<const uint8_t*>(desc_src->code) + desc_src->code_size
    );

    // Copy descriptor
    desc_dest = *desc_src;
    desc_dest.code = bytecode_dest.data();
    desc_dest.code_size = bytecode_dest.size();
}

//----------------------------------------------------------------------------------------
// main function : Copy one pipeline in the list of saved pipelines
void save_pipeline_in_list(
    reshade::api::device* device,
    reshade::api::pipeline_layout layout,
    uint32_t subobject_count,
    const reshade::api::pipeline_subobject* subobjects,
    reshade::api::pipeline pipeline) {

    save_pipeline temp_pipe;

    // Copy base handles
    temp_pipe.device = device;
    temp_pipe.layout = layout;
    temp_pipe.pipeline = pipeline;

    // Initialiser hash to 0
    temp_pipe.vs_hash = 0;
    temp_pipe.ps_hash = 0;
    temp_pipe.subobject_count = subobject_count;

    //handle some invalid pipelines
    if (subobject_count > MAX_PIPELINE_OBJECTS)
    {
        log_error_too_many_objectsl(pipeline, subobject_count);
        return;
    }

    // Init default values
    temp_pipe.topology = reshade::api::primitive_topology::triangle_list;
    temp_pipe.depth_stencil_format = reshade::api::format::unknown;
    memset(&temp_pipe.vs_desc, 0, sizeof(temp_pipe.vs_desc));
    memset(&temp_pipe.ps_desc, 0, sizeof(temp_pipe.ps_desc));
    memset(&temp_pipe.gs_desc, 0, sizeof(temp_pipe.gs_desc));
    memset(&temp_pipe.hs_desc, 0, sizeof(temp_pipe.hs_desc));
    memset(&temp_pipe.ds_desc, 0, sizeof(temp_pipe.ds_desc));
    memset(&temp_pipe.rasterizer, 0, sizeof(temp_pipe.rasterizer));
    memset(&temp_pipe.blend, 0, sizeof(temp_pipe.blend));
    memset(&temp_pipe.depth_stencil, 0, sizeof(temp_pipe.depth_stencil));

    // store the first PS pipeline for shader hunting
    if (subobject_count == 1 && subobjects[0].type == reshade::api::pipeline_subobject_type::pixel_shader && a_shared.first_PS_pipeline_handle == 0)
    {       
        a_shared.first_PS_pipeline_handle = pipeline.handle;
	}


    bool to_store = false;

    // browse and copy all subobjects
    for (uint32_t i = 0; i < subobject_count; i++) {
        const auto& sub = subobjects[i];

        temp_pipe.hash[i] = 0;

        // if (std::find(ALLOWED_SHADERS.begin(), ALLOWED_SHADERS.end(), type) != ALLOWED_SHADERS.end())
        
        if (ALLOWED_SHADERS.count(sub.type) > 0)
        {

            to_store = true;

            switch (sub.type) {
            case reshade::api::pipeline_subobject_type::vertex_shader: {
                save_code_and_desc(sub, temp_pipe.vs_bytecode, temp_pipe.vs_desc);
                //temp_pipe.vs_hash = calculateShaderHash(temp_pipe.vs_desc);
                temp_pipe.hash[i] = calculateShaderHash(temp_pipe.vs_desc);
                //temp_pipe.vs_hash = hash;

                break;
            }

            case reshade::api::pipeline_subobject_type::pixel_shader: {
                save_code_and_desc(sub, temp_pipe.ps_bytecode, temp_pipe.ps_desc);
                //temp_pipe.ps_hash = calculateShaderHash(temp_pipe.ps_bytecode.data());
                // temp_pipe.ps_hash = calculateShaderHash(temp_pipe.ps_desc);
                temp_pipe.hash[i] = calculateShaderHash(temp_pipe.ps_desc);
                break;
            }

            case reshade::api::pipeline_subobject_type::geometry_shader: {
                save_code_and_desc(sub, temp_pipe.gs_bytecode, temp_pipe.gs_desc);
                temp_pipe.hash[i] = calculateShaderHash(temp_pipe.gs_desc);
                break;
            }

            case reshade::api::pipeline_subobject_type::hull_shader: {
                save_code_and_desc(sub, temp_pipe.hs_bytecode, temp_pipe.hs_desc);
                temp_pipe.hash[i] = calculateShaderHash(temp_pipe.hs_desc);
                break;
            }

            case reshade::api::pipeline_subobject_type::domain_shader: {
                save_code_and_desc(sub, temp_pipe.ds_bytecode, temp_pipe.ds_desc);
                temp_pipe.hash[i] = calculateShaderHash(temp_pipe.ds_desc);
                break;
            }

            case reshade::api::pipeline_subobject_type::input_layout: {
                auto* elements = static_cast<const reshade::api::input_element*>(sub.data);
                temp_pipe.input_elements.assign(elements, elements + sub.count);
                break;
            }

            case reshade::api::pipeline_subobject_type::primitive_topology: {
                temp_pipe.topology = *static_cast<const reshade::api::primitive_topology*>(sub.data);
                break;
            }

            case reshade::api::pipeline_subobject_type::rasterizer_state: {
                temp_pipe.rasterizer = *static_cast<const reshade::api::rasterizer_desc*>(sub.data);
                break;
            }

            case reshade::api::pipeline_subobject_type::blend_state: {
                temp_pipe.blend = *static_cast<const reshade::api::blend_desc*>(sub.data);
                break;
            }

            case reshade::api::pipeline_subobject_type::depth_stencil_state: {
                temp_pipe.depth_stencil = *static_cast<const reshade::api::depth_stencil_desc*>(sub.data);
                break;
            }

            case reshade::api::pipeline_subobject_type::render_target_formats: {
                auto* formats = static_cast<const reshade::api::format*>(sub.data);
                temp_pipe.render_target_formats.assign(formats, formats + sub.count);
                break;
            }

            case reshade::api::pipeline_subobject_type::depth_stencil_format: {
                temp_pipe.depth_stencil_format = *static_cast<const reshade::api::format*>(sub.data);
                break;
            }
            }
        }

        // rebuild subobjects array with pointers to copies
        temp_pipe.subobjects.clear();

        if (to_store)
        {
            // Vertex Shader
            if (!temp_pipe.vs_bytecode.empty()) {
                temp_pipe.subobjects.push_back({
                    reshade::api::pipeline_subobject_type::vertex_shader,
                    1,
                    &temp_pipe.vs_desc
                    });
            }

            // Pixel Shader
            if (!temp_pipe.ps_bytecode.empty()) {
                temp_pipe.subobjects.push_back({
                    reshade::api::pipeline_subobject_type::pixel_shader,
                    1,
                    &temp_pipe.ps_desc
                    });
            }

            // Geometry Shader
            if (!temp_pipe.gs_bytecode.empty()) {
                temp_pipe.subobjects.push_back({
                    reshade::api::pipeline_subobject_type::geometry_shader,
                    1,
                    &temp_pipe.gs_desc
                    });
            }

            // Hull Shader
            if (!temp_pipe.hs_bytecode.empty()) {
                temp_pipe.subobjects.push_back({
                    reshade::api::pipeline_subobject_type::hull_shader,
                    1,
                    &temp_pipe.hs_desc
                    });
            }

            // Domain Shader
            if (!temp_pipe.ds_bytecode.empty()) {
                temp_pipe.subobjects.push_back({
                    reshade::api::pipeline_subobject_type::domain_shader,
                    1,
                    &temp_pipe.ds_desc
                    });
            }

            // Input Layout
            if (!temp_pipe.input_elements.empty()) {
                temp_pipe.subobjects.push_back({
                    reshade::api::pipeline_subobject_type::input_layout,
                    static_cast<uint32_t>(temp_pipe.input_elements.size()),
                    temp_pipe.input_elements.data()
                    });
            }

            // Primitive Topology
            temp_pipe.subobjects.push_back({
                reshade::api::pipeline_subobject_type::primitive_topology,
                1,
                &temp_pipe.topology
                });

            // Rasterizer State
            temp_pipe.subobjects.push_back({
                reshade::api::pipeline_subobject_type::rasterizer_state,
                1,
                &temp_pipe.rasterizer
                });

            // Blend State
            temp_pipe.subobjects.push_back({
                reshade::api::pipeline_subobject_type::blend_state,
                1,
                &temp_pipe.blend
                });

            // Depth-Stencil State
            temp_pipe.subobjects.push_back({
                reshade::api::pipeline_subobject_type::depth_stencil_state,
                1,
                &temp_pipe.depth_stencil
                });

            // Render Target Formats
            if (!temp_pipe.render_target_formats.empty()) {
                temp_pipe.subobjects.push_back({
                    reshade::api::pipeline_subobject_type::render_target_formats,
                    static_cast<uint32_t>(temp_pipe.render_target_formats.size()),
                    temp_pipe.render_target_formats.data()
                    });
            }

            // Depth-Stencil Format
            if (temp_pipe.depth_stencil_format != reshade::api::format::unknown) {
                temp_pipe.subobjects.push_back({
                    reshade::api::pipeline_subobject_type::depth_stencil_format,
                    1,
                    &temp_pipe.depth_stencil_format
                    });
            }
#ifndef _DEBUG
            //filter pipeline to keep only those with shaders in mod
            auto shader_def_opt = is_in_mod_hash(temp_pipe.hash, temp_pipe.subobject_count);
            if (shader_def_opt.has_value())
#endif
            {
                // add to global list
                g_shared_state->VREM_pipelines.saved_pipelines.push_back(std::move(temp_pipe));

                const auto& last_pipe = g_shared_state->VREM_pipelines.saved_pipelines.back();
#if _DEBUG_LOGS
                // too verbose !
                //log_saved_pipelines_value(last_pipe);
#endif
            }

        }
    }
}

//----------------------------------------------------------------------------------------
// to access pipelines in the list

// find a pipeline per handle
save_pipeline* find__pipeline_per_handle(uint64_t handle) {



    for (auto& p : g_shared_state->VREM_pipelines.saved_pipelines) {
        if (p.pipeline.handle == handle) {
            return &p;
        }
    }
    return nullptr;
}

// find a pipeline per hash 
std::vector<save_pipeline*> find_pipelines_per_vs_hash(uint32_t hash) {
    std::vector<save_pipeline*> resultats;
    for (auto& p : g_shared_state->VREM_pipelines.saved_pipelines) {
        if (p.vs_hash == hash) {
            resultats.push_back(&p);
        }
    }
    return resultats;
}

std::vector<save_pipeline*> find__pipelines_per_ps_hash(uint32_t hash) {
    std::vector<save_pipeline*> resultats;
    for (auto& p : g_shared_state->VREM_pipelines.saved_pipelines) {
        if (p.ps_hash == hash) {
            resultats.push_back(&p);
        }
    }
    return resultats;
}

std::vector<save_pipeline*> find__pipelines_per_hash(uint32_t hash) {
    std::vector<save_pipeline*> resultats;
    for (auto& p : g_shared_state->VREM_pipelines.saved_pipelines) {
        if (p.ps_hash == hash || p.vs_hash == hash) {
            resultats.push_back(&p);
        }
    }
    return resultats;
}

// get the number of stored pipelines
size_t number_of_saved_pipeline() {
    return g_shared_state->VREM_pipelines.saved_pipelines.size();
}
