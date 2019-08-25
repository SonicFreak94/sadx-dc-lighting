#include "stdafx.h"

// Direct3D
#include <d3d9.h>

// Mod loader
#include <Trampoline.h>

// Local
#include "d3d.h"
#include "../include/lanternapi.h"

static D3DFOGMODE fog_mode = D3DFOG_NONE;

static void __cdecl njDisableFog_r();
static void __cdecl njEnableFog_r();
static void __cdecl njSetFogColor_r(Uint32 c);
static void __cdecl njSetFogTable_r(NJS_FOG_TABLE fogtable);

static Trampoline njDisableFog_t(0x00794980, 0x00794985, njDisableFog_r);
static Trampoline njEnableFog_t(0x00794950, 0x00794955, njEnableFog_r);
static Trampoline njSetFogColor_t(0x00787240, 0x00787245, njSetFogColor_r);
static Trampoline njSetFogTable_t(0x00794E40, 0x00794E46, njSetFogTable_r);

using namespace d3d;

static void __cdecl njDisableFog_r()
{
	TARGET_STATIC(njDisableFog)();
	set_flags(LanternShaderFlags_Fog, false);
}

static void __cdecl njEnableFog_r()
{
	TARGET_STATIC(njEnableFog)();
	param::FogMode = fog_mode;
	set_flags(LanternShaderFlags_Fog, true);
}

static void __cdecl njSetFogColor_r(Uint32 c)
{
	TARGET_STATIC(njSetFogColor)(c);
	param::FogColor = float4(c);
}

static void __cdecl njSetFogTable_r(NJS_FOG_TABLE fogtable)
{
	TARGET_STATIC(njSetFogTable)(fogtable);

	if (device == nullptr)
	{
		return;
	}

	device->GetRenderState(D3DRS_FOGTABLEMODE, reinterpret_cast<DWORD*>(&fog_mode));
	param::FogMode = fog_mode;
	set_flags(LanternShaderFlags_Fog, true);

	float3 fog_config {};

	float start, end, density;
	device->GetRenderState(D3DRS_FOGSTART, reinterpret_cast<DWORD*>(&start));
	device->GetRenderState(D3DRS_FOGEND, reinterpret_cast<DWORD*>(&end));

	fog_config.x = start;
	fog_config.y = end;

	if (fog_mode != D3DFOG_LINEAR)
	{
		device->GetRenderState(D3DRS_FOGDENSITY, reinterpret_cast<DWORD*>(&density));
		fog_config.z = density;
	}

	param::FogConfig = fog_config;
}
