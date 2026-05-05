///////////////////////////////////////////////////////////////////////
//
// Reshade IL2 VREM addon. VR Enhancer Mod for IL2 using reshade
// "hot" reload of mod possible using a Reshade addon as launcher (loaded with the game)
// and a dll containing the mod logic itselve. Mod settings are in uniforms of a technique
// 
// ----------------------------------------------------------------------------------------
//  VREM mod object class & struct definition
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
#include <unordered_map>

#include "addon_injection.h"
#include "loader_addon_shared.h"

extern SharedState* g_shared_state;
extern bool addon_init;

#ifdef _DEBUG
#define DEBUG_LOGS 1
#else
#define DEBUG_LOGS 0
#endif

//*****************************************************************************
// mod parameters
// 
//name of ini file to save techniques status
inline std::string technique_iniFileName = "techniques.ini";

// number of CB modified by VREM (used for an array allocation)
static const int NUMBER_OF_MODIFIED_CB = 2;

// number value that can be modified in a CB (for backup)
static const int MAX_OF_MODIFIED_VALUES = 3;

// to use for all tables related to CB (pipeline_layout,...)
static const int MOD_CB_NB = 0;
static const int CPERFRAME_CB_NB = 1;

// CB index in saved layout for VREM settings
static const int SETTINGS_CB_NB = 0;

// maximum size of all CB
static const int MAX_CBSIZE = 152;

// index of values to change in CB
// index of gAtmIntensity in the float array mapped for cPerFrame (cb6) 
#define FOG_INDEX 11 // c2.w => 2*4 + 3 = 11
// #define OPACITY_INDEX 55 // 13.z => 13*4+3 = 55
#define GCOCKPITIBL_INDEX_X 30*4
#define GCOCKPITIBL_INDEX_Y 30*4+1

// size of CB
#define CPERFRAME_SIZE 152 //in float

// number of frame before enabling technique
#define FRAME_BEFORE_TECHNIQUE 5

//texture for stopwatch
inline const wchar_t* STOPWATCH_TEXT_NAME = L"stopwatch.png";

// obsolete : only one value to save for CPerFrame
/*
static const int  GATMINTENSITY_SAVE = 0;
static const int  GCOCKPITIBL_X_SAVE = 1;
static const int  GCOCKPITIBL_Y_SAVE = 1;
*/

//*****************************************************************************
// for techniques
#define MAXNAMECHAR 30
#define DEPTH_NAME "DepthBufferTex"
#define STENCIL_NAME "StencilBufferTex"
//#define QV_TARGET_NAME "VREMQuadViewTarget"
//#define VR_ONLY_NAME "VREM_technique_in_VR_only"
//#define VR_ONLY_EFFECT "VRONLY_VREM.fx"
/*#define QVALL 0
#define QVOUTER 1
#define QVINNER 2
*/
constexpr size_t CHAR_BUFFER_SIZE = 256;


//for hunting shaders
constexpr uint32_t CONSTANT_HASH = 0x00000001;
constexpr const wchar_t* CONSTANT_COLOR_NAME = L"full_red.cso";


//*****************************************************************************
//mod actions (not as a class for easier use of &)
// 
//replace = the shader will be replaced by a modded one during init
static const uint32_t action_replace = 0b000000001;
// skip : the shader is to be skipped after a count of draw
static const uint32_t action_skip = 0b000000010;
//log : the shader will trigger logging of resources 
static const uint32_t action_log = 0b000000100;
//identify : the shader will be used to identify a configuration of the game (eg VR,..)
static const uint32_t action_track_RT = 0b000001000;
//inject Texture : the shader need to have textures pushed as additional parameters 
static const uint32_t action_injectText = 0b000010000;
//inject count : the shader will trigger call count 
static const uint32_t action_count = 0b000100000;
//replace_bind = the shader will be replaced by a modded one during bind
static const uint32_t action_replace_bind = 0b001000000;
//inject constant buffer
static const uint32_t action_injectCB = 0b010000000;
// dump texture or resources (for hunting in debug version)
static const uint32_t action_dump = 0b100000000;
// get texture 
static const uint32_t action_get_text = 0b1000000000;
// get texture 
static const uint32_t action_renderTechnique = 0b10000000000;

//for logging and debugging : mapping between action flag and name for display
struct ActionFlag {
	uint32_t value;
	const char* name;
};

