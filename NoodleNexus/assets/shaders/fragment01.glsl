#version 330
// (Pixel) Fragment fragment
in vec3 fColour;			// Actual 3D model colour (from vertex buffer)
in vec4 fvertexWorldLocation;
in vec4 fvertexNormal;
in vec2 fUV;				// Texture (UV) coordinates
in mat3 fTBN;  // Updated to use fTBN from geometry shader

uniform mat4 matView;
uniform mat4 matProjection;
uniform vec3 muzzlePos;
uniform vec3 fireDir;
uniform float timeSinceShot;

uniform sampler2D normalMap;

uniform float iTime;
uniform vec2 iMouse;
// for animations
uniform sampler2D texture_diffuse1;

// for objectives
uniform sampler2D objectiveTexture;
uniform float deltaTime;
uniform float health;
uniform bool bIsIPressed;
uniform vec4 objectColour;			// Override colour 
uniform bool bUseObjectColour;
uniform vec4 eyeLocation;			// Where the camera is
uniform bool bDoNotLight;			// if true, skips lighting
// 0.0 to 1.0 (invisible to solid)
// Controls the alpha channel
uniform float wholeObjectTransparencyAlpha;

// 0 = Regular forward (non-deferred) rendering pass (like the good old days)
//      i.e. this is regular "forward" rendering
const int REGULAR_FORWARD_RENDER = 0; 
// 1 = deferred G buffer pass
const int DEFERRED_G_BUFFER_PASS = 1;
// 2 = Some optional pass (2nd pass effect, maybe?)
const int EFFECTS_PASS = 2;
// 3 = deferred lighting pass and output to full screen quad
const int DEFERRED_LIGHTING_TO_FSQUAD_PASS = 3;

uniform int renderPassNumber;


// Written to the framebuffer (backbuffer)
//out vec4 finalPixelColour;	// RGB_A

// With deferred, we are writing to muliple outputs at the same time
// NOTE: first output layer is ALSO the colour buffer on final pass
//       (i.e. vertexWorldLocationXYZ is finalPixelColour on final pass)
//       (Because we are using the same shader)
out vec4 vertexWorldLocationXYZ;	// w = TBD
out vec4 vertexNormalXYZ;			// w : 1 = lit, 0 = do not light
out vec4 vertexDiffuseRGB;			// w = TBD
out vec4 vertexSpecularRGA_P;		// w = power

// Our lighting pass textures (the inputs for them)
// These come from the deferred rendering FBO 1st pass
uniform sampler2D vertexWorldLocationXYZ_texture;
uniform sampler2D vertexNormalXYZ_texture;
uniform sampler2D vertexDiffuseRGB_texture;
uniform sampler2D vertexSpecularRGA_P_texture;
uniform vec2 screenSize_width_height;

uniform sampler2D FBOTexture;

const int POINT_LIGHT_TYPE = 0;
const int SPOT_LIGHT_TYPE = 1;
const int DIRECTIONAL_LIGHT_TYPE = 2;

uniform bool bIsCeiling;
uniform bool bIsWall;
uniform bool bIsPlayer;
uniform bool bIsZombie;


void RenderGrass(out vec4 fragColor, in vec2 fragCoord);
void RenderClouds( out vec4 fragColor, in vec2 fragCoord );
void RenderBloodVignette( out vec4 fragColor, in vec2 fragCoord );
struct sLight
{
	vec4 position;			
	vec4 diffuse;	
	vec4 specular;	// rgb = highlight colour, w = power
	vec4 atten;		// x = constant, y = linear, z = quadratic, w = DistanceCutOff
	vec4 direction;	// Spot, directional lights
	vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
	                // 0 = pointlight
					// 1 = spot light
					// 2 = directional light
	vec4 param2;	// x = 0 for off, 1 for on
	                // yzw are TBD
};

const int NUMBEROFLIGHTS = 20;
uniform sLight theLights[NUMBEROFLIGHTS]; 
// uniform vec4 thelights[0].position;
// uniform vec4 thelights[1].position;

// Inspired by Mike Bailey's Graphic Shader, chapter 6
// (you should read it. Yes, you)
vec4 calculateLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular );

// Allows us to lookup the RGB colour from a 2D texture
// Give it the UV and it returns the colour at that UV location
uniform sampler2D texture00;
uniform sampler2D texture01;
uniform sampler2D texture02;
uniform sampler2D texture03;
uniform vec4 texRatio_0_to_3;	// x index 0, y index 1, etc/
//uniform float texRatio[4];
uniform bool bUseTextureAsColour;	// If true, then sample the texture

// Skybox (or reflection, refraction, etc.)
uniform samplerCube skyBoxTextureSampler;
uniform bool bIsSkyBoxObject;
//
uniform bool b_Is_FBO_Texture;
//

// For discard stencil example
uniform sampler2D stencilTexture;
uniform bool bUseStencilTexture;

uniform bool bHasNormalMap;

//uniform sampler2D textures[4];
//uniform sampler2DArray textures[3]
void RenderWater(out vec4 fragColor, in vec2 fragCoord);
void RenderGrass( out vec4 fragColor, in vec2 fragCoord );
void Pass_0_RegularForward(void);
void Pass_1_DeferredGBuffer(void);
// Pass_2...
void Pass_2_Effects_Pass(void);
//
void Pass_3_DeferredLightingToFSQ(void);		// FSQ (Full Screen Quad)

// for shadows
uniform samplerCube shadowMap;
uniform vec3 lightPos;
uniform float far_plane;

