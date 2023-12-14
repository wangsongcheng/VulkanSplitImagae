#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTextCoord;

layout(location = 0) out vec3 outColor;

layout(push_constant) uniform uPushConstant {
	mat4 projection;
	vec3 color;
} pc;

layout(binding = 0)uniform Uniform{
	mat4 model;
}ubo;

void main() {
	outColor = pc.color;
	gl_Position = pc.projection * ubo.model * vec4(inPos, 1.0);
}
