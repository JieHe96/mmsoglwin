#version 440 core
//compatibility

out vec4 frag_colour;

in vec2 vTexCoord;

uniform sampler2D dtex[25];
//uniform float texcoordscalar;
uniform float xtexcoordmultiplier;
uniform float ytexcoordmultiplier;
uniform int numvideos;

#define OUT_COLOR(x)	\
if(index == x) return texture(dtex[x], texcoords);

vec4 func_cal_color25(int index, vec2 texcoords)
{
	OUT_COLOR(0)
	OUT_COLOR(1)
	OUT_COLOR(2)
	OUT_COLOR(3)
	OUT_COLOR(4)
	OUT_COLOR(5)
	OUT_COLOR(6)
	OUT_COLOR(7)
	OUT_COLOR(8)
	OUT_COLOR(9)
	OUT_COLOR(10)
	OUT_COLOR(11)
	OUT_COLOR(12)
	OUT_COLOR(13)
	OUT_COLOR(14)
	OUT_COLOR(15)
	OUT_COLOR(16)
	OUT_COLOR(17)
	OUT_COLOR(18)
	OUT_COLOR(19)
	OUT_COLOR(20)
	OUT_COLOR(21)
	OUT_COLOR(22)
	OUT_COLOR(23)
	OUT_COLOR(24)
	return vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 func_cal_tex25()
{
	vec2 finaltexcoords;

	float xArr[4] = {0.0f, 0.4f, 0.0f, 0.4f};
	float yArr[4] = {0.f, 0.0f, 0.4f, 0.4f};
	
	int rIndex[25] = {
		0, 0, 1, 1,12,
		0, 0, 1, 1,11,
		2, 2, 3, 3,10,
		2, 2, 3, 3, 9,
		4, 5, 6, 7, 8
	};

	float modcoordsx = floor(vTexCoord.x*xtexcoordmultiplier);
	float modcoordsy = floor(vTexCoord.y*ytexcoordmultiplier);

	int testindex = int(mod(modcoordsx + xtexcoordmultiplier*modcoordsy, xtexcoordmultiplier*ytexcoordmultiplier));

	int realIndex;
	realIndex = (numvideos == 13 ? rIndex[testindex] : testindex);

	float multi=xtexcoordmultiplier/2.0;

	if (numvideos == 13 && realIndex <= 3)
	{
		finaltexcoords = vec2((vTexCoord.x - xArr[realIndex])*multi,
							(vTexCoord.y - yArr[realIndex])*multi);
	}
	else	
		finaltexcoords = vec2((vTexCoord.x - (modcoordsx/xtexcoordmultiplier))*xtexcoordmultiplier,
						  (vTexCoord.y - (modcoordsy/ytexcoordmultiplier))*ytexcoordmultiplier);

	return vec3(finaltexcoords,float(realIndex));
}

void main() 
{
	vec3 retval = func_cal_tex25();
	vec2 finaltexcoords = retval.xy;
	int realIndex = int(retval.z);
	
	#if 0
	
	int i=0;
	for(int j=0; j<numvideos;j++)
	{
		i = (realIndex == j ? j : i);
	}

	frag_colour = texture(dtex[i], finaltexcoords);
	
	#else

	frag_colour = func_cal_color25(realIndex, finaltexcoords);

	#endif
}

