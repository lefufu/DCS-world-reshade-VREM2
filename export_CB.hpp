///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// export constant buffers functions
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
#include <vector>

#include "to_string.hpp"
#include "config.hpp"

namespace fs = std::filesystem;

extern SharedState* g_shared_state;

#define DELAY_BETWEEN_CB_EXPORT_MS 50

#ifdef _DEBUG
class ConstantBufferExporter {
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

    // generate a filename including some meta data
    std::string generate_filename(
        uint64_t ps_hash,
        uint32_t draw_num,
        uint64_t timestamp,
        uint32_t binding_index,
        const std::string& extension = "bin"
    ) {
        std::ostringstream oss;
        oss << "CB_"
            << "PS_0x" << std::hex << std::setfill('0') << std::setw(16) << ps_hash
            << "_draw_" << std::dec << draw_num
            << "_ts_" << timestamp
            << "_binding_" << binding_index
            << "." << extension;
        return oss.str();
    }

    // Sauvegarder un constant buffer en fichier binaire
    bool save_constant_buffer_as_bin(
        reshade::api::device* device,
        reshade::api::command_list* cmd_list,
        reshade::api::buffer_range cb_buffer,
        const std::string& filepath
    ) {
        using namespace reshade::api;
        
        // Récupérer les informations du constant buffer
        resource_desc desc = device->get_resource_desc(cb_buffer.buffer);



        if (desc.type != resource_type::buffer) {
            log_error_not_buffer();
            return false;
        }

        // Vérifier que la taille est valide
        if (desc.buffer.size == 0) {
            void log_error_CB_size();
            return false;
        }

        // Limiter la taille pour éviter les problèmes
        const uint64_t MAX_CB_SIZE = 65536; // 64 KB
        if (desc.buffer.size > MAX_CB_SIZE) {
            log_error_CB_tooLarge();
            return false;
        }

        // Créer un buffer de staging pour la copie CPU
        resource_desc staging_desc = {};
        staging_desc.type = resource_type::buffer;
        staging_desc.buffer.size = desc.buffer.size;
        staging_desc.usage = resource_usage::copy_dest;
        staging_desc.heap = memory_heap::gpu_to_cpu;
        staging_desc.flags = resource_flags::none;

        resource staging_buffer = {};
        if (!device->create_resource(staging_desc, nullptr, resource_usage::copy_dest, &staging_buffer)) {
            log_error_staging();
            return false;
        }

        // Copier le constant buffer vers le staging buffer
        cmd_list->barrier(cb_buffer.buffer, resource_usage::constant_buffer, resource_usage::copy_source);
        cmd_list->copy_resource(cb_buffer.buffer, staging_buffer);
        cmd_list->barrier(cb_buffer.buffer, resource_usage::copy_source, resource_usage::constant_buffer);

        // Mapper le staging buffer
        void* mapped_data = nullptr;
        if (!device->map_buffer_region(
            staging_buffer,
            0, // offset
            UINT64_MAX, // size (tout le buffer)
            map_access::read_only,
            &mapped_data
        )) {
            log_error_map_staging();
            device->destroy_resource(staging_buffer);
            return false;
        }

        // Vérifier que les données mappées sont valides
        if (!mapped_data) {
            log_error_map_data();
            device->unmap_buffer_region(staging_buffer);
            device->destroy_resource(staging_buffer);
            return false;
        }

        // Créer le fichier binaire
        std::ofstream file(filepath, std::ios::binary);
        if (!file.is_open()) {

            log_error_create_file(filepath);

            device->unmap_buffer_region(staging_buffer);
            device->destroy_resource(staging_buffer);
            return false;
        }

        // Écrire la taille du buffer (header simple)
        uint64_t size = desc.buffer.size;
        file.write(reinterpret_cast<char*>(&size), sizeof(size));

        // Écrire les données du constant buffer
        file.write(reinterpret_cast<const char*>(mapped_data), desc.buffer.size);

        file.close();
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_BETWEEN_CB_EXPORT_MS));

        // Nettoyer
        device->unmap_buffer_region(staging_buffer);
        device->destroy_resource(staging_buffer);

        return true;
    }

    // Sauvegarder un constant buffer en format texte (pour inspection)
    bool save_constant_buffer_as_text(
        reshade::api::device* device,
        reshade::api::command_list* cmd_list,
        reshade::api::buffer_range cb_buffer,
        const std::string& filepath
    ) {
        using namespace reshade::api;
        
        // Récupérer les informations du constant buffer
        resource_desc desc = device->get_resource_desc(cb_buffer.buffer);

        // Créer un buffer de staging
        resource_desc staging_desc = {};
        staging_desc.type = resource_type::buffer;
        staging_desc.buffer.size = desc.buffer.size;
        staging_desc.usage = resource_usage::copy_dest;
        staging_desc.heap = memory_heap::gpu_to_cpu;
        //staging_desc.flags = resource_flags::none;
        staging_desc.buffer.stride = desc.buffer.stride;

        resource staging_buffer = {};
        if (!device->create_resource(staging_desc, nullptr, resource_usage::copy_dest, &staging_buffer, nullptr)) {
            log_error_create_ressource();
            return false;
        }

        // Copier
        cmd_list->barrier(cb_buffer.buffer, resource_usage::constant_buffer, resource_usage::copy_source);
        cmd_list->copy_resource(cb_buffer.buffer, staging_buffer);
        cmd_list->barrier(cb_buffer.buffer, resource_usage::copy_source, resource_usage::constant_buffer);

        // Mapper
        void* mapped_data = nullptr;
        if (!device->map_buffer_region(staging_buffer, 0, UINT64_MAX, map_access::read_only, &mapped_data)) {
            device->destroy_resource(staging_buffer);
            log_error_map_staging();
            return false;
        }

        if (!mapped_data) {
            device->unmap_buffer_region(staging_buffer);
            device->destroy_resource(staging_buffer);
            return false;
        }

        // Créer le fichier texte
        std::ofstream file(filepath);
        if (!file.is_open()) {
            device->unmap_buffer_region(staging_buffer);
            device->destroy_resource(staging_buffer);
            return false;
        }

        // Écrire en-tête
        file << "Constant Buffer Dump\n";
        file << "Size: " << desc.buffer.size << " bytes\n";
        file << "Data (as float4 registers):\n\n";

        // Interpréter comme tableau de floats (supposons des registres float4)
        const float* float_data = reinterpret_cast<const float*>(mapped_data);
        uint32_t num_floats = desc.buffer.size / sizeof(float);
        uint32_t num_registers = num_floats / 4;

        for (uint32_t i = 0; i < num_registers; ++i) {
            file << "cb" << i << ": ("
                << std::fixed << std::setprecision(6)
                << float_data[i * 4 + 0] << ", "
                << float_data[i * 4 + 1] << ", "
                << float_data[i * 4 + 2] << ", "
                << float_data[i * 4 + 3] << ")\n";
        }

        // Si des floats restants (pas un multiple de 4)
        uint32_t remaining = num_floats % 4;
        if (remaining > 0) {
            file << "cb" << num_registers << ": (";
            for (uint32_t i = 0; i < remaining; ++i) {
                file << float_data[num_registers * 4 + i];
                if (i < remaining - 1) file << ", ";
            }
            file << ")\n";
        }

        /*
        file << "\nHexadecimal dump:\n";
        const uint8_t* byte_data = reinterpret_cast<const uint8_t*>(mapped_data);
        for (uint64_t i = 0; i < desc.buffer.size; i += 16) {
            file << std::hex << std::setfill('0') << std::setw(8) << i << ": ";
            for (uint64_t j = 0; j < 16 && (i + j) < desc.buffer.size; ++j) {
                file << std::setw(2) << static_cast<int>(byte_data[i + j]) << " ";
            }
            file << "\n";
        }
        */

        file.close();
        //std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_BETWEEN_CB_EXPORT_MS));

		// cleanup
        device->unmap_buffer_region(staging_buffer);
        device->destroy_resource(staging_buffer);

        return true;
    }

