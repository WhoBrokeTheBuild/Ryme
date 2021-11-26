#ifndef RYME_TRANSFORM_INC_GLSL
#define RYME_TRANSFORM_INC_GLSL

layout(binding = 1, std140) uniform RymeTransform
{
    mat4 u_Model;
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_MVP;

};

#endif // RYME_TRANSFORM_INC_GLSL