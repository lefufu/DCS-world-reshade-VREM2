///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
// on_reshade_reload_effect : read uniforms for settings + technique for rendring in VR
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

#include "loader_addon_shared.h"
#include "addon_functions.h"
#include "addon_objects.h"
#include "addon_logs.h"

#include "to_string.hpp"

using namespace reshade::api;

// *******************************************************************************************************
// read  techniques if uniform are available
bool  get_uniform_and_techniques(effect_runtime* runtime) {

	bool result = false;
		
	// reload all uniforms
	get_settings_from_uniforms(runtime);

	//secure things by ens
	if (a_shared.VREM_setting[SET_DEFAULT])
	{
#if _DEBUG_LOGS
		//log_display_settings();

#endif
	
		// read techniques activated to use them later for technique rendering in VR
		if (a_shared.VREM_setting[SET_TECHNIQUE])
		{
			enumerateTechniques(runtime);
#if _DEBUG_LOGS
			//log_effect_reloaded();
#endif
			a_shared.technique_compiled = true;
		}
		result = true;
	}
	
	return result;
}


#ifdef _DEBUG
extern "C" {
#endif
	// *******************************************************************************************************
	// vrem_on_reshade_reloaded_effects() : to call when effects are reloaded, should not do aything before real effect compilation (another call from on_present)
	// called a lot !n
	VREM_EXPORT void vrem_on_reshade_reloaded_effects(effect_runtime* runtime)
	{

#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "addon - vrem_on_reshade_reloaded_effects started");
#endif

		// should work only for reload
		bool status = get_uniform_and_techniques(runtime);

		// if reload: new options may be settup => re generate the filtered shader list
		g_shared_state->filtered_pipeline_to_setup = true;
		filtered_pipeline.clear();

#if _DEBUG_CRASH 
		reshade::log::message(reshade::log::level::info, "addon - vrem_on_reshade_reloaded_effects ended");
#endif

	}
#ifdef _DEBUG
}
#endif