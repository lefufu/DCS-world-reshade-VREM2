//
// seitings for VREM
//
// 2026/04 							Lefuneste.
//
// https://forum.dcs.world/topic/356128-reshade-vr-enhancer-mod-vrem/#comment-5503216

#include "ReShade.fxh"
#include "ReShadeUI.fxh"

//*************************
// DO NOT CHANGE THIS ENTRY, IT IS USED TO TRIGGER THE MOD
uniform float set_default <
	hidden = true;
> = 1.0;
//*************************
// misc section
// flag for the section
uniform bool set_misc <
    ui_label = "Enable misc. effects";
    ui_tooltip = "enable miscellaneous options (haze,...)";
    ui_category = "1. Misc.";
> = true;

uniform bool var_label <
    ui_label = "Label masking";
    ui_tooltip = "label masked by airframe";
    ui_category = "1. Misc.";
> = true;

uniform float var_haze_factor <
    ui_type = "slider";
    ui_label = "Haze strenght";
    ui_tooltip = "0: no haze, 1: orginal haze";
    ui_category = "1. Misc.";
    ui_min = 0.0;
    ui_max = 1.0;
    ui_step = 0.1;
> = 1.0;

uniform float var_reflection <
    ui_type = "slider";
    ui_label = "Dynamic reflection factor (A10C, AH64, F4E + ?";
    ui_tooltip = "0: no reflecion, 1: orginal reflection";
    ui_category = "1. Misc.";
    ui_min = 0.0;
    ui_max = 1.0;
    ui_step = 0.1;
> = 1.0;

//*************************
// helicopter section
// flag for the section
uniform bool set_helo <
    ui_label = "Enable helicopter effects";
    ui_tooltip = "enable helicopter options (rotor, IHADSS, ...)";
    ui_category = "2. Helicopter";
> = true;

uniform bool var_rotor <
    ui_label = "Rotor hidden";
    ui_tooltip = "flashing rotor hidden (own plane)";
    ui_category = "2. Helicopter";
> = true;

uniform int text_IHADSS_PNVS <
    ui_type = "radio";
    ui_label = " ";
    ui_text = "Disable TADS or PNVS in IHADSS";
    ui_category = "2. Helicopter";
> = 0;

uniform int text_IHADSS_PNVS_KEY <
    ui_type = "radio";
    ui_label = " ";
    ui_text = "press 'ALT'+'F6' to toggle on/off the night mode for the map";
    ui_category = "2. Helicopter";
> = 0;

uniform float var_TADSDay <
    ui_type = "slider";
    ui_label = "Trigger value for Day";
    ui_tooltip = "define value to be used to detect PNVS/TADS during day";
    ui_category = "2. Helicopter";
    ui_min = 0.0;
    ui_max = 1.0;
    ui_step = 0.01;
> = 0.28;

uniform float var_TADSNight <
    ui_type = "slider";
    ui_label = "Trigger value for Night";
    ui_tooltip = "define value to be used to detect PNVS/TADS during night";
    ui_category = "2. Helicopter";
    ui_min = 0.0;
    ui_max = 1.0;
    ui_step = 0.01;
> = 0.20;

uniform int text_IHADSS_BORESIGHT <
    ui_type = "radio";
    ui_label = " ";
    ui_text = "Boresight convergence for IHASSS";
    ui_category = "2. Helicopter";
> = 0;

uniform int text_IHADSS_BORESIGHT_KEY <
    ui_type = "radio";
    ui_label = " ";
    ui_text = "press 'ALT'+'F8' in game to toggle on/off the boresight convergence for IHADSS";
    ui_category = "2. Helicopter";
> = 0;

uniform float var_IHADSSxOffset <
    ui_type = "slider";
    ui_label = "Boresight convergence offset";
    ui_tooltip = "Modified convergence to align with BullsEye (when Boresight convergence activated)";
    ui_category = "2. Helicopter";
    ui_min = -0.25;
    ui_max = 0.25;
    ui_step = 0.01;
> = 0.10;

uniform int text_IHADSS_NOLEFT_KEY <
    ui_type = "radio";
    ui_label = " ";
    ui_text = "press 'ALT'+'F10' to toggle on/off the display of IHADSS in left eye only";
    ui_category = "2. Helicopter";
> = 0;

//*************************
// NVG section
// flag for the section
uniform bool set_nvg <
    ui_label = "Enable NVG effects";
    ui_tooltip = "enable NVG options (resize, position,...)";
    ui_category = "3. NVG";
> = true;


uniform float var_NVG_size <
    ui_type = "slider";
    ui_label = "NVG size";
    ui_tooltip = "0: small size, 1: orginal size";
    ui_category = "3. NVG";
    ui_min = 0.9;
    ui_max = 3.0;
    ui_step = 0.01;
> = 1.0;

uniform float var_NVG_YPOS <
    ui_type = "slider";
    ui_label = "NVG vert. Pos";
    ui_tooltip = "0: no reflecion, 1: orginal reflection";
    ui_category = "3. NVG";
    ui_min = -0.5;
    ui_max = 1.0;
    ui_step = 0.01;
