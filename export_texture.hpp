///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// export textures functions
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
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <Windows.h>

#include <thread>
#include <chrono>

#include "config.hpp"
#include "to_string.hpp"

namespace fs = std::filesystem;

extern SharedState* g_shared_state;
#ifdef _DEBUG
class TextureExporter {
private:
    uint32_t draw_call_counter = 0;
    uint64_t current_ps_hash = 0;

    // get a timestamp
    uint64_t get_reshade_timestamp() {
        static auto start_time = std::chrono::high_resolution_clock::now();
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_time - start_time
        );
        return duration.count();
    }

    // genrate a filename including some meta data
    std::string generate_filename(
        uint64_t ps_hash,
        uint32_t draw_num,
        uint64_t timestamp,
        uint32_t texture_index,
        const std::string& extension = RESHADE_ADDON_TEXTURE_SAVE_FORMAT
    ) {
        std::ostringstream oss;
        // oss << "texture_"
        oss << "PS_0x" << std::hex << std::setfill('0') << std::setw(16) << ps_hash
            << "_draw_" << std::dec << draw_num
            << "_ts_" << timestamp
            << "_idx_" << texture_index
            << extension;
        return oss.str();
    }

    // save a texture
    bool save_texture(
        reshade::api::device* device,
        reshade::api::command_list* cmd_list,
        reshade::api::resource texture,
        const std::string& filepath
    ) {
        using namespace reshade::api;

        // get source texture info
        resource_desc desc = device->get_resource_desc(texture);

        /*if (desc.type != resource_type::texture_2d) {
			return false; // only 2D texture supported
        }*/

		// create a staging texture
        resource_desc staging_desc = desc;
        staging_desc.usage = resource_usage::copy_dest;
        staging_desc.heap = memory_heap::gpu_to_cpu;
        staging_desc.flags = resource_flags::none;

        resource staging_texture = {};
        if (!device->create_resource(staging_desc, nullptr, resource_usage::copy_dest, &staging_texture)) {
            log_error_staging();
            return false;
        }

		// Copier source texture to staging texture
        cmd_list->barrier(texture, resource_usage::shader_resource, resource_usage::copy_source);
        cmd_list->copy_resource(texture, staging_texture);
        cmd_list->barrier(texture, resource_usage::copy_source, resource_usage::shader_resource);

		// wait idle is not working here ?
        //cmd_list->wait_idle();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

		// Map staging texture to read data
        subresource_data mapped_data = {};
        if (!device->map_texture_region(
            staging_texture,
            0, // subresource
            nullptr, // box (toute la texture)
            map_access::read_only,
            &mapped_data
        )) {
            log_error_map_staging();
            device->destroy_resource(staging_texture);
            return false;
        }


		// chek mapped data
        if (!mapped_data.data) {
            log_error_map_data();
            device->unmap_texture_region(staging_texture, 0);
            device->destroy_resource(staging_texture);
            return false;
        }

		// save texture image using reshade function
        bool status = save_texture_image(desc, mapped_data, filepath);

		// cleanup
        device->unmap_texture_region(staging_texture, 0);
        device->destroy_resource(staging_texture);

        return status;
    }

public:
    // main function to call
    void export_descriptors(
        reshade::api::command_list* cmd_list,
        reshade::api::shader_stage stages,
        reshade::api::pipeline_layout layout,
        uint32_t param_index,
        const reshade::api::descriptor_table_update& update,
        uint64_t ps_hash, // Hash du pixel shader qui sera bindé
        uint32_t vr_draw_num // Numéro de draw pour quad view VR (0, 1, etc.)
    ) {
        using namespace reshade::api;

        // Obtenir le device depuis la command list
        device* dev = cmd_list->get_device();

        // Timestamp ReShade
        uint64_t timestamp = get_reshade_timestamp();
                
        std::filesystem::path  path = g_shared_state->g_vrem_base_path;
        path /= RESHADE_ADDON_TEXTURE_SAVE_DIR;

        if (!fs::exists(path)) {
            fs::create_directories(path);
        }

        // read all desriptors
        for (uint32_t i = 0; i < update.count; ++i) {
            // Vérifier si c'est une texture (SRV)
            if (update.type == descriptor_type::shader_resource_view) {
                resource_view srv = static_cast<const resource_view*>(update.descriptors)[i];

                // generate filename
                std::string filename = generate_filename(
                    ps_hash,
                    vr_draw_num,
                    timestamp,
                    i // Index de la texture dans la table
                );

                fs::path filepath = path / filename;

                if (srv.handle == 0)
                {
                    log_error_srv_handle_null(filepath.string());
                    continue;
                }

                resource tex = dev->get_resource_from_view(srv);

                if (tex.handle == 0)
                {
                    log_error_txt_handle_null(filepath.string());
                    continue;
                }

                // Exporter la texture
                if (save_texture(dev, cmd_list, tex, filepath.string())) {

                    log_exported_texture(filepath.string(), dev, tex, srv);
                }
            }
        }
    }
};
#endif