float ShadowCalculation(vec3 fragPos)
{
    vec3 fragToLight = fragPos - lightPos;
    float closestDepth = texture(shadowMap, fragToLight).r * far_plane;
    float currentDepth = length(fragToLight);
    
    float bias = 0.05;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

// Utility: get ray from camera through screen pixel
vec3 getWorldRay(vec2 uv) {
    vec4 screenPos = vec4(uv * 2.0 - 1.0, 1.0, 1.0); // NDC space
	mat4 invProjView = inverse(matProjection * matView);
    vec4 worldPos = invProjView * screenPos;
    worldPos /= worldPos.w;
    return normalize(worldPos.xyz - eyeLocation.xyz);
}

// Main procedural muzzle flash cone function
vec4 drawMuzzleFlash(vec2 uv) {
    vec3 ray = getWorldRay(uv);

    // Ray-plane intersection (fireDir is plane normal)
    float denom = dot(ray, fireDir);
    if (abs(denom) < 0.001) return vec4(0.0);

    float t = dot(muzzlePos - eyeLocation.xyz, fireDir) / denom;
    if (t < 0.0) return vec4(0.0);

    vec3 hitPos = eyeLocation.xyz + ray * t;
    vec3 dirToFrag = hitPos - muzzlePos;
    float dist = length(dirToFrag);
    vec3 normDir = normalize(dirToFrag);

    float cosAngle = dot(normDir, fireDir);
    float coneCutoff = cos(radians(20.0)); // adjust cone width
    if (cosAngle < coneCutoff) return vec4(0.0);

    float angleFalloff = pow((cosAngle - coneCutoff) / (1.0 - coneCutoff), 3.0);
    float distFalloff = exp(-dist * 30.0);
    float alpha = max(0.0, 1.0 - timeSinceShot * 25.0);

    vec3 color = vec3(1.0, 0.7, 0.3); // muzzle flash color
    return vec4(color * angleFalloff * distFalloff, alpha * angleFalloff);
}

void RenderObjectives( out vec4 fragColor, in vec2 fragCoord );
void main()
{
	// Am casting this an int 
	// even thought glUniform1i() 'should' work OK
	
	switch (int(renderPassNumber))
	{
	case REGULAR_FORWARD_RENDER:
		// 0 = Regular forward (non-deferred) rendering pass (like the good old days)
		//      i.e. this is regular "forward" rendering
		Pass_0_RegularForward();
		break;
	case DEFERRED_G_BUFFER_PASS:
		// 1 = deferred G buffer pass
		Pass_1_DeferredGBuffer();
		break;
	// This could be a pass for blur, whatever...
	//   
	case EFFECTS_PASS:
		Pass_2_Effects_Pass();
		break;
		
	case DEFERRED_LIGHTING_TO_FSQUAD_PASS:
		// 3 = deferred lighting pass and output to full screen quad
		Pass_3_DeferredLightingToFSQ();
		break;
	}
	
// 1 = deferred G buffer pass
// 2 = Some optional pass (2nd pass effect, maybe?)
// 3 = deferred lighting pass and output to full screen quad
	
}//void main()

void Pass_1_DeferredGBuffer(void)
{
	if ( b_Is_FBO_Texture ) {
		vertexWorldLocationXYZ.xyz = fvertexWorldLocation.xyz;	
		vertexWorldLocationXYZ.w = 1.0f;	// Not being used, so set to 1.0f

		vertexNormalXYZ.xyz = fvertexNormal.xyz;
		vertexNormalXYZ.w = 0.0f;	// 1 is lit, 0 is not lit

		vertexDiffuseRGB.rgb = texture( FBOTexture, fUV.st ).rgb;
		vertexDiffuseRGB.a = 1.0f;

		vec4 vertexSpecular = vec4(1.0f, 1.0f, 1.0f, 100.0f);	// w = power
		vertexSpecularRGA_P = vertexSpecular;		

		return;
	}

	// GL_COLOR_ATTACHMENT0
	vertexWorldLocationXYZ.xyz = fvertexWorldLocation.xyz;	
	vertexWorldLocationXYZ.w = 1.0f;	// Not being used, so set to 1.0f

	// GL_COLOR_ATTACHMENT1
	vertexNormalXYZ.xyz = fvertexNormal.xyz;
	// Default is vertex is to be lit
	vertexNormalXYZ.w = 1.0f;	// 1 is lit, 0 is not lit

	if(bHasNormalMap){
		//vec3 tangentSpaceFragPos = fTBN * fvertexWorldLocation.xyz;
		//vertexWorldLocationXYZ.xyz = tangentSpaceFragPos;
		
		vec3 normal = texture(normalMap, fUV).rgb;
		normal = normalize(normal * 2.0 - 1.0);

		vertexNormalXYZ.xyz = normal;
	}

	// *********************************************
	// Calcuate the colour (material) of this vertex

	//GL_COLOR_ATTACHMENT2
	vertexDiffuseRGB.rgb = vec3(0.0f, 0.0f, 0.0f);	// black
	vertexDiffuseRGB.a = 1.0f;
	


	if ( bUseStencilTexture )
	{
		float stencilColour = texture( stencilTexture, fUV.st ).r;
		//
		if ( stencilColour < 0.5f )	// Is it "black enough"
		{
			discard;	// don't draw this pixel
		}
	}
	

	// For the skybox object
	if ( bIsSkyBoxObject )
	{
		vertexDiffuseRGB.rgb = texture( skyBoxTextureSampler, fvertexNormal.xyz ).rgb;
		vertexDiffuseRGB.a = 1.0f;
		// Indicate that this it NOT to be lit (do lighting calculation in later pass)
		vertexNormalXYZ.w = 0.0f;	// 1 is lit, 0 is not lit
	}
	else
	{
		vec3 vertexColour = fColour;
		if ( bUseObjectColour )
		{
			vertexColour = objectColour.rgb;
		}
		
		if ( bUseTextureAsColour )
		{
			vec3 texColour00 = texture( texture00, fUV.st ).rgb;
			vec3 texColour01 = texture( texture01, fUV.st ).rgb;	
			vec3 texColour02 = texture( texture02, fUV.st ).rgb;	
			vec3 texColour03 = texture( texture03, fUV.st ).rgb;	
			
			// All these ratios should add up to 1.0
			vertexColour.rgb =   (texColour00.rgb * texRatio_0_to_3.x)
							   + (texColour01.rgb * texRatio_0_to_3.y)
		                       + (texColour02.rgb * texRatio_0_to_3.z)
		                       + (texColour03.rgb * texRatio_0_to_3.w);
			
		}//if ( bUseTextureAsColour )
		else{
			vertexColour.rgb = texture(texture_diffuse1, fUV.st).rgb;
			//vertexColour.rgb = vec3(1.0f, 0.0f, 0.0f);
		}

		vertexDiffuseRGB.rgb = vertexColour.rgb;
		vertexDiffuseRGB.a = 1.0f;
		


		// Use lighting?
		if ( bDoNotLight )  // Debug objects, for example
		{
			// Indicate that this it NOT to be lit (do lighting calculation in later pass)
			vertexNormalXYZ.w = 0.0f;	// 1 is lit, 0 is not lit
			return;
		}

		// GL_COLOR_ATTACHMENT3
		vec4 vertexSpecular = vec4(1.0f, 1.0f, 1.0f, 100.0f);	// w = power
		vertexSpecularRGA_P = vertexSpecular;		
	

	}//if ( bIsSkyBoxObject )

	

	return;
}

// Pass_2...
void Pass_3_DeferredLightingToFSQ(void)
{

	// Remember: Because we are using the same shader, 
	//	this 1st location is actually the frame buffer colour
	// (was called: out vec4 finalPixelColour; in the forward render
	//
	vertexWorldLocationXYZ.rgba = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	

	
//	uniform sampler2D vertexWorldLocationXYZ_texture;
//	uniform sampler2D vertexNormalXYZ_texture;
//	uniform sampler2D vertexDiffuseRGB_texture;
//	uniform sampler2D vertexSpecularRGA_P_texture;

	// While you CAN use the UVs of a full screen quad or other object,
	// that is tricky to align it to the screen, so we are 
	// using the built in "vec4 gl_FragCoord" variable.
	// https://www.khronos.org/opengl/wiki/Built-in_Variable_(GLSL)#Fragment_shader_inputs
	// "The location of the fragment in window space. The X, Y and Z components are
	//  the window-space position of the fragment."

	// The 'catch' is the gl_FragCoord.xy is pixel locations, not 0.0 to 1.0
	// We'll use the: uniform vec2 screenSize_width_height 
	//	to calculate the UV sampler values
	vec2 screenUV = vec2( gl_FragCoord.x / screenSize_width_height.x, 
	                      gl_FragCoord.y / screenSize_width_height.y );

	
	// Get the vertex colour from the 1st pass
//	vec3 vertexDiffuseRGB = texture( vertexDiffuseRGB_texture, fUV.st ).rgb;
	
	
	vec4 G_Buff_vertWorldLocationXYZ = texture( vertexWorldLocationXYZ_texture, screenUV.st ).rgba;
	vec4 G_Buff_vertNormXYZ = texture( vertexNormalXYZ_texture, screenUV.st ).rgba;
	vec4 G_Buff_vertDiffRGB = texture( vertexDiffuseRGB_texture, screenUV.st ).rgba;
	vec4 G_Buff_vertSpecRGA_P = texture( vertexSpecularRGA_P_texture, screenUV.st ).rgba;
	
	

	// Shift the world locations and scale them
//	verWorldLocationXYZ.xyz += 1000.0f;	
//	verWorldLocationXYZ.xyz /= 2000.0f;	
//	// Sent to colour buffer (for now)
//	vertexWorldLocationXYZ.rgb = verSpecRGA_P.rgb;
//	vertexWorldLocationXYZ.a = 1.0f;

	// Was this object to be lit? 
	// vertexNormalXYZ.w = 0.0 if NOT lit, non-0 if it IS to be lit
	//
	// The skybox, the debug objects, etc. 
	//
	if (  G_Buff_vertNormXYZ.w == 0.0f )
	{
		// Object ISN'T being lit by the lights
		vertexWorldLocationXYZ.rgb = G_Buff_vertDiffRGB.rgb;
//		vertexWorldLocationXYZ.rgb = vec3(1.0f, 0.0f, 0.0f);
		vertexWorldLocationXYZ.a = 1.0f;
			
		//vertexWorldLocationXYZ.rgb = (fullScreenBlendRato * vertexWorldLocationXYZ.rgb) +
	    //                         (1.0f-fullScreenBlendRato) * G_Buff_vertDiffRGB.rgb;
		if(bIsIPressed){
			RenderObjectives(vertexWorldLocationXYZ, gl_FragCoord.xy);
		}
		if(health <= 25.0f){
			RenderBloodVignette(vertexWorldLocationXYZ, gl_FragCoord.xy);
		}
		return;
	}


	// Do the lighting calculation just like before
	// (except we are reading the information from the 1st pass G buffer)
	vec4 pixelColour = calculateLightContrib( G_Buff_vertDiffRGB.rgb, 
	                                          G_Buff_vertNormXYZ.xyz, 
	                                          G_Buff_vertWorldLocationXYZ.xyz, 
											  G_Buff_vertSpecRGA_P );

	vertexWorldLocationXYZ.rgb = pixelColour.rgb;
	vertexWorldLocationXYZ.a = pixelColour.a;
	
	if(bIsIPressed){
		//RenderWater(vertexWorldLocationXYZ, gl_FragCoord.xy);
	
		RenderObjectives(vertexWorldLocationXYZ, gl_FragCoord.xy);
	}

	//RenderWater(vertexWorldLocationXYZ, gl_FragCoord.xy);

	vec4 blood = vec4(0.0f,0.0f,0.0f,1.0f);
	
	if(health <= 25.0f){
		RenderBloodVignette(blood, gl_FragCoord.xy);
		vertexWorldLocationXYZ += blood;
	}	//fullScreenBlend 0 to 1
	
	//vertexWorldLocationXYZ.rgb = (fullScreenBlendRato * vertexWorldLocationXYZ.rgb) +
	//                             (1.0f-fullScreenBlendRato) * pixelColour;

//	float depth = texture(shadowMap, vertexWorldLocationXYZ.rgb - lightPos).r;
//  vertexWorldLocationXYZ.rgb = vec3(depth);  // Visualize depth
//	vertexWorldLocationXYZ.a = 1.0f;

	//if(bHasNormalMap){
	//	vertexWorldLocationXYZ.rgb = texture(normalMap, screenUV).rgb;
	//}
	//vertexWorldLocationXYZ.rgb = normalize(G_Buff_vertNormXYZ.xyz) * 0.5 + 0.5;
	
	// Output the tangent, bitangent, and normal in the fragment shader
	//vertexWorldLocationXYZ.rgb = vec3(fTBN[0] * 0.5 + 0.5); // Visualize tangent
	// or
	//vertexWorldLocationXYZ.rgb = vec3(fTBN[1] * 0.5 + 0.5); // Visualize bitangent
	// or
	//vertexWorldLocationXYZ.rgb = vec3(fTBN[2] * 0.5 + 0.5); // Visualize normal

	//vertexWorldLocationXYZ += drawMuzzleFlash(screenUV);
	if ( b_Is_FBO_Texture ) {
		vertexWorldLocationXYZ.a = 1.0f;
	}
	return;
}


void Pass_0_RegularForward(void)
{

	// discard transparency
	// uniform sampler2D stencilTexture;
	// uniform bool bUseStencilTexture;
	if ( bUseStencilTexture )
	{
		float stencilColour = texture( stencilTexture, fUV.st ).r;
		//
		if ( stencilColour < 0.5f )	// Is it "black enough"
		{
			discard;	// don't draw this pixel
		}
	}


	// For the skybox object
	if ( bIsSkyBoxObject )
	{
		//finalPixelColour.rgb = fvertexNormal.xyz;
		//uniform samplerCube skyBoxTextureSampler;
		// Note: We are passing the NORMALS (a ray to hit the inside
		// 	of the cube) and NOT the texture coordinates
		vertexWorldLocationXYZ.rgb = texture( skyBoxTextureSampler, fvertexNormal.xyz ).rgb;
		vertexWorldLocationXYZ.a = 1.0f;
//		finalPixelColour.rgb = texture( skyBoxTextureSampler, fvertexNormal.xyz ).rgb;
//		finalPixelColour.a = 1.0f;
		return;
	}
	
	



	vec3 vertexColour = fColour;
	if ( bUseObjectColour )
	{
		vertexColour = objectColour.rgb;
	}
	
	if ( bUseTextureAsColour )
	{

		vec3 texColour00 = texture( texture00, fUV.st ).rgb;
		vec3 texColour01 = texture( texture01, fUV.st ).rgb;	
		vec3 texColour02 = texture( texture02, fUV.st ).rgb;	
		vec3 texColour03 = texture( texture03, fUV.st ).rgb;	
		
		
		// All these ratios should add up to 1.0
		vertexColour.rgb =   (texColour00.rgb * texRatio_0_to_3.x)
		                   + (texColour01.rgb * texRatio_0_to_3.y)
		                   + (texColour02.rgb * texRatio_0_to_3.z)
		                   + (texColour03.rgb * texRatio_0_to_3.w);
				
						   
	} 
	

	if ( b_Is_FBO_Texture )
	{
//		vec3 texColour00 = texture( texture00, fUV.st ).rgb;
//		vertexColour.rgb = texColour00.rgb;	
		//     -1   0   +1
		//  -1  .   +    .
		//   0  +   O    +
		//  +1  .   +    .
		//
		// Kernel size is 3 or 3x3 
		// size of 5 would 5 samples x 5 samples = 25
		// 
		// +++++
		// +++++
		// ++O++
		// +++++
		// +++++
		
		// ....+.....
		// ....+.....
		// ....+.....
		// ++++O+++++
		// ....+.....
		// ....+.....
		// ....+.....
		
		// Note: While you CAN pass multi-dimensional arrays
		//       through uniform variables, you CAN'T as constant arrays.
		//
		// int[5][5] gaussian_blur_5x5;		// Illegal
		//
		// So I'm doing this strange thing:
		//struct sRow
		//{
		//	int column[5];
		//};
		
		//sRow gaussian_5x5[5];
		// I took the 5x5 kernel from here:
//		// https://en.wikipedia.org/wiki/Kernel_(image_processing)
		//gaussian_5x5[0].column = int[](1,  4,  6,  4, 1);	// Note strange array init
		//gaussian_5x5[1].column = int[](4, 16, 24, 16, 1);	// Note strange array init
		//gaussian_5x5[2].column = int[](6, 24, 36, 24, 6);	// Note strange array init
		//gaussian_5x5[3].column = int[](4, 16, 24, 16, 1);	// Note strange array init
		//gaussian_5x5[4].column = int[](1,  4,  6,  4, 1);	// Note strange array init
		// There are a total of 256 samples in this gaussian 
		
		//vertexColour = vec3(0.0f, 0.0f, 0.0f);	// black
		
		
		
		
		
		//const float OFFSET_UV = 1.0f/1920.0f;	// Screen is 1920x1080
		
		//int samplesTaken = 0;
		
		// 3x3 starts at -1 less than 2
		// 5x5 starts at -2 less than 3
		// 9x9 starts at -4 less than 5  --> 81 samples
		//for ( int xIndex = -2; xIndex < 3; xIndex++ )
		//{
		//	for ( int yIndex = -2; yIndex < 3; yIndex++ )
		//	{
		//		vec2 UV_Offset = vec2(0.0f);
		//		UV_Offset.s = fUV.s + (xIndex * OFFSET_UV);
		//		UV_Offset.t = fUV.t + (yIndex * OFFSET_UV);
		//		//vertexColour.rgb += texture( texture00, UV_Offset).rgb;
		//
		//		// Multiple the sample by the value in the [x][y] gaussian matrix above
		//		vec3 sampleColour = texture( texture00, UV_Offset).rgb;
		//		int gassianWeight = gaussian_5x5[xIndex + 2].column[yIndex + 2];
		//		vertexColour.rgb += ( sampleColour * gassianWeight );

//				samplesTaken++;
		//		samplesTaken += gassianWeight;
				
		//	}//for ( int yIndex...
		//}// for ( int xIndex...	

		//vertexColour.rgb /= float(samplesTaken);

	}//if ( b_Is_FBO_Texture )

	
	
//	finalPixelColour = vec4(finalColour, 1.0);

//	vec3 fvertexNormal = vec3(0.0f, 1.0f, 0.0f);
	vec4 vertexSpecular = vec4(1.0f, 1.0f, 1.0f, 100.0f);	


	// Use lighting?
	if ( bDoNotLight )
	{
		vertexWorldLocationXYZ.rgb = vertexColour.rgb;
		vertexWorldLocationXYZ.a = wholeObjectTransparencyAlpha;
//		finalPixelColour.rgb = vertexColour.rgb;
//		finalPixelColour.a = wholeObjectTransparencyAlpha;
		return;
	}
	
	// Gold colour and highlight
	//https://i.pinimg.com/736x/49/71/e1/4971e1a994f6a208e04b53a2b98968d4.jpg
//	vertexColour.rgb = vec3(212.0f/255.0f, 175.0f/255.0f, 55.0f/255.0f);
//	vertexSpecular.rgba = vec4(255.0f/255.0f, 223.0f/255.0f, 0.0f/255.0f, 100.0f);

	vec4 pixelColour = calculateLightContrib( vertexColour.rgb, 
	                                          fvertexNormal.xyz, 
	                                          fvertexWorldLocation.xyz, 
											  vertexSpecular );

	vertexWorldLocationXYZ = pixelColour;
	if(vertexWorldLocationXYZ.r < 0.4){
		vertexWorldLocationXYZ.r += vertexColour.r * 0.2;
	}
	if(vertexWorldLocationXYZ.g < 0.4){
		vertexWorldLocationXYZ.g += vertexColour.g * 0.2;
	}
	if(vertexWorldLocationXYZ.b < 0.4){
		vertexWorldLocationXYZ.b += vertexColour.b * 0.2;
	}
	if(bIsZombie){
		vertexWorldLocationXYZ.rgb = vec3(1.0,0.0,0.0);
	}
	vertexWorldLocationXYZ.a = wholeObjectTransparencyAlpha;	// Set the alpha channel											
//	finalPixelColour = pixelColour;
//	finalPixelColour.a = wholeObjectTransparencyAlpha;	// Set the alpha channel
	
//	// Reflection:
//	vec3 eyeToVertexRay = normalize(eyeLocation.xyz - fvertexWorldLocation.xyz);
//	
//	vec3 reflectRay = reflect(eyeToVertexRay, fvertexNormal.xyz);	
//	vec3 refractRay = refract(eyeToVertexRay, fvertexNormal.xyz, 1.2f);
//	
//	vec3 reflectColour = texture( skyBoxTextureSampler, reflectRay.xyz ).rgb;
//	vec3 refractColour = texture( skyBoxTextureSampler, refractRay.xyz ).rgb;
//	
//	finalPixelColour.rgb += reflectColour.rgb * 0.3f
//	                       + refractColour.rgb * 0.3f;
						   


	
	
	
	// Use the 4th value (alpha) as the transparency
//	finalPixelColour.a = 1.0f - texture( texture03, fUV.st ).r;

//	finalPixelColour.a = 1.0f;		
//	finalPixelColour.a = 0.9f;		

	// Make the actual colour almost black
	// Apply the UVs as a colour
//	finalPixelColour.rgb *= 0.001f;	// Almost black
//	finalPixelColour.rg += fUV.xy;	// Add the UVs as colours

	// uniform sampler2D texture01;
	//vec3 texColour = texture( texture00, fUV.st ).rgb;
	//finalPixelColour.rgb += texColour;

	return;
}




// Inspired by Mike Bailey's Graphic Shader, chapter 6
// (you should read it. Yes, you)
vec4 calculateLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular )
{
	vec2 screenUV = vec2( gl_FragCoord.x / screenSize_width_height.x, 
	                      gl_FragCoord.y / screenSize_width_height.y );

	//vec3 norm = texture(normalMap, gl_FragCoord.xy).rgb;
	//norm = normalize(norm * 2.0 - 1.0);
	vec3 norm = normalize(vertexNormal) * 0.5 + 0.5;
	
	vec4 finalObjectColour = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	vec3 T_eyeLocation = vec3(0.0);
	if(bHasNormalMap){
		T_eyeLocation = fTBN * eyeLocation.xyz;
	}
	else{
		T_eyeLocation = eyeLocation.xyz;
	}

	vec3 ambientLight = vec3(0.08, 0.1, 0.13);


	finalObjectColour.rgb += vertexMaterialColour.rgb * ambientLight;


	for ( int index = 0; index < NUMBEROFLIGHTS; index++ )
	{	
		vec3 T_lightPos = vec3(0.0);
		if(bHasNormalMap){
			T_lightPos = fTBN * theLights[index].position.xyz;
		}
		else{
			T_lightPos = theLights[index].position.xyz;
		}

		// ********************************************************
		// is light "on"
		if ( theLights[index].param2.x == 0.0f )
		{	// it's off
			continue;
		}
		
		// Cast to an int (note with c'tor)
		int intLightType = int(theLights[index].param1.x);
		
		// We will do the directional light here... 
		// (BEFORE the attenuation, since sunlight has no attenuation, really)
		if ( intLightType == DIRECTIONAL_LIGHT_TYPE )		// = 2
		{

			// This is supposed to simulate sunlight. 
			// SO: 
			// -- There's ONLY direction, no position
			// -- Almost always, there's only 1 of these in a scene
			// Cheapest light to calculate. 

			vec3 lightContrib = theLights[index].diffuse.rgb * theLights[index].diffuse.rgb;
			
			// Get the dot product of the light and normalize
			float dotProduct = dot( normalize(norm.xyz) , theLights[index].direction.xyz);	// -1 to 1

			dotProduct = max( dotProduct, 0.0f);		// 0 to 1
		
			lightContrib *= dotProduct;		

			vec3 lightDir = normalize(-theLights[index].direction.xyz);

			vec3 viewDir = normalize(T_eyeLocation.xyz - vertexWorldPos.xyz);

			vec3 reflectDir = reflect(-lightDir, normalize(norm.xyz));
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), theLights[index].specular.w); // specular power in w component
			lightContrib += spec * theLights[index].specular.rgb ;

			finalObjectColour.rgb += (vertexMaterialColour.rgb * lightContrib); 
									 //+ (materialSpecular.rgb * lightSpecularContrib.rgb);
			// NOTE: There isn't any attenuation, like with sunlight.
			// (This is part of the reason directional lights are fast to calculate)

			//return finalObjectColour;		
		}
		
		// Assume it's a point light 
		// intLightType = 0
		
		// Contribution for this light
		vec3 vLightToVertex = T_lightPos - vertexWorldPos.xyz;
		float distanceToLight = length(vLightToVertex);	
		vec3 lightVector = normalize(vLightToVertex);
		float dotProduct = dot(lightVector, norm);	 
		
		// Cut off the light after the distance cut off 
		if ( distanceToLight > theLights[index].atten.w )
		{
			finalObjectColour = vec4(0.0f, 0.0f, 0.0f, 1.0f);
			return finalObjectColour;
		}
		
		dotProduct = max( 0.0f, dotProduct );	
		
		vec3 lightDiffuseContrib = dotProduct * theLights[index].diffuse.rgb;
			

		// Specular 
		vec3 lightSpecularContrib = vec3(0.0f);
			
		vec3 reflectVector = reflect( -lightVector, normalize(norm.xyz) );

		// Get eye or view vector
		// The location of the vertex in the world to your eye
		vec3 eyeVector = normalize(T_eyeLocation.xyz - vertexWorldPos.xyz);

		// To simplify, we are NOT using the light specular value, just the object’s.
		float objectSpecularPower = vertexSpecular.w; 
		
