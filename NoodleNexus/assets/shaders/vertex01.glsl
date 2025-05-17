#version 330
// Vertex shader
in vec3 vCol;
in vec3 vPos;
in vec3 vNormal;	// Normal from the model ("model" space)
in vec2 vUV;		// Texture coordinates
in vec3 vTangent;
in vec3 vBitangent;

// Added for animations
in vec3 tangent;
in vec3 bitangent;
in ivec4 boneIds; 
in vec4 weights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

// Changed 'f' (Fragment) to 'g' for Geometry shader
out vec3 gColour;
out vec4 gVertexWorldLocation;
out vec4 gVertexNormal;		// Normal in "world" space
out vec2 gUV;				// Texture coordinates (to the fragment shader)
out mat3 gTBN;              // TBN matrix for normal mapping

//uniform mat4 MVP;
uniform mat4 matView;
uniform mat4 matProjection;
uniform mat4 matModel;

uniform bool bHasNormalMap;


void main()
{
	vec3 finalVert = vPos;	

	vec4 totalPosition = vec4(0.0f);

    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(finalVert,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(finalVert,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * vNormal;
   }
	
    //mat4 viewModel = view * model;
    //gl_Position =  projection * viewModel * totalPosition;

	// Screen space location of vertex
	mat4 matMVP = matProjection * matView * matModel;

	//gl_Position = matMVP * vec4(finalVert, 1.0);
	gl_Position = matMVP * totalPosition;
	
	// Calculate location of the vertex in the "world"
	//gVertexWorldLocation = matModel * vec4(finalVert, 1.0);
	gVertexWorldLocation = matModel * totalPosition;
	
	// Calculatte the vertex normal
	// Don't wank scaling or translation
	//fvertexNormal = matRoationOnly * vec4(vNormal, 1.0);
	mat4 matInvTransModel = inverse(transpose(matModel));
	// Just in case
	
	vec3 vNormNormalize = normalize(vNormal.xyz);
	gVertexNormal = matInvTransModel * vec4(vNormNormalize, 1.0);
	// Just in case
	gVertexNormal.xyz = normalize(gVertexNormal.xyz);

	if(bHasNormalMap){
		// Compute the TBN matrix (Tangent-Bitangent-Normal)
		// Transform tangent, bitangent, and normal to world space
		mat3 normalMatrix = transpose(inverse(mat3(matModel)));
		vec3 T = normalize(normalMatrix * vTangent);
		vec3 N = normalize(normalMatrix * vNormal);
		T = normalize(T - dot(T, N) * N);
		vec3 B = cross(N, T);
		//vec3 B = normalize(normalMatrix * vBitangent);
		//gTBN = mat3(T, B, N);
		gTBN = transpose(mat3(T, B, N));
	}
	else{
		gTBN = mat3(1.0);
	}

	gColour = vCol;
	gUV = vUV;			// Sent UVs to fragment shader
}
