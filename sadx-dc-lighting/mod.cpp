#include "stdafx.h"
#include <d3d9.h>

// Mod loader
#include <SADXModLoader.h>
#include <Trampoline.h>

// Local
#include "d3d.h"
#include "datapointers.h"
#include "fog.h"
#include "globals.h"
#include "lantern.h"
#include "Obj_Past.h"

static Trampoline* CharSel_LoadA_t          = nullptr;
static Trampoline* Direct3D_ParseMaterial_t = nullptr;
static Trampoline* GoToNextLevel_t          = nullptr;
static Trampoline* IncrementAct_t           = nullptr;
static Trampoline* LoadLevelFiles_t         = nullptr;
static Trampoline* SetLevelAndAct_t         = nullptr;
static Trampoline* GoToNextChaoStage_t      = nullptr;
static Trampoline* SetTimeOfDay_t           = nullptr;
static Trampoline* DrawLandTable_t          = nullptr;

DataArray(PaletteLight, LightPaletteData, 0x00903E88, 256);
DataArray(StageLightData, CurrentStageLights, 0x03ABD9F8, 4);

DataPointer(EntityData1*, Camera_Data1, 0x03B2CBB0);
DataPointer(NJS_COLOR, EntityVertexColor, 0x03D0848C);
DataPointer(NJS_COLOR, LandTableVertexColor, 0x03D08494);
DataPointer(PaletteLight, LSPalette, 0x03ABDAF0);
DataPointer(Uint32, LastRenderFlags, 0x03D08498);

static void DisplayLightDirection()
{
#ifdef _DEBUG
	using namespace globals;

	auto player = CharObj1Ptrs[0];
	if (player == nullptr)
		return;

	NJS_POINT3 points[2] = {
		player->Position,
		{}
	};

	int colors[2] = {};

	LineInfo info = {
		points, colors, 0, 2
	};

	points[1] = points[0];
	points[1].x += light_dir.x * 10.0f;
	colors[0] = colors[1] = 0xFFFF0000;
	DrawLineList(&info, 1, 0);

	points[1] = points[0];
	points[1].y += light_dir.y * 10.0f;
	colors[0] = colors[1] = 0xFF00FF00;
	DrawLineList(&info, 1, 0);

	points[1] = points[0];
	points[1].z += light_dir.z * 10.0f;
	colors[0] = colors[1] = 0xFF0000FF;
	DrawLineList(&info, 1, 0);

	points[1] = points[0];
	points[1].x += light_dir.x * 10.0f;
	points[1].y += light_dir.y * 10.0f;
	points[1].z += light_dir.z * 10.0f;
	colors[0] = colors[1] = 0xFFFFFF00;
	DrawLineList(&info, 1, 0);
#endif
}

static void SetMaterialParameters(const D3DMATERIAL9& material)
{
	using namespace d3d;

	if (!UsePalette() || effect == nullptr)
		return;

	// This will need to be re-evaluated for chunk models.
	//D3DMATERIALCOLORSOURCE colorsource;
	//device->GetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, (DWORD*)&colorsource);
	//effect->SetInt(param::DiffuseSource, colorsource);
	effect->SetVector(param::MaterialDiffuse, (D3DXVECTOR4*)&material.Diffuse);
}

static void __cdecl CorrectMaterial_r()
{
	using namespace d3d;
	D3DMATERIAL9 material; // [sp+8h] [bp-44h]@1

	device->GetMaterial(&material);
	material.Power = LSPalette.SP_pow;

	material.Ambient.r /= 255.0f;
	material.Ambient.g /= 255.0f;
	material.Ambient.b /= 255.0f;
	material.Ambient.a /= 255.0f;
	material.Diffuse.r /= 255.0f;
	material.Diffuse.g /= 255.0f;
	material.Diffuse.b /= 255.0f;
	material.Diffuse.a /= 255.0f;
	material.Specular.r /= 255.0f;
	material.Specular.g /= 255.0f;
	material.Specular.b /= 255.0f;
	material.Specular.a /= 255.0f;

	SetMaterialParameters(material);
	device->SetMaterial(&material);
}