//		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
//			                   * vertexSpecular.rgb;	//* theLights[lightIndex].Specular.rgb
		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
			                   * theLights[index].specular.rgb;
							   
		// Attenuation
		float attenuation = 1.0f / 
				( theLights[index].atten.x + 										
				  theLights[index].atten.y * distanceToLight +						
				  theLights[index].atten.z * distanceToLight*distanceToLight );  	
				  
		// total light contribution is Diffuse + Specular
		lightDiffuseContrib *= attenuation;
		lightSpecularContrib *= attenuation;
		
		
		// But is it a spot light
		if ( intLightType == SPOT_LIGHT_TYPE )		// = 1
		{	
		

			// Yes, it's a spotlight
			// Calcualate light vector (light to vertex, in world)
			vec3 vertexToLight = vertexWorldPos.xyz - T_lightPos;

			vertexToLight = normalize(vertexToLight);

			float currentLightRayAngle
					= dot( vertexToLight.xyz, theLights[index].direction.xyz );
					
			currentLightRayAngle = max(0.0f, currentLightRayAngle);

			//vec4 param1;	
			// x = lightType, y = inner angle, z = outer angle, w = TBD

			// Is this inside the cone? 
			float outerConeAngleCos = cos(radians(theLights[index].param1.z));
			float innerConeAngleCos = cos(radians(theLights[index].param1.y));
							
			// Is it completely outside of the spot?
			if ( currentLightRayAngle < outerConeAngleCos )
			{
				// Nope. so it's in the dark
				lightDiffuseContrib = vec3(0.0f, 0.0f, 0.0f);
				lightSpecularContrib = vec3(0.0f, 0.0f, 0.0f);
			}
			else if ( currentLightRayAngle < innerConeAngleCos )
			{
				// Angle is between the inner and outer cone
				// (this is called the penumbra of the spot light, by the way)
				// 
				// This blends the brightness from full brightness, near the inner cone
				//	to black, near the outter cone
				float penumbraRatio = (currentLightRayAngle - outerConeAngleCos) / 
									  (innerConeAngleCos - outerConeAngleCos);
									  
				lightDiffuseContrib *= penumbraRatio;
				lightSpecularContrib *= penumbraRatio;
			}
						
		}// if ( intLightType == 1 )
		
		finalObjectColour.rgb += vertexMaterialColour.rgb * (lightDiffuseContrib.rgb
								  + vertexSpecular.rgb  * lightSpecularContrib.rgb);

		//float shadow = ShadowCalculation(vertexWorldPos);
		
		//finalObjectColour.rgb *= 1 - shadow;

	}//for(intindex=0...
	

	finalObjectColour.a = 1.0f;
	
	return finalObjectColour;
}


