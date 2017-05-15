// IN
in vec3 pos;
in vec2 texCoord;

// OUT
out VSOut {
	flat vec4 pos;
	smooth vec2 texCoord;
} vsOut;

void main()
{
	vsOut.pos = vec4(pos.xy, 1.f, 1.f);
	gl_Position = vsOut.pos;
	vsOut.texCoord = texCoord;
}