static Uint32 last_flags = 0;
static Uint32 last_texid = 0xFFFFFFFF;
static NJS_TEXLIST* last_texlist = nullptr;

static void __fastcall Direct3D_ParseMaterial_r(NJS_MATERIAL* material)
{
	using namespace d3d;

	TARGET_DYNAMIC(Direct3D_ParseMaterial)(material);

	if (effect == nullptr)
		return;

	do_effect = false;

	auto pad = ControllerPointers[0];
	if (!UsePalette() || pad && pad->HeldButtons & Buttons_Z)
		return;

	Uint32 flags = material->attrflags;
	Uint32 texid = material->attr_texId & 0xFFFF;
	bool use_texture = (flags & NJD_FLAG_USE_TEXTURE) != 0;

	if (_nj_control_3d_flag_ & NJD_CONTROL_3D_CONSTANT_ATTR)
	{
		flags = _nj_constant_attr_or_ | _nj_constant_attr_and_ & flags;
	}

	globals::light = (flags & NJD_FLAG_IGNORE_LIGHT) == 0;
	SetPaletteLights(globals::light_type, globals::no_specular ? flags | NJD_FLAG_IGNORE_SPECULAR : flags);

	if ((last_flags & NJD_FLAG_USE_ENV) != (flags & NJD_FLAG_USE_ENV))
	{
		effect->SetBool(param::EnvironmentMapped, (flags & NJD_FLAG_USE_ENV) != 0);
	}

	if ((last_flags & NJD_FLAG_USE_ALPHA) != (flags & NJD_FLAG_USE_ALPHA))
	{
		effect->SetBool(param::AlphaEnabled, (flags & NJD_FLAG_USE_ALPHA) != 0);
	}

	if ((last_flags & NJD_FLAG_USE_TEXTURE) != (flags & NJD_FLAG_USE_TEXTURE))
	{
		effect->SetBool(param::TextureEnabled, use_texture);
	}

	if (use_texture && (Direct3D_CurrentTexList != last_texlist || texid != last_texid))
	{
		auto textures = Direct3D_CurrentTexList->textures;
		NJS_TEXMEMLIST* texmem = textures ? (NJS_TEXMEMLIST*)textures[texid].texaddr : nullptr;
		if (texmem != nullptr)
		{
			auto texture = (Direct3DTexture8*)texmem->texinfo.texsurface.pSurface;
			if (texture != nullptr)
			{
				last_texlist = Direct3D_CurrentTexList;
				last_texid = texid;
				effect->SetTexture(param::BaseTexture, texture->GetProxyInterface());
			}
		}
	}

	D3DMATERIAL9 mat;
	device->GetMaterial(&mat);
	SetMaterialParameters(mat);

	last_flags = flags;
	do_effect = true;
}

static void __cdecl CharSel_LoadA_r()
{
	auto original = TARGET_DYNAMIC(CharSel_LoadA);

	NJS_VECTOR dir = { 1.0f, -1.0f, -1.0f };

	njUnitVector(&dir);
	UpdateLightDirections(dir);

	LoadLanternPalette(LevelIDs_SkyDeck, 0);

	globals::last_level = CurrentLevel;
	globals::last_act = CurrentAct;

	original();
}

static void __cdecl SetLevelAndAct_r(Uint8 level, Uint8 act)
{
	TARGET_DYNAMIC(SetLevelAndAct)(level, act);
	LoadLanternFiles();
}

static void __cdecl GoToNextChaoStage_r()
{
	TARGET_DYNAMIC(GoToNextChaoStage)();
	switch (GetCurrentChaoStage())
	{
		case SADXChaoStage_StationSquare:
			CurrentLevel = LevelIDs_SSGarden;
			break;

		case SADXChaoStage_EggCarrier:
			CurrentLevel = LevelIDs_ECGarden;
			break;

		case SADXChaoStage_MysticRuins:
			CurrentLevel = LevelIDs_MRGarden;
			break;

		default:
			return;
	}

	LoadLanternFiles();
}

static void __cdecl GoToNextLevel_r()
{
	TARGET_DYNAMIC(GoToNextLevel)();
	LoadLanternFiles();
}

static void __cdecl IncrementAct_r(int amount)
{
	TARGET_DYNAMIC(IncrementAct)(amount);

	if (amount != 0)
	{
		LoadLanternFiles();
	}
}

