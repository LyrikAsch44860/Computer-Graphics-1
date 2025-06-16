#version 150

in  vec3 pass_Normal;
in  vec3 pass_Direction;
out vec4 out_Color;

void main() {
  float r;
  float g;
  float b;

  r = 1.0 * 5 * 0.5 * dot(normalize(pass_Normal),normalize(pass_Direction));
  g = 0.7 * 5 * 0.5 * dot(normalize(pass_Normal),normalize(pass_Direction));
  b = 0.0 * 5 * 0.5 * dot(normalize(pass_Normal),normalize(pass_Direction));
  out_Color = vec4(r, g, b, 1.0);
  //out_Color = vec4(abs(normalize(pass_Normal)), 1.0);
}