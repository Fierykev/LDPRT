// UNIFORM
uniform float blend;
uniform sampler2DArray sourceTex;

// IN
in GSOut {
	smooth vec3 texCoord;
	flat vec3 deltaMax;
} fsIn;

void main()
{
	vec3 sample = textureLod(sourceTex, fsIn.texCoord, 0).xyz;
	//sample *= fsIn.deltaMax;

	gl_FragColor = vec4(sample, blend);
}