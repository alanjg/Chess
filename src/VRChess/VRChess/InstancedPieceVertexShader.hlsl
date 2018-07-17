float4x4 modelViewProj[2];
float4 MasterCol;
void main(in  float4 Position  : POSITION,	
	in  float4 Normal  : NORMAL,    
	in  float4 Color : COLOR0, 
	in  float2 TexCoord  : TEXCOORD0,
    in  uint instanceID : SV_InstanceID, 
    out float4 oPosition : SV_Position, 
	out float4 oColor: COLOR0, 
	out float2 oTexCoord : TEXCOORD0,
    out float oClipDist : SV_ClipDistance0, 
	out float oCullDist : SV_CullDistance0)
{
   const float4 EyeClipPlane[2] = { { -1, 0, 0, 0 }, { 1, 0, 0, 0 } };
   uint eyeIndex = instanceID & 1;
// transform to clip space for correct eye (includes offset and scale)
   oPosition = mul(modelViewProj[eyeIndex], Position); 
// calculate distance from left/right clip plane (try setting to 0 to see why clipping is necessary)
   oCullDist = oClipDist = dot(EyeClipPlane[eyeIndex], oPosition);
   oTexCoord = TexCoord; 
   oColor = MasterCol * Color;
}