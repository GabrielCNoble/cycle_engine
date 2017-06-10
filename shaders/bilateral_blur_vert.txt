attribute vec4 vPosition;

varying vec2 UV;

void main()
{
    gl_Position=vPosition;
    //UV=vec2(vPosition.x+1.0, vPosition.y-1.0)/2.0;
    //UV=vec2(vPosition.x*0.5+0.5, (-vPosition.y)*0.5+0.5);
    UV=vec2(vPosition.x+1.0, vPosition.y+1.0)/2.0;

}