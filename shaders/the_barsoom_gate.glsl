// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// @var title The Barsoom Gate
// @var author jimmikaelkael
// @var url https://www.shadertoy.com/view/MlB3zh
// @var tex0 tex16.png

// @var headSize 0.5
// @var eyePos 0.78 1.15 -0.8

vec3 skyColor = vec3(0.65, 0.28, 0.1);
vec3 sunColor = vec3(1.8, 1.0, 0.6);
vec3 sunLightColor = vec3(1.7, 1.25, 0.9);
vec3 skyLightColor = vec3(0.8, 0.35, 0.15);
vec3 indLightColor = vec3(0.4, 0.3, 0.2);
vec3 horizonColor = vec3(0.85, 0.35, 0.15);
vec3 sunDirection = normalize(vec3(0.6, 0.4, 0.9));

float noise(in vec2 p) {
    p += vec2(16.6, 17.0);
    return texture2D(iChannel0, p / 256.0, -100.0).x;
}

float noise3d(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
    vec2 rg = texture2D(iChannel0, (uv + 0.5) / 256.0, -100.0).yx;
    return mix(rg.x, rg.y, f.z);
}

float fBm(in vec2 p) {
    float sum = 0.0;
    float amp = 1.0;
    for(int i = 0; i < 4; i++) {
        sum += amp * noise(p);
        amp *= 0.5;
        p *= 2.5;
    }
    return sum * 0.5 + 0.15;
}

float fBm3d(in vec3 p) {
    float sum = 0.0;
    float amp = 1.0;
    for(int i = 0; i < 4; i++) {
        sum += amp * noise3d(p);
        amp *= 0.5;
        p *= 2.0;
    }
    return sum;
}

float warp(in vec3 p) {
    vec3 q, r;
    q.x = fBm3d(p);
    q.y = fBm3d(p + vec3(5.2,1.3,8.4));
    q.z = fBm3d(p + vec3(2.2,5.4,7.9));
    r.x = fBm3d(p + 2.0 * q + vec3(1.7,9.2,5.2));
    r.y = fBm3d(p + 2.0 * q + vec3(8.3,2.8,4.8));
    r.z = fBm3d(p + 2.0 * q + vec3(5.7,4.3,2.4));
    return fBm3d(p + 4.0 * r);
}

float length2(in vec2 p) {
    return sqrt(p.x*p.x + p.y*p.y);
}

float length8(in vec2 p) {
    p = p*p; p = p*p; p = p*p;
    return pow(p.x + p.y, 1.0 / 8.0);
}

float sdTorus82(in vec3 p, in vec2 t) {
    vec2 q = vec2(length2(p.xz) - t.x, p.y);
    return length8(q) - t.y;
}

