#version 140


// --------------------------------------------------
// shader definition
// --------------------------------------------------

uniform sampler3D mask; // déclaration de la map mask

uniform float xCutPosition;
uniform float yCutPosition;
uniform float zCutPosition;

uniform int xCutDirection; 
uniform int yCutDirection;
uniform int zCutDirection;

uniform float xMax;
uniform float yMax;
uniform float zMax;

uniform mat4 mv_matrix;
uniform mat4 proj_matrix;

varying vec3 position;
varying vec3 textCoord;

bool ComputeVisibility(vec3 point){
	vec3 x = vec3(1.*xCutDirection,0.,0.);
	vec3 y = vec3(0.,1.*yCutDirection,0.);
	vec3 z = vec3(0.,0.,1.*zCutDirection);

	vec3 xCut = vec3(xCutPosition, 0.,0.);
	vec3 yCut = vec3(0., yCutPosition,0.);
	vec3 zCut = vec3(0., 0.,zCutPosition);

	 bool visible = true; // On commence par supposer que le point est visible

    // Vérification pour le plan X
    if (xCutDirection != 0) {
        float xVisibility = dot(point - xCut, vec3(xCutDirection, 0.0, 0.0));
        if (xVisibility < 0) {
            visible = false; 
        }
    }

    // Vérification pour le plan Y
    if (yCutDirection != 0) {
        float yVisibility = dot(point - yCut, vec3(0.0, yCutDirection, 0.0));
        if (yVisibility < 0) {
            visible = false; 
        }
    }

    // Vérification pour le plan Z
    if (zCutDirection != 0) {
        float zVisibility = dot(point - zCut, vec3(0.0, 0.0, zCutDirection));
        if (zVisibility < 0) {
            visible = false; 
        }
    }

    return visible;
}

vec3 rayTrace(vec3 inpos){
	vec3 camPos = (inverse(mv_matrix) * vec4(0, 0, 0, 1)).xyz;
	vec3 dir = normalize(inpos - camPos);

	//TODO raytrace


	return vec3(0);
}

// --------------------------------------------------
// Fragment Shader:
// --------------------------------------------------
void main() {

	if(!ComputeVisibility(position)){
		//discard;
	}

	//TODO fetch color in texture
	vec4 textureColor = texture(mask, textCoord);

    gl_FragColor = textureColor;



	//gl_FragColor = vec4(0,0,0,1);
}
