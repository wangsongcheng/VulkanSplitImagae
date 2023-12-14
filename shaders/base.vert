#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out float outImageIndex;

layout(push_constant) uniform uPushConstant {
	mat4 projection;
} pc;

layout(binding = 0)uniform Uniform{
	mat4 model;
	float imageIndex;
}ubo;

void main() {
	outTexCoord = inTexCoord;
	outImageIndex = ubo.imageIndex;
	gl_Position = pc.projection * ubo.model * vec4(inPos, 1.0);
}
