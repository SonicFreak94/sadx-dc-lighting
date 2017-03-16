// Based on FixedFuncEMU.fx
// Copyright (c) 2005 Microsoft Corporation. All rights reserved.
//

// TODO: maybe convert all the preprocessor definitions to uniform booleans
// the result would be the same, but it would be easier to maintain

#define FOGMODE_NONE   0
#define FOGMODE_EXP    1
#define FOGMODE_EXP2   2
#define FOGMODE_LINEAR 3
#define E 2.71828
#define EPSILON 1e-5

#define D3DMCS_MATERIAL 0 // Color from material is used
#define D3DMCS_COLOR1   1 // Diffuse vertex color is used
#define D3DMCS_COLOR2   2 // Specular vertex color is used

#define D3DBLEND_ZERO            1
#define D3DBLEND_ONE             2
#define D3DBLEND_SRCCOLOR        3
#define D3DBLEND_INVSRCCOLOR     4
#define D3DBLEND_SRCALPHA        5
#define D3DBLEND_INVSRCALPHA     6
#define D3DBLEND_DESTALPHA       7
#define D3DBLEND_INVDESTALPHA    8
#define D3DBLEND_DESTCOLOR       9
#define D3DBLEND_INVDESTCOLOR    10
#define D3DBLEND_SRCALPHASAT     11

struct VS_IN
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
	float2 tex      : TEXCOORD0;
	float4 color    : COLOR0;
};

struct PS_IN
{
	float4 position : POSITION0;
	float4 diffuse  : COLOR0;
	float4 specular : COLOR1;
	float2 tex      : TEXCOORD0;
	float2 depth    : TEXCOORD1;
	float  fogDist  : FOG;
};

// This never changes
static float AlphaRef = 16.0f / 255.0f;

shared float4x4 WorldMatrix;
shared float4x4 wvMatrix;
shared float4x4 ProjectionMatrix;

shared float4 MaterialDiffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
shared uint   DiffuseSource = (uint)D3DMCS_COLOR1;

shared Texture2D BaseTexture;
sampler2D baseSampler = sampler_state
{
	Texture = <BaseTexture>;
};

// The inverse transpose of the world view matrix - used for environment mapping.
shared float4x4 wvMatrixInvT;
// Used primarily for environment mapping.
shared float4x4 TextureTransform = {
	-0.5, 0.0, 0.0, 0.0,
	 0.0, 0.5, 0.0, 0.0,
	 0.0, 0.0, 1.0, 0.0,
	 0.5, 0.5, 0.0, 1.0
};

shared Texture2D PaletteA;
sampler2D atlasSamplerA = sampler_state
{
	Texture   = <PaletteA>;
	MinFilter = Point;
	MagFilter = Point;
	AddressU  = Clamp;
	AddressV  = Clamp;
};

shared Texture2D PaletteB;
sampler2D atlasSamplerB = sampler_state
{
	Texture   = <PaletteB>;
	MinFilter = Point;
	MagFilter = Point;
	AddressU  = Clamp;
	AddressV  = Clamp;
};

shared Texture2D OpaqueDepth;
shared Texture2D AlphaDepth;

sampler opaqueDepthSampler = sampler_state
{
	Texture   = <OpaqueDepth>;
	MinFilter = Point;
	MagFilter = Point;
	AddressU  = Clamp;
	AddressV  = Clamp;
};
sampler alphaDepthSampler = sampler_state
{
	Texture   = <AlphaDepth>;
	MinFilter = Point;
	MagFilter = Point;
	AddressU  = Clamp;
	AddressV  = Clamp;
};

// Enables or disables depth tests against the previous alpha depth buffer.
shared bool AlphaDepthTest;

// This will likely need to be non-static for more robust control.
// Enabled or disabled depth tests against the opaque-only depth buffer.
static bool OpaqueDepthTest = true;
// Used for correcting screen-space coordinates to sample the depth buffer.
shared float2 ViewPort;

shared uint SourceBlend, DestinationBlend;

// Pre-adjusted on the CPU before being sent to the shader.
shared float DiffuseIndexA  = 0;
shared float DiffuseIndexB  = 0;
shared float SpecularIndexA = 0;
shared float SpecularIndexB = 0;

shared float3 LightDirection = float3(0.0f, -1.0f, 0.0f);
shared float3 NormalScale = float3(1, 1, 1);

shared uint   FogMode = (uint)FOGMODE_NONE;
shared float  FogStart;
shared float  FogEnd;
shared float  FogDensity;
shared float4 FogColor;

shared float BlendFactor = 0.0f;

#ifdef USE_SL

struct SourceLight_t
{
	int y, z;
	float3 ambient;
	float2 unknown;
	float power;
};

shared bool   UseSourceLight   = false;
shared float4 MaterialSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f);
shared float  MaterialPower    = 1.0f;

shared SourceLight_t SourceLight;

#endif

// Helpers

float4 GetDiffuse(in float4 vcolor)
{
	float4 color = (DiffuseSource == D3DMCS_COLOR1 && any(vcolor)) ? vcolor : MaterialDiffuse;

	if (floor(color.r * 255) == 178
		&& floor(color.g * 255) == 178
		&& floor(color.b * 255) == 178)
	{
		return float4(1, 1, 1, color.a);
	}

	return color;
}

#ifdef USE_FOG

