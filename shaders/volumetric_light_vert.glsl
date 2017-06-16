
#define LIGHT_POINT 1
#define LIGHT_SPOT 2
#define LIGHT_DIRECTIONAL 4

#define sysLightType int(gl_LightSource[1].spotCutoff)
#define sysLightRadius gl_LightSource[0].diffuse.a

#define DegToRad(x) ((3.14159265*x)/180.0)

attribute vec4 vPosition;

//varying vec2 UV;
//varying vec3 viewRay;
varying float z;

uniform mat4 sysCameraProjectionMatrix;
//varying mat4 inverse_projection_matrix;
//varying mat4 inverse_modelview_matrix;
mat3 l_rot;


void main()
{
    //gl_Position=vPosition;
    //UV=vec2(vPosition.x+1.0, vPosition.y-1.0)/2.0;
    //UV=vec2(vPosition.x+1.0, vPosition.y+1.0)/2.0;
    //viewRay=vec3(inverse(sysCameraProjectionMatrix)*vec4(vPosition.xyz, 1.0));
    
    l_rot[0] = gl_LightSource[2].spotDirection;
	l_rot[1] = gl_LightSource[1].spotDirection;
	l_rot[2] = gl_LightSource[0].spotDirection;
	
	l_rot = transpose(l_rot);
	
	vec4 p = vPosition;
	//gl_Position = p;
	
	if(int(gl_LightSource[1].spotExponent) == LIGHT_POINT)
	{
		p = vec4((p.xyz * gl_LightSource[0].diffuse.a * 1.08) + gl_LightSource[0].position.xyz, 1.0);
		z = p.z;
		p = gl_ProjectionMatrix * p;
	}
	else if(int(gl_LightSource[1].spotExponent) == LIGHT_SPOT)
	{
		p.xy *= tan(((3.14159265*gl_LightSource[0].spotCutoff)/180.0)) * gl_LightSource[0].diffuse.a * 1.05;
		p.z *= gl_LightSource[0].diffuse.a;
		p = vec4(p.xyz * l_rot + gl_LightSource[0].position.xyz, 1.0);
		z = p.z;
		p = gl_ProjectionMatrix * p;
	}
	//inverse_modelview_matrix = gl_ModelViewMatrixInverse;
	
	gl_Position = p;
	//inverse_projection_matrix = gl_ProjectionMatrixInverse;
	//inverse_projection_matrix = inverse(sysCameraProjectionMatrix);
}
