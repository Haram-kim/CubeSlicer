#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec4 outColor;

uniform bool effect2;
uniform float dt;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	if(effect2){
		outColor = vec4(5.0, 5.0, 5.0, 0.5*exp(-5* dt));
	}
	else{
		outColor = vec4(aColor, exp(-10 * dt));
	}
	gl_Position = projection * view * vec4(aPos, 1.0f);
}