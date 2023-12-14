#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0)in float inDrawID;
layout(location = 0)out vec4 outColor;
void main(){
	outColor = vec4(inDrawID, inDrawID, inDrawID, 1.0);
//	outColor = vec4(inDrawID, inDrawID, gl_PrimitiveID + 1.0f, 1.0);
}
