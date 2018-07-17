Texture2D Texture   : register(t0); 
SamplerState Linear : register(s0); 

cbuffer eyePos: register(b0)
{
	float3 eyePos;
	float3 lightDir;
};

cbuffer material: register(b1)
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float specPower;
};

float4 main(in float4 Position : SV_Position, 
	in float4 positionW : POSITION,
	in float4 normal : NORMAL,
	in float4 Color: COLOR0, 
	in float2 TexCoord : TEXCOORD0) 
	: SV_Target
{
	//interpolation can denormalize
	normal = normalize(normal);

	
	// Initialize outputs.
	float4 dif = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 flat = float4(0.2f, 0.2f, 0.2f, 1.0);
	return diffuse;
	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -lightDir;
	float3 top = float3(0, -1, 0);
	float ndotl = dot(top, normal.xyz);
	float4 litColorF = flat;
	if (ndotl > 0.0f)
	{
		litColorF += ndotl;
	}
	
	litColorF.a = 1.0f;
	float3 toEye = normalize(eyePos - positionW.xyz);
	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal.xyz);
	const float specPower = 2.0;
	float4 specularMaterial = specular;
	float4 diffuseMaterial = diffuse;

	float4 specularLight = float4(0.5f, 0.5f, 0.5f, 1.0f);
	float4 diffuseLight = float4(0.5f, 0.5f, 0.5f, 1.0f);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal.xyz);
		
		float specFactor = pow(max(dot(v, toEye), 0.0f), specPower);

		dif = diffuseFactor * diffuseMaterial * diffuseLight;
		spec = specFactor * specularMaterial * specularLight;
	}
	float4 litColor = dif + spec;
	//float4 litColor = flat;
	litColor.a = 1.0;

	float4 TexCol = Texture.Sample(Linear, TexCoord); 
    if (TexCol.a==0) clip(-1);  // If alpha = 0, don't draw
    return(Color * TexCol * litColorF);
}