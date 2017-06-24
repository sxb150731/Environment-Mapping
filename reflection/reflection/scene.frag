#version 330 core

in vec3 FragNormal;
in vec3 FragPos;

uniform samplerCube envText;	
uniform vec3 cameraPos;

out vec4 color;


void main()
{
	vec3 viewDir = normalize(FragPos - cameraPos); 
	vec3 reflectDir = reflect(viewDir, normalize(FragNormal));
	color = texture(envText, reflectDir);	
}