float CalcFogFactor(float d)
{
	float fogCoeff;

	switch (FogMode)
	{
		default:
			break;

		case FOGMODE_EXP:
			fogCoeff = 1.0 / pow(E, d * FogDensity);
			break;

		case FOGMODE_EXP2:
			fogCoeff = 1.0 / pow(E, d * d * FogDensity * FogDensity);
			break;

		case FOGMODE_LINEAR:
			fogCoeff = (FogEnd - d) / (FogEnd - FogStart);
			break;
	}

	return clamp(fogCoeff, 0, 1);
}

#endif

PS_IN vs_main(VS_IN input)
{
	PS_IN output;

	float4 position;

	position = mul(float4(input.position, 1), wvMatrix);
	output.fogDist = position.z;
	position = mul(position, ProjectionMatrix);

	output.position = position;

#if defined(USE_TEXTURE) && defined(USE_ENVMAP)
	output.tex = (float2)mul(float4(input.normal, 1), wvMatrixInvT);
	output.tex = (float2)mul(float4(output.tex, 0, 1), TextureTransform);
#else
	output.tex = input.tex;
#endif

	output.depth = output.position.zw;

#ifdef USE_LIGHT
	{
		float3 worldNormal = mul(input.normal * NormalScale, (float3x3)WorldMatrix);

		// This is the "brightness index" calculation. Just a dot product
		// of the vertex normal (in world space) and the light direction.
		float _dot = dot(LightDirection, worldNormal);
		float4 diffuse = GetDiffuse(input.color);

		// The palette's brightest point is 0, and its darkest point is 1,
		// so we push the dot product (-1 .. 1) into the rage 0 .. 1, and
		// subtract it from 1. This is the value we use for indexing into
		// the palette.
		// HACK: This clamp prevents a visual bug in the Mystic Ruins past (shrine on fire)
		float i = floor(clamp(1 - (_dot + 1) / 2, 0, 0.99) * 255) / 255;

	#ifdef USE_SL
		{
			output.diffuse = max(0, float4(diffuse.rgb * SourceLight.ambient * _dot, diffuse.a));
			output.specular = max(0, float4(1, 1, 1, 0.0f) * pow(_dot, MaterialPower));
		}
	#else
		{
			float4 pdiffuse = tex2Dlod(atlasSamplerA, float4(i, DiffuseIndexA, 0, 0));
			float4 pspecular = tex2Dlod(atlasSamplerA, float4(i, SpecularIndexA, 0, 0));

		#ifdef USE_BLEND
			float4 bdiffuse = tex2Dlod(atlasSamplerB, float4(i, DiffuseIndexB, 0, 0));
			float4 bspecular = tex2Dlod(atlasSamplerB, float4(i, SpecularIndexB, 0, 0));

			pdiffuse = lerp(pdiffuse, bdiffuse, BlendFactor);
			pspecular = lerp(pspecular, bspecular, BlendFactor);
		#endif

			output.diffuse = float4((diffuse * pdiffuse).rgb, diffuse.a);
			output.specular = float4(pspecular.rgb, 0.0f);
		}
	#endif
	}
#else
	{
		// Just spit out the vertex or material color if lighting is off.
		output.diffuse = GetDiffuse(input.color);
		output.specular = 0;
	}
#endif

	return output;
}

uniform float alpha_bias = 1 / 255;

#ifdef USE_OIT
float4 ps_main(PS_IN input, float2 vpos : VPOS, out float4 blend : COLOR1) : COLOR0
#else
float4 ps_main(PS_IN input) : COLOR0
#endif
{
	float4 result;

#ifdef USE_TEXTURE
	result = tex2D(baseSampler, input.tex);
	result = result * input.diffuse + input.specular;
#else
	result = input.diffuse + input.specular;
#endif

#ifdef USE_ALPHA

	#ifdef USE_OIT
		if (result.a - alpha_bias <= EPSILON)
		{
			discard;
		}
	#else
		clip(result.a < AlphaRef ? -1 : 1);
	#endif

#endif

#ifdef USE_OIT
	float currentDepth = clamp(input.depth.x / input.depth.y, 0, 1);
	float2 depthcoord = vpos / ViewPort;

	// If opaque depth tests are enabled...
	if (OpaqueDepthTest)
	{
		// ...exclude any fragment whose depth exceeds that of any opaque fragment.
		// (equivalent to D3DCMP_LESS)
		float baseDepth = tex2D(opaqueDepthSampler, depthcoord).r;
		if (currentDepth - baseDepth >= EPSILON)
		{
			discard;
		}
	}

	// If alpha depth tests are enabled...
	if (AlphaDepthTest)
	{
		// ...discard any fragment whose depth is less than the last fragment depth.
		// (equivalent to D3DCMP_GREATER)
		float lastDepth = tex2D(alphaDepthSampler, depthcoord).r;
		if (currentDepth - lastDepth <= EPSILON)
		{
			discard;
		}
	}

	blend = float4((float)SourceBlend / 15.0f, (float)DestinationBlend / 15.0f, 0, 1);
#endif

#ifdef USE_FOG
	float factor = CalcFogFactor(input.fogDist);
	result.rgb = (factor * result + (1.0 - factor) * FogColor).rgb;
#endif

	return result;
}

technique Main
{
	pass p0
	{
		VertexShader = compile vs_3_0 vs_main();
		PixelShader = compile ps_3_0 ps_main();
	}
}