void Pass_2_Effects_Pass(void)
{

	return;
}

#define NOISE_TYPE_PERLIN 1
#define NOISE_TYPE_VALUE 2
#define NOISE_TYPE NOISE_TYPE_PERLIN

#define MOD3 vec3(.1031,.11369,.13787)
//#define MOD3 vec3(443.8975,397.2973, 491.1871)
float hash31(vec3 p3)
{
	p3  = fract(p3 * MOD3);
    p3 += dot(p3, p3.yzx + 19.19);
    return -1.0 + 2.0 * fract((p3.x + p3.y) * p3.z);
}

vec3 hash33(vec3 p3)
{
	p3 = fract(p3 * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return -1.0 + 2.0 * fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

// ========= Noise ===========
#if NOISE_TYPE == NOISE_TYPE_VALUE
float value_noise(vec3 p)
{
    vec3 pi = floor(p);
    vec3 pf = p - pi;
    
    vec3 w = pf * pf * (3.0 - 2.0 * pf);
    
    return 	mix(
        		mix(
        			mix(hash31(pi + vec3(0, 0, 0)), hash31(pi + vec3(1, 0, 0)), w.x),
        			mix(hash31(pi + vec3(0, 0, 1)), hash31(pi + vec3(1, 0, 1)), w.x), 
                    w.z),
        		mix(
                    mix(hash31(pi + vec3(0, 1, 0)), hash31(pi + vec3(1, 1, 0)), w.x),
        			mix(hash31(pi + vec3(0, 1, 1)), hash31(pi + vec3(1, 1, 1)), w.x), 
                    w.z),
        		w.y);
}
#define noise(p) value_noise(p)
#else
float perlin_noise(vec3 p)
{
    vec3 pi = floor(p);
    vec3 pf = p - pi;
    
    vec3 w = pf * pf * (3.0 - 2.0 * pf);
    
    return 	mix(
        		mix(
                	mix(dot(pf - vec3(0, 0, 0), hash33(pi + vec3(0, 0, 0))), 
                        dot(pf - vec3(1, 0, 0), hash33(pi + vec3(1, 0, 0))),
                       	w.x),
                	mix(dot(pf - vec3(0, 0, 1), hash33(pi + vec3(0, 0, 1))), 
                        dot(pf - vec3(1, 0, 1), hash33(pi + vec3(1, 0, 1))),
                       	w.x),
                	w.z),
        		mix(
                    mix(dot(pf - vec3(0, 1, 0), hash33(pi + vec3(0, 1, 0))), 
                        dot(pf - vec3(1, 1, 0), hash33(pi + vec3(1, 1, 0))),
                       	w.x),
                   	mix(dot(pf - vec3(0, 1, 1), hash33(pi + vec3(0, 1, 1))), 
                        dot(pf - vec3(1, 1, 1), hash33(pi + vec3(1, 1, 1))),
                       	w.x),
                	w.z),
    			w.y);
}
#define noise(p) perlin_noise(p)
#endif

float fbm0(vec2 p)
{
    float d = 0.0;
    //const mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 ); 
    float m = 4.0;
    d += 0.5 * noise(p.xyy); p = m * p;
    d += 0.25 * noise(p.xyy); p = m * p;
    d += 0.125 * noise(p.xyy); p = m * p;
    d += 0.0625 * noise(p.xyy); p = m * p;
    return d;
}

float fbm1(vec2 p)
{
    float d = 0.0;
    //const mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 ); 
    float m = 1.5;
    d -= 0.5 * noise(p.xyy); p = m * p;
    d -= 0.25 * noise(p.xyy); p = m * p;
    d -= 0.125 * noise(p.xyy); p = m * p;
    d -= 0.0625 * noise(p.xyy); p = m * p;
    return d;
}

float FBM(vec2 p)
{
    float d = 0.0;
    p.xy -= 0.5;
    vec2 op = p;
    p *= 4.0;
    if (p.x < 10.0) {        
        float w = 1.0;
        d += w * noise(p.xyy); 
        
        p *= 4.0;
        w /= 8.0;
        //float m = 1.8;
        const mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 ); 
        d -= w * noise(p.xyy); p = m * p; w *= 0.5;
        d -= w * noise(p.xyy); p = m * p; w *= 0.5;
        d -= w * noise(p.xyy); p = m * p; w *= 0.5;
        d -= w * noise(p.xyy); p = m * p; w *= 0.5;
    }
    else {
        if (p.x < 0.0) {
            //return fbm0((p+vec2(0.5,0.0))*10.);
            d = noise((p+vec2(0.5,0.0)).xyy*10.);
        }
        else {
            d = fbm1(p*2.);
        }
        d = fbm1(p*1.);
    }
    
    //float l = length(op) - 0.5*sqrt(2.0);
    //d += l;
    
    d += 0.5;
    
    float x = clamp(length(op) / (0.5*sqrt(2.0)), 0.0, 1.0);
    x = pow(x,1.0);
    d *= x;
    
    d -= 0.5;

    return max(d, -0.5);
}

float sdGrid(vec2 coord, float num)
{
	vec2 f = fract(coord.xy * num);
	return 1.0 - smoothstep(0.00,0.01,min(f.x,f.y));
}

#define COLOR_LEAD vec3(.95, .35, .01)
#define COLOR_TRAIL vec3(.95, .75, .01)
#define COLOR_BLACK vec3(.1, .1, .1)
void RenderObjectives( out vec4 fragColor, in vec2 fragCoord )
{
    float time = deltaTime;
    time = deltaTime * 5.0f;
    
    vec2 uv = fragCoord/screenSize_width_height.xy;

	//vec2 uv = fragCoord/screenSize_width_height.xy;
    
    //vec3 heightmap = texture(iChannel2, uv).rrr;
    vec3 heightmap  = vec3(FBM(uv)+0.5);
    vec3 background = texture(objectiveTexture, uv).rgb;
    //vec3 foreground = texture(vertexWorldLocationXYZ, uv.st).rgb;
    vec3 foreground = vertexWorldLocationXYZ.rgb;
    
    float t = fract(-time*0.2);
    //vec3 erosion = smoothstep(t-0.1*length(uv-0.5)*4., t, heightmap);
    vec3 erosion = smoothstep(t-0.2, t, heightmap);
    //erosion = erosion * erosion * erosion;
    
    //vec3 col = mix(foreground, background, erosion);
    
    
    vec3 border = smoothstep(0.0, 0.1, erosion) - smoothstep(0.1, 1.0, erosion);
    vec3 bordercolor = mix(COLOR_LEAD, COLOR_TRAIL, smoothstep(0.9, 1.0, border));
    vec3 col = mix(foreground, bordercolor * border, step(0.01, border));
    col = mix(col, background, erosion);
    
    //col = heightmap;
    fragColor = vec4(col,1.0);// + vec4(sdGrid(uv-0.5, 2.));
}

///////////////////////////////////////////////RayMarchedWater//////////////////////////////////////////////
// afl_ext 2017-2024
// MIT License

// Use your mouse to move the camera around! Press the Left Mouse Button on the image to look around!

#define DRAG_MULT 0.38 // changes how much waves pull on the water
#define WATER_DEPTH 1.5 // how deep is the water
#define CAMERA_HEIGHT 1.5 // how high the camera should be
#define ITERATIONS_RAYMARCH 12 // waves iterations of raymarching
#define ITERATIONS_NORMAL 36 // waves iterations when calculating normals

#define NormalizedMouse (iMouse.xy / screenSize_width_height.xy) // normalize mouse coords

// Calculates wave value and its derivative, 
// for the wave direction, position in space, wave frequency and time
vec2 wavedx(vec2 position, vec2 direction, float frequency, float timeshift) {
  float x = dot(direction, position) * frequency + timeshift;
  float wave = exp(sin(x) - 1.0);
  float dx = wave * cos(x);
  return vec2(wave, -dx);
}

// Calculates waves by summing octaves of various waves with various parameters
float getwaves(vec2 position, int iterations) {
  float wavePhaseShift = length(position) * 0.1; // this is to avoid every octave having exactly the same phase everywhere
  float iter = 0.0; // this will help generating well distributed wave directions
  float frequency = 1.0; // frequency of the wave, this will change every iteration
  float timeMultiplier = 2.0; // time multiplier for the wave, this will change every iteration
  float weight = 1.0;// weight in final sum for the wave, this will change every iteration
  float sumOfValues = 0.0; // will store final sum of values
  float sumOfWeights = 0.0; // will store final sum of weights
  for(int i=0; i < iterations; i++) {
    // generate some wave direction that looks kind of random
    vec2 p = vec2(sin(iter), cos(iter));
    
    // calculate wave data
    vec2 res = wavedx(position, p, frequency, iTime * timeMultiplier + wavePhaseShift);

    // shift position around according to wave drag and derivative of the wave
    position += p * res.y * weight * DRAG_MULT;

    // add the results to sums
    sumOfValues += res.x * weight;
    sumOfWeights += weight;

    // modify next octave ;
    weight = mix(weight, 0.0, 0.2);
    frequency *= 1.18;
    timeMultiplier *= 1.07;

    // add some kind of random value to make next wave look random too
    iter += 1232.399963;
  }
  // calculate and return
  return sumOfValues / sumOfWeights;
}

// Raymarches the ray from top water layer boundary to low water layer boundary
float raymarchwater(vec3 camera, vec3 start, vec3 end, float depth) {
  vec3 pos = start;
  vec3 dir = normalize(end - start);
  for(int i=0; i < 64; i++) {
    // the height is from 0 to -depth
    float height = getwaves(pos.xz, ITERATIONS_RAYMARCH) * depth - depth;
    // if the waves height almost nearly matches the ray height, assume its a hit and return the hit distance
    if(height + 0.01 > pos.y) {
      return distance(pos, camera);
    }
    // iterate forwards according to the height mismatch
    pos += dir * (pos.y - height);
  }
  // if hit was not registered, just assume hit the top layer, 
  // this makes the raymarching faster and looks better at higher distances
  return distance(start, camera);
}

// Calculate normal at point by calculating the height at the pos and 2 additional points very close to pos
vec3 normal(vec2 pos, float e, float depth) {
  vec2 ex = vec2(e, 0);
  float H = getwaves(pos.xy, ITERATIONS_NORMAL) * depth;
  vec3 a = vec3(pos.x, H, pos.y);
  return normalize(
    cross(
      a - vec3(pos.x - e, getwaves(pos.xy - ex.xy, ITERATIONS_NORMAL) * depth, pos.y), 
      a - vec3(pos.x, getwaves(pos.xy + ex.yx, ITERATIONS_NORMAL) * depth, pos.y + e)
    )
  );
}

// Helper function generating a rotation matrix around the axis by the angle
mat3 createRotationMatrixAxisAngle(vec3 axis, float angle) {
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;
  return mat3(
    oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 
    oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 
    oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c
  );
}

// Helper function that generates camera ray based on UV and mouse
vec3 getRay(vec2 fragCoord) {
  vec2 uv = ((fragCoord.xy / screenSize_width_height.xy) * 2.0 - 1.0) * vec2(screenSize_width_height.x / screenSize_width_height.y, 1.0);
  // for fisheye, uncomment following line and comment the next one
  //vec3 proj = normalize(vec3(uv.x, uv.y, 1.0) + vec3(uv.x, uv.y, -1.0) * pow(length(uv), 2.0) * 0.05);  
  vec3 proj = normalize(vec3(uv.x, uv.y, 1.5));
  if(screenSize_width_height.x < 600.0) {
    return proj;
  }
  return createRotationMatrixAxisAngle(vec3(0.0, -1.0, 0.0), 3.0 * ((NormalizedMouse.x + 0.5) * 2.0 - 1.0)) 
    * createRotationMatrixAxisAngle(vec3(1.0, 0.0, 0.0), 0.5 + 1.5 * (((NormalizedMouse.y == 0.0 ? 0.27 : NormalizedMouse.y) * 1.0) * 2.0 - 1.0))
    * proj;
}

// Ray-Plane intersection checker
float intersectPlane(vec3 origin, vec3 direction, vec3 point, vec3 normal) { 
  return clamp(dot(point - origin, normal) / dot(direction, normal), -1.0, 9991999.0); 
}

// Some very barebones but fast atmosphere approximation
vec3 extra_cheap_atmosphere(vec3 raydir, vec3 sundir) {
  //sundir.y = max(sundir.y, -0.07);
  float special_trick = 1.0 / (raydir.y * 1.0 + 0.1);
  float special_trick2 = 1.0 / (sundir.y * 11.0 + 1.0);
  float raysundt = pow(abs(dot(sundir, raydir)), 2.0);
  float sundt = pow(max(0.0, dot(sundir, raydir)), 8.0);
  float mymie = sundt * special_trick * 0.2;
  vec3 suncolor = mix(vec3(1.0), max(vec3(0.0), vec3(1.0) - vec3(5.5, 13.0, 22.4) / 22.4), special_trick2);
  vec3 bluesky= vec3(5.5, 13.0, 22.4) / 22.4 * suncolor;
  vec3 bluesky2 = max(vec3(0.0), bluesky - vec3(5.5, 13.0, 22.4) * 0.002 * (special_trick + -6.0 * sundir.y * sundir.y));
  bluesky2 *= special_trick * (0.24 + raysundt * 0.24);
  return bluesky2 * (1.0 + 1.0 * pow(1.0 - raydir.y, 3.0));
} 

// Calculate where the sun should be, it will be moving around the sky
vec3 getSunDirection() {
  return normalize(vec3(-0.0773502691896258 , 0.5 + sin(iTime * 0.2 + 2.6) * 0.45 , 0.5773502691896258));
}

// Get atmosphere color for given direction
vec3 getAtmosphere(vec3 dir) {
   return extra_cheap_atmosphere(dir, getSunDirection()) * 0.5;
}

// Get sun color for given direction
float getSun(vec3 dir) { 
  return pow(max(0.0, dot(dir, getSunDirection())), 720.0) * 210.0;
}

// Great tonemapping function from my other shader: https://www.shadertoy.com/view/XsGfWV
vec3 aces_tonemap(vec3 color) {  
  mat3 m1 = mat3(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
  );
  mat3 m2 = mat3(
    1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
  );
  vec3 v = m1 * color;  
  vec3 a = v * (v + 0.0245786) - 0.000090537;
  vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
  return pow(clamp(m2 * (a / b), 0.0, 1.0), vec3(1.0 / 2.2));  
}

// Main
void RenderWater(out vec4 fragColor, in vec2 fragCoord) {
  // get the ray
  vec3 ray = getRay(fragCoord);
  if(ray.y >= 0.0) {
    // if ray.y is positive, render the sky
    vec3 C = getAtmosphere(ray) + getSun(ray);
    fragColor = vec4(aces_tonemap(C * 2.0),1.0);   
    return;
  }

  // now ray.y must be negative, water must be hit
  // define water planes
  vec3 waterPlaneHigh = vec3(0.0, 0.0, 0.0);
  vec3 waterPlaneLow = vec3(0.0, -WATER_DEPTH, 0.0);

  // define ray origin, moving around
  vec3 origin = vec3(iTime * 0.2, CAMERA_HEIGHT, 1);

  // calculate intersections and reconstruct positions
  float highPlaneHit = intersectPlane(origin, ray, waterPlaneHigh, vec3(0.0, 1.0, 0.0));
  float lowPlaneHit = intersectPlane(origin, ray, waterPlaneLow, vec3(0.0, 1.0, 0.0));
  vec3 highHitPos = origin + ray * highPlaneHit;
  vec3 lowHitPos = origin + ray * lowPlaneHit;

  // raymatch water and reconstruct the hit pos
  float dist = raymarchwater(origin, highHitPos, lowHitPos, WATER_DEPTH);
  vec3 waterHitPos = origin + ray * dist;

  // calculate normal at the hit position
  vec3 N = normal(waterHitPos.xz, 0.01, WATER_DEPTH);

  // smooth the normal with distance to avoid disturbing high frequency noise
  N = mix(N, vec3(0.0, 1.0, 0.0), 0.8 * min(1.0, sqrt(dist*0.01) * 1.1));

  // calculate fresnel coefficient
  float fresnel = (0.04 + (1.0-0.04)*(pow(1.0 - max(0.0, dot(-N, ray)), 5.0)));

  // reflect the ray and make sure it bounces up
  vec3 R = normalize(reflect(ray, N));
  R.y = abs(R.y);
  
  // calculate the reflection and approximate subsurface scattering
  vec3 reflection = getAtmosphere(R) + getSun(R);
  vec3 scattering = vec3(0.0293, 0.0698, 0.1717) * 0.1 * (0.2 + (waterHitPos.y + WATER_DEPTH) / WATER_DEPTH);

  // return the combined result
  vec3 C = fresnel * reflection + scattering;
  fragColor = vec4(aces_tonemap(C * 2.0), 1.0);
}

void RenderBloodVignette(out vec4 fragColor, in vec2 fragCoord)
{
    // Normalized pixel coordinates (0 to 1)
    vec2 uv = fragCoord / screenSize_width_height.xy;

    // Distance from center
    float d = length(uv - vec2(0.5, 0.5));

    // Smooth red vignette effect only at edges (no color in center)
    float edge = smoothstep(0.3, 0.7, d); // Adjust 0.3 and 0.7 for softness and size
    vec3 red = vec3(1.0, 0.0, 0.0);       // Pure red

    // Optional: pulse intensity over time
    float pulse = sin(iTime * 2.0) * 0.1 + 0.2; // Subtle pulsing

    // Final output: no color in center, soft red at edges
    fragColor = vec4(red * edge * pulse, edge * pulse);
}

const float cloudscale = 1.1;
const float speed = 0.03;
const float clouddark = 0.5;
const float cloudlight = 0.3;
const float cloudcover = 0.2;
const float cloudalpha = 8.0;
const float skytint = 0.5;
const vec3 skycolour1 = vec3(0.2, 0.4, 0.6);
const vec3 skycolour2 = vec3(0.4, 0.7, 1.0);

const mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );

