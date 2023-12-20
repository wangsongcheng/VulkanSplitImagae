#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inTexCoords;
layout(location = 1) in float inCartoons;
layout(location = 2) in float inImageIndex;
layout(location = 3) in vec3  inCartoonsDegre;
layout(location = 4) in float inUseImageArray;

layout(location = 0) out vec4 outColor;

layout(binding = 1)uniform sampler2D textureMap;
layout(binding = 2)uniform sampler2DArray diffuseMap;

vec3 Cartoons(float cartoons, vec3 degre, sampler2D tex){
	float uMagTol = .2f;
	ivec2 ires = textureSize(tex, 0);
	float uResS = float(ires.s);
	float uResT = float(ires.t);
	vec3 rgb = texture(tex, inTexCoords).rgb;
	vec2 stp0 = vec2(1.0 / uResS, 0);
	vec2 st0p = vec2(0, 1 / uResT);
	vec2 stpp = vec2(1 / uResS, 1 / uResT);
	vec2 stpm = vec2(1 / uResS, -1 / uResT);
	const vec3 W = vec3(.2125, .7154, .0721);
	float im1m1 = dot(texture(tex, inTexCoords - stpp).rgb, W);
	float ip1p1 = dot(texture(tex, inTexCoords + stpp).rgb, W);
	float im1p1 = dot(texture(tex, inTexCoords - stpm).rgb, W);
	float ip1m1 = dot(texture(tex, inTexCoords + stpm).rgb, W);
	float im10 = dot(texture(tex, inTexCoords - stp0).rgb, W);
	float ip10 = dot(texture(tex, inTexCoords + stp0).rgb, W);
	float i0m1 = dot(texture(tex, inTexCoords - st0p).rgb, W);
	float i0p1 = dot(texture(tex, inTexCoords + st0p).rgb, W);
	float h = -1 * im1p1 - 2 * i0p1 - 1 * ip1p1 + 1 * im1m1 + 2 * i0m1 + 1 * ip1m1;
	float v = -1 * im1m1 - 2 * im10 - 1 * im1p1 + 1 * ip1m1 + 2 * ip10 + 1 * ip1p1;
	float mag = length(vec2(h, v));
	if(mag > uMagTol)
		return vec3(0, 0, 0);//该值为边缘点, 应为黑色
	else{
		rgb.rgb *= cartoons;
		rgb.rgb += degre;//卡通化程度
		// rgb.rgb += vec3(.5, .5, .5);//卡通化程度
		ivec3 intrgb = ivec3(rgb.rgb);
		rgb.rgb = vec3(intrgb) / cartoons;
		return rgb;
	}
}
vec3 Cartoons(float cartoons, vec3 degre, sampler2DArray tex){
	float uMagTol = .2f;
	ivec3 ires = textureSize(tex, 0);
	float uResS = float(ires.s);
	float uResT = float(ires.t);
	vec3 rgb = texture(tex, vec3(inTexCoords, inImageIndex)).rgb;
	vec2 stp0 = vec2(1.0 / uResS, 0);
	vec2 st0p = vec2(0, 1 / uResT);
	vec2 stpp = vec2(1 / uResS, 1 / uResT);
	vec2 stpm = vec2(1 / uResS, -1 / uResT);
	const vec3 W = vec3(.2125, .7154, .0721);
	float im1m1 = dot(texture(tex, vec3(inTexCoords - stpp, inImageIndex)).rgb, W);
	float ip1p1 = dot(texture(tex, vec3(inTexCoords + stpp, inImageIndex)).rgb, W);
	float im1p1 = dot(texture(tex, vec3(inTexCoords - stpm, inImageIndex)).rgb, W);
	float ip1m1 = dot(texture(tex, vec3(inTexCoords + stpm, inImageIndex)).rgb, W);
	float im10 = dot(texture(tex, vec3(inTexCoords - stp0, inImageIndex)).rgb, W);
	float ip10 = dot(texture(tex, vec3(inTexCoords + stp0, inImageIndex)).rgb, W);
	float i0m1 = dot(texture(tex, vec3(inTexCoords - st0p, inImageIndex)).rgb, W);
	float i0p1 = dot(texture(tex, vec3(inTexCoords + st0p, inImageIndex)).rgb, W);
	float h = -1 * im1p1 - 2 * i0p1 - 1 * ip1p1 + 1 * im1m1 + 2 * i0m1 + 1 * ip1m1;
	float v = -1 * im1m1 - 2 * im10 - 1 * im1p1 + 1 * ip1m1 + 2 * ip10 + 1 * ip1p1;
	float mag = length(vec2(h, v));
	if(mag > uMagTol)
		return vec3(0, 0, 0);//该值为边缘点, 应为黑色
	else{
		rgb.rgb *= cartoons;
		rgb.rgb += degre;//卡通化程度
		// rgb.rgb += vec3(.5, .5, .5);//卡通化程度
		ivec3 intrgb = ivec3(rgb.rgb);
		rgb.rgb = vec3(intrgb) / cartoons;
		return rgb;
	}
}
void main() {
	if(inCartoons != 0){
		if(inUseImageArray != 0)
			outColor = vec4(Cartoons(inCartoons, inCartoonsDegre, diffuseMap), 1.0);
		else
			outColor = vec4(Cartoons(inCartoons, inCartoonsDegre, textureMap), 1.0);
	}
	else{
		if(inUseImageArray != 0)
			outColor = texture(diffuseMap, vec3(inTexCoords, inImageIndex));
		else
			outColor = texture(textureMap, inTexCoords);
	}
}
