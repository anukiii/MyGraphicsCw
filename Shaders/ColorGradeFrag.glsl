 # version 330 core

uniform sampler2D diffuseTex ;


in Vertex {
	vec2 texCoord ;
} IN ;

out vec4 fragColor ;


void main ( void ) {
	fragColor = texture ( diffuseTex , IN . texCoord );
	float greyScale = fragColor.r * 0.3 + fragColor.g * 0.59 + fragColor.b * 0.11;
	fragColor.rgb = vec3(greyScale, greyScale, greyScale);


}