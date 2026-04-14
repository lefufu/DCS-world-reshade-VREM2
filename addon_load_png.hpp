///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  load png pictures, based of crosire code modified by Claude
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
#include <stb_image.h>      // dans deps\stb, pas de define ici

#include <cstdint>
#include <vector>
#include <sstream>
#include <filesystem>

#include "addon_objects.h"

namespace LoadPNG
{

    inline bool LoadPNGTexture(
        reshade::api::device* device,
        const std::filesystem::path& path,
        reshade::api::resource& outResource,
        reshade::api::resource_view& outView)
    {
        outResource = { 0 };
        outView = { 0 };

        // ---- 1. Charger le PNG via stbi_load ----
        // Exactement comme crosire : u8string().c_str()
        int width = 0, height = 0, channels = 0;

        stbi_uc* const pixels = stbi_load(
            path.string().c_str(),
            &width, &height,
            &channels,
            STBI_rgb_alpha);

        if (pixels == nullptr)
        {
            log_error_png_not_found(path);
            return false;
        }

        // Copier dans un vector (comme crosire) pour gérer la durée de vie
        std::vector<uint8_t> pixelData(
            pixels,
            pixels + static_cast<size_t>(width) * static_cast<size_t>(height) * 4);

        stbi_image_free(pixels);

        // ---- 2. Créer la resource SANS initData ----
        reshade::api::resource_desc resDesc = {};
        resDesc.type = reshade::api::resource_type::texture_2d;
        resDesc.texture.width = static_cast<uint32_t>(width);
        resDesc.texture.height = static_cast<uint32_t>(height);
        resDesc.texture.depth_or_layers = 1;
        resDesc.texture.levels = 1;
        resDesc.texture.format = reshade::api::format::r8g8b8a8_unorm;
        resDesc.texture.samples = 1;
        resDesc.heap = reshade::api::memory_heap::gpu_only;       // 1
        resDesc.usage = reshade::api::resource_usage::shader_resource; // 192 seul

        if (!device->create_resource(
            resDesc,
            nullptr,
            reshade::api::resource_usage::shader_resource,
            &outResource))
        {
            log_error_png_resource(path, "resource");
            return false;
        }

        // ---- 3. Upload via update_texture_region ----
        // Exactement comme crosire dans on_update_texture / on_copy_texture
        reshade::api::subresource_data uploadData = {};
        uploadData.data = pixelData.data();
        uploadData.row_pitch = static_cast<uint32_t>(width) * 4u;
        uploadData.slice_pitch = uploadData.row_pitch * static_cast<uint32_t>(height);

        device->update_texture_region(
            uploadData,
            outResource,
            0,        // subresource 0 = mip 0, layer 0
            nullptr); // nullptr = toute la texture

        // ---- 4. Créer la resource_view ----
        reshade::api::resource_view_desc viewDesc = {};
        viewDesc.type = reshade::api::resource_view_type::texture_2d;
        viewDesc.format = reshade::api::format::r8g8b8a8_unorm;
        viewDesc.texture.first_level = 0;
        viewDesc.texture.level_count = 1;
        viewDesc.texture.first_layer = 0;
        viewDesc.texture.layer_count = 1;

        if (!device->create_resource_view(
            outResource,
            reshade::api::resource_usage::shader_resource,
            viewDesc,
            &outView))
        {
            device->destroy_resource(outResource);
            outResource = { 0 };
            log_error_png_resource(path, "resource_view");
            return false;
        }

        //add the resource and resource view in a_shared.copied_textures for usage in inject_texture
        current_StopWatch_handle = outResource.handle;
        resource_DS_copy text_copy = {};
        text_copy.texresource_view = outView;
        text_copy.texresource = outResource;
        text_copy.texresource_view_stencil = { };
        a_shared.copied_textures.emplace(current_StopWatch_handle, text_copy);
        

#if _DEBUG_LOGS
        log_png_loaded(path);
#endif

        return true;
    }

    inline void DestroyPNGTexture(
        reshade::api::device* device,
        reshade::api::resource& resource,
        reshade::api::resource_view& view)
    {
        if (view.handle != 0)
        {
            device->destroy_resource_view(view);
            view = { 0 };
        }
        if (resource.handle != 0)
        {
            device->destroy_resource(resource);
            resource = { 0 };
        }
    }

} // namespace Chrono1940
