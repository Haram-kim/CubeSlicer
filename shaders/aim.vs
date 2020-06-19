#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in float time;
layout (location = 2) in float aSliceOn;
out vec3 outColor;
out float dt;
out float sliceOn;
uniform mat4 view;
uniform mat4 projection;
uniform float curTime;

void main()
{
    dt = (curTime - time);
	vec3 color;
	if( aSliceOn > 0.5 ){
		color = vec3(1.0f, 1.0f, 0.0f);
	}
	else{
		color = vec3(1.0f, 1.0f, 1.0f);
	}
	outColor = color;
	sliceOn = aSliceOn;
	gl_Position = projection * view * vec4(aPos, 1.0f);
}