static const ActionFlag action_flags[] = {
	{ action_replace, "replace" },
	{ action_skip, "skip" },
	{ action_log, "log" },
	{ action_track_RT, "track render target" },
	{ action_injectText, "injectText" },
	{ action_count, "count" },
	{ action_replace_bind, "replace_bind" },
	{ action_injectCB, "injectCB" },
	{ action_dump, "dump" },
	{ action_get_text, "action_get_text" },
	{ action_renderTechnique, "action_renderTechnique" },

};


//*****************************************************************************
// mod features
enum class Feature : uint32_t
{
	//null
	Null = 0,
	// own plane : try to get textures/masks for the user's plane
	PS_ownPlane = 1,
	VS_ext_ownPlane = 2,
	PS_global = 3,
	VS_global = 4,
	PS_external = 5,
	PS_sight = 6,
	PS_sun = 7,
	PS_VRMirror = 8,
	PS_preGlobal = 9,
	VS_ownPlane = 10,
	PS_VR_GUI = 11,
	PS_icon_text = 12,
	PS_icon = 13,
	PS_lastGlobal = 14,
	VS_test = 98,
	VS_dump = 99,
	//old things for compatibility
	/*
	// Rotor : disable rotor when in cockpit view
	Rotor = 100,
	// Global : global effects, change color, sharpen, ... for cockpit or outside
	Global = 200,
	// Label : mask labels by cockpit frame
	Label = 300,
	// Get stencil : copy texture t4 from global illum shader
	GetStencil = 400,
	// IHADSS : handle feature for AH64 IHADSS
	IHADSS = 500,
	// define if view is in welcome screen or map
	mapMode = 700,
	// haze control
	Haze = 800,
	// haze control & flag MSAA
	HazeMSAA2x = 900,
	// remove A10C instrument reflect
	NoReflect = 110,
	// NS430 
	NS430 = 120,
	// NVG
	NVG = 130,
	//GUI 
	GUI = 140,
	// Testing : for testing purpose
	Testing = 200,
	// VS of 2nd global color change PS
	VS_global2 = 210,
	// PS of sky to not modify gAtmInstensity
	Sky = 220
	*/
};

// mapping between technique name and feature for debug display
inline std::unordered_map<Feature, std::string> debug_feature_name = {
	{Feature::PS_ownPlane, "PS_ownPlane"},
	{Feature::VS_ext_ownPlane, "VS_ext_ownPlane"},
	{Feature::PS_global, "PS_global"},
	{Feature::VS_global, "VS_global"},
	{Feature::PS_external, "PS_external"},
	{Feature::PS_sight, "PS_sight"},
	{Feature::PS_sun, "PS_sun"},
	{Feature::PS_VRMirror, "PS_VRMirror"},
	{Feature::PS_preGlobal, "PS_preGlobal"},
	{Feature::VS_ownPlane, "VS_ownPlane"},
	{Feature::PS_icon, "PS_icon"},
	{Feature::PS_icon_text, "PS_icon_text"},	
	{Feature::VS_dump, "VS_dump"},
	{Feature::VS_test, "VS_test"},
	{Feature::PS_lastGlobal, "PS_lastGlobal"},
	{Feature::PS_VR_GUI, "PS_VR_GUI"},
	
	
};

//*****************************************************************************
// mod settings
#define VREM_SETTINGS_NAME "VREM_settings.fx"

// mapping SETTINGS value are in get_settings_from_uniforms (used to filter activie pipelines)
// !!! a_shared.VREM_setting[SET_TECHNIQUE] is duplicated in loader_addon_shared as technique list is managed in imgui !!!

static const int SETTINGS_SIZE = 11;

constexpr uint8_t SET_DEFAULT = 0;
constexpr uint8_t SET_SIGHT = 1;
constexpr uint8_t SET_MISC = 2;
constexpr uint8_t SET_PHOTO = 3;
constexpr uint8_t SET_ICON = 4;
constexpr uint8_t SET_TECHNIQUE = 8;
constexpr uint8_t SET_TESTVS = 9;
constexpr uint8_t SET_DEBUG = 10;
constexpr uint8_t SET_STOPWATCH = 5;
//will have to be cleaned up later
/*
constexpr uint8_t SET_REFLECT = 6;
constexpr uint8_t SET_NVG = 7;
*/
// !!!
// update mapping between technique name and feature at bottom of the file

//*****************************************************************************
// Key mapping
//pilote note on/off: K
static const uint32_t VK_PILOTE_NOTE = 0x4B; //'k'
static const uint32_t VK_PILOTE_NOTE_MOD = VK_SHIFT;
static const uint32_t VK_TEST_VS = VK_DIVIDE;
static const uint32_t VK_NIGHT_MODE = 0x55; //'u'
static const uint32_t VK_NIGHT_MODE_MOD = VK_CONTROL;
static const uint32_t VK_STOPWATCH = 0x4A; // 'j'
static const uint32_t VK_STOPWATCH_MOD_START = VK_SHIFT;
static const uint32_t VK_STOPWATCH_MOD_RESET = VK_CONTROL;



