#version 460

in layout(location=0) vec3 position;
in layout(location=1) vec3 vertexColor;
in layout(location=1) vec2 vertexUV;
in layout(location=2) vec3 normal;
uniform vec4 ambientLight;


uniform mat4 modelTransformMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 theColor;
out vec2 UV;
out vec3 normalWorld;
out vec3 vertexPositionWorld;

void main()
{
	vec4 v = vec4(position, 1.0);
	vec4 newPosition = modelTransformMatrix * v;
	vec4 out_position = projectionMatrix * viewMatrix * newPosition;
	gl_Position = out_position;	

	vec4 normal_temp = modelTransformMatrix * vec4(normal, 0);
	normalWorld = normal_temp.xyz;
	vertexPositionWorld = newPosition.xyz;
	theColor = vertexColor;
	UV = vertexUV;
}