float sdBox(in vec3 p, in vec3 box) {
    vec3 d = abs(p) - box;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float sdSphere(in vec3 p, float s) {
    return length(p) - s;
}

float sdTriPrism(in vec3 p, vec2 h) {
    vec3 q = abs(p);
    return max(q.z - h.y, max(q.x * 0.866025 + p.y * 0.5, -p.y) - h.x * 0.5);
}

float smin(float a, float b, float k) {
    float res = exp(-k * a) + exp(-k * b);
    return -log(res) / k;
}

vec3 rotateX(in vec3 p, float an) {
    float c = cos(an);
    float s = sin(an);
    return vec3(p.x, c * p.y - s * p.z, s * p.y + c * p.z);
}

vec3 rotateY(in vec3 p, float an) {
    float c = cos(an);
    float s = sin(an);
    return vec3(c * p.x + s * p.z, p.y, -s * p.x + c * p.z);
}

vec3 rotateZ(in vec3 p, float an) {
    float c = cos(an);
    float s = sin(an);
    return vec3(c * p.x - s * p.y, s * p.x + c * p.y, p.z);
}

vec2 map(in vec3 p, bool frame) {
    vec2 res = vec2(0.0, p.y - fBm(p.xz));
    if (frame) {
        float d1 = sdBox(rotateX(rotateY(p - vec3(0.0, 1.0, 0.0), -1.25), 1.58), vec3(0.5, 0.0, 0.5));
        vec2 obj = vec2(1.0, max(d1, sdSphere(p - vec3(0.0, 1.0, 0.0), 0.5)));
        if (obj.y < res.y) res = obj;
    }
    float d2 = sdTriPrism(rotateX(rotateY(p - vec3(0.0, 1.51, 0.0), -1.25), 3.14159), vec2(0.025, 0.055));
    float d3 = sdTriPrism(rotateZ(rotateX(rotateY(p - vec3(-0.112, 0.635, 0.35), -1.25), 3.14159), 2.35), vec2(0.025, 0.055));
    float d4 = sdTriPrism(rotateZ(rotateX(rotateY(p - vec3(0.112, 0.635, -0.35), -1.25), 3.14159), -2.35), vec2(0.025, 0.055));
    vec2 obj = vec2(2.0, smin(smin(smin(sdTorus82(rotateX(rotateY(p - vec3(0.0, 1.0, 0.0), -1.25), 1.58), vec2(0.5, 0.05)), d2, 16.0), d3, 16.0), d4, 16.0));
    if (obj.y < res.y) res = obj;
    obj = vec2(3.0, sdTriPrism(rotateX(rotateY(p - vec3(0.0, 1.51, 0.0), -1.25), 3.14159), vec2(0.025, 0.1)));
    if (obj.y < res.y) res = obj;
    obj = vec2(3.1, sdTriPrism(rotateZ(rotateX(rotateY(p - vec3(-0.122, 1.36, 0.35), -1.25), 3.14159), 0.84), vec2(0.025, 0.055)));
    if (obj.y < res.y) res = obj;
    obj = vec2(3.2, sdTriPrism(rotateZ(rotateX(rotateY(p - vec3(0.117, 1.36, -0.35), -1.25), 3.14159), -0.84), vec2(0.025, 0.055)));
    if (obj.y < res.y) res = obj;
    obj = vec2(3.3, sdTriPrism(rotateZ(rotateX(rotateY(p - vec3(-0.162, 1.0, 0.49), -1.25), 3.14159), 1.6), vec2(0.025, 0.055)));
    if (obj.y < res.y) res = obj;
    obj = vec2(3.4, sdTriPrism(rotateZ(rotateX(rotateY(p - vec3(0.162, 1.0, -0.49), -1.25), 3.14159), -1.6), vec2(0.025, 0.055)));
    if (obj.y < res.y) res = obj;
    obj = vec2(3.5, sdTriPrism(rotateZ(rotateX(rotateY(p - vec3(-0.112, 0.635, 0.35), -1.25), 3.14159), 2.35), vec2(0.025, 0.1)));
    if (obj.y < res.y) res = obj;
    obj = vec2(3.6, sdTriPrism(rotateZ(rotateX(rotateY(p - vec3(0.115, 0.635, -0.35), -1.25), 3.14159), -2.35), vec2(0.025, 0.1)));
    if (obj.y < res.y) res = obj;

    return res;
}

vec2 map(in vec3 p) {
    return map(p, true);
}

vec3 raymarchTerrain(in vec3 ro, in vec3 rd, in float tmin, in float tmax) {
    float t = tmin;
    vec3 res = vec3(-1.0);
    for (int i = 0; i < 110; i++) {
        vec3 p = ro + rd * t;
        res = vec3(map(p), t);
        float d = res.y;
        if (d < (0.001 * t) || t > tmax)
            break;
        t += 0.5 * d;
    }
    return res;
}

vec3 getTerrainNormal(in vec3 p) {
    float eps = 0.025;
    return normalize(vec3(fBm(vec2(p.x - eps, p.z)) - fBm(vec2(p.x + eps, p.z)),
                          2.0 * eps,
                          fBm(vec2(p.x, p.z - eps)) - fBm(vec2(p.x, p.z + eps))));
}

vec3 getGateNormal(in vec3 p, float e) {
    vec2 eps = vec2(e, 0.0);
    return normalize(vec3(map(p + eps.xyy).y - map(p - eps.xyy).y,
                          2.0 * eps.x,
                          map(p + eps.yyx).y - map(p - eps.yyx).y));
}

vec3 getGateBump(in vec3 p, float e) {
    vec2 eps = vec2(e, 0.0);
    return normalize(vec3(fBm3d(p + eps.xyy) - fBm3d(p - eps.xyy),
                          fBm3d(p + eps.yxy) - fBm3d(p - eps.yxy),
                          fBm3d(p + eps.yyx) - fBm3d(p - eps.yyx)));
}

float raymarchAO(in vec3 ro, in vec3 rd, float tmin) {
    float ao = 0.0;
    for (float i = 0.0; i < 5.0; i++) {
        float t = tmin + pow(i / 5.0, 2.0);
        vec3 p = ro + rd * t;
        float d = p.y - fBm(p.xz);
        ao += max(0.0, t - 0.5 * d - 0.05);
    }
    return 1.0 - 0.4 * ao;
}

float raymarchShadow(in vec3 ro, in vec3 rd, float tmin, float tmax) {
    float sh = 1.0;
    float t = tmin;
    for (int i = 0; i < 40; i++) {
        vec3 p = ro + rd * t;
        float d = p.y - fBm(p.xz);
        sh = min(sh, 16.0 * d / t);
        t += 0.5 * d;
        if (d < (0.001 * t) || t > tmax)
            break;
    }
    return sh;
}

float raymarchGateShadow(in vec3 ro, in vec3 rd, float tmin, float tmax) {
    float sh = 1.0;
    float t = tmin;
    for (int i = 0; i < 80; i++) {
        vec3 p = ro + rd * t;
        float d = map(p, false).y;
        sh = min(sh, 16.0 * d / t);
        t += 0.5 * d;
        if (d < (0.001 * t) || t > tmax)
            break;
    }
    return sh;
}

vec3 getSceneColor( in vec3 ro, in vec3 rd )
{
 
    // background
    vec3 color = skyColor;

    // terrain marching
    float tmin = 0.1;
    float tmax = 20.0;
    vec3 res = raymarchTerrain(ro, rd, tmin, tmax);
    float t = res.z;
    if (t < tmax) {
        vec3 pos = ro + rd * t;
        vec3 nor;
        // add bumps
        if (res.x == 0.0) {
            nor = getTerrainNormal(pos);
            nor = normalize(nor + 0.5 * getTerrainNormal(pos * 8.0));
        } else if (res.x == 1.0){
            nor = getGateNormal(pos, 0.1);
            nor = normalize(nor + 0.05 * getGateBump(pos * 4.0 + vec3(iGlobalTime * 0.25, 1.0, 1.0), 0.005));
        } else if (res.x == 2.0){
            nor = getGateNormal(pos, 0.005);
            nor = normalize(nor + 0.25 * getGateBump(pos * 32.0, 0.005));
        } else {
            nor = getGateNormal(pos, 0.005);
            nor = normalize(nor + 0.1 * getGateBump(pos * 16.0, 0.005));
        }

        // 3 lights rig
        float sun = clamp(dot(sunDirection, nor), 0.0, 1.0);
        float sky = clamp(0.5 + 0.5 * nor.y, 0.0, 1.0);
        float ind = clamp(dot(vec3(-sunDirection.x, 0.0, -sunDirection.z), nor), 0.0, 1.0);
        float occ = clamp(raymarchAO(pos, nor, 0.1), 0.0, 1.0);
        float sha1 = clamp(raymarchShadow(pos, sunDirection, 0.25, tmax), 0.0, 1.0);
        float sha2 = clamp(raymarchGateShadow(pos, sunDirection, 0.05, tmax), 0.0, 1.0);

        vec3 lightColor = 1.0 * sun * sunLightColor;
        lightColor *= pow(vec3(sha1), vec3(1.0, 1.2, 1.5));
        lightColor *= pow(vec3(sha2), vec3(1.0, 1.2, 1.5));
        lightColor += 0.7 * sky * skyLightColor * occ;
        lightColor += 0.3 * ind * indLightColor * occ;

        if (res.x == 1.0) {
            vec3 h = normalize(-rd + sunDirection);
            float spe = pow(clamp(dot(h, nor), 0.0, 1.0), 2.0);
            float fresnel = 1.0;
            float fre = fresnel + (1.0 - fresnel) * pow(1.0 - dot(nor, -rd), 5.0);
            lightColor += 32.0 * spe * sun * occ;
            color = vec3(0.05, 0.2, 0.8) * 2.0 * warp(pos * 2.0 + vec3(iGlobalTime * 0.15, 1.0, 1.0));
            lightColor *= pow(indLightColor, vec3(1.0)) * fre;
        } else if (res.x == 2.0) {
            vec3 ref = reflect(rd, -sunDirection);
            float spe = pow(clamp(dot(ref, nor), 0.0, 1.0), 2.0);
            float fresnel = 1.0;
            float fre = fresnel + (1.0 - fresnel) * pow(1.0 - dot(nor, -rd), 5.0);
            lightColor += 32.0 * spe * sun * occ;
            color = vec3(0.1, 0.11, 0.15) * 0.05;
            lightColor *= pow(indLightColor, vec3(1.0)) * fre;
        } else if (res.x > 2.0) {
            vec3 ref = reflect(rd, -sunDirection);
            float spe = pow(clamp(dot(ref, nor), 0.0, 1.0), 2.0);
            float fresnel = 1.0;
            float fre = fresnel + (1.0 - fresnel) * pow(1.0 - dot(nor, -rd), 5.0);
            lightColor += 16.0 * spe * sun * occ;
            color = vec3(0.2, 0.1, 0.01);
            lightColor *= pow(indLightColor, vec3(1.0)) * fre;
        }

        color *= 0.8 * lightColor;

        // there no air on mars but a bit of fog is better
        color = mix(color, horizonColor, 1.0 - exp(-0.01 * t * t));
    } else {
        float sunDot = clamp(dot(sunDirection, rd), 0.0, 1.0);
        float sky = clamp(0.8 * (1.0 - 0.8 * rd.y), 0.0, 1.0);
        float diffuse = clamp(0.4 * sunDot, 0.0, 1.0);
        color = sky * skyColor + pow(sunDot, 2000.0) * sunColor;

        // horizon
        color = mix(color, horizonColor, pow(1.0 - rd.y, 4.0));
    }

    // gamma correction
    vec3 gamma = vec3(1.0 / 2.2);
    return pow(color, gamma);
}

#ifndef RIFTRAY
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 p = (-iResolution.xy + 2.0 * fragCoord.xy) / iResolution.y;

    vec3 eye = vec3(0.0, 1.25, 1.5);
    vec2 rot = 6.2831 * (vec2(-0.05 + iGlobalTime * 0.025, 0.0 - sin(iGlobalTime * 0.5) * 0.01) + vec2(1.0, 0.0) * (iMouse.xy - iResolution.xy * 0.25) / iResolution.x);
    eye.yz = cos(rot.y) * eye.yz + sin(rot.y) * eye.zy * vec2(-1.0, 1.0);
    eye.xz = cos(rot.x) * eye.xz + sin(rot.x) * eye.zx * vec2(1.0, -1.0);

    vec3 ro = eye;
    vec3 ta = vec3(0.5, 1.0, 0.0);

    vec3 cw = normalize(ta - ro);
    vec3 cu = normalize(cross(vec3(0.0, 1.0, 0.0), cw));
    vec3 cv = normalize(cross(cw, cu));
    mat3 cam = mat3(cu, cv, cw);

    vec3 rd = cam * normalize(vec3(p.xy, 1.0));

    fragColor = vec4(getSceneColor(ro,rd),1.);
}
#endif