vec2 hash( vec2 p ) {
	p = vec2(dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)));
	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

float noise( in vec2 p ) {
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;
	vec2 i = floor(p + (p.x+p.y)*K1);	
    vec2 a = p - i + (i.x+i.y)*K2;
    vec2 o = (a.x>a.y) ? vec2(1.0,0.0) : vec2(0.0,1.0); //vec2 of = 0.5 + 0.5*vec2(sign(a.x-a.y), sign(a.y-a.x));
    vec2 b = a - o + K2;
	vec2 c = a - 1.0 + 2.0*K2;
    vec3 h = max(0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );
	vec3 n = h*h*h*h*vec3( dot(a,hash(i+0.0)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));
    return dot(n, vec3(70.0));	
}

float fbm(vec2 n) {
	float total = 0.0, amplitude = 0.1;
	for (int i = 0; i < 7; i++) {
		total += noise(n) * amplitude;
		n = m * n;
		amplitude *= 0.4;
	}
	return total;
}

// -----------------------------------------------

void RenderClouds( out vec4 fragColor, in vec2 fragCoord ) {
    vec2 p = fragCoord.xy / screenSize_width_height.xy;
	vec2 uv = p*vec2(screenSize_width_height.x/screenSize_width_height.y,1.0);    
    float time = iTime * speed;
    float q = fbm(uv * cloudscale * 0.5);
    
    //ridged noise shape
	float r = 0.0;
	uv *= cloudscale;
    uv -= q - time;
    float weight = 0.8;
    for (int i=0; i<8; i++){
		r += abs(weight*noise( uv ));
        uv = m*uv + time;
		weight *= 0.7;
    }
    
    //noise shape
	float f = 0.0;
    uv = p*vec2(screenSize_width_height.x/screenSize_width_height.y,1.0);
	uv *= cloudscale;
    uv -= q - time;
    weight = 0.7;
    for (int i=0; i<8; i++){
		f += weight*noise( uv );
        uv = m*uv + time;
		weight *= 0.6;
    }
    
    f *= r + f;
    
    //noise colour
    float c = 0.0;
    time = iTime * speed * 2.0;
    uv = p*vec2(screenSize_width_height.x/screenSize_width_height.y,1.0);
	uv *= cloudscale*2.0;
    uv -= q - time;
    weight = 0.4;
    for (int i=0; i<7; i++){
		c += weight*noise( uv );
        uv = m*uv + time;
		weight *= 0.6;
    }
    
    //noise ridge colour
    float c1 = 0.0;
    time = iTime * speed * 3.0;
    uv = p*vec2(screenSize_width_height.x/screenSize_width_height.y,1.0);
	uv *= cloudscale*3.0;
    uv -= q - time;
    weight = 0.4;
    for (int i=0; i<7; i++){
		c1 += abs(weight*noise( uv ));
        uv = m*uv + time;
		weight *= 0.6;
    }
	
    c += c1;
    
    vec3 skycolour = mix(skycolour2, skycolour1, p.y);
    vec3 cloudcolour = vec3(1.1, 1.1, 0.9) * clamp((clouddark + cloudlight*c), 0.0, 1.0);
   
    f = cloudcover + cloudalpha*f*r;
    
    vec3 result = mix(skycolour, clamp(skytint * skycolour + cloudcolour, 0.0, 1.0), clamp(f + c, 0.0, 1.0));
    
	fragColor = vec4( result, 1.0 );
}

