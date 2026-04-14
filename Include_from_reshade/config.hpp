/*
 * Copyright (C) 2024 Patrick Mours
 * SPDX-License-Identifier: BSD-3-Clause OR MIT
 */

// The subdirectory to save shader binaries to
#define RESHADE_ADDON_SHADER_SAVE_DIR ".\\VREM_shaderdump"

// The subdirectory to load shader binaries from
#define RESHADE_ADDON_SHADER_LOAD_DIR ".\\VREM_shaderreplace"

// The subdirectory to save textures to
#define RESHADE_ADDON_TEXTURE_SAVE_DIR ".\\VREM_tex_dump"
#define RESHADE_ADDON_TEXTURE_SAVE_FORMAT ".png"
#define RESHADE_ADDON_TEXTURE_SAVE_HASH_TEXMOD 0
// Skip any textures that were already dumped this session, to reduce lag at the cost of increased memory usage
#define RESHADE_ADDON_TEXTURE_SAVE_ENABLE_HASH_SET 0

// The subdirectory to load textures from
#define RESHADE_ADDON_TEXTURE_LOAD_DIR ".\\texreplace"
#define RESHADE_ADDON_TEXTURE_LOAD_FORMAT ".png"
#define RESHADE_ADDON_TEXTURE_LOAD_HASH_TEXMOD 0

#define VREM_RT_SAVE_DIR ".\\VREM_captures"

#define VREM_CB_SAVE_DIR ".\\VREM_CB_dump"