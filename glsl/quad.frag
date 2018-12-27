#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D tex;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 fragColor;

void main() {
    fragColor = vec4(pow(texture(tex,uv).rgb,vec3(1.0f/2.4f)),1.0f);//vec4(vec3(uv,0.0), 1.0);
}