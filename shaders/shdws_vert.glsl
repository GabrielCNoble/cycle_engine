attribute vec4 vPosition;

varying vec2 uv;
void main()
{
	gl_Position = vPosition;
    uv=vec2(vPosition.x+1.0, vPosition.y+1.0)/2.0;
}