> = 0.0;

uniform int var_nvg_color <
    ui_type = "combo";
    ui_label = "VVG color override";
    ui_tooltip = "None: standard color, Green = all vNG as US, white = all NVG as RU";
    ui_category = "3. NVG";
    ui_items = "None\0MGreen\0White\0";
> = 0;


//*************************
// NS430 section
// flag for the section
uniform bool set_ns430 <
    ui_label = "Enable NS430 effects";
    ui_tooltip = "enable NVG options (resize, position,...)";
    ui_category = "4. NS430";
> = true;


uniform float var_ns430scale <
    ui_type = "slider";
    ui_label = "NS430 Scale";
    ui_tooltip = "lower => smaller, higher => bigger";
    ui_category = "4. NS430";
    ui_min = 3.0;
    ui_max = 10.0;
    ui_step = 0.01;
> = 3.0;

uniform float var_ns430xpos <
    ui_type = "slider";
    ui_label = "NS430 horiz. Pos";
    ui_tooltip = "0: full left, 1: full right";
    ui_category = "4. NS430";
    ui_min = 0.0;
    ui_max = 1.0;
    ui_step = 0.01;
> = 0.5;

uniform float var_ns430ypos <
    ui_type = "slider";
    ui_label = "NS430 vert. Pos";
    ui_tooltip = "0: top, 1: bottom";
    ui_category = "4. NS430";
    ui_min = 0.0;
    ui_max = 1.0;
    ui_step = 0.01;
> = 0.5;

uniform float var_ns430convergence <
    ui_type = "slider";
    ui_label = "NS430 convergence";
    ui_tooltip = "move in front or back the NS430";
    ui_category = "4. NS430";
    ui_min = 0.5;
    ui_max = 1.4;
    ui_step = 0.01;
> = 1.0;

uniform float var_ns430guiyScale <
    ui_type = "slider";
    ui_label = "NS430 area height";
    ui_tooltip = "enlarge the height of area on which NS430 is drawned";
    ui_category = "4. NS430";
    ui_min = 1.0;
    ui_max = 2.0;
    ui_step = 0.01;
> = 1.0;

uniform int text_NS430_KEY <
    ui_type = "radio";
    ui_label = " ";
    ui_text = "press 'ALT'+'F5' to toggle on/off the display of NS430";
    ui_category = "4. NS430";
> = 0;


//*************************
// technique section
uniform bool set_technique <
    ui_label = "Enable technique in game pipeline (reload)";
    ui_tooltip = "enable rendering of technique in game pipeline";
    ui_category = "6. Technique.";
> = true;

//*************************
// visor section

// flag for the section
uniform bool set_visor <
    ui_label = "Enable visor effects";
    ui_tooltip = "enable visor hole removal by scaling up)";
    ui_category = "7. Visor";
> = true;

uniform bool var_visor <
    ui_label = "visor hole removal";
    ui_tooltip = "remove the noze hole in visor";
    ui_category = "7. Visor";
> = false;

uniform float var_visorfactor <
    ui_type = "slider";
    ui_label = "visor horizontal scale";
    ui_tooltip = "increase scale to shift the hole outside FoV";
    ui_category = "7. Visor";
    ui_min = 1.0;
    ui_max = 2.0;
    ui_step = 0.01;
> = 1.3;



//*************************
//Debug section

uniform int var_test_color <
    ui_type = "combo";
    ui_label = "color for fixed color";
    ui_tooltip = "display mask in PS";
    ui_category = "10. debug";
    ui_items = "value 1\0value 2\0value 3\0value 4\0";
	//hidden = true;
> = 0;

/*uniform bool var_save_cso <
    ui_label = "save .cso for all shaders";
    ui_tooltip = "save .cso for all shaders at game launch (need restart)";
    ui_category = "7. Debug";
> = true;
*/


