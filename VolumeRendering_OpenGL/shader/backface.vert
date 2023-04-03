#version 400
layout(location = 0) in vec3 VerPos;

out vec3 fragPos;   // ����ռ��еĶ���λ��

uniform mat4 MVP;


void main()
{
    fragPos = VerPos;
    gl_Position = MVP * vec4(VerPos, 1.0);  // MVP�任ֻ��Ϊ�˵õ��ڲü��ռ��м��������λ�ã�����Χ��ͶӰ��ƽ���ϵķ�Χ
}
