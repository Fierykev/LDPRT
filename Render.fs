#define NUM_BANDS 6
#define COEFF_GRPS (NUM_BANDS * NUM_BANDS) >> 2
#define NUM_COEFF_GRPS 2

#define M_PI 3.14159265358979323846
#define saturate(in) (clamp(in, 0, 1))

// environment
uniform sampler2D environment;

// lighting
uniform vec3 lightDir;

// intensity
uniform float lightIntensity;

// diffuse color
uniform bool useDiffuseTex;
uniform sampler2D diffuseTex;
uniform vec4 diffuseCol;

// specular color
uniform bool useSpecTex;
uniform sampler2D specTex;

// specular highlight
uniform bool useSpecHighTex;
uniform sampler2D specHighTex;

// specular intensity
uniform float shininess;

// bump norm
uniform bool useBumpTex;
uniform float bumpMult;
uniform sampler2D bumpTex;

// albedo
uniform bool useAlbedo;
uniform sampler2D albedoTex;

// spherical harmonics
uniform samplerCube harmonics[COEFF_GRPS];

// setup light uniforms
uniform vec4 lightR[COEFF_GRPS];
uniform vec4 lightG[COEFF_GRPS];
uniform vec4 lightB[COEFF_GRPS];

// setup back-light uniforms
uniform vec4 backLightR[COEFF_GRPS];
uniform vec4 backLightG[COEFF_GRPS];
uniform vec4 backLightB[COEFF_GRPS];

// setup camera uniforms
uniform vec3 camPos;

// setup oily
uniform float oily;

// IN
in GSOut {
	flat vec4 pos;
	flat vec4 origPos;
	smooth vec3 norm;
	smooth vec3 tan;
	smooth vec2 texCoord;
	flat float wrinkle;

	smooth vec4 coeffsR[NUM_COEFF_GRPS];
	smooth vec4 coeffsG[NUM_COEFF_GRPS];
	smooth vec4 coeffsB[NUM_COEFF_GRPS];
} fsIn;

vec4 sampleEnvironment(vec3 dir)
{
	float x = dir.x, y = dir.y, z = dir.z;
	float sqDir = sqrt(x * x + y * y);

	float r;

	if (sqDir == 0.f)
		r = 0.f;
	else
		r = 1.f / M_PI / 2.f * acos(z) / sqDir;

	vec2 uv = {
		.5f + x * r,
		.5f + y * r
	};

	return texture(environment, uv);
}

