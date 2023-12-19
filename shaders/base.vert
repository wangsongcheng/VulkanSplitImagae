#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out float outCartoons;
layout(location = 2) out float outImageIndex;
layout(location = 3) out vec3  outCartoonsDegre;
struct Cartoons{
	vec3 degree;
	float cartoons;
};
layout(push_constant) uniform uPushConstant {
	mat4 projection;
} pc;

layout(binding = 0)uniform Uniform{
	mat4 model;
	Cartoons cartoon;
	float imageIndex;
}ubo;

void main() {
	outTexCoord = inTexCoord;
	outImageIndex = ubo.imageIndex;
	outCartoons = ubo.cartoon.cartoons;
	outCartoonsDegre = ubo.cartoon.degree;
	gl_Position = pc.projection * ubo.model * vec4(inPos, 1.0);
}
