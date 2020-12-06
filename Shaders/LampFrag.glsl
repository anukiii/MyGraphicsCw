# version 330 core

 uniform sampler2D diffuseTex ;
 uniform int useTexture ;

in Vertex {
	vec4 colour ;
} IN ;

out vec4 fragColour ;

void main ( void ) {
	fragColour = IN . colour ;
}