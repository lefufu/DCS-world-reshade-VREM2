///////////////////////////////////////////////////////////////////////
//
// Reshade DCS VREM2 addon. VR Enhancer Mod for IDCS using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  data shared between the mod and the shaders using CB 
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

// CB number to be injected in the shaders
static const int CBINDEX = 13;
static const int CPERFRAME_INDEX = 6;

// texture number to be injected in the shaders for depth/stencil
static const int RVINDEX = 3;

// size of ShaderInjectData
static const int CBSIZE = 44;

// Must be 32bit aligned
struct ShaderInjectData {
	float testFlag; //0.x
	float rotorFlag; //0.y
	float testGlobal; //0.z
	float disable_video_IHADSS; //0.w
	float count_display; //1.x
	float mapMode = 1.0; //1.y
	float VRMode; //1.z
	float maskLabels; //1.w
	float hazeReduction; //2.x => used in asm !
	float noReflect; //2.y
	float cockpitSat; //2.z
	float cockpitMul = 1.0; //2.w
	float cockpitAdd; //3.x
	float extSat; //3.y
	float extMul = 1.0; //3.z
	float extAdd; //3.w
	float colorFlag; //4.x
	float fSharpenIntensity; //4.y
	float lumaFactor; //4.z
	float sharpenFlag; //4.w
	float debandFlag; //5.x
	float Threshold; //5.y
	float Range; //5.z
	float Iterations; //5.w
	float Grain; //6.x
	float frame_counter; //6.y
	float AAxFactor = 1.0; //6.z => to be injected in shaders, contain all super and under sampling values, at the opposite of MSAAxfactor only for technique and only for MSAA
	float AAyFactor = 1.0; //6.w => to be injected in shaders, at the opposite of MSAAyfactor only for technique, at the opposite of MSAAxfactor only for technique and only for MSAA
	float IHADSSxOffset; //7.x
	float IHADSSBoresight; //7.y
	float IHADSSNoLeft; //7.z
	float NS430Flag; //7.w
	float NS430Xpos; //8.x
	float NS430Ypos; //8.y
	float NS430Scale; //8.z
	float NS430Convergence; //8.w
	float NVGSize; //9.x
	float GUIYScale; //9.y
	float GUItodraw; //9.z
	float NVGYPos; //9.w
	float TADSNight; //10.x
	float TADSDay; //10.y
	float gCockpitIBL; //10.z
	float max_display; //10.w
};