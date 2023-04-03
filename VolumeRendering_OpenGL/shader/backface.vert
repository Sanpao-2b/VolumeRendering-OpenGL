#version 400
layout(location = 0) in vec3 VerPos;

out vec3 fragPos;   // 对象空间中的顶点位置

uniform mat4 MVP;


void main()
{
    fragPos = VerPos;
    gl_Position = MVP * vec4(VerPos, 1.0);  // MVP变换只是为了得到在裁剪空间中几个顶点的位置，即包围盒投影到平面上的范围
}
