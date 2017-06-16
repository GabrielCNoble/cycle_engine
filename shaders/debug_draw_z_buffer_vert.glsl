attribute vec4 vPosition;

uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;

#define SCALE 0.25


void main()
{
	float q = sysRenderTargetWidth / sysRenderTargetHeight;	
	float h = 2.0 * SCALE;
	float w = h * q * SCALE;
    gl_Position = vec4(vPosition.x * w, vPosition.y * SCALE - 1.0 + SCALE, vPosition.z, 1.0);
}
