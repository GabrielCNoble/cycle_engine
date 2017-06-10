uniform sampler2D sysTextureSampler0;
uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;

uniform float sysZNear;
uniform float sysZFar;

#define SCALE 0.25

float linear_depth(float depth_sample)
{
    float zlin;
    depth_sample=2.0*depth_sample-1.0;
    zlin=2.0*sysZNear*sysZFar/(sysZFar+sysZNear-depth_sample*(sysZFar-sysZNear));
    return zlin;
}


void main()
{
	float q = sysRenderTargetWidth / sysRenderTargetHeight;	
	float h = 2.0 * SCALE;
	float w = h * q * SCALE;
	
    gl_FragColor = vec4(linear_depth(texture2D(sysTextureSampler0, vec2((gl_FragCoord.x / w) / sysRenderTargetWidth, (gl_FragCoord.y / SCALE) / sysRenderTargetHeight)).x)) * 0.01;
    //gl_FragColor = vec4(1.0);
}

