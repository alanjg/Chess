Texture2D Texture   : register(t0); 
SamplerState Linear : register(s0); 

cbuffer lightInfo: register(b0)
{
	float3 eyePos;
	float3 lightDir;
	float4 ambientLight;
	float4 diffuseLight;
	float4 specularLight;
};

cbuffer material: register(b1)
{
	float4 ambientMaterial;
	float4 diffuseMaterial;
	float4 specularMaterial;
	float specPower;
};

float4 main(in float4 Position : SV_Position, 
	in float3 positionW : POSITION,
	in float3 normal : NORMAL,
	in float4 Color: COLOR0, 
	in float2 TexCoord : TEXCOORD0) 
	: SV_Target
{
	//interpolation can denormalize
	normal = normalize(normal);

	// Initialize outputs.
	float4 dif = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 flat = float4(0.0f, 0.0f, 0.0f, 0.0);
	
	float3 toEye = normalize(eyePos - positionW.xyz);
	float4 litColorT = flat + float4(normal.x, normal.y, normal.z, 0.0f);
	
	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -lightDir;
	
	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	float diffuseFactor = dot(lightVec, normal.xyz);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal.xyz);

		dif = diffuseFactor * diffuseMaterial * diffuseLight;
		
		if (specPower > 0.0f)
		{
			float specFactor = pow(max(dot(v, toEye), 0.0f), specPower);
			spec = specFactor * specularMaterial * specularLight;
		}		
	}

	flat = ambientLight * ambientMaterial;

	float4 litColor = dif + spec + flat;
	litColor.a = 1.0;

	float4 TexCol = Texture.Sample(Linear, TexCoord); 
    if (TexCol.a==0) clip(-1);  // If alpha = 0, don't draw
    return(Color * TexCol * litColor);
}