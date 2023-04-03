// for raycasting
#version 400

in vec3 fragPos;    // 插值后的每个像素对应的包围盒的点在对象空间中的位置
layout (location = 0) out vec4 gExisPoints;


void main()
{
    gExisPoints = vec4(fragPos, 1.0);
}
