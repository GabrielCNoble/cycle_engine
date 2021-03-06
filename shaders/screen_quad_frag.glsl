varying vec2 UV;

uniform float time;
uniform sampler2D sysTextureSampler0;
uniform samplerCube sysTextureSamplerCube0;
uniform samplerCube sys3DShadowSampler;
//uniform sampler2D textureSampler1;
uniform samplerCube sysTextureSampler1;
//uniform sampler2D textureSampler1;
uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;

uniform float zNear;
uniform float zFar;

uniform float sysZNear;
uniform float sysZFar;

float linearDepth(float depthSample)
{
    float zlin;
    depthSample=2.0*depthSample-1.0;
    zlin=2.0*sysZNear*sysZFar/(sysZFar+sysZNear-depthSample*(sysZFar-sysZNear));
    return zlin;
}

void main()
{
    vec4 texel;
    //vec4 t_texel;
    //vec3 vec_to_cam;
    //vec2 v;
    ////float l;
    //float t;
    //float q;
    //float b=time;
    //float intensity;
    
    float A = 0.15;
    float B = 0.5;
    float C = 0.1;
    float D = 0.2;
    float E = 0.02;
    float F = 0.3;
    
    
    
    //if(b>1.0)b=0.0;
    //v=vec2(gl_FragCoord.x/sysRenderTargetWidth, gl_FragCoord.y/sysRenderTargetHeight);
    //t=v.y-0.5;
    //q=v.x-0.5;
    //l=length(v-vec2(0.5, 0.5));
    //l*=2.0;
    //l-=0.5;
    /* very trippy effect... */
    /*vec2(cos((2.0+gl_FragCoord.x/renderTargetWidth)*10.0*l)*0.01, sin((2.0+gl_FragCoord.y/renderTargetHeight)*10.0*l)*0.01);*/
    //texel=texture2D(textureSampler0, UV+vec2(cos((2.0+time+gl_FragCoord.x/renderTargetWidth)*10.0*l)*0.01, sin((2.0+gl_FragCoord.y/renderTargetHeight)*10.0*l)*0.05));
    //texel=texture2D(textureSampler0, UV+vec2(cos(time*2.0 + gl_FragCoord.x/renderTargetWidth)*0.02, sin(time*5.0 + gl_FragCoord.y/renderTargetHeight)*0.02));
    
    //vec_to_cam=vec3(-((gl_FragCoord.x/renderTargetWidth)-0.5)*2.0, -((gl_FragCoord.y/renderTargetHeight)-0.5)*2.0, 1.0);
	
	texel = texelFetch(sysTextureSampler0, ivec2(gl_FragCoord.xy), 0);
	//texel=texture2D(sysTextureSampler0, UV);
	//texel=textureCube(textureSamplerCube0, vec3(-1.0, UV.y*2.0-1.0, UV.x*2.0-1.0));
	

	//texel=vec4(linearDepth(texture2D(sysTextureSampler0, vec2(UV.x, UV.y)).r)) * 0.001;
    //texel=vec4(linearDepth((textureCube(sys3DShadowSampler, vec3(UV.x*4.0-2.0, 1.0, UV.y*4.0-2.0)).r)))* 0.01;
	//texel=textureCube(textureSampler1, vec3(1.0, UV.y*4.0-2.0, UV.x*4.0-2.0));
	//texel = vec4(1.0);
	
	/* uncharted 2 tone mapper */
    gl_FragData[0]=((texel * (A * texel + C * B) + D * E) / (texel * (A * texel + B) + D * F)) - E / F;
    //gl_FragData[0] = texel;


}



















