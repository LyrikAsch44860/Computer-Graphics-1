#version 150

in vec2 pass_TextureCoord; 

uniform sampler2D Texture;

out vec4 out_Color;

void main() {
 
    out_Color = texture(Texture, pass_TextureCoord);
  
}