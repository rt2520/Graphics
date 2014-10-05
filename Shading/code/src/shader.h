#ifndef SHADER_INC
#   define SHADER_INC

#define STRINGIFY(A) #A

const char* toonVS = STRINGIFY(
varying vec3 normal;            \n
void main()                     \n
{                               \n
    normal = gl_Normal;         \n
    gl_Position = ftransform(); \n
}                               \n
);

const char* toonFS = STRINGIFY(
uniform vec3 lightDir;          \n
varying vec3 normal;            \n
void main()                     \n
{                               \n
    float intensity = dot(normalize(lightDir), normalize(normal)); \n
    if ( intensity > 0.95 )     \n
        gl_FragColor = gl_FrontMaterial.diffuse; \n
    else if ( intensity > 0.5 ) \n
        gl_FragColor = gl_FrontMaterial.diffuse*0.9; \n
    else if ( intensity > 0.25 ) \n
        gl_FragColor = gl_FrontMaterial.diffuse*0.7; \n
    else                        \n
        gl_FragColor = gl_FrontMaterial.diffuse*0.4; \n
}                               \n
);

const char *gouraudVS = STRINGIFY(
uniform int numLights;          \n
void main()			\n
{
	gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0);
	for (int i = 0; i < numLights; i++)
	{
	vec3 nLightDir = normalize(vec3(gl_LightSource[i].position));  \n
	vec3 normalEyeSpace = normalize(gl_NormalMatrix * gl_Normal);  \n
	float temp_nl = dot(nLightDir, normalEyeSpace);  \n
	float nl = max(0.0, temp_nl);  \n
	//dR = diffuse reflectance of the material and dL = diffuse coeff of the light
	vec4 dRdL = gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse;  \n
	//aR = ambient reflectance of the material and aL = ambient light term
	vec4 aRaL = gl_FrontMaterial.ambient * gl_LightSource[i].ambient;  \n
	//also adding the global ambient lighting term (approximation of global illumination)
	vec4 diffuse = nl * dRdL + aRaL;  \n
        if (i == 0)  \n
                diffuse += gl_LightModel.ambient * gl_FrontMaterial.ambient;  \n
	if (temp_nl > 0.0)  \n
	{  \n
		vec3 r = normalize(2 * temp_nl * normalEyeSpace - nLightDir);  \n
		vec3 vPosEyeSpace = (gl_ModelViewMatrix * gl_Vertex).xyz;  \n
		//Eye Direction normalized
		vec3 e = normalize(vec3(0.0, 0.0, 0.0) - vPosEyeSpace);  \n
		float er = max(0.0, dot(e, r));  \n
		vec4 specular = gl_FrontMaterial.specular * gl_LightSource[i].specular * pow(er, gl_FrontMaterial.shininess);  \n
		gl_FrontColor += (diffuse + specular);  \n
	}  \n
	else  \n
		gl_FrontColor += diffuse;  \n
	}
	gl_Position = ftransform();  \n
}
);

const char *gouraudFS = STRINGIFY(
void main()  \n
{	  \n
	gl_FragColor = gl_Color;  \n
}
);

const char *blinnPhongVS = STRINGIFY(
varying vec3 normal;  \n
//varying vec3 halfVector;  \n
varying vec4 pos;  \n

void main()                     \n
{
        //vec3 nLightDir = normalize(lightDir);  \n
        normal = gl_Normal;  \n
	//halfVector = normalize(gl_LightSource[0].halfVector.xyz);  \n
	//nLightDir = normalize(vec3(gl_LightSource[0].position));  \n	
	pos = gl_Vertex;  \n
        gl_Position = ftransform();  \n
}
);

const char *blinnPhongFS = STRINGIFY(
varying vec3 normal;  \n
//varying vec3 halfVector;  \n
varying vec4 pos;  \n
uniform int numLights;          \n
void main()                     \n
{
	gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	for (int i = 0; i < numLights; i++)
	{
        vec3 nLightDir = normalize(vec3(gl_LightSource[0].position));  \n
        vec3 normalEyeSpace = normalize(gl_NormalMatrix * normal);  \n
        float temp_nl = dot(nLightDir, normalEyeSpace);  \n
        float nl = max(0.0, temp_nl);  \n
        //dR = diffuse reflectance of the material and dL = diffuse coeff of the light
        vec4 dRdL = gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse;  \n
        //aR = ambient reflectance of the material and aL = ambient light term
        vec4 aRaL = gl_FrontMaterial.ambient * gl_LightSource[i].ambient;  \n
        //also adding the global ambient lighting term (approximation of global illumination)
        vec4 diffuse = nl * dRdL + aRaL;  \n
	if (i == 0)  \n
		diffuse += gl_LightModel.ambient * gl_FrontMaterial.ambient;  \n
        if (temp_nl > 0.0)  \n
        {  \n
		vec3 vPosEyeSpace = (gl_ModelViewMatrix * pos).xyz;  \n
                //Eye Direction normalized
                vec3 e = normalize(vec3(0.0, 0.0, 0.0) - vPosEyeSpace);  \n
		vec3 halfVector = e + nLightDir;
                float hn = max(0.0, dot(normalEyeSpace, normalize(halfVector)));  \n
                vec4 specular = gl_FrontMaterial.specular * gl_LightSource[i].specular * pow(hn, 0.35 * gl_FrontMaterial.shininess);  \n
                gl_FragColor += (diffuse + specular);  \n
        }  \n
        else  \n
                gl_FragColor += diffuse;  \n
	}
}
);


