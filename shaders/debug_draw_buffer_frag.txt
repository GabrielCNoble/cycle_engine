uniform sampler2D sysTextureSampler0;
uniform float sysRenderTargetWidth;
uniform float sysRenderTargetHeight;





void main()
{
    gl_FragColor = texture2D(sysTextureSampler0, vec2(gl_FragCoord.x / sysRenderTargetWidth, gl_FragCoord.y / sysRenderTargetHeight));
}

