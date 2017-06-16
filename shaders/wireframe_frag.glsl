void main()
{
    //gl_FragColor=vec4(0.6, 0.5, 0.5, 1.0);
    gl_FragData[0]=vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
    gl_FragData[1]=vec4(0.0);
    //gl_FragData[0]=vec4(gl_FrontMaterial.diffuse.rgb, 1.0);
}
