#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inTexCoords;
layout(location = 1) in float inImageIndex;

layout(location = 0) out vec4 outColor;

layout(binding = 1)uniform sampler2D diffuseMap;

void main() {
	outColor = texture(diffuseMap, inTexCoords);
}
