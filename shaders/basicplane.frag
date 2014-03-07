// basicplane.frag

varying vec2 vfTexCoord;


void main()
{
    float freq = 32.0;
    float xval = floor( sin( freq*vfTexCoord.x ) + 1.0 );
    float yval = floor( sin( freq*vfTexCoord.y ) + 1.0 );
    yval = 1.0 - yval;
    if ((xval==0.0) && (yval==0.0))
    {
        xval = 1.0;
        yval = 1.0;
    }
    gl_FragColor = vec4(vec3(xval*yval), 1.0);
}
