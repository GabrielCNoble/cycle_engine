uniform sampler2D sysTextureSampler0;
uniform float sysBloomIntensity;
varying vec2 UV;


void main()
{
    //gl_FragColor = texture2D(sysTextureSampler0, UV) * vec4(1.0, 1.0, 1.0, 0.0) * sysBloomIntensity;
    //gl_FragColor = texture2D(sysTextureSampler0, UV);
    gl_FragColor = texture2D(sysTextureSampler0, UV) * 0.085;
}
