///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// hunting functions
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

using namespace reshade::api;

//hunting function called when binding a pipeline
void on_bind_pipeline_hunting(command_list* commandList, pipeline_stage stages, pipeline pipelineHandle)
{
	// build list of PS shaders and log messages if capture is active
	if (flag_capture)
	{
		
		if (stages == pipeline_stage::pixel_shader)
		{
			/*if (g_shared_state->debug && flag_capture)
			{
				std::stringstream s;
				s << "--> pipelineHandle.handle =  " << std::hex << pipelineHandle.handle << " g_shared_state->PSshader_list.size() = "  << g_shared_state->PSshader_list.size() <<  "; ";
				reshade::log::message(reshade::log::level::info, s.str().c_str());
			}*/
			
			auto it = std::find(g_shared_state->PSshader_list.begin(), g_shared_state->PSshader_list.end(), pipelineHandle.handle);

			if (it == g_shared_state->PSshader_list.end()) {
				// new handle, store it
				g_shared_state->PSshader_list.push_back(pipelineHandle.handle);
			}
		}
		// log bind pipeline
#if _DEBUG_LOGS  
		log_hunting_bind_pipeline(commandList, stages, pipelineHandle);
#endif
		
	}

	// if current pipeline is hunted, eiter replace it by constant color or skip draw call
	if (g_shared_state->PSshader_list.size() > 0)
	{
		if (pipelineHandle.handle == g_shared_state->PSshader_list[g_shared_state->PSshader_index])
		{
			if (g_shared_state->color_PS)
			{
				//color
				commandList->bind_pipeline(stages, a_shared.cloned_constant_color_pipeline);
			}
			else
			{
				// skip next draw call
				do_not_draw = true;
			}
		}
	}

}