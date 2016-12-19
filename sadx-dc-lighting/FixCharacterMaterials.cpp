#include "stdafx.h"
#include <SADXModLoader.h>
#include "FixCharacterMaterials.h"

static HMODULE chrmodels_handle;

inline int get_handle()
{
	if (chrmodels_handle == nullptr)
	{
		chrmodels_handle = GetModuleHandle(L"CHRMODELS_orig");
	}

	if (chrmodels_handle == nullptr)
	{
		throw;
	}

	return (int)chrmodels_handle;
}

void FixCharacterMaterials()
{
	auto handle = get_handle();
	
	// These will only be initialized once because they're static.
	DataPointer(NJS_MATERIAL, mat_0000D030, (0x0000D030 + handle));
	DataPointer(NJS_MATERIAL, mat_0000D044, (0x0000D044 + handle));
	DataPointer(NJS_MATERIAL, mat_00010B2C, (0x00010B2C + handle));
	DataPointer(NJS_MATERIAL, mat_00010B40, (0x00010B40 + handle));
	DataPointer(NJS_MATERIAL, mat_00011A00, (0x00011A00 + handle));
	DataPointer(NJS_MATERIAL, mat_00012048, (0x00012048 + handle));
	DataPointer(NJS_MATERIAL, mat_00015A00, (0x00015A00 + handle));
	DataPointer(NJS_MATERIAL, mat_00015DF8, (0x00015DF8 + handle));
	DataPointer(NJS_MATERIAL, mat_0011B788, (0x0011B788 + handle));
	DataPointer(NJS_MATERIAL, mat_0011B978, (0x0011B978 + handle));
	DataPointer(NJS_MATERIAL, mat_00127554, (0x00127554 + handle));
	DataPointer(NJS_MATERIAL, mat_001276B4, (0x001276B4 + handle));
	DataPointer(NJS_MATERIAL, mat_001284F0, (0x001284F0 + handle));
	DataPointer(NJS_MATERIAL, mat_00128504, (0x00128504 + handle));
	DataPointer(NJS_MATERIAL, mat_001286B8, (0x001286B8 + handle));
	DataPointer(NJS_MATERIAL, mat_001286CC, (0x001286CC + handle));
	DataPointer(NJS_MATERIAL, mat_001286E0, (0x001286E0 + handle));
	DataPointer(NJS_MATERIAL, mat_001FDBB4, (0x001FDBB4 + handle));
	DataPointer(NJS_MATERIAL, mat_001FDBC8, (0x001FDBC8 + handle));
	DataPointer(NJS_MATERIAL, mat_00200540, (0x00200540 + handle));
	DataPointer(NJS_MATERIAL, mat_00200554, (0x00200554 + handle));
	DataPointer(NJS_MATERIAL, mat_002009E8, (0x002009E8 + handle));
	DataPointer(NJS_MATERIAL, mat_00200DD4, (0x00200DD4 + handle));
	DataPointer(NJS_MATERIAL, mat_00204EF0, (0x00204EF0 + handle));
	DataPointer(NJS_MATERIAL, mat_002054FC, (0x002054FC + handle));
	DataPointer(NJS_MATERIAL, mat_00205510, (0x00205510 + handle));
	DataPointer(NJS_MATERIAL, mat_00206314, (0x00206314 + handle));
	DataPointer(NJS_MATERIAL, mat_00206924, (0x00206924 + handle));
	DataPointer(NJS_MATERIAL, mat_00206938, (0x00206938 + handle));
	DataPointer(NJS_MATERIAL, mat_0020B9E0, (0x0020B9E0 + handle));
	DataPointer(NJS_MATERIAL, mat_002D71A8, (0x002D71A8 + handle));
	DataPointer(NJS_MATERIAL, mat_002D7934, (0x002D7934 + handle));
	DataPointer(NJS_MATERIAL, mat_002D8930, (0x002D8930 + handle));
	DataPointer(NJS_MATERIAL, mat_002D90BC, (0x002D90BC + handle));
	DataPointer(NJS_MATERIAL, mat_002E3C28, (0x002E3C28 + handle));
	DataPointer(NJS_MATERIAL, mat_002E43B4, (0x002E43B4 + handle));
	DataPointer(NJS_MATERIAL, mat_002E53B0, (0x002E53B0 + handle));
	DataPointer(NJS_MATERIAL, mat_002E5B3C, (0x002E5B3C + handle));
	DataPointer(NJS_MATERIAL, mat_004202A4, (0x004202A4 + handle));
	DataPointer(NJS_MATERIAL, mat_004208CC, (0x004208CC + handle));
	DataPointer(NJS_MATERIAL, mat_00421A04, (0x00421A04 + handle));
	DataPointer(NJS_MATERIAL, mat_0042202C, (0x0042202C + handle));
	DataPointer(NJS_MATERIAL, mat_00426884, (0x00426884 + handle));
	DataPointer(NJS_MATERIAL, mat_00426BC4, (0x00426BC4 + handle));
	DataPointer(NJS_MATERIAL, mat_00433774, (0x00433774 + handle));
	DataPointer(NJS_MATERIAL, mat_00433AB4, (0x00433AB4 + handle));
	DataPointer(NJS_MATERIAL, mat_0046E05C, (0x0046E05C + handle));
	DataPointer(NJS_MATERIAL, mat_0046E670, (0x0046E670 + handle));
	DataPointer(NJS_MATERIAL, mat_0046E684, (0x0046E684 + handle));
	DataPointer(NJS_MATERIAL, mat_0046E698, (0x0046E698 + handle));
	DataPointer(NJS_MATERIAL, mat_0046E6AC, (0x0046E6AC + handle));
	DataPointer(NJS_MATERIAL, mat_0046EE8C, (0x0046EE8C + handle));
	DataPointer(NJS_MATERIAL, mat_0046F4A0, (0x0046F4A0 + handle));
	DataPointer(NJS_MATERIAL, mat_0046F4B4, (0x0046F4B4 + handle));
	DataPointer(NJS_MATERIAL, mat_0046F4C8, (0x0046F4C8 + handle));
	DataPointer(NJS_MATERIAL, mat_0046F4DC, (0x0046F4DC + handle));
	DataPointer(NJS_MATERIAL, mat_00579C94, (0x00579C94 + handle));
	DataPointer(NJS_MATERIAL, mat_00582D08, (0x00582D08 + handle));

	mat_0000D030.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // Rings with env map
	mat_0000D044.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // Rings with env map
	mat_00010B2C.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // Rings with env map
	mat_00010B40.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // Rings with env map
	mat_00011A00.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_00012048.attrflags |=  NJD_FLAG_IGNORE_LIGHT;
	mat_00015A00.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_00015DF8.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_0011B788.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // eye white 2
	mat_0011B978.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // eye white 1
	mat_00127554.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_001276B4.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_001284F0.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_00128504.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_001286B8.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_001286CC.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_001286E0.attrflags |=  NJD_FLAG_IGNORE_SPECULAR;
	mat_001FDBB4.attrflags |=  NJD_FLAG_IGNORE_LIGHT;    // object_001FDF7C
	mat_001FDBC8.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // object_001FDF7C
	mat_00200540.attrflags |=  NJD_FLAG_IGNORE_LIGHT;    // object_0020098C
	mat_00200554.attrflags |=  NJD_FLAG_IGNORE_LIGHT;    // object_0020098C
	mat_002009E8.attrflags |=  NJD_FLAG_IGNORE_LIGHT;    // object_00200D14
	mat_00200DD4.attrflags |=  NJD_FLAG_IGNORE_LIGHT;    // object_00201AE4
	mat_00204EF0.attrflags |=  NJD_FLAG_IGNORE_LIGHT;    // object_0020524C
	mat_002054FC.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // object_00205794
	mat_00205510.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // object_00205794
	mat_00206314.attrflags |=  NJD_FLAG_IGNORE_LIGHT;    // object_00206674
	mat_00206924.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // object_00206BBC
	mat_00206938.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // object_00206BBC
	mat_0020B9E0.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // object_0020BCAC
	mat_002D71A8.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_002D7934.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_002D8930.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_002D90BC.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_002E3C28.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_002E43B4.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_002E53B0.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_002E5B3C.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_004202A4.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_004208CC.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_00421A04.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0042202C.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_00426884.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // eye 2
	mat_00426BC4.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // eye 1
	mat_00433774.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // eye 2
	mat_00433AB4.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // eye 1
	mat_0046E05C.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046E670.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046E684.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046E698.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046E6AC.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046EE8C.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046F4A0.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046F4B4.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046F4C8.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_0046F4DC.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR;
	mat_00579C94.attrflags |=  NJD_FLAG_IGNORE_SPECULAR; // object_0057BC44
	mat_00582D08.attrflags |=  NJD_FLAG_IGNORE_LIGHT;    // object_00583284

	FixCharacterMaterials_OnFrame();
}

