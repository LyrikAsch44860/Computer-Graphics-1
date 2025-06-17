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

  // Blinn halfway vector
  vec3 H = normalize(pass_CameraDirection) + normalize(pass_Direction);
  float cos_b = dot(normalize(pass_Normal),normalize(H));

  float cos_a = dot(normalize(pass_Normal),normalize(pass_Direction));
  float cos_view = dot(normalize(pass_Normal),normalize(pass_CameraDirection));

  // to prevent negative values of the dot product
  if (cos_a < 0)
  {
    cos_a = 0;
  }
   if (cos_b < 0)
  {
    cos_b = 0;
  }
  
  r = 0.1 * pass_PlanetColor[0]/255 + 0.3 * 3 * pass_PlanetColor[0]/255 * cos_a + pass_PlanetColor[0]/255 * 1.0 * 1 * pow(cos_b,30);
  g = 0.1 * pass_PlanetColor[1]/255 + 0.3 * 3 * pass_PlanetColor[1]/255 * cos_a + pass_PlanetColor[1]/255 * 1.0 * 1 * pow(cos_b,30);
  b = 0.1 * pass_PlanetColor[2]/255 + 0.3 * 3 * pass_PlanetColor[2]/255 * cos_a + pass_PlanetColor[2]/255 * 1.0 * 1 * pow(cos_b,30);

  // if the outline is visable
  if (cos_view < 0.3 && pass_outline == 1.0)
  {
    r = 1;
    g = 1;
    b = 1;
  }
  out_Color = vec4(r, g, b, 1.0);
  //out_Color = vec4(abs(normalize(pass_Normal)), 1.0);
}
