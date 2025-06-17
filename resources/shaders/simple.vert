#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec3 PlanetColor;
uniform vec3 LightColor;
uniform float outline;

out vec3 pass_Normal;
out vec3 pass_Direction;
out vec3 pass_PlanetColor;
out vec3 pass_LightColor;
out vec3 pass_CameraDirection;
out float pass_outline;

void main(void)
{
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
	pass_Normal = (NormalMatrix * vec4(in_Normal, 1.0)).xyz;

	// vector from point to lightsource, which is at (0,0,0)
	pass_Direction = -1 * (ModelMatrix * vec4(in_Position, 1.0)).xyz;

	pass_PlanetColor = PlanetColor;
	pass_LightColor = LightColor;

	// vector from point to camera
	pass_CameraDirection = (inverse(ViewMatrix) * (vec4(0.0,0.0,0.0,1.0)) - ModelMatrix * vec4(in_Position, 1.0)).xyz;
	pass_outline = outline;
}