/*

// 1. SLIDERS (int et float)
//--------------------------------------------------------------------
uniform float fSliderFloat <
    ui_type = "slider";
    ui_label = "Float Slider";
    ui_tooltip = "Test d'un slider float standard";
    ui_category = "1. Sliders";
    ui_min = 0.0;
    ui_max = 100.0;
    ui_step = 0.1;
	ui_category_closed = true;
> = 50.0;

uniform int iSliderInt <
    ui_type = "slider";
    ui_label = "Integer Slider";
    ui_tooltip = "Test d'un slider entier";
    ui_category = "1. Sliders";
    ui_min = 0;
    ui_max = 10;
    ui_step = 1;
> = 5;

// 2. DRAG (valeurs sans contraintes visuelles strictes)
//--------------------------------------------------------------------
uniform float fDragFloat <
    ui_type = "drag";
    ui_label = "Float Drag";
    ui_tooltip = "Drag pour valeurs float (plus précis qu'un slider)";
    ui_category = "2. Drag Controls";
    ui_min = -1000.0;
    ui_max = 1000.0;
    ui_step = 0.01;
> = 0.0;

uniform int iDragInt <
    ui_type = "drag";
    ui_label = "Integer Drag";
    ui_tooltip = "Drag pour valeurs entières";
    ui_category = "2. Drag Controls";
    ui_min = -100;
    ui_max = 100;
> = 0;

// 3. INPUT (saisie directe de valeurs)
//--------------------------------------------------------------------
uniform float fInputFloat <
    ui_type = "input";
    ui_label = "Float Input";
    ui_tooltip = "Saisie directe de valeur float";
    ui_category = "3. Input Fields";
> = 1.0;

uniform int iInputInt <
    ui_type = "input";
    ui_label = "Integer Input";
    ui_tooltip = "Saisie directe de valeur entière";
    ui_category = "3. Input Fields";
> = 1;

// 4. COLOR PICKER
//--------------------------------------------------------------------
uniform float3 fColor3 <
    ui_type = "color";
    ui_label = "RGB Color";
    ui_tooltip = "Sélecteur de couleur RGB";
    ui_category = "4. Color Pickers";
> = float3(1.0, 0.5, 0.0);

uniform float4 fColor4 <
    ui_type = "color";
    ui_label = "RGBA Color";
    ui_tooltip = "Sélecteur de couleur avec alpha";
    ui_category = "4. Color Pickers";
> = float4(0.0, 0.5, 1.0, 1.0);

// 5. CHECKBOX (booléens)
//--------------------------------------------------------------------
uniform bool bCheckbox <
    ui_label = "Simple Checkbox";
    ui_tooltip = "Case à cocher booléenne";
    ui_category = "5. Checkboxes";
> = true;

uniform bool bToggle <
    ui_label = "Toggle Option";
    ui_tooltip = "Autre option booléenne";
    ui_category = "5. Checkboxes";
> = false;

// 6. COMBO BOX (listes déroulantes)
//--------------------------------------------------------------------
uniform int iComboMode <
    ui_type = "combo";
    ui_label = "Mode Selection";
    ui_tooltip = "Liste déroulante d'options";
    ui_category = "6. Combo Boxes";
    ui_items = "Mode 1\0Mode 2\0Mode 3\0Mode 4\0";
> = 0;

uniform int iComboEffect <
    ui_type = "combo";
    ui_label = "Effect Type";
    ui_tooltip = "Sélection du type d'effet";
    ui_category = "6. Combo Boxes";
    ui_items = "None\0Grayscale\0Sepia\0Invert\0Edge Detect\0";
> = 0;

// 7. RADIO BUTTONS
//--------------------------------------------------------------------
uniform int iRadioQuality <
    ui_type = "radio";
    ui_label = "Quality Setting";
    ui_tooltip = "Boutons radio pour la qualité";
    ui_category = "7. Radio Buttons";
    ui_items = "Low\0Medium\0High\0Ultra\0";
> = 2;

// 8. VECTORS (float2, float3, float4)
//--------------------------------------------------------------------
uniform float2 fVector2 <
    ui_type = "slider";
    ui_label = "Vector 2D";
    ui_tooltip = "Vecteur à 2 composantes";
    ui_category = "8. Vectors";
    ui_min = 0.0;
    ui_max = 1.0;
> = float2(0.5, 0.5);

uniform float3 fVector3 <
    ui_type = "drag";
    ui_label = "Vector 3D";
    ui_tooltip = "Vecteur à 3 composantes";
    ui_category = "8. Vectors";
    ui_min = -10.0;
    ui_max = 10.0;
    ui_step = 0.1;
> = float3(0.0, 0.0, 0.0);

// 9. BUTTON (via combo avec reset)
//--------------------------------------------------------------------
uniform int iResetButton <
    ui_type = "combo";
    ui_label = "Reset All";
    ui_tooltip = "Simulateur de bouton (utiliser combo)";
    ui_category = "9. Buttons & Special";
    ui_items = "Active\0Reset Now\0";
> = 0;

// 10. SPACING & TEXT (catégories et organisation)
//--------------------------------------------------------------------
uniform int iSpacerA <
    ui_type = "radio";
    ui_label = " ";
    ui_text = "=== Section A ===";
    ui_category = "10. Spacing & Organization";
> = 0;

uniform float fTestA <
    ui_type = "slider";
    ui_label = "Test Parameter A";
    ui_category = "10. Spacing & Organization";
    ui_min = 0.0;
    ui_max = 1.0;
> = 0.5;

uniform int iSpacerB <
    ui_type = "radio";
    ui_label = " ";
    ui_text = "=== Section B ===";
    ui_category = "10. Spacing & Organization";
> = 0;

uniform float fTestB <
    ui_type = "slider";
    ui_label = "Test Parameter B";
    ui_category = "10. Spacing & Organization";
    ui_min = 0.0;
    ui_max = 1.0;
> = 0.5;

*/
//====================================================================
// TECHNIQUE
//====================================================================



technique VREM_settings
{
	
}
