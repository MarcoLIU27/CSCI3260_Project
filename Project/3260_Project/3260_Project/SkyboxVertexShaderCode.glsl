#version 460
in layout(location=0) vec3 position;

out vec3 TexCoords;  // 注意是三维纹理坐标

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    TexCoords = position;  //使用顶点位置作为方向向量来映射纹理
    vec4 pos = projectionMatrix * viewMatrix * vec4(position, 1.0);
    gl_Position = pos.xyww;
}  