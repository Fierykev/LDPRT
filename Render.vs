#define NUM_BANDS 6
#define NUM_COEFF_GRPS 2

// UNIFORM
uniform uint size;
uniform sampler2DArray objTex;
uniform sampler2DArray origTex;
uniform mat4 worldViewProjection;

// IN
in uint vertID;
in vec2 texCoord;
in vec4 coeffsR[NUM_COEFF_GRPS];
in vec4 coeffsG[NUM_COEFF_GRPS];
in vec4 coeffsB[NUM_COEFF_GRPS];

// OUT
out VSOut {
	flat vec4 pos;
	flat vec4 origPos;
	smooth vec3 norm;
	smooth vec3 tan;
	smooth vec2 texCoord;

	smooth vec4 coeffsR[NUM_COEFF_GRPS];
	smooth vec4 coeffsG[NUM_COEFF_GRPS];
	smooth vec4 coeffsB[NUM_COEFF_GRPS];
} vsOut;

void main()
{
	uint x = vertID % size;
	uint y = vertID / size;

	vec2 sample = vec2(x, y);

	// offset for float error
	sample += vec2(.5, .5);
	sample /= size;

	// get position, normal, tangent, and texcoord
	vec3 pos = texture(objTex, vec3(sample, 0.f)).xyz;

	vec3 origPos = texture(origTex, vec3(sample, 0.f)).xyz;

	vec3 norm = texture(objTex, vec3(sample, 1.f)).xyz;

	vec3 tan = texture(objTex, vec3(sample, 2.f)).xyz;

	// calc position
	vsOut.pos = vec4(pos, 1.f); //worldViewProjection * vec4(pos, 1.f);
	gl_Position = vsOut.pos;

	vsOut.origPos = worldViewProjection * vec4(origPos, 1.f);
	vsOut.norm = normalize(norm);
	vsOut.tan = normalize(tan);

	vsOut.texCoord = texCoord;

	// copy coeffs
	for (int i = 0; i < NUM_COEFF_GRPS; i++)
	{
		vsOut.coeffsR[i] = coeffsR[i];
		vsOut.coeffsG[i] = coeffsG[i];
		vsOut.coeffsB[i] = coeffsB[i];
	}
}