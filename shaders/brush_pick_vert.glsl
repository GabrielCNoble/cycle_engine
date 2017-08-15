attribute vec4 vPosition;


varying int face_index;


void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * vPosition;
    face_index = gl_VertexID / 3;

}