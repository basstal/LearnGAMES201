#version 330 core
out vec4 FragColor;

uniform sampler2D cubeTexture;

in vec2 TexCoord;
void main()
{
    FragColor = texture(cubeTexture, TexCoord);
}