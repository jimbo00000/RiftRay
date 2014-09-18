// hydrabase.frag

varying vec3 vfColor;

void main()
{
    vec2 tc = vfColor.xy;
    vec3 bc = vec3(tc, 1.0-tc.x-tc.y); // something like barycentric...
    float mincomp = 2.0 * min(min(bc.x, bc.y), bc.z);

    float mid = 0.4;
    float halfthick = 0.05;
    float band = smoothstep(mid-halfthick, mid, mincomp) * (1.0-smoothstep(mid, mid+halfthick, mincomp));
    vec3 col = vec3(0.0, 1.0, 0.0);

    gl_FragColor = vec4(band*col, 1.0);
}
