#version 400

in vec3 EntryPoint;     // ����㣨����ռ䣩
in vec4 ExitPointCoord; // ���ڵ㣨�ü��ռ䣩������������UV�����

uniform sampler2D u_ExitPoints;
uniform sampler3D u_VolumeTex;
uniform sampler1D u_TransferFunc;  
uniform float     u_StepSize; // �԰ٷֱȵ���ʽ
uniform vec2      u_Resolution;

layout (location = 0) out vec4 FragColor;

void main()
{
    vec3 exitPoint = texture(u_ExitPoints, gl_FragCoord.st/u_Resolution).xyz;
    
    if (EntryPoint == exitPoint)
    	discard;

    vec3 dir = exitPoint - EntryPoint;  
    float len = length(dir);
    vec3 deltaDir = normalize(dir) * u_StepSize;
    float deltaDirLen = length(deltaDir);
    
    vec3 voxelCoord = EntryPoint;   // ���ڲ���3D��������������ʼֵ

    vec4 bgColor = vec4(1.0, 1.0, 1.0, 0.0); // ����ɫ
    bgColor = vec4(1., 1., 1., 1.); // ����ɫ
    vec4 colorAcum = vec4(0.0); 
    float alphaAcum = 0.0; 
 
    float intensity;
    float lengthAcum = 0.0;
    vec4 colorSample; 
    float alphaSample; 
    
   
    for(int i = 0; i < 1600; i++)
    {
        // ��ȡ������
    	intensity =  texture(u_VolumeTex, voxelCoord).x;  

        // ת����RGBA
    	colorSample = texture(u_TransferFunc, intensity); 
    		
        // ǰ����
    	if (colorSample.a > 0.0) {
            // �Բ������alpha�����������������һ��˥�����ӣ�ʹ��Խ�����ӵ�Ĳ���Խ��͸����ԽԶ���ӵ�Ĳ���Խ͸��
    	    //colorSample.a = 1.0 - pow(1.0 - colorSample.a, u_StepSize*200.0f);
    	    colorSample.a = 1.0 - pow(1.0 - colorSample.a, .1);
    	    colorAcum.rgb += (1.0 - colorAcum.a) * colorSample.rgb * colorSample.a;
    	    colorAcum.a += (1.0 - colorAcum.a) * colorSample.a;
    	}
    	
        voxelCoord += deltaDir;     // ǰ��һ��
    	lengthAcum += deltaDirLen;  // �ۼ�ǰ���ľ���
    	
        // ���ǰ���ľ��볬����Χ�����뱳��ɫ���
        if (lengthAcum >= len ){
    	    colorAcum.rgb = colorAcum.rgb*colorAcum.a + (1 - colorAcum.a)*bgColor.rgb;		
    	    break;
    	}
        // ����ۼ�͸�����Ѿ�����1������ǰ��ֹ
    	else if (colorAcum.a > 1.0){
    	    colorAcum.a = 1.0;
    	    break;
    	}
    }

    FragColor = colorAcum;
    // ����
    //FragColor = vec4(EntryPoint, 1.0);
    //FragColor = vec4(exitPoint, 1.0);
}
