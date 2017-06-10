attribute vec4 vPosition;
attribute vec4 vNormal;
attribute vec4 vTangent;
//attribute vec4 vBTangent;
attribute vec2 vTexCoord;

varying vec3 bi_tangent;
varying vec3 normal;
varying vec2 UV;

void main()
{
    gl_Position=gl_ModelViewProjectionMatrix*vPosition;
    normal = gl_NormalMatrix * vNormal.xyz; 
    //bi_tangent = vec3(gl_ModelViewMatrix * vec4(vTangent.xyz, 0.0));
    //bi_normal = gl_NormalMatrix * vTangent.xyz;
    UV = vTexCoord;
}
