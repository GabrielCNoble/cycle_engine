attribute vec4 vPosition;
attribute vec4 vNormal;
attribute vec4 vTexCoord;


varying vec3 normal;
varying vec2 uv;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * vPosition;
	uv = vTexCoord;
	normal = vNormal;
}
