#version 400
layout (location = 0) in vec3 VerPos;

out vec3 EntryPoint;    // ����㣨����ռ䣩
out vec4 ExitPointCoord;// �ü��ռ�Ķ�������

uniform mat4 MVP;

void main()
{
    EntryPoint = VerPos;                    // �������Ⱦ�����棬����õ���������Ķ�������
    gl_Position = MVP * vec4(VerPos,1.0);   // gl_Position��VS֮������͸�ӳ���
}
