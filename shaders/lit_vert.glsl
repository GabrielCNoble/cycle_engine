attribute vec4 vPosition;
attribute vec2 vTexCoord;
attribute vec4 vTangent;
attribute vec4 vNormal;



uniform mat4 sysLastModelViewMatrix;


vec4 p;

varying vec3 normal;
varying vec4 position;
varying vec4 last_position;
varying vec3 tangent;
varying vec2 UV;
varying vec2 delta;
varying float entity_id;

void main()
{
    gl_Position=gl_ModelViewProjectionMatrix * vPosition;
    position = gl_ModelViewMatrix * vPosition;
	//last_position =  sysLastModelViewMatrix * vPosition;
	//delta = vec2(position.x - last_position.x, position.y - last_position.y);
    normal=gl_NormalMatrix*vNormal.xyz;
    tangent = gl_NormalMatrix * vTangent.xyz;
    UV = vTexCoord - vec2(0.0, 0.5);
}
