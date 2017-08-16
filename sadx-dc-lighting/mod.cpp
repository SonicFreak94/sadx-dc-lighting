#include "stdafx.h"
#include <d3d9.h>

// Mod loader
#include <SADXModLoader.h>
#include <Trampoline.h>

// MinHook
#include <MinHook.h>

// Local
#include "d3d.h"
#include "datapointers.h"
#include "globals.h"
#include "lantern.h"
#include "../include/lanternapi.h"
#include "Obj_Past.h"
#include "Obj_SkyDeck.h"
#include "Obj_Chaos7.h"
#include "FixChaoGardenMaterials.h"
#include "FixCharacterMaterials.h"

static Trampoline* CharSel_LoadA_t                 = nullptr;
static Trampoline* Direct3D_ParseMaterial_t        = nullptr;
static Trampoline* GoToNextLevel_t                 = nullptr;
static Trampoline* IncrementAct_t                  = nullptr;
static Trampoline* LoadLevelFiles_t                = nullptr;
static Trampoline* SetLevelAndAct_t                = nullptr;
static Trampoline* GoToNextChaoStage_t             = nullptr;
static Trampoline* SetTimeOfDay_t                  = nullptr;
static Trampoline* DrawLandTable_t                 = nullptr;
static Trampoline* Direct3D_SetTexList_t           = nullptr;
static Trampoline* SetCurrentStageLights_t         = nullptr;
static Trampoline* SetCurrentStageLight_EggViper_t = nullptr;

DataArray(PaletteLight, LightPaletteData, 0x00903E88, 256);
DataArray(StageLightData, CurrentStageLights, 0x03ABD9F8, 4);
DataArray(NJS_TEXLIST*, LevelObjTexlists, 0x03B290B4, 4);

DataPointer(NJS_COLOR, EntityVertexColor, 0x03D0848C);
DataPointer(NJS_COLOR, LandTableVertexColor, 0x03D08494);
DataPointer(PaletteLight, LSPalette, 0x03ABDAF0);
DataPointer(Uint32, LastRenderFlags, 0x03D08498);
DataPointer(NJS_VECTOR, NormalScaleMultiplier, 0x03B121F8);
DataPointer(NJS_TEXLIST*, CommonTextures, 0x03B290B0);

#ifdef _DEBUG
static void DisplayLightDirection()
{
	using namespace globals;

	auto player = CharObj1Ptrs[0];
	if (player == nullptr)
		return;

	NJS_POINT3 points[2] = {
		player->Position,
		{}
	};

	NJS_COLOR colors[2] = {};

	NJS_POINT3COL info = {
		points, colors, nullptr, 2
	};

	points[1] = points[0];
	points[1].x += light_dir.x * 10.0f;
	colors[0].color = colors[1].color = 0xFFFF0000;
	DrawLineList(&info, 1, 0);

	points[1] = points[0];
	points[1].y += light_dir.y * 10.0f;
	colors[0].color = colors[1].color = 0xFF00FF00;
	DrawLineList(&info, 1, 0);

	points[1] = points[0];
	points[1].z += light_dir.z * 10.0f;
	colors[0].color = colors[1].color = 0xFF0000FF;
	DrawLineList(&info, 1, 0);

	points[1] = points[0];
	points[1].x += light_dir.x * 10.0f;
	points[1].y += light_dir.y * 10.0f;
	points[1].z += light_dir.z * 10.0f;
	colors[0].color = colors[1].color = 0xFFFFFF00;
	DrawLineList(&info, 1, 0);
}
#endif

static void SetMaterialParameters(const D3DMATERIAL9& material)
{
	using namespace d3d;

	if (!LanternInstance::UsePalette() || !shaders_not_null())
		return;

	D3DMATERIALCOLORSOURCE colorsource;
	device->GetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, (DWORD*)&colorsource);

	param::DiffuseSource   = colorsource;
	param::MaterialDiffuse = material.Diffuse;

#ifdef USE_SL
	param::MaterialSpecular = material.Specular;
	param::MaterialPower    = material.Power;
#endif
}