/*const char *wireFrameVS = STRINGIFY(
varying vec3 bcCoord;           \n
attribute vec3 vertexBCCoord;
void main()                     \n
{
        //vec3 nLightDir = normalize(lightDir);  \n
	bcCoord = vertexBCCoord;  \n
        vec3 nLightDir = normalize(vec3(gl_LightSource[0].position));  \n
        vec3 normalEyeSpace = normalize(gl_NormalMatrix * gl_Normal);  \n
        float temp_nl = dot(nLightDir, normalEyeSpace);  \n
        float nl = max(0.0, temp_nl);  \n
        //dR = diffuse reflectance of the material and dL = diffuse coeff of the light
        float dRdL = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;  \n
        //aR = ambient reflectance of the material and aL = ambient light term
        float aRaL = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;  \n
        //also adding the global ambient lighting term (approximation of global illumination)
        float diffuse = min(nl * dRdL + aRaL + gl_LightModel.ambient * gl_FrontMaterial.ambient, 1.0);  \n
        float hn = max(0.0, dot(normalEyeSpace, gl_LightSource[0].halfVector.xyz));
        float specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(hn, gl_FrontMaterial.shininess);  \n
        gl_FrontColor = min(diffuse + specular, 1.0);  \n
        gl_Position = ftransform();  \n
}
);

const char *wireFrameFS = STRINGIFY(
varying vec3 bcCoord;           \n
void main()  \n
{	  \n
	if (bcCoord.x <= 0.02f || bcCoord.y <= 0.02f || bcCoord.z <= 0.02f)  \n
		gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);  \n
	else  \n
		gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);  \n//gl_Color;  \n
}
);*/

const char *checkerVS = STRINGIFY(
uniform int numLights;          \n
varying vec3 texCoord;  \n
void main()                     \n
{
	texCoord = gl_MultiTexCoord0;  \n
        gl_FrontColor = vec4(0.0, 0.0, 0.0, 0.0);
        for (int i = 0; i < numLights; i++)
        {
        vec3 nLightDir = normalize(vec3(gl_LightSource[i].position));  \n
        vec3 normalEyeSpace = normalize(gl_NormalMatrix * gl_Normal);  \n
        float temp_nl = dot(nLightDir, normalEyeSpace);  \n
        float nl = max(0.0, temp_nl);  \n
        //dR = diffuse reflectance of the material and dL = diffuse coeff of the light
        vec4 dRdL = gl_FrontMaterial.diffuse * gl_LightSource[i].diffuse;  \n
        //aR = ambient reflectance of the material and aL = ambient light term
        vec4 aRaL = gl_FrontMaterial.ambient * gl_LightSource[i].ambient;  \n
        //also adding the global ambient lighting term (approximation of global illumination)
        vec4 diffuse = nl * dRdL + aRaL + gl_LightModel.ambient * gl_FrontMaterial.ambient;  \n
        if (temp_nl > 0.0)  \n
        {  \n
                vec3 r = normalize(2 * temp_nl * normalEyeSpace - nLightDir);  \n
                vec3 vPosEyeSpace = (gl_ModelViewMatrix * gl_Vertex).xyz;  \n
                //Eye Direction normalized
                vec3 e = normalize(vec3(0.0, 0.0, 0.0) - vPosEyeSpace);  \n
                float er = max(0.0, dot(e, r));  \n
                vec4 specular = gl_FrontMaterial.specular * gl_LightSource[i].specular * pow(er, gl_FrontMaterial.shininess);  \n
                gl_FrontColor += (diffuse + specular);  \n
        }  \n
        else  \n
                gl_FrontColor += diffuse;  \n
        }
        gl_Position = ftransform();  \n
}
);

const char *checkerFS = STRINGIFY(
uniform sampler2D tex;  \n
varying vec3 texCoord;  \n
void main()                     \n
{  \n
	gl_FragColor = gl_Color;  \n
	if (texCoord.s < 0.0)  \n
		texCoord.s = 0.0;  \n
	else if (texCoord.s >= 2.0)  \n
                texCoord.s = 1.0;  \n
	else  \n
		texCoord.s *= 4.0;  \n
	if (texCoord.t < 0.0)  \n
                texCoord.t = 0.0;  \n
	else if (texCoord.t >= 2.0)  \n
                texCoord.t = 1.0;  \n
	else  \n
		texCoord.t *= 4.0;  \n
	if (gl_FragColor.r > 0.0 || gl_FragColor.g > 0.0 || gl_FragColor.b > 0.0)  \n
	{
		if (mod(floor (texCoord.s) + floor(texCoord.t), 2.0) == 0.0)  \n
			gl_FragColor += vec4(0.2, 0.2, 0.2, 0.0);  \n
		else  \n
			gl_FragColor -= vec4(0.2, 0.2, 0.2, 0.0);  \n
	}
	gl_FragColor.a = 1.0;  \n
}
);

#endif
