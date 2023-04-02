#version 400
layout (location = 0) in vec3 VerPos;

out vec3 EntryPoint;    // 进入点（对象空间）
out vec4 ExitPointCoord;// 裁剪空间的顶点坐标

uniform mat4 MVP;

void main()
{
    EntryPoint = VerPos;                    // 这次是渲染的正面，因此拿到的是正面的顶点数据
    gl_Position = MVP * vec4(VerPos,1.0);   // gl_Position在VS之后会进行透视除法
}
