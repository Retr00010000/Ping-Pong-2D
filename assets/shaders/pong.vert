#version 460 core

layout (location = 0) in vec2 aPos;

uniform vec2 offset;
uniform vec2 scale;

void main()
{
    gl_Position = vec4(aPos.x * scale.x + offset.x, aPos.y * scale.y + offset.y, 0.0, 1.0);
}
