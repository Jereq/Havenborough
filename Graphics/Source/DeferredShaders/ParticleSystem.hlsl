#pragma pack_matrix(row_major)

SamplerState m_textureSampler	: register(s0);
Texture2D diffuse				: register(t0);

cbuffer cb : register(b1)
{
	float4x4 view;
	float4x4 projection;
	float3 cameraPos;
}

cbuffer cbWorld : register(b2)
{
	float4x4 world;

}

struct VSIn
{
	float4 centerPos: POSITION;
	float4 eyePos	: EYEPOSITION;
	float2 uvCoord	: COORD;
	float2 size		: SIZE;
};

struct PSIn
{
	float4 centerPos: SV_POSITION;
	float4 eyePos	: EYEPOSITION;
	float2 uvCoord	: COORD;
	float4 color	: COLOR;
	float2 size		: SIZE;
};

struct GSOut
{
	float4 centerPos: SV_POSITION;
	float2 uvCoord	: COORD;
};

PSIn VS(VSIn input)
{
	PSIn output;
	
	output.centerPos = mul(projection, mul(view, mul(world, input.centerPos)));
	output.uvCoord = input.uvCoord;
	output.eyePos = mul(view, input.eyePos);
	//output.wPos = mul(world, input.centerPos);

	return output;
}

[maxvertexcount(4)]
GSOut GS(point VSIn input[1], inout TriangleStream<PSIn> triangleStream)
{
	// We need to create a matrix for the local coordinate system for the billboard of the given particle.
    // One axis points from the particle to the camera, one axis is the camera's side axis (for example to
    // the left) and the third one is perpendicular to both.
    GSOut outVertex = GSOut[0];

	float3 zAxis = normalize(eyePos - outVertex.centerPos);
	float3 xAxis = normalize(cross(float3(0,1,0), zAxis));
	float3 yAxis = cross(zAxis, xAxis);

	// The matrix to describe the local coordinate system is easily constructed:
	float4x4 localToWorld;
	localToWorld._11 = xAxis.x;
    localToWorld._21 = xAxis.y;
    localToWorld._31 = xAxis.z;
    localToWorld._12 = yAxis.x;
    localToWorld._22 = yAxis.y;
    localToWorld._32 = yAxis.z;
    localToWorld._13 = zAxis.x;
    localToWorld._23 = zAxis.y;
    localToWorld._33 = zAxis.z;
	localToWorld._14 = outVertex.centerPos.x;
    localToWorld._24 = outVertex.centerPos.y;
    localToWorld._34 = outVertex.centerPos.z;
    localToWorld._41 = 0;
    localToWorld._42 = 0;
    localToWorld._43 = 0;
    localToWorld._44 = 1;

	// And the matrix to transform from local to screen space.
    float4x4 transform = localToWorld * world * view * projection;

	// The positions of that quad is easily described in the local coordinate system:
    // -z points towards the camera, y points upwards and x towards the right.
    // The position marks the center of the quad, hence (0, 0, 0) is the center of the quad in
    // local coordinates and the quad has an edge-length of particle.Size to either side.
    PSIn v1, v2, v3, v4;

    float size = 0.5f; //????
    v1.centerPos = mul(float4(-size, size, 0, 1), transform);
    v1.uvCoord = float2(0, 0);
    v1.color    = triangleStream.color;
    v2.centerPos = mul(float4(size, size, 0, 1), transform);
    v2.uvCoord = float2(1, 0);
    v2.color    = triangleStream.color;
    v3.centerPos = mul(float4(-size,-size, 0, 1), transform);
    v3.uvCoord = float2(0, 1);
    v3.color    = triangleStream.color;
    v4.centerPos = mul(float4(size, -size, 0, 1), transform);
    v4.uvCoord = float2(1, 1);
    v4.color    = triangleStream.color;

    triangleStream.Append(v1);
    triangleStream.Append(v2);
    triangleStream.Append(v3);
    triangleStream.Append(v4);
}

PSOut PS(PSIn input)
{	
	input.color = diffuse.Sample(m_textureSampler, input.uvCoord);
	if(input.color.w == 0.f)
		discard;

	return output;
}
