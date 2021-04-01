#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fPosLightSpace;
out vec4 fColor;

//lighting
uniform	mat3 normalMatrix;
uniform vec3 lightDir;
uniform vec3 lightColor;

uniform vec3 pointLightSource;
uniform vec3 pointLightColor;

uniform vec3 wolfLightColor;
uniform vec3 wolfLightSource;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 wolfAmbient;
vec3 wolfDiffuse;
vec3 wolfSpecular;

vec3 pointAmbient;
vec3 pointDiffuse;
vec3 pointSpecular;

vec3 ambient;
vec3 diffuse;
vec3 specular;

float ambientStrength = 0.2f;
float specularStrength = 0.5f;
float shininess = 32.0f;

float constant = 2.0f;
float linear = 0.4f;
float quadratic = 0.3f; 

float computeShadow(){

	vec3 normalizedCoords = fPosLightSpace.xyz / fPosLightSpace.w;

	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float bias = 0.005f;
	float shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	if(normalizedCoords.z > 1.0f){
		return 0.0f;
	}

	return shadow ;
}

void computePointLight(){

	vec3 cameraPosEye = vec3(0.0f);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 normalEye = normalize(normalMatrix * fNormal);	

	vec3 lightDir = normalize(pointLightSource - fPosEye.xyz);
	vec3 reflection = reflect(-lightDir, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	float distance = length(pointLightSource - fPosEye.xyz);

	float att = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
	pointAmbient = att * ambientStrength * pointLightColor;
	pointDiffuse = att * max(dot(normalEye, lightDir), 0.0f) * pointLightColor;
	pointSpecular = att * specularStrength * specCoeff * pointLightColor;

}


void computeWolfLight(){

	vec3 cameraPosEye = vec3(0.0f);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 normalEye = normalize(normalMatrix * fNormal);	

	vec3 lightDir = normalize(wolfLightSource - fPosEye.xyz);
	vec3 reflection = reflect(-lightDir, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	float distance = length(wolfLightSource - fPosEye.xyz);

	float att = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
	wolfAmbient = att * ambientStrength * wolfLightColor;
	wolfDiffuse = att * max(dot(normalEye, lightDir), 0.0f) * wolfLightColor;
	wolfSpecular = att * specularStrength * specCoeff * wolfLightColor;

}


void computeLightComponents(){
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);

	vec3 reflection = reflect(-lightDirN, normalEye);
	//vec3 halfVector = normalize(lightDirN + viewDirN);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	//float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	
	ambient = ambientStrength * lightColor;
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	specular = specularStrength * specCoeff * lightColor;
}

float computeFog(){
	float fogDensity = 0.1f; 
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity / 2, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

void main(){
	computeLightComponents();
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	computePointLight();
	pointAmbient *= 3 * vec3(texture(diffuseTexture, fTexCoords));
	pointDiffuse *= 3 * vec3(texture(diffuseTexture, fTexCoords));
	pointSpecular *= 3 * vec3(texture(specularTexture, fTexCoords));
	vec3 pointColor = min((pointAmbient + (1.0f) * pointDiffuse) + (1.0f) * pointSpecular, 1.0f);

	computeWolfLight();
	wolfAmbient *= 3 * vec3(texture(diffuseTexture, fTexCoords));
	wolfDiffuse *= 3 * vec3(texture(diffuseTexture, fTexCoords));
	wolfSpecular *= 3 * vec3(texture(specularTexture, fTexCoords));
	vec3 wolfColor = min((wolfAmbient + (1.0f) * wolfDiffuse) + (1.0f) * wolfSpecular, 1.0f);

	float shadow = computeShadow();
	vec3 color = min((ambient + (1.0f - shadow) * diffuse) + (1.0f - shadow) * specular, 1.0f);
	
	float fogFactor = computeFog();
	vec3 fogColor = vec3(0.5f, 0.5f, 0.5f);
	fColor = vec4(fogColor * (1 - fogFactor) + color * fogFactor + pointColor + wolfColor, 1.0f);
	//fColor = vec4(color, 1.0f);
}