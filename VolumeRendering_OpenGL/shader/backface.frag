// for raycasting
#version 400

in vec3 fragPos;    // ��ֵ���ÿ�����ض�Ӧ�İ�Χ�еĵ��ڶ���ռ��е�λ��
layout (location = 0) out vec4 FragColor;


void main()
{
    FragColor = vec4(fragPos, 1.0);
}