vec4 evalLDPRT6(vec3 normal)
{
	vec4 color;
	vec4 samples[COEFF_GRPS];

	// get all samples for coeffs
	for (int i = 0; i < COEFF_GRPS; i++)
		samples[i] = texture(harmonics[i], normal);

	// albedo
	vec4 albedoColor;
	float thickness;

	if (useAlbedo)
	{
		albedoColor = texture(albedoTex, fsIn.texCoord);

		thickness = 1.f - albedoColor.a;
	} else
	{
		albedoColor = vec4(0.f, 0.f, 0.f, 1.f);

		thickness = 0.f;
	}
	
	// 1, 3, 5, 7, 9, 11
	// compute dot product for rgb channels (registers are packed GO GO!!!)
	color.r = dot(samples[0] * fsIn.coeffsR[0].xyyy, lightR[0] + backLightR[0] * albedoColor.r * thickness);
	color.g = dot(samples[0] * fsIn.coeffsG[0].xyyy, lightG[0] + backLightG[0] * albedoColor.g * thickness);
	color.b = dot(samples[0] * fsIn.coeffsB[0].xyyy, lightB[0] + backLightB[0] * albedoColor.b * thickness);

	color.r += dot(samples[1] * fsIn.coeffsR[0].zzzz, lightR[1] + backLightR[1] * albedoColor.r * thickness);
	color.g += dot(samples[1] * fsIn.coeffsG[0].zzzz, lightG[1] + backLightG[1] * albedoColor.g * thickness);
	color.b += dot(samples[1] * fsIn.coeffsB[0].zzzz, lightB[1] + backLightB[1] * albedoColor.b * thickness);
	
	color.r += dot(samples[2] * fsIn.coeffsR[0].zwww, lightR[2] + backLightR[2] * albedoColor.r * thickness);
	color.g += dot(samples[2] * fsIn.coeffsG[0].zwww, lightG[2] + backLightG[2] * albedoColor.g * thickness);
	color.b += dot(samples[2] * fsIn.coeffsB[0].zwww, lightB[2] + backLightB[2] * albedoColor.b * thickness);

	color.r += dot(samples[3] * fsIn.coeffsR[0].wwww, lightR[3] + backLightR[3] * albedoColor.r * thickness);
	color.g += dot(samples[3] * fsIn.coeffsG[0].wwww, lightG[3] + backLightG[3] * albedoColor.g * thickness);
	color.b += dot(samples[3] * fsIn.coeffsB[0].wwww, lightB[3] + backLightB[3] * albedoColor.b * thickness);

	color.r += dot(samples[4] * fsIn.coeffsR[1].xxxx, lightR[4] + backLightR[4] * albedoColor.r * thickness);
	color.g += dot(samples[4] * fsIn.coeffsG[1].xxxx, lightG[4] + backLightG[4] * albedoColor.g * thickness);
	color.b += dot(samples[4] * fsIn.coeffsB[1].xxxx, lightB[4] + backLightB[4] * albedoColor.b * thickness);

	color.r += dot(samples[5] * fsIn.coeffsR[1].xxxx, lightR[5] + backLightR[5] * albedoColor.r * thickness);
	color.g += dot(samples[5] * fsIn.coeffsG[1].xxxx, lightG[5] + backLightG[5] * albedoColor.g * thickness);
	color.b += dot(samples[5] * fsIn.coeffsB[1].xxxx, lightB[5] + backLightB[5] * albedoColor.b * thickness);

	color.r += dot(samples[6] * fsIn.coeffsR[1].xyyy, lightR[6] + backLightR[6] * albedoColor.r * thickness);
	color.g += dot(samples[6] * fsIn.coeffsG[1].xyyy, lightG[6] + backLightG[6] * albedoColor.g * thickness);
	color.b += dot(samples[6] * fsIn.coeffsB[1].xyyy, lightB[6] + backLightB[6] * albedoColor.b * thickness);

	color.r += dot(samples[7] * fsIn.coeffsR[1].yyyy, lightR[7] + backLightR[7] * albedoColor.r * thickness);
	color.g += dot(samples[7] * fsIn.coeffsG[1].yyyy, lightG[7] + backLightG[7] * albedoColor.g * thickness);
	color.b += dot(samples[7] * fsIn.coeffsB[1].yyyy, lightB[7] + backLightB[7] * albedoColor.b * thickness);

	color.r += dot(samples[8] * fsIn.coeffsR[1].yyyy, lightR[8] + backLightR[8] * albedoColor.r * thickness);
	color.g += dot(samples[8] * fsIn.coeffsG[1].yyyy, lightG[8] + backLightG[8] * albedoColor.g * thickness);
	color.b += dot(samples[8] * fsIn.coeffsB[1].yyyy, lightB[8] + backLightB[8] * albedoColor.b * thickness);
	
	// set alpha
	color.w = 1;

	return color;
}

void main()
{
	vec4 diffuse;

	// diffuse
	if (useDiffuseTex)
		diffuse = texture(diffuseTex, fsIn.texCoord);
	else
		diffuse = diffuseCol;

	vec3 normal;

	// normal
	if (useBumpTex)
	{
		vec3 bump;

		bump = texture(bumpTex, fsIn.texCoord).xyz * 2.f
			- vec3(1.f, 1.f, 1.f);
		normalize(bump);

		vec3 binormal = normalize(
				cross(fsIn.norm, fsIn.tan)
			);
		mat3 tanMat = mat3(binormal, fsIn.tan, fsIn.norm);
		bump = tanMat * bump;

		normal = mix(fsIn.norm, bump, saturate(fsIn.wrinkle / (1.f - bumpMult) + bumpMult));
		normalize(normal);
	} else // no bump
	{
		normal = fsIn.norm;
	}
	
	// eval harmonics
	vec4 color = evalLDPRT6(normal);

	// specular
	vec4 specularColor;
	if (useSpecTex)
		specularColor = texture(specTex, fsIn.texCoord);
	else
		specularColor = vec4(1.f, 1.f, 1.f, 1.f);

	vec3 I = -normalize(lightDir);
	vec3 refDir = reflect(I, normal);

	vec3 camDir = normalize(camPos);
	float specCoeff = pow(saturate(dot(camDir, refDir)), 6);
	
	vec4 specularHighlight;
	if (useSpecHighTex)
		specularHighlight = texture(specHighTex, fsIn.texCoord);
	else
		specularHighlight = vec4(1.f, 1.f, 1.f, 1.f);
	
	vec4 specular = saturate(
		sampleEnvironment(refDir)
		* oily
		* specCoeff
		* specularColor
		* specularHighlight
		* shininess
	);

	gl_FragColor = saturate(
		diffuse * (color + specular) * lightIntensity
	);
}