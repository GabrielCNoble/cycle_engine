attribute vec4 vPosition;
attribute vec4 vTangent;
attribute vec4 vNormal;
attribute vec2 vTexCoord;

varying vec3 position;
varying vec3 normal;
varying vec3 tangent;
varying vec2 uv;

void main()
{
	position = vec3(gl_ModelViewMatrix * vPosition);
	gl_Position = gl_ProjectionMatrix * vec4(position, 1.0);
	uv = vTexCoord;
	normal = gl_NormalMatrix * vNormal.xyz;
	tangent = gl_NormalMatrix * vTangent.xyz;
}