//*****************************************************************************
// not to be modified : declaration of class & objects used for the mod logic and shared between functions
// 
// structure to contain actions to process shader/pipeline
struct Shader_Definition {
	uint32_t action; //what is to be done for the pipeline/shader
	Feature feature; // to class pipeline/shader by mod feature
	wchar_t replace_filename[MAXNAMECHAR]; //file name of the modded shader, used only for action "replace"
	uint32_t draw_count; //used only for action "skip"
	reshade::api::pipeline substitute_pipeline; //cloned pipeline/shader with code changed
	uint32_t hash;
	std::vector<uint32_t> VREM_options;


	// Constructor

	// default Constructor needed for unordered map
	Shader_Definition()
		: action(0), feature(Feature{}), draw_count(0), hash(0) {
		replace_filename[0] = L'\0';
	}

	Shader_Definition(uint32_t act, Feature feat, const wchar_t* filename, uint32_t count)
		: action(act), feature(feat), draw_count(count) {
		wcsncpy_s(replace_filename, filename, MAXNAMECHAR);
		hash = 0;
		
	}

	// Constructeur avec paramètres (avec liste)
	Shader_Definition(uint32_t act, Feature feat, const wchar_t* filename, uint32_t count,
		std::initializer_list<uint32_t> list)
		: action(act), feature(feat), draw_count(count), hash(0), VREM_options(list) {
		wcsncpy_s(replace_filename, filename, MAXNAMECHAR);
	}

};

// a class to host all global variables shared between reshade on_* functions. 
// 

// for resource handling
struct resource_trace {
	bool created = false;
	bool copied = false;
	reshade::api::resource texresource;
};

struct resourceview_trace {
	bool created = false;
	bool compiled = false;
	reshade::api::resource_view texresource_view;
	uint32_t width;
	uint32_t height;
	// bool depth_exported_for_technique;
};

//test resource for depthStencil copy
struct resource_DS_copy {
	bool copied = false;
	reshade::api::resource texresource = {};
	reshade::api::resource_view texresource_view = {};
	reshade::api::resource_view texresource_view_stencil = {}; //for depth stencil resource
};

struct saved_RenderTargetView {
	bool copied = false;
	resource_view RV = {};
	uint32_t width = 0;
	uint32_t height = 0;
};

// to read texture from file
struct AddonText {
      reshade::api::resource      resource = {  };
      reshade::api::resource_view rView     = {  };
 };

struct __declspec(uuid("6598CABA-191D-4E3C-8D3E-F61427F2BA51")) addon_shared
{

	// DX11 pipeline_layout for VREM CB (only used if thet need to be modified)
	reshade::api::pipeline_layout saved_pipeline_layout_CB[NUMBER_OF_MODIFIED_CB];

	// VREM settings values to be used to select shaders to be processed
	float VREM_setting[SETTINGS_SIZE] = { 0 };

	// VREM injection values to be used to inject in shaders
	struct ShaderInjectData cb_inject_values;

	// to avoid "holes" in count_display, as the PS used to increment can be called 2 time consecutivelly
	Feature last_feature = Feature::Null;


	// counter for the current display (eye + quad view)
	short int count_display = 0;

	//track mask for inside/outside view
	bool not_track_mask_anymore = false;
	
	
	// flag for drawing or not

	bool track_for_render_target = false;

	bool render_technique = false;
	bool draw_passed = false;
	uint32_t count_draw = 0;

	//to flag import of .cso
	bool cso_imported = false;

	//to flag technique compiled
	bool technique_compiled = false;

	// to flag PS shader is used for 2D mirror of VR and not VR rendering
	int mirror_VR = -1;

	// to copy texture
	// DX11 pipeline_layout for ressource view
	reshade::api::pipeline_layout saved_pipeline_layout_RV = {};
	// reshade::api::descriptor_table_update update;

	//resource for texture copy
	std::unordered_map<uint64_t, resource_DS_copy> copied_textures = {};

	// for constant buffer modification
	float dest_CB_array[NUMBER_OF_MODIFIED_CB][MAX_CBSIZE];
	bool CB_copied[NUMBER_OF_MODIFIED_CB];
	float orig_values[NUMBER_OF_MODIFIED_CB][MAX_OF_MODIFIED_VALUES];
	bool track_for_CB[NUMBER_OF_MODIFIED_CB];

