float4x4 ProjView;
float4x4 World;
float4x4 WorldInvTranspose;
float4 MasterCol;

void main(in  float4 Position  : POSITION,	
	in  float4 Normal  : NORMAL,    
	in  float4 Color : COLOR0, 
	in  float2 TexCoord  : TEXCOORD0,
	out float4 oPosition : SV_Position, 
	out float3 oPositionW : POSITION,
	out float3 oNormal : NORMAL,
	out float4 oColor: COLOR0, 
	out float2 oTexCoord : TEXCOORD0)
{	
	oPosition = mul(ProjView, Position); 
	oTexCoord = TexCoord; 
	oColor = MasterCol * Color;
	oPositionW = mul(World, Position);
	oNormal = mul(WorldInvTranspose, Normal);
}