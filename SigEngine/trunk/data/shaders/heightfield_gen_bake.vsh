#pragma pack_matrix(row_major)
float3x4 gLocalToWorld	: register( c0 );
float4x4 gWorldToProj	: register( c3 );
float4 gWorldEyePos		: register( c7 );

//compressed heightfield verts
struct tVSInput
{
	float4 vP : POSITION;
	float2 vN : NORMAL;
};

struct tVSOutput
{
	float4 vCP	: POSITION;
	float3 vLP	: TEXCOORD0;
};

tVSOutput main( tVSInput i )
{
	tVSOutput o;
	float3 pworld = mul( gLocalToWorld, float4( i.vP.xyz, 1.f ) );
	o.vCP = mul( gWorldToProj, float4( pworld, 1.f ) );
	o.vLP = i.vP.xyz;
	return o;
}