	bool track_for_NS430 = false;

	// to avoid doing things before 3D rendering started
	bool cockpit_rendering_started = false;

	// render target for technique 
	// store resource_views
	// std::unordered_map<uint64_t, saved_RenderTargetView> saved_RenderTargetViews = {};

	// flag to ensure preprocessor variables will be setup once
	bool init_preprocessor = false;

	//for techniques
	//map of technique selected 
	//std::vector<technique_trace> technique_vector;
	// to share uniform / texture only if needed
	bool uniform_needed = false;
	bool texture_needed = false;
	effect_technique VR_only_technique_handle;
	// for MSAA management (no way to detect it by resolution)
	float MSAAxfactor = 1.0;
	float MSAAyfactor = 1.0;
	// to compute super/down sampling factor
	float renderTargetX = -1.0;
	float SSfactor = 1.0;

	bool flag_re_enabled = false;

	// render target (all(0)/outer(1)/inner(2)) for effect
	int effect_target_QV = 0;
	// for technique refresh
	bool button_technique = false;
	bool VRonly_technique = false;
	bool init_VRonly_technique = false;
	bool button_preprocess = false;

	// for shader hunting
	//list of PS handle 
	// std::vector <uint64_t> PSshader_list;
	//current index in the list
	// int32_t PSshader_index = 0;
	// fixed color pipeline for replacing any PS during hunting
	uint64_t first_PS_pipeline_handle = 0;
	pipeline cloned_constant_color_pipeline = {};

	//for saving texture & CB
	bool flag_texture_dump = false;
	uint32_t ps_hash_for_text_dump = 0;
	bool flag_cb_dump = false;
	uint32_t ps_hash_for_cb_dump = 0;

	// render targets
	resource_view g_current_rtv = {};
	uint32_t draw_counter = 0;
	uint32_t last_pipeline_hash_PS = 0;
	bool technique_status_loaded = false;

	//flag to not engage technique too soon
	uint32_t wait_for_technique = 0;

	//to cycle photos
	uint32_t current_photo_number = 0;
	uint32_t max_photo_number = 0;
	uint32_t target_photo_number = 0;
	bool default_photo_number = true;
	bool photo_copied = false;

	//texture readed from file
	bool texture_to_read = true;
	//stopwatch
	struct AddonText stopWatchText;


};

extern struct addon_shared a_shared;

extern std::unordered_map<uint32_t, Shader_Definition> shader_by_hash;
extern std::unordered_map<uint64_t, Shader_Definition> filtered_pipeline;
extern std::unordered_map<uint64_t, reshade::api::pipeline> cloned_pipeline_list;


// not in a_shared in order to try to avoid issue if multi threaded...not sure it would work
extern bool request_capture;   // demande utilisateur
extern bool flag_capture;      // capture ACTIVE (ex-capturing)
extern bool frame_started;     // au moins un bind_pipeline vu

// to skip draw call if some shader are to be skipped
extern bool do_not_draw;

//*****************************************************************************
// add here variables to track and handle texture copy or technique injection
// 
// for logging shader_resource_view in push_descriptors() to get depthStencil 
// extern bool track_for_texture;
inline bool track_for_texture = false;
// current depth Stencil handle
inline uint64_t current_PlaneMask_handle = 0;
//current texture handle
inline uint64_t current_depth_handle =0;
inline uint64_t current_Photo_handle = 0;
inline uint64_t current_StopWatch_handle = 0;

// track render target
// extern bool track_for_render_target; 
// current render target view handle
inline saved_RenderTargetView last_RTV_saved;
inline uint64_t current_RTV_handle = 0;

