#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;

out vec2 TexCoord;

void main() {
    // note that we read the multiplication from right to left
    gl_Position = view * model * vec4(aPos, 0.0, 1.0);
    TexCoord    = aTexCoord;
}