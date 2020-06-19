#version 330 core
out vec4 FragColor;

in vec3 FragNorm;;

uniform sampler2D texture1;
uniform float useFrag1;
uniform float mass;


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec4 color = vec4(FragNorm, 0.5f);
    // vec4 color = vec4(vec3(texture(texture1, vec2(mass, mass))), 0.4f);
    FragColor = vec4(color);   
	if(useFrag1 > 0.5f)
	{
        FragColor = vec4(rand(vec2(mass, mass*15674)), rand(vec2(mass*5, mass*1342)), rand(vec2(mass*154, mass*7561)), 0.4);
    }
    
}