//*****************************************************************************
// definition of action triggered by shaders/pipeline
inline std::unordered_map<uint32_t, Shader_Definition> shader_by_hash =
{

	// ** get maks for own plane, t8 should be OK
	//own plane texture
	{0xf7fce9a6, Shader_Definition(action_log | action_get_text  , Feature::VS_ext_ownPlane, L"", 0, {SET_DEFAULT})},
	//cockpit+test
	{0x63ba565f, Shader_Definition(action_log| action_get_text| action_replace , Feature::VS_ownPlane, L"test_far_VS.cso", 0, {SET_PHOTO, SET_TESTVS })},

	// external only
	{0xd966cd46, Shader_Definition(action_log, Feature::PS_external, L"", 0, {SET_DEFAULT})},

	//global PS before the one below, used to get render target
	{0xe2d95d7a, Shader_Definition(action_track_RT, Feature::PS_preGlobal, L"", 0, {SET_TECHNIQUE})},

	//last global PS, to postpone rendering of technique
	{0xe2d95d7a, Shader_Definition(action_log, Feature::PS_preGlobal, L"", 0, {SET_TECHNIQUE})},

	//global PS for image modification (last PS), used to set eye, display mask for debug. Its render target is used for effect
	{0x9f694be6, Shader_Definition(action_replace | action_injectText | action_log | action_renderTechnique, Feature::PS_global, L"Global.cso", 0, {SET_DEFAULT, SET_DEBUG, SET_TECHNIQUE, SET_STOPWATCH })},

	//VR mirror
	{0x39aa3616, Shader_Definition(action_log, Feature::PS_VRMirror, L"", 0, {SET_DEFAULT})},
	
	//sight PS
	{0x45983fba, Shader_Definition(action_replace , Feature::PS_sight, L"sight_PS.cso", 0, {SET_SIGHT})},

	// sun halo
	{0x27fca33b, Shader_Definition(action_replace | action_injectText , Feature::PS_sun, L"mask_sun.cso", 0, {SET_MISC})},
	
	//VR GUI
	{0x7379c02c, Shader_Definition(action_replace | action_injectText , Feature::PS_VR_GUI, L"VR_GUI_PS.cso", 0, {SET_PHOTO})},
	
	// icons
	{0x8c76b5ee, Shader_Definition(action_replace | action_injectText , Feature::PS_icon, L"icon_PS.cso", 0, {SET_ICON})},
	// icon text
	{0xdcb7b073, Shader_Definition(action_replace | action_injectText , Feature::PS_icon_text, L"icon_text_PS.cso", 0, {SET_ICON})},

	//to dump textures & CB (currenlty filled : VS for global PS)
	//{0xdf640d43, Shader_Definition(action_dump , Feature::VS_dump, L"", 0, {SET_DEFAULT})},

	// test
	{0x3b7d44c2, Shader_Definition(action_replace , Feature::VS_test, L"test_near_VS.cso", 0, {SET_TESTVS})},
	
};

//*****************************************************************************
// mapping between variable name in technique and variable in CB to inject in shader

// settings
inline std::unordered_map<std::string, int> settings_mapping = {
	{"set_default", SET_DEFAULT},
	{"set_sight", SET_SIGHT},
	{"set_misc", SET_MISC },
	{"set_technique", SET_TECHNIQUE },
	{"set_debug", SET_DEBUG },
	{"set_photo", SET_PHOTO },
	{"set_icon", SET_ICON },
	{"set_stopwatch", SET_STOPWATCH },
	{"set_testVS", SET_TESTVS },
	
};

//variables 
static const std::unordered_map<std::string, float*> var_mapping = {
	//to read settings
	//mask
	{"var_mask_sun",& a_shared.cb_inject_values.maskSun },
	{"var_debugMask", &a_shared.cb_inject_values.testFlag},
	//sight
	{"var_sightFactor", &a_shared.cb_inject_values.sightFactor},
	{"var_sightEye", &a_shared.cb_inject_values.sightEye},
	// pilot note
	{"var_photo_scale", &a_shared.cb_inject_values.photo_scale},
	{"var_photo_XPOS", &a_shared.cb_inject_values.photo_XPOS},
	{"var_photo_YPOS", &a_shared.cb_inject_values.photo_YPOS},
	{"var_triangle", &a_shared.cb_inject_values.disable_triangle},
	{"var_grey", &a_shared.cb_inject_values.grey_icons},
	{"var_grey_level", &a_shared.cb_inject_values.grey_level},
	{"var_mask_icon", &a_shared.cb_inject_values.mask_icon},
	{"var_map_bright", &a_shared.cb_inject_values.map_bright},
	{"var_clock_scale", &a_shared.cb_inject_values.clock_scale},
	{"var_clock_XPOS", &a_shared.cb_inject_values.clock_XPOS},
	{"var_clock_YPOS", &a_shared.cb_inject_values.clock_YPOS},
	{"set_clock_hours", &a_shared.cb_inject_values.clock_hours_flag},
	/*{"var_clock_hours", &a_shared.cb_inject_values.clock_hours},
	{"var_clock_mins", &a_shared.cb_inject_values.clock_mins},
	{"var_clock_secs", &a_shared.cb_inject_values.clock_secs},*/
	
	// to share variables from addon to technique 
	{"unif_display", &a_shared.cb_inject_values.count_display},
	//test
	{"unif_test", &a_shared.cb_inject_values.sightEye},
};
