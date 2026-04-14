///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// Save render target function
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
#include <string>
#include "addon_functions.h"
#include "addon_logs.h"
#include "to_string.hpp"
#include "config.hpp"
#include <filesystem>


using namespace reshade::api;

bool save_render_target(command_list* cmd_list, uint32_t fisrthash)
{
    // Vérifier qu'on a un render target
    if (a_shared.g_current_rtv.handle == 0) return 0;

    auto device = cmd_list->get_device();

    // Obtenir la resource depuis la view
    resource render_target = device->get_resource_from_view(a_shared.g_current_rtv);

    if (render_target.handle == 0) return 0;

    // Obtenir la description
    resource_desc desc = device->get_resource_desc(render_target);

    // Créer une texture staging
    resource_desc staging_desc = desc;
    staging_desc.usage = resource_usage::copy_dest;
    staging_desc.heap = memory_heap::gpu_to_cpu;

    reshade::api::resource staging_texture = {};

    if (device->create_resource(staging_desc, nullptr, reshade::api::resource_usage::copy_dest, &staging_texture))
    {
        // Copier la ressource
        cmd_list->barrier(render_target,
            reshade::api::resource_usage::render_target,
            reshade::api::resource_usage::copy_source);

        cmd_list->copy_resource(render_target, staging_texture);

        cmd_list->barrier(render_target,
            reshade::api::resource_usage::copy_source,
            reshade::api::resource_usage::render_target);

        // Mapper et sauvegarder
        reshade::api::subresource_data data = {};
        if (device->map_texture_region(staging_texture, 0, nullptr, reshade::api::map_access::read_only, &data))
        {
            
            std::filesystem::path  path = g_shared_state->g_vrem_base_path;
            path /= VREM_RT_SAVE_DIR;

            // Ensure target directory exists
            if (!std::filesystem::exists(path))
                std::filesystem::create_directory(path);

            // Ensure VREM_RT_SAVE_DIR is a std::string or wrap it
            std::stringstream ss;
            ss << std::hex << fisrthash;


            std::string filepath = path.string() + "\\draw_" + std::to_string(a_shared.draw_counter++) + "_PS_" + ss.str() + ".png";

            save_texture_image(desc, data, filepath);

            device->unmap_texture_region(staging_texture, 0);
        }

        device->destroy_resource(staging_texture);
    }
}