public:
    void export_constant_buffers(
        reshade::api::command_list* cmd_list,
        reshade::api::shader_stage stages,
        reshade::api::pipeline_layout layout,
        uint32_t param_index,
        const reshade::api::descriptor_table_update& update,
        uint64_t ps_hash,
        uint32_t vr_draw_num,
        bool export_as_text = true // Si true, exporte en .txt, sinon en .bin
    ) {
        using namespace reshade::api;

        if (update.type != descriptor_type::constant_buffer) {
            return;
        }

        device* dev = cmd_list->get_device();

        uint64_t timestamp = get_reshade_timestamp();

		// create output directory if not exists

        std::filesystem::path  path = g_shared_state->g_vrem_base_path;
        path /= VREM_CB_SAVE_DIR;

        if (!fs::exists(path)) {
            fs::create_directories(path);
        }

		// browse all constant buffers in the update
        for (uint32_t i = 0; i < update.count; ++i) {
            buffer_range buff = static_cast<const buffer_range*>(update.descriptors)[i];

            if (buff.buffer.handle == 0) continue;

            // Générer le nom de fichier
            std::string extension = export_as_text ? "txt" : "bin";
            std::string filename = generate_filename(
                ps_hash,
                vr_draw_num,
                timestamp,
                update.binding + i, // Binding index
                extension
            );

            fs::path filepath = path / filename;

            // Exporter le constant buffer
            bool success = false;
            if (export_as_text) {
                success = save_constant_buffer_as_text(dev, cmd_list, buff, filepath.string());
            }
            else {
                success = save_constant_buffer_as_bin(dev, cmd_list, buff, filepath.string());
            }

            if (success) {
                log_exported_CB(filepath.string());

            }
        }
    }
};
#endif