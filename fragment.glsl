#version 410

uniform vec2 window_size;
uniform dvec4 view;

out vec4 frag_color; 

vec3 spectral_color(float l) {
    l = l * 300 + 400;
    vec3 v;
    if ((l >= 400.0) && (l < 410.0)) {
        float t = (l - 400.0) / (410.0 - 400.0);
        v.r = +(0.33 * t) - (0.20 * t * t);
    } else if ((l >= 410.0) && (l < 475.0)) {
        float t = (l - 410.0) / (475.0 - 410.0);
        v.r = 0.14 - (0.13 * t * t);
    } else if ((l >= 545.0) && (l < 595.0)) {
        float t = (l - 545.0) / (595.0 - 545.0);
        v.r = +(1.98 * t) - (t * t);
    } else if ((l >= 595.0) && (l < 650.0)) {
        float t = (l - 595.0) / (650.0 - 595.0);
        v.r = 0.98 + (0.06 * t) - (0.40 * t * t);
    } else if ((l >= 650.0) && (l < 700.0)) {
        float t = (l - 650.0) / (700.0 - 650.0);
        v.r = 0.65 - (0.84 * t) + (0.20 * t * t);
    }
    if ((l >= 415.0) && (l < 475.0)) {
        float t = (l - 415.0) / (475.0 - 415.0);
        v.g = +(0.80 * t * t);
    } else if ((l >= 475.0) && (l < 590.0)) {
        float t = (l - 475.0) / (590.0 - 475.0);
        v.g = 0.8 + (0.76 * t) - (0.80 * t * t);
    } else if ((l >= 585.0) && (l < 639.0)) {
        float t = (l - 585.0) / (639.0 - 585.0);
        v.g = 0.84 - (0.84 * t);
    }
    if ((l >= 400.0) && (l < 475.0)) {
        float t = (l - 400.0) / (475.0 - 400.0);
        v.b = +(2.20 * t) - (1.50 * t * t);
    } else if ((l >= 475.0) && (l < 560.0)) {
        float t = (l - 475.0) / (560.0 - 475.0);
        v.b = 0.7 - (t) + (0.30 * t * t);
    }
    return v;
}



int max_iter = 255;

float escape_radius = 4;

void main() { 
    dvec2 uv = gl_FragCoord.xy / window_size;

    uv = uv * (view.zw - view.xy) + view.xy;
    
    dvec2 xy = uv;
    
    int i;
    for (i = 0; i < max_iter; i++) {
        if (xy.x * xy.x + xy.y * xy.y > escape_radius) {
            break;
        }
        
        xy = uv + dvec2(xy.x * xy.x - xy.y * xy.y, 2. * xy.x * xy.y);;
    }
    xy = uv + dvec2(xy.x * xy.x - xy.y * xy.y, 2. * xy.x * xy.y);;
    xy = uv + dvec2(xy.x * xy.x - xy.y * xy.y, 2. * xy.x * xy.y);;

    float mu = clamp(float(i) - log2(log2(float(length(xy)))), 0, max_iter) / float(max_iter + 2);


    vec3 color = vec3(spectral_color(mu));

    frag_color = vec4(color,1.0);
}
