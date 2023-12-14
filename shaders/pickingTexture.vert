#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 Position;

layout (location = 0) out float outDrawID;

layout(binding = 0)uniform Uniform{
	mat4 view;
}ubo;

layout(push_constant)uniform PushConstant{
	mat4 model;
	mat4 projection;
	float id;
}pc;

void main(){
	outDrawID = pc.id;
	gl_Position = pc.projection * ubo.view * pc.model * vec4(Position, 1.0);
} 
