#version 450 core

#include <Ryme/Material.inc.glsl>

layout(location = 0) in vec4 v_Normal;

layout (location = 0) out vec4 o_Color;

void main() {
    o_Color = vec4(v_Normal.xyz, 1);
}
