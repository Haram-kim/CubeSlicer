#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;

out vec3 FragNorm;
out vec3 FragPos;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

uniform float useEffect2;
uniform float dt;
uniform vec3 sliceDir;
uniform vec3 sliceNor;

void main()
{	
	vec4 pos = world * vec4(aPos, 1.0f);
	if(useEffect2 > 0.5 && false){
		if(dot(vec3(pos),sliceNor) > -1.0f){
			pos += vec4(sliceDir * dt, 0.0f);
		}
		if(dot(vec3(pos),sliceNor) < 1.0f){
			pos -= vec4(sliceDir * dt, 0.0f);
		}
	}
	FragNorm = vec3(projection * world * vec4(20.0f * aNorm, 1.0));
	FragPos = vec3(world * vec4(aPos, 1.0));
	gl_Position = projection * view * pos;


}
