in vec3 fragmentPosition;
in vec3 fragmentColor;
in vec2 fragmentTexcoord;
in float fragmentNoiseValue;
in float fragmentCurrentObject;

out vec4 outColor;

uniform sampler2D tex;
uniform sampler2D texWater;
uniform sampler2D texSand;
uniform sampler2D texGrass;
uniform sampler2D texMountain;
uniform sampler2D texSnow;
uniform samplerCube texSkybox;

float threshold = 0.05f;
float waterHeight = 0.25f;
float sandHeight = 0.33f;
float grassHeight = 0.40f;
float mountainHeight = 0.75f;
float snowHeight = 1.0f;

void main()
{
    //Terrain
    if (fragmentCurrentObject < 0.5)
    {
        if (fragmentNoiseValue < waterHeight - threshold)
        {
            //water
            outColor = texture(texWater, fragmentTexcoord);
        }
        else if (fragmentNoiseValue < waterHeight)
        {
            //sand and water
            float mixFactor = 1.0f - ((waterHeight - fragmentNoiseValue) / threshold);
            vec4 tex1 = texture(texWater, fragmentTexcoord);
            vec4 tex2 = texture(texSand, fragmentTexcoord);
            outColor = mix(tex1, tex2, mixFactor);
        }
        else if (fragmentNoiseValue < sandHeight - threshold)
        {
            //sand
            outColor = texture(texSand, fragmentTexcoord);
        }
        else if (fragmentNoiseValue < sandHeight)
        {
            //grass and sand
            float mixFactor = 1.0f - ((sandHeight - fragmentNoiseValue) / threshold);
            vec4 tex1 = texture(texSand, fragmentTexcoord);
            vec4 tex2 = texture(texGrass, fragmentTexcoord);
            outColor = mix(tex1, tex2, mixFactor);
        }
        else if (fragmentNoiseValue < grassHeight - threshold)
        {
            //grass
            outColor = texture(texGrass, fragmentTexcoord);
        }
        else if (fragmentNoiseValue < grassHeight)
        {
            //grass and mountain
            float mixFactor = 1.0f - ((grassHeight - fragmentNoiseValue) / threshold);
            vec4 tex1 = texture(texGrass, fragmentTexcoord);
            vec4 tex2 = texture(texMountain, fragmentTexcoord);
            outColor = mix(tex1, tex2, mixFactor);
        }
        else if (fragmentNoiseValue < mountainHeight - threshold)
        {
            //mountain
            outColor = texture(texMountain, fragmentTexcoord);
        }
        else if (fragmentNoiseValue < mountainHeight)
        {
            //mountain and snow
            float mixFactor = 1.0f - ((mountainHeight - fragmentNoiseValue) / threshold);
            vec4 tex1 = texture(texMountain, fragmentTexcoord);
            vec4 tex2 = texture(texSnow, fragmentTexcoord);
            outColor = mix(tex1, tex2, mixFactor);
        }
        else if (fragmentNoiseValue < snowHeight)
        {
            //snow
            outColor = texture(texSnow, fragmentTexcoord);
        }
    }
    else
    {
        outColor = texture(texSkybox, fragmentPosition);
    }

}