static void __cdecl CorrectMaterial_r()
{
	using namespace d3d;
	D3DMATERIAL9 material;

	device->GetMaterial(&material);

	if (!LanternInstance::UsePalette())
	{
		material.Power = LSPalette.SP_pow;
	}

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

static void __fastcall Direct3D_ParseMaterial_r(NJS_MATERIAL* material)
{
	using namespace d3d;

	TARGET_DYNAMIC(Direct3D_ParseMaterial)(material);

	if (!shaders_not_null())
	{
		return;
	}

	do_effect = false;

	if (!LanternInstance::UsePalette())
	{
		return;
	}

	reset_overrides();

#ifdef _DEBUG
	auto pad = ControllerPointers[0];
	if (pad && pad->HeldButtons & Buttons_Z)
	{
		return;
	}
#endif

	Uint32 flags = material->attrflags;
	//Uint32 texid = material->attr_texId & 0xFFFF;

	if (material->specular.argb.a == 0)
	{
		flags |= NJD_FLAG_IGNORE_SPECULAR;
	}

	if (globals::first_material)
	{
		static constexpr auto FLAG_MASK = NJD_FLAG_IGNORE_SPECULAR;

		_nj_constant_attr_and_ &= ~FLAG_MASK;

		if (!(_nj_control_3d_flag_ & NJD_CONTROL_3D_CONSTANT_ATTR))
		{
			_nj_constant_attr_or_ = flags & FLAG_MASK;
		}
		else
		{
			_nj_constant_attr_or_ &= ~FLAG_MASK;
			_nj_constant_attr_or_ |= flags & FLAG_MASK;
		}

		globals::first_material = false;
		_nj_control_3d_flag_ |= NJD_CONTROL_3D_CONSTANT_ATTR;
	}

	if (_nj_control_3d_flag_ & NJD_CONTROL_3D_CONSTANT_ATTR)
	{
		flags = _nj_constant_attr_or_ | _nj_constant_attr_and_ & flags;
	}

	globals::palettes.SetPalettes(globals::light_type, flags);

	bool use_texture = (flags & NJD_FLAG_USE_TEXTURE) != 0;
	shader_flags(ShaderFlags_Texture, use_texture);
	shader_flags(ShaderFlags_Alpha, (flags & NJD_FLAG_USE_ALPHA) != 0);
	shader_flags(ShaderFlags_EnvMap, (flags & NJD_FLAG_USE_ENV) != 0);
	shader_flags(ShaderFlags_Light, (flags & NJD_FLAG_IGNORE_LIGHT) == 0);

	// Environment map matrix
	param::TextureTransform = *(D3DXMATRIX*)0x038A5DD0;
	
	/*if (use_texture)
	{
		auto textures = Direct3D_CurrentTexList->textures;
		NJS_TEXMEMLIST* texmem = textures ? (NJS_TEXMEMLIST*)textures[texid].texaddr : nullptr;
		if (texmem != nullptr)
		{
			auto texture = (Direct3DTexture8*)texmem->texinfo.texsurface.pSurface;
			if (texture != nullptr)
			{
				param::BaseTexture = texture->GetProxyInterface();
			}
		}
	}*/

	D3DMATERIAL9 mat;
	device->GetMaterial(&mat);
	SetMaterialParameters(mat);

	do_effect = true;

	if (globals::material_callbacks.empty())
	{
		return;
	}

	auto it = globals::material_callbacks.find(material);
	if (it == globals::material_callbacks.end())
	{
		return;
	}

	for (auto& cb : it->second)
	{
		if (cb(material, flags))
		{
			break;
		}
	}
}

static void __cdecl CharSel_LoadA_r()
{
	auto original = TARGET_DYNAMIC(CharSel_LoadA);

	globals::palettes.LoadPalette(LevelIDs_SkyDeck, 0);
	globals::palettes.SetLastLevel(CurrentLevel, CurrentAct);

	NJS_VECTOR dir = { 1.0f, -1.0f, -1.0f };
	njUnitVector(&dir);
	globals::palettes.LightDirection(dir);

	original();
}

static void __cdecl SetLevelAndAct_r(Uint8 level, Uint8 act)
{
	TARGET_DYNAMIC(SetLevelAndAct)(level, act);
	globals::palettes.LoadFiles();
}

static void __cdecl GoToNextChaoStage_r()
{
	TARGET_DYNAMIC(GoToNextChaoStage)();

	auto level = CurrentLevel;
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

	globals::palettes.LoadFiles();
	CurrentLevel = level;
}

static void __cdecl GoToNextLevel_r()
{
	TARGET_DYNAMIC(GoToNextLevel)();
	globals::palettes.LoadFiles();
}

static void __cdecl IncrementAct_r(int amount)
{
	TARGET_DYNAMIC(IncrementAct)(amount);

	if (amount != 0)
	{
		globals::palettes.LoadFiles();
	}
}

static void __cdecl SetTimeOfDay_r(Sint8 time)
{
	TARGET_DYNAMIC(SetTimeOfDay)(time);
	globals::palettes.LoadFiles();
}

static void __cdecl LoadLevelFiles_r()
{
	TARGET_DYNAMIC(LoadLevelFiles)();
	globals::palettes.LoadFiles();
}

static void __cdecl DrawLandTable_r()
{
	if (globals::landtable_specular)
	{
		TARGET_DYNAMIC(DrawLandTable)();
		return;
	}

	auto flag = _nj_control_3d_flag_;
	auto or = _nj_constant_attr_or_;

	_nj_control_3d_flag_ |= NJD_CONTROL_3D_CONSTANT_ATTR;
	_nj_constant_attr_or_ |= NJD_FLAG_IGNORE_SPECULAR;

	TARGET_DYNAMIC(DrawLandTable)();

	_nj_control_3d_flag_ = flag;
	_nj_constant_attr_or_ = or;
}

static Sint32 __fastcall Direct3D_SetTexList_r(NJS_TEXLIST* texlist)
{
	if (texlist != Direct3D_CurrentTexList)
	{
		param::AllowVertexColor = true;

		if (!globals::light_type)
		{
			if (texlist == CommonTextures)
			{
				globals::palettes.SetPalettes(0, 0);
				param::AllowVertexColor = globals::object_vcolor;
			}
			else
			{
				for (int i = 0; i < 4; i++)
				{
					if (LevelObjTexlists[i] != texlist)
					{
						continue;
					}

					globals::palettes.SetPalettes(0, 0);
					param::AllowVertexColor = globals::object_vcolor;
					break;
				}
			}
		}
	}

	return TARGET_DYNAMIC(Direct3D_SetTexList)(texlist);
}

static void __cdecl NormalScale(float x, float y, float z)
{
	if (x > FLT_EPSILON || y > FLT_EPSILON || z > FLT_EPSILON)
	{
		param::NormalScale = D3DXVECTOR3(x, y, z);
	}
	else
	{
		param::NormalScale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	}
}

void setStageLightDirection()
{
	if (globals::palettes.Size())
	{
		const auto& dir = globals::palettes.LightDirection();
		CurrentStageLights[0].direction = dir;
		CurrentStageLights[1].direction = dir;
		CurrentStageLights[2].direction = dir;
		CurrentStageLights[3].direction = dir;
	}
}

void __cdecl SetCurrentStageLights_r(int level, int act)
{
	TARGET_DYNAMIC(SetCurrentStageLights)(level, act);
	setStageLightDirection();
}

void __cdecl SetCurrentStageLight_EggViper_r(ObjectMaster* a1)
{
	TARGET_DYNAMIC(SetCurrentStageLight_EggViper)(a1);
	setStageLightDirection();
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

		MH_Initialize();

		LanternInstance base(&param::PaletteA);
		globals::palettes.Add(base);

		globals::mod_path    = path;
		globals::system_path = globals::mod_path + "\\system\\";
		globals::cache_path  = globals::mod_path + "\\cache\\";
		globals::shader_path = globals::system_path + "lantern.hlsl";

		d3d::InitTrampolines();

		CharSel_LoadA_t                 = new Trampoline(0x00512BC0, 0x00512BC6, CharSel_LoadA_r);
		Direct3D_ParseMaterial_t        = new Trampoline(0x00784850, 0x00784858, Direct3D_ParseMaterial_r);
		GoToNextLevel_t                 = new Trampoline(0x00414610, 0x00414616, GoToNextLevel_r);
		IncrementAct_t                  = new Trampoline(0x004146E0, 0x004146E5, IncrementAct_r);
		LoadLevelFiles_t                = new Trampoline(0x00422AD0, 0x00422AD8, LoadLevelFiles_r);
		SetLevelAndAct_t                = new Trampoline(0x00414570, 0x00414576, SetLevelAndAct_r);
		GoToNextChaoStage_t             = new Trampoline(0x00715130, 0x00715135, GoToNextChaoStage_r);
		SetTimeOfDay_t                  = new Trampoline(0x00412C00, 0x00412C05, SetTimeOfDay_r);
		DrawLandTable_t                 = new Trampoline(0x0043A6A0, 0x0043A6A8, DrawLandTable_r);
		Direct3D_SetTexList_t           = new Trampoline(0x0077F3D0, 0x0077F3D8, Direct3D_SetTexList_r);
		SetCurrentStageLights_t         = new Trampoline(0x0040A950, 0x0040A955, SetCurrentStageLights_r);
		SetCurrentStageLight_EggViper_t = new Trampoline(0x0057E560, 0x0057E567, SetCurrentStageLight_EggViper_r);

		// Material callback hijack
		WriteJump((void*)0x0040A340, CorrectMaterial_r);

		FixCharacterMaterials();
		FixChaoGardenMaterials();
		Past_Init();
		SkyDeck_Init();
		Chaos7_Init();

		// Vertex normal correction for certain objects in
		// Red Mountain and Sky Deck.
		WriteCall((void*)0x00411EDA, NormalScale);
		WriteCall((void*)0x00411F1D, NormalScale);
		WriteCall((void*)0x00411F44, NormalScale);
		WriteCall((void*)0x00412783, NormalScale);

		NormalScaleMultiplier = { 1.0f, 1.0f, 1.0f };
	}

#ifdef _DEBUG
	EXPORT void __cdecl OnFrame()
	{
		auto pad = ControllerPointers[0];
		if (pad)
		{
			auto pressed = pad->PressedButtons;
			if (pressed & Buttons_C)
			{
				d3d::load_shader();
			}

			if (pressed & Buttons_Left)
			{
				globals::palettes.LoadPalette(globals::system_path + "diffuse test.bin");
			}
			else if (pressed & Buttons_Right)
			{
				globals::palettes.LoadPalette(globals::system_path + "specular test.bin");
			}
			else if (pressed & Buttons_Down)
			{
				globals::palettes.LoadPalette(CurrentLevel, CurrentAct);
			}
		}

		if (!d3d::shaders_not_null())
		{
			return;
		}

		DisplayLightDirection();
	}
#endif
}