void FixCharacterMaterials_OnFrame()
{
	auto handle = get_handle();

	// These will only be initialized once because they're static.
	DataPointer(NJS_MATERIAL, mat_00565C68, (0x00565C68 + handle));
	DataPointer(NJS_MATERIAL, mat_00426F18, (0x00426F18 + handle));
	DataPointer(NJS_MATERIAL, mat_00426F2C, (0x00426F2C + handle));
	DataPointer(NJS_MATERIAL, mat_002DD8FC, (0x002DD8FC + handle));
	DataPointer(NJS_MATERIAL, mat_002DD1E4, (0x002DD1E4 + handle));
	DataPointer(NJS_MATERIAL, mat_002E9C64, (0x002E9C64 + handle));
	DataPointer(NJS_MATERIAL, mat_00012358, (0x00012358 + handle));
	DataPointer(NJS_MATERIAL, mat_0011BB58, (0x0011BB58 + handle));

	mat_00565C68.attrflags |= NJD_FLAG_IGNORE_SPECULAR; // Sonic's nose
	mat_00426F18.attrflags |= NJD_FLAG_IGNORE_SPECULAR; // Tails' eye whites
	mat_00426F2C.attrflags |= NJD_FLAG_IGNORE_SPECULAR; // Tails' nose
	mat_002DD8FC.attrflags |= NJD_FLAG_IGNORE_SPECULAR; // Knuckles' nose
	mat_002DD1E4.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR; // Knuckles' eye whites
	mat_002E9C64.attrflags &= ~NJD_FLAG_IGNORE_SPECULAR; // Knuckles' eye whites
	mat_00012358.attrflags |= NJD_FLAG_IGNORE_SPECULAR; // Amy's nose
	mat_0011BB58.attrflags |= NJD_FLAG_IGNORE_SPECULAR; // Amy's nose

}
