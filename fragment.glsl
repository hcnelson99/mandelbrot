#version 140

out vec4 frag_color; 

void main() { 
    vec2 win_size = vec2(800, 600);
    vec2 uv = gl_FragCoord.xy / win_size;

    uv = uv * vec2(3.5, 2) + vec2(-2.5, -1);
    
    vec2 xy = uv;
    
    int i;
    for (i = 0; i < 255; i++) {
        if (xy.x * xy.x + xy.y * xy.y > 4.) {
            break;
        }
        
        vec2 offset = vec2(xy.x * xy.x - xy.y * xy.y, 2. * xy.x * xy.y);
        xy = uv + offset;
        
    }

    vec3 color = vec3(i) / vec3(255);

    frag_color = vec4(color,1.0);
}
