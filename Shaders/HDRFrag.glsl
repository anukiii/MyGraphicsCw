 # version 330 core

uniform sampler2D diffuseTex ;


in Vertex {
	vec2 texCoord ;
} IN ;

out vec4 fragColor ;


void main ( void ) {
	fragColor = texture ( diffuseTex , IN . texCoord );

	if (fragColor.r>0.7){
	fragColor.r =1;
	}
	if (fragColor.b>0.8){
	fragColor.b =1;
	}
	if (fragColor.g>0.8){
	fragColor.g =1;
	}

}