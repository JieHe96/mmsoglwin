#version 440 core

attribute vec4 texCoord;
attribute vec4 verCoord;

uniform float xoffset;
uniform float yoffset;

out vec2 vTexCoord;

void main(void)
{
	vec4 a = verCoord;
	a.x = a.x*(1.0) + xoffset;
	a.y = -a.y + yoffset;
	vTexCoord = texCoord.xy;
	gl_Position = a;
}