varying int face_index;



void main()
{
    gl_FragData[0] = gl_FrontMaterial.diffuse.rgba;

    gl_FragData[0].b -= face_index;
}