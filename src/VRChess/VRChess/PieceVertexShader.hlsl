float4x4 ProjView;
float4x4 View;
float4x4 ViewInvTranspose;
float4 MasterCol;

void main(in  float4 Position  : POSITION,	
	in  float4 Normal  : NORMAL,    
	in  float4 Color : COLOR0, 
	in  float2 TexCoord  : TEXCOORD0,
	out float4 oPosition : SV_Position, 
	out float4 oPositionW : POSITION,
	out float4 oNormal : NORMAL,
	out float4 oColor: COLOR0, 
	out float2 oTexCoord : TEXCOORD0)
{	
	oPosition = mul(ProjView, Position); 
	oTexCoord = TexCoord; 
	oColor = MasterCol * Color;
	oPositionW = mul(View, Position);
	oNormal = mul(ViewInvTranspose, Normal);
}