///////////////////////////////////////Grass///////////////////////////////////////////////
// a simple implementation of Shell Texturing, as described by Acerola: https://www.youtube.com/watch?v=9dr-tRQzij4
// coded by Michael Tedder (@_falken / brs^bas)
//
// uses SDFs for each layer in place of drawing multiple quads.
// for a slightly better demo, see: https://www.shadertoy.com/view/dlVyWm
// (not terribly optimized due to lack of time, sorry!)
//
// hat tips to:
// 1. sdf/lighting: https://www.shadertoy.com/view/stcGRX
// 2. noise: https://www.shadertoy.com/view/4dS3Wd

// feel free to tweak any of these values below:
const int kSubsampleFactor = 1;         // sampling quality, any positive integer > 0
const int kNumberOfLayers = 32;         // number of shell layers, any positive integer > 0
const float kShellDensity = 500.0;      // detail of points in each layer, any positive value > 0
const float kShellLayerSpacing = 0.1;   // spacing between each layer, any positive value > 0

// precision-adjusted variations of https://www.shadertoy.com/view/4djSRW
float hashNew(vec2 p) {vec3 p3 = fract(vec3(p.xyx) * 0.13); p3 += dot(p3, p3.yzx + 3.333); return fract((p3.x + p3.y) * p3.z); }



