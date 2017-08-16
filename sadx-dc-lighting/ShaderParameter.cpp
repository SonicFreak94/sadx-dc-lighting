#include "stdafx.h"
#include "ShaderParameter.h"

std::vector<IShaderParameter*> IShaderParameter::values_assigned {};

template<>
bool ShaderParameter<bool>::Commit(IDirect3DDevice9* device)
{
	if (Modified())
	{
		BOOL value = current;
		device->SetVertexShaderConstantB(index, &value, 1);
		device->SetPixelShaderConstantB(index, &value, 1);

		Clear();
		return true;
	}

	assigned = false;
	return false;
}

template<>
bool ShaderParameter<int>::Commit(IDirect3DDevice9* device)
{
	if (Modified())
	{
		int buffer[4] = { current, current, current, current };
		device->SetVertexShaderConstantI(index, buffer, 1);
		device->SetPixelShaderConstantI(index, buffer, 1);

		Clear();
		return true;
	}

	assigned = false;
	return false;
}

template<>
bool ShaderParameter<float>::Commit(IDirect3DDevice9* device)
{
	if (Modified())
	{
		D3DXVECTOR4 value = { current, current, current, current };

		device->SetVertexShaderConstantF(index, value, 1);
		device->SetPixelShaderConstantF(index, value, 1);

		Clear();
		return true;
	}

	assigned = false;
	return false;
}

template<>
bool ShaderParameter<D3DXVECTOR4>::Commit(IDirect3DDevice9* device)
{
	if (Modified())
	{
		device->SetVertexShaderConstantF(index, current, 1);
		device->SetPixelShaderConstantF(index, current, 1);

		Clear();
		return true;
	}

	assigned = false;
	return false;
}

template<>
bool ShaderParameter<D3DXVECTOR3>::Commit(IDirect3DDevice9* device)
{
	if (Modified())
	{
		D3DXVECTOR4 value = { current.x, current.y, current.z, 1.0f };

		device->SetVertexShaderConstantF(index, value, 1);
		device->SetPixelShaderConstantF(index, value, 1);

		Clear();
		return true;
	}

	assigned = false;
	return false;
}

template<>
bool ShaderParameter<D3DXCOLOR>::Commit(IDirect3DDevice9* device)
{
	if (Modified())
	{
		static_assert(sizeof(D3DXCOLOR) == sizeof(D3DXVECTOR4), "D3DXCOLOR size does not match D3DXVECTOR4.");
		device->SetVertexShaderConstantF(index, current, 1);
		device->SetPixelShaderConstantF(index, current, 1);

		Clear();
		return true;
	}

	assigned = false;
	return false;
}

template<>
bool ShaderParameter<D3DXMATRIX>::Commit(IDirect3DDevice9* device)
{
	if (Modified())
	{
		auto result = device->SetVertexShaderConstantF(index, current, 4);
		//device->SetPixelShaderConstantF(index, current, 4);

		Clear();
		return true;
	}

	assigned = false;
	return false;
}

template<>
bool ShaderParameter<Texture>::Commit(IDirect3DDevice9* device)
{
	if (Modified())
	{
		device->SetTexture(index, current);

		Clear();
		return true;
	}

	assigned = false;
	return false;
}

template<>
void ShaderParameter<Texture>::Release()
{
	Clear();
	current = nullptr;
	last = nullptr;
}
