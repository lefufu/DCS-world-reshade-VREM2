///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  all functions to clean persistent objects not deleted by addon 
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

#include "loader_addon_shared.h"
#include "loader_logs.h"
//#include "addon_objects.h"

extern SharedState g_shared_state_l;

//----------------------------------------------------------------------------------------
// delete one pipeline in the list of saved pipelines

// Fonction pour nettoyer un pipeline stocké
void delete_saved_pipeline(save_pipeline& p) {

    //  clean vectors (free memory)
    p.subobjects.clear();
    p.subobjects.shrink_to_fit();

    p.vs_bytecode.clear();
    p.vs_bytecode.shrink_to_fit();

    p.ps_bytecode.clear();
    p.ps_bytecode.shrink_to_fit();

    p.gs_bytecode.clear();
    p.gs_bytecode.shrink_to_fit();

    p.hs_bytecode.clear();
    p.hs_bytecode.shrink_to_fit();

    p.ds_bytecode.clear();
    p.ds_bytecode.shrink_to_fit();

    p.input_elements.clear();
    p.input_elements.shrink_to_fit();

    p.render_target_formats.clear();
    p.render_target_formats.shrink_to_fit();

    // initialize pointers
    p.device = nullptr;
    p.layout = {};
    p.pipeline = {};
}

//----------------------------------------------------------------------------------------
// Delete the full list
void delete_all_saved_pipelines() {
    log_delete_saved_pipelines();
        for (auto& p : g_shared_state_l.VREM_pipelines.saved_pipelines) {
        // *********************************
        // disabled because too verbose
        // log_saved_pipelines_value(p);
        delete_saved_pipeline(p);
    }
    g_shared_state_l.VREM_pipelines.saved_pipelines.clear();
    g_shared_state_l.VREM_pipelines.saved_pipelines.shrink_to_fit();
}



//----------------------------------------------------------------------------------------
// free memory of addon persistant unordered_map 
void delete_persistant_objects() {
	// may be useless but to be sure
    //std::unordered_map<uint32_t, PipeLine_Definition>().swap(g_shared_state_l.VREM_pipelines.pipeline_by_hash);
    //std::unordered_map<uint64_t, PipeLine_Definition>().swap(g_shared_state_l.VREM_pipelines.pipeline_by_handle);
    // delete saved pipelines
    delete_all_saved_pipelines();
}