float sdBox(vec3 position, vec3 halfSize)
{
	position = abs(position) - halfSize;
	return length(max(position, 0.0)) + min(max(max(position.x, position.y), position.z), 0.0);
}

float sdf(in vec3 position)
{ 
	float halfWidth = 0.5; 
	float halfHeight = 0.001;
	float halfDepth = 0.5;
	return sdBox(position, vec3(halfWidth, halfHeight, halfDepth));
}

vec3 normal(vec3 position)
{
	float epsilon = 0.001;
	vec3 gradient = vec3(
		sdf(position + vec3(epsilon, 0, 0)) - sdf(position + vec3(-epsilon, 0, 0)),
		sdf(position + vec3(0, epsilon, 0)) - sdf(position + vec3(0, -epsilon, 0)),
		sdf(position + vec3(0, 0, epsilon)) - sdf(position + vec3(0, 0, -epsilon))
	);
	return normalize(gradient);
}

float raycast(vec3 rayOrigin, vec3 rayDirection, vec3 translation)
{
	int stepCount = 128 * 2;
	float maximumDistance = 5.0;
	float t = 0.0;
	for (int i = 0; i < stepCount; i++) {
		if (t > maximumDistance) {
			break;
		}
		vec3 position = rayOrigin + rayDirection * t;
		float d = sdf(position + translation);
		if (d < 0.0001) {
			return t;
		}
		t += d;
	}
	return 0.0;
}

