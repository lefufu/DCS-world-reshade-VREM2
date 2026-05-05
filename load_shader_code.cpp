///////////////////////////////////////////////////////////////////////
//
// Reshade IL2 VREM addon. VR Enhancer Mod for IL2 using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// load shader code : functions to 
//		read mod shader to replace from .cso file 
//		delete shader code cached
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
#include <filesystem>
#include <fstream>

#include "config.hpp"
#include "addon_logs.h"
#include "addon_objects.h"

//shader code cache to keep loaded code in memory, avoid mutliple load of same shader, handle option change without reloading all cso key=hash of shader
extern std::unordered_map<uint32_t, std::vector<uint8_t>> shader_code_cache;

using namespace reshade::api;

// *******************************************************************************************************
// load_shader_code() : load shader code from cso file
// 
//bool load_shader_code(std::vector<std::vector<uint8_t>>& shaderCode, wchar_t filename[])

bool load_shader_code(std::unordered_map<uint32_t, std::vector<uint8_t>>& shader_cache,
    uint32_t hash,
    const wchar_t filename[])
{
    // Vérifier si déjà en cache
    if (shader_cache.find(hash) != shader_cache.end())
        return true; // Déjà chargé

    // Prepend executable file name to image files
    wchar_t file_prefix[MAX_PATH] = L"";
    GetModuleFileNameW(nullptr, file_prefix, ARRAYSIZE(file_prefix));
    std::filesystem::path replace_path = file_prefix;
    replace_path = replace_path.parent_path();
    replace_path /= RESHADE_ADDON_SHADER_LOAD_DIR;
    replace_path /= filename;

    // Check if a replacement file for this shader hash exists
    if (!std::filesystem::exists(replace_path))
        return false;

    std::ifstream file(replace_path, std::ios::binary);
    if (!file)
    {
		log_shader_code_error(filename, hash);
        return false;
    }
        

    file.seekg(0, std::ios::end);
    std::vector<uint8_t> shader_code(static_cast<size_t>(file.tellg()));
    file.seekg(0, std::ios::beg).read(reinterpret_cast<char*>(shader_code.data()), shader_code.size());
#if _DEBUG_LOGS  
    log_shader_code_readed(filename, hash, shader_code.size());
#endif

    // Stocker dans la map
    shader_cache[hash] = std::move(shader_code);

    return true;
}


// *******************************************************************************************************
// read_all_shader_code() : load all .cso shader code from the shader_by_hash list
// 

void read_all_shader_code()
{
	// to be done once per dll load
    if (!a_shared.cso_imported)
    {
        for (const auto& [hash, shader_def] : shader_by_hash)
        {
            // read code only for replace options
            if (shader_def.action & action_replace || shader_def.action & action_replace_bind)
            {
                // check if a replacement filename is specified
                if (shader_def.replace_filename[0] != L'\0')
                {
                    // Vos actions ici
                    load_shader_code(shader_code_cache, hash, shader_def.replace_filename);
                }
            }
        }

        //read cso for constant color shader

        load_shader_code(shader_code_cache, CONSTANT_HASH, CONSTANT_COLOR_NAME);

        a_shared.cso_imported = true;
    }
}