varying vec2 UV;


uniform sampler2D sysTextureSampler0;
uniform sampler2D sysDepthSampler;
uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;


uniform float sysZNear;
uniform float sysZFar;


float bilateralBlendWeight(float centerDepth, float sampleDepth)
{
#if 1
    float bilateralStrenght = 1000.0;
    float weight = 1.0/ (bilateralStrenght*abs(centerDepth-sampleDepth)+1.0);
    return weight;
#else
    // From LotF
    #define BLUR_DEPTH_FALLOFF 100.0
    float depthDiff = abs ( sampleDepth - centerDepth );
    float r2 = BLUR_DEPTH_FALLOFF * depthDiff;
    float w = exp ( -r2*r2 );
    return w;
#endif
}


float linear_depth(float depth_sample)
{
    float zlin;
    depth_sample = 2.0 * depth_sample - 1.0;
    zlin = 2.0*sysZNear*sysZFar/(sysZFar+sysZNear-depth_sample*(sysZFar-sysZNear));
    return zlin;
}


//float gaussian[]={0.00038771, 0.01330373, 0.11098164, 0.22508352};

void main()
{
	float px=(1.0/sysRenderTargetWidth);
	float py=(1.0/sysRenderTargetHeight);
	
	vec2 uvStep = vec2(px, py);
	vec2 uvCenter = UV;
	vec4 rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	vec4 rgbaAccum = rgbaSample;
	
	float centerDepth = texture2D(sysDepthSampler, UV).r;
	float sampleDepth = centerDepth;
	
	float weight;
	float weightAccum = 1.0;

	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	
	
	//uvCenter -= vec2(0.0, py);
	uvCenter.y = max(py*2.0, uvCenter.y - py);
	rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	sampleDepth = texture2D(sysDepthSampler, uvCenter).r;
	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	
	uvCenter.y = max(py*2.0, uvCenter.y - py);
	rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	sampleDepth = texture2D(sysDepthSampler, uvCenter).r;
	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	
	uvCenter = UV;
	
	uvCenter.y = min(1.0 - py*2.0, uvCenter.y + py);
	rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	sampleDepth = texture2D(sysDepthSampler, uvCenter).r;
	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	
	uvCenter.y = min(1.0 - py*2.0, uvCenter.y + py);
	rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	sampleDepth = texture2D(sysDepthSampler, uvCenter).r;
	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	
	
	uvCenter = UV;
	
	uvCenter.x = max(px * 2.0, uvCenter.x - px);
	rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	sampleDepth = texture2D(sysDepthSampler, uvCenter).r;
	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	
	uvCenter.x = max(px * 2.0, uvCenter.x - px);
	rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	sampleDepth = texture2D(sysDepthSampler, uvCenter).r;
	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	
	uvCenter = UV;
	
	uvCenter.x = min(1.0 - px*2.0, uvCenter.x + px);
	rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	sampleDepth = texture2D(sysDepthSampler, uvCenter).r;
	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	
	uvCenter.x = min(1.0 - px*2.0, uvCenter.x + px);
	rgbaSample = texture2D(sysTextureSampler0, uvCenter);
	sampleDepth = texture2D(sysDepthSampler, uvCenter).r;
	weight = bilateralBlendWeight(centerDepth, sampleDepth);
	rgbaAccum += rgbaSample * weight;
	weightAccum += weight;
	

	gl_FragColor = rgbaAccum / weightAccum;

}

