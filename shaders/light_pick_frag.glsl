varying vec4 color;

void main()
{
    //gl_FragColor = color;
    gl_FragColor = gl_FrontMaterial.diffuse;
}