static void __cdecl SetTimeOfDay_r(Sint8 time)
{
	TARGET_DYNAMIC(SetTimeOfDay)(time);
	LoadLanternFiles();
}

static void __cdecl LoadLevelFiles_r()
{
	TARGET_DYNAMIC(LoadLevelFiles)();
	LoadLanternFiles();
}

static void __cdecl DrawLandTable_r()
{
	auto flag = _nj_control_3d_flag_;
	auto or = _nj_constant_attr_or_;

	_nj_control_3d_flag_ |= NJD_CONTROL_3D_CONSTANT_ATTR;
	_nj_constant_attr_or_ |= NJD_FLAG_IGNORE_SPECULAR;

	TARGET_DYNAMIC(DrawLandTable)();

	_nj_control_3d_flag_ = flag;
	_nj_constant_attr_or_ = or;
}

extern "C"
{
	EXPORT ModInfo SADXModInfo = { ModLoaderVer };
	EXPORT void __cdecl Init(const char *path)
	{
		auto handle = GetModuleHandle(L"d3d9.dll");
		if (handle == nullptr)
		{
			MessageBoxA(WindowHandle, "Unable to detect Direct3D 9 DLL. The mod will not function.",
				"D3D9 not loaded", MB_OK | MB_ICONERROR);
			return;
		}

		globals::system = path;
		globals::system.append("\\system\\");

		d3d::InitTrampolines();
		CharSel_LoadA_t          = new Trampoline(0x00512BC0, 0x00512BC6, CharSel_LoadA_r);
		Direct3D_ParseMaterial_t = new Trampoline(0x00784850, 0x00784858, Direct3D_ParseMaterial_r);
		GoToNextLevel_t          = new Trampoline(0x00414610, 0x00414616, GoToNextLevel_r);
		IncrementAct_t           = new Trampoline(0x004146E0, 0x004146E5, IncrementAct_r);
		LoadLevelFiles_t         = new Trampoline(0x00422AD0, 0x00422AD8, LoadLevelFiles_r);
		SetLevelAndAct_t         = new Trampoline(0x00414570, 0x00414576, SetLevelAndAct_r);
		GoToNextChaoStage_t      = new Trampoline(0x00715130, 0x00715135, GoToNextChaoStage_r);
		SetTimeOfDay_t           = new Trampoline(0x00412C00, 0x00412C05, SetTimeOfDay_r);
		DrawLandTable_t          = new Trampoline(0x0043A6A0, 0x0043A6A8, DrawLandTable_r);

		// Correcting a function call since they're relative
		WriteCall(IncrementAct_t->Target(), (void*)0x00424830);

		// Material callback hijack
		WriteJump((void*)0x0040A340, CorrectMaterial_r);

		// Too lazy to use a trampoline
		WriteJump(Obj_Past, Obj_Past_r);

		FixCharacterMaterials();
	}

	EXPORT void __cdecl OnFrame()
	{
		auto pad = ControllerPointers[0];
		if (pad)
		{
			auto pressed = pad->PressedButtons;
			if (pressed & Buttons_C)
			{
				d3d::LoadShader();
				SetFogParameters();
			}

			if (pressed & Buttons_Left)
			{
				LoadLanternPalette(globals::system + "diffuse test.bin");
			}
			else if (pressed & Buttons_Right)
			{
				LoadLanternPalette(globals::system + "specular test.bin");
			}
			else if (pressed & Buttons_Down)
			{
				LoadLanternPalette(CurrentLevel, CurrentAct);
			}
		}

		if (d3d::effect == nullptr)
		{
			return;
		}

		DisplayLightDirection();
	}

	EXPORT void __cdecl OnRenderDeviceLost()
	{
		if (d3d::effect != nullptr)
		{
			d3d::effect->OnLostDevice();
			d3d::UpdateParameterHandles();
		}
	}

	EXPORT void __cdecl OnRenderDeviceReset()
	{
		if (d3d::effect != nullptr)
		{
			d3d::effect->OnResetDevice();
			d3d::UpdateParameterHandles();
			SetFogParameters();
		}
	}
}
