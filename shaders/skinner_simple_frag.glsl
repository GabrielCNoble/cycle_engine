#version 150 

in vec3 test;
void main()
{
	gl_FragColor = vec4(test, 1.0);
}
