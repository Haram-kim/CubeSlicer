#version 330 core
out vec4 FragColor;
in vec3 outColor;
in float dt;
void main()
{
    FragColor = exp(- 5*dt) * vec4(outColor, 1.0);
}