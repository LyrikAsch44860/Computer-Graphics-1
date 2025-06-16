#version 150
#extension GL_ARB_explicit_attrib_location : require
// glVertexAttribPointer mapped positions to first
layout(location = 0) in float angle;

//Matrix Uniforms uploaded with glUniform*
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ParentMatrix;
uniform mat4 ProjectionMatrix;

out vec3 pass_Color;

void main() {
	float pi = 3.1415926535897932384626433832795;
	float angle1 = angle * (pi/180);
    mat4 rotation = mat4(cos(angle1), 0, -1 * sin(angle1), 0, 0, 1, 0, 0, sin(angle1), 0, cos(angle1), 0, 0, 0, 0, 1);
	gl_Position =  ProjectionMatrix * ViewMatrix * ParentMatrix * rotation * ModelMatrix * vec4(0.0, 0.0, 0.0, 1.0);
	pass_Color = vec3(1, 0, 1);
}