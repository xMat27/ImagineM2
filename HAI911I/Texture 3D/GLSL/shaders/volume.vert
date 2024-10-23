// --------------------------------------------------
// shader definition
// --------------------------------------------------

#version 130

// --------------------------------------------------
// Uniform variables:
// --------------------------------------------------
	uniform float xCutPosition;
	uniform float yCutPosition;
	uniform float zCutPosition;

	uniform float xMax;
	uniform float yMax;
	uniform float zMax;

	uniform mat4 mv_matrix;
	uniform mat4 proj_matrix;

// --------------------------------------------------
// varying variables
// --------------------------------------------------
varying vec3 position;
varying vec3 textCoord;
// --------------------------------------------------
// Vertex-Shader
// --------------------------------------------------


void main()
{
	gl_Position = proj_matrix * mv_matrix * gl_Vertex;
	position = gl_Vertex.xyz;

	//Todo : compute textCoord
	position = gl_Vertex.xyz;

	// Calculer les coordonnées de texture normalisées (de 0 à 1) en fonction des dimensions max
	textCoord.x = position.x / xMax;
	textCoord.y = position.y / yMax;
	textCoord.z = position.z / zMax;

}
