in vec3 vertexPosition;
in vec3 vertexColor;
in vec2 vertexTexcoord;
in vec2 vertexHeightmap;

out vec3 fragmentPosition;
out vec3 fragmentColor;
out vec2 fragmentTexcoord;
out float fragmentNoiseValue;
out float fragmentCurrentObject;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform float vertexCurrentObject;
uniform float vertexFPS;

uniform sampler2D tex;

void main()
{
	//outs for fragment shader
	fragmentColor = vertexColor;
	fragmentTexcoord = vertexTexcoord;
	fragmentNoiseValue = texture(tex, vertexHeightmap).r;
	fragmentCurrentObject = vertexCurrentObject;
	fragmentPosition = vertexPosition;

	//position matrix
	mat4 modelView = view * model;
	mat4 normalMatrix = transpose(inverse(modelView));

	if (fragmentNoiseValue <= 0.27f)
	{
		gl_Position = proj * modelView * vec4(vertexPosition.x, vertexPosition.y + (0.27f * 20.0f), vertexPosition.z, 1.0);
	}
	else
	{
		gl_Position = proj * modelView * vec4(vertexPosition.x, vertexPosition.y + (fragmentNoiseValue * 20.0f), vertexPosition.z, 1.0);
	}
}
