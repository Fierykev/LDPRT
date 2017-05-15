#define MORPH_CHAN 3

// LAYOUT
layout(triangles) in;
layout(triangle_strip, max_vertices = 3 * MORPH_CHAN) out;

// IN
in VSOut {
	flat vec4 pos;
	smooth vec2 texCoord;
} gsIn[];

// OUT

out GSOut {
	smooth vec3 texCoord;
	flat vec3 deltaMax;
} gsOut;

void main()
{
	for (int z = 0; z < MORPH_CHAN; z++)
	{
		for (int i = 0; i < 3; i++)
		{
			gl_Layer = z;
			gl_Position = gsIn[i].pos;
			gsOut.texCoord = vec3(gsIn[i].texCoord, gl_Layer);
			gsOut.deltaMax = vec3(1, 1, 1); // TMP

			EmitVertex();
		}

		EndPrimitive();
	}
}