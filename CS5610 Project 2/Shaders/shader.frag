#version 330 core

// shader inspored from: https://community.khronos.org/t/adding-normals-into-fragment-shader/73506

in vec3 FragPos;
in vec3 WorldPos_FS_in;
in vec3 Normal_FS_in;
in vec4 Color_FS_in;
in vec2 TexCoord_FS_in;

layout(location = 0) out vec4 color;

// define a vector which translates texture coordinates to vertex space
vec2 normCoord = vec2(TexCoord_FS_in.x, TexCoord_FS_in.y);

// uniform vec3 objColor;
// uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;
uniform float tColor;
uniform float shading;

vec3 objColor;
float alpha = Color_FS_in.a;
vec3 specColor = vec3(1.0, 0.8, 0.1);
vec3 lightColor = vec3(1.0, 1.0, 1.0);
float lightPower = 5.0;
float shininess = 200.0;
float shadowEffect = 0.5;



void main()
{
	if (tColor > 0.5) {
		objColor = Color_FS_in.rgb;
		// objColor = Normal_FS_in.rgb;
	}
	else {
		objColor = vec3(1);
	}

	if (objColor.b == 1.0)
	{
		shininess = 1;
	}

	if (shading > 0.5) {
	vec3 lightDir = lightPos - FragPos;
	float distance = length(lightDir);
	distance = distance * distance;
	lightDir = normalize(lightDir);

	// ambient
	float ambientStrength = 0.5f;
	vec3 ambient = ambientStrength * lightColor;

	// Diffuse 
	vec3 norm = Normal_FS_in;
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// Specular (blinn phong shading)
	float cosAngIncidence = dot(norm, lightDir);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);
	vec3 viewDir = normalize(camPos);
	vec3 halfDir = normalize(lightDir + viewDir);
	float specAngle = max(dot(norm, halfDir), 0.0);
	specAngle = clamp(specAngle, 0, 1);
	specAngle = cosAngIncidence != 0.0 ? specAngle : 0.0;
	vec3 specular = pow(specAngle, shininess) * specColor;
	
	// calculate color of obj without shadows
	vec3 result = (ambient + diffuse) * objColor  + specular;
	color = vec4(result, alpha);
	}
	else
	{
		color = vec4(objColor, alpha);
	}

	// debugging
	//color = vec4(WorldPos_FS_in, alpha);
	// color = vec4(Normal_FS_in, alpha);
	// color = vec4(TexCoord_FS_in, 0, alpha);
}