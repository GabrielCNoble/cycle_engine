attribute vec4 vPosition;
attribute vec2 vTexCoord;
attribute vec4 vTangent;
attribute vec4 vNormal;


varying vec4 position;
varying vec2 uv;
varying vec3 normal;
void main()
{
	position = gl_ModelViewProjectionMatrix * vPosition;
    gl_Position = position;
	uv = vTexCoord;
	normal = gl_NormalMatrix * vNormal.xyz;
	//uv = vec2(vPosition.x *0.5 + 0.5, vPosition.z * 0.5 + 0.5)
}
