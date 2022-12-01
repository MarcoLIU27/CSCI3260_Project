#version 460
in layout(location=0) vec3 position;

out vec3 TexCoords;  // ע������ά��������

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    TexCoords = position;  //ʹ�ö���λ����Ϊ����������ӳ������
    vec4 pos = projectionMatrix * viewMatrix * vec4(position, 1.0);
    gl_Position = pos.xyww;
}  