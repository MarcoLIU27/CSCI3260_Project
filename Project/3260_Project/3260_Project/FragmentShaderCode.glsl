#version 460

out vec4 Color;

in vec3 theColor;
in vec2 UV;
in vec3 normalWorld;
in vec3 vertexPositionWorld;

uniform sampler2D ourTexture;

uniform vec4 ambientLight;
uniform vec3 lightPositionWorld;
uniform vec3 eyePositionWorld;

struct DirLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {    
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;  

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

#define NR_POINT_LIGHTS 1
uniform PointLight pointLight;
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 vertexPositionWorld, vec3 eyeVectorWorld )
{
    vec3 lightVectorWorld  = normalize(light.position - vertexPositionWorld);
    // diffuse shading
    float diff = max(dot(normal, lightVectorWorld ), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightVectorWorld , normal);
    float spec = pow(max(dot(eyeVectorWorld , reflectDir), 0.0), 30.0f);
    // combine results
    vec3 ambient  = light.ambient  * texture(ourTexture, UV).rgb;
    vec3 diffuse  = light.diffuse * diff * texture(ourTexture, UV).rgb;
    vec3 specular = light.specular * spec * texture(ourTexture, UV).rgb;
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 vertexPositionWorld, vec3 eyeVectorWorld )
{
    vec3 lightVectorWorld  = normalize(light.position - vertexPositionWorld);
    // diffuse shading
    float diff = max(dot(normal, lightVectorWorld ), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightVectorWorld , normal);
    float spec = pow(max(dot(eyeVectorWorld , reflectDir), 0.0), 30.0f);
    // attenuation
    float distance    = length(light.position - vertexPositionWorld);
    float attenuation = 1.0 / (1.0f + 0.09f * distance + 
  			     0.032f * (distance * distance));    
    // combine results
    vec3 ambient  = light.ambient  * vec3(texture(ourTexture, UV));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(ourTexture, UV));
    vec3 specular = light.specular * spec * vec3(texture(ourTexture, UV));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

void main()
{
    vec3 norm = normalize(normalWorld);
    vec3 eyeVectorWorld  = normalize(eyePositionWorld  - vertexPositionWorld);

    vec3 result = vec3(texture(ourTexture, UV)) * 0.2f;
    // phase 1: Directional lighting
    result += CalcDirLight(dirLight, norm, vertexPositionWorld, eyeVectorWorld);
    // phase 2: Point light
    result += CalcPointLight(pointLight, norm, vertexPositionWorld, eyeVectorWorld);      
    
    Color = vec4(result, 1.0);
}
