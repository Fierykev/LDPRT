#define NUM_COEFF_GRPS 2

#define saturate(in) (clamp(in, 0, 1))

// LAYOUT
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// IN
in VSOut {
	flat vec4 pos;
	flat vec4 origPos;
	smooth vec3 norm;
	smooth vec3 tan;
	smooth vec2 texCoord;

	smooth vec4 coeffsR[NUM_COEFF_GRPS];
	smooth vec4 coeffsG[NUM_COEFF_GRPS];
	smooth vec4 coeffsB[NUM_COEFF_GRPS];
} gsIn[];

// OUT

out GSOut {
	flat vec4 pos;
	flat vec4 origPos;
	smooth vec3 norm;
	smooth vec3 tan;
	smooth vec2 texCoord;
	flat float wrinkle;

	smooth vec4 coeffsR[NUM_COEFF_GRPS];
	smooth vec4 coeffsG[NUM_COEFF_GRPS];
	smooth vec4 coeffsB[NUM_COEFF_GRPS];
} gsOut;

void main()
{
		// get triangle area for wrinkles
		vec3 origA = gsIn[1].pos.xyz - gsIn[0].pos.xyz;
		vec3 origB = gsIn[2].pos.xyz - gsIn[0].pos.xyz;
		float origArea = length(cross(origA, origB)) / 2.f;

		vec3 morphA = gsIn[1].pos.xyz - gsIn[0].pos.xyz;
		vec3 morphB = gsIn[2].pos.xyz - gsIn[0].pos.xyz;
		float morphArea = length(cross(morphA, morphB)) / 2.f;

		for (int i = 0; i < 3; i++)
		{
			gsOut.pos = gsIn[i].pos;
			gl_Position = gsOut.pos;
			gsOut.origPos = gsIn[i].origPos;
			gsOut.norm = gsIn[i].norm;
			gsOut.tan = gsIn[i].tan;
			gsOut.texCoord = gsIn[i].texCoord;

			// create wrinkle
			gsOut.wrinkle = (origArea - morphArea) / origArea;

			if (gsOut.wrinkle < 0.f)
				gsOut.wrinkle *= .005f;

			gsOut.wrinkle = saturate(.3f + gsOut.wrinkle);

			// copy coeffs
			for (int j = 0; j < NUM_COEFF_GRPS; j++)
			{
				gsOut.coeffsR[j] = gsIn[i].coeffsR[j];
				gsOut.coeffsG[j] = gsIn[i].coeffsG[j];
				gsOut.coeffsB[j] = gsIn[i].coeffsB[j];
			}

			EmitVertex();
		}

		EndPrimitive();
}