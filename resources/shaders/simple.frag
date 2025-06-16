#version 150

in vec3 pass_Normal;
in vec3 pass_Direction;
in vec3 pass_PlanetColor;
in vec3 pass_LightColor;
in vec3 pass_CameraDirection;
in float pass_outline;
out vec4 out_Color;

void main() {
  float r;
  float g;
  float b;
  vec3 HV = normalize(pass_CameraDirection) + normalize(pass_Direction);
  float cos = dot(normalize(pass_Normal),normalize(HV));
  float cos_a = dot(normalize(pass_Normal),normalize(pass_Direction));
  float cos_view = dot(normalize(pass_Normal),normalize(pass_CameraDirection));

  
  r = 0.5 * 3 * pass_PlanetColor[0]/255 * cos_a + pass_PlanetColor[0]/255 * 1.5 * 1 * pow(cos,10);
  g = 0.5 * 3 * pass_PlanetColor[1]/255 * cos_a + pass_PlanetColor[1]/255 * 1.5 * 1 * pow(cos,10);
  b = 0.5 * 3 * pass_PlanetColor[2]/255 * cos_a + pass_PlanetColor[2]/255 * 1.5 * 1 * pow(cos,10);
  if (cos_view < 0.3 && pass_outline == 1.0)
  {
    r = 1;
    g = 1;
    b = 1;
  }
  out_Color = vec4(r, g, b, 1.0);
  //out_Color = vec4(abs(normalize(pass_Normal)), 1.0);
}