mat3 lookAtMatrix(vec3 from, vec3 to)
{
	vec3 forward = normalize(to - from);
	vec3 right = normalize(cross(forward, vec3(0.0, 1.0, 0.0)));
	vec3 up = cross(right, forward);
	return mat3(right, up, forward);
}

void RenderGrass(out vec4 fragColor, in vec2 fragCoord)
{
	float angle = 0.5 * (iTime - 8.0);
	vec3 cameraOrigin = eyeLocation.xyz;
	vec3 cameraTarget = vec3(0.0);
	mat3 cameraTransform = lookAtMatrix(cameraOrigin, cameraTarget);
	vec3 result = vec3(0.0);
    
    const float layerYInc = 1.0 / float(kNumberOfLayers);
    
	ivec2 sampleCount = ivec2(kSubsampleFactor, kSubsampleFactor);
	for (int y = 0; y < sampleCount.y; y++)
    {
		for (int x = 0; x < sampleCount.x; x++)
        {
			vec2 uv = fragCoord + (vec2(float(x), float(y)) / vec2(sampleCount) - 0.5);
			uv = uv / screenSize_width_height.xy;
			uv = (uv * 2.0) - 1.0;
			uv.x *= screenSize_width_height.x / screenSize_width_height.y;

            vec3 rayDirection = normalize(vec3(uv, 1.5));
			rayDirection = cameraTransform * rayDirection;
            
			vec3 color = vec3(0.0);
            float layerY = 0.0;
            for (int layerIndex = 0; layerIndex < kNumberOfLayers; ++layerIndex)
            {
                vec3 translation = vec3(0.0, -layerY * kShellLayerSpacing, 0.0);
                float t = raycast(cameraOrigin, rayDirection, translation);
                if (t > 0.0)
                {
                    // same style that iq uses in his shaders
                    vec3 position = cameraOrigin + rayDirection * t;
                    vec3 lightDirection = vec3(0.57735);
                    vec3 n = normal(position);
                    float diffuseAngle = max(dot(n, lightDirection), 0.0);

                    // position (xz along plane) is in range [-0.5 .. +0.5] so renormalize for noise
                    float v = noise(position.xz * kShellDensity);
                    if (v > max(0.1, layerY)) {
                        color = vec3(0.0, max(0.1, layerY), 0.0);
                        color *= diffuseAngle;
                    }
                }
                layerY += layerYInc;
            }
			// gamma        
			color = sqrt(color);
			result += color;
		}
	}
	result /= float(sampleCount.x * sampleCount.y);
	fragColor = vec4(result, 1.0);
}
