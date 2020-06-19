#version 330 core
out vec4 FragColor;

in vec3 TexCoords;
uniform float useEffect2;
uniform float dt;
uniform vec3 sliceDir;
uniform vec3 sliceNor;

uniform samplerCube skyboxTexture1;

void main()
{
    vec3 pos = TexCoords;
	if(useEffect2 > 0.5){
		if(dot(pos,sliceNor) >= 0){
			pos += 0.15 * sliceDir * dt;
		}
		if(dot(pos,sliceNor) < 0){
			pos -= 0.15 * sliceDir * dt;
		}
	}
    FragColor = vec4(texture(skyboxTexture1, pos).rgb, 1.0f);
}
