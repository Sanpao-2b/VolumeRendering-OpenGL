#version 400

in vec3 EntryPoint;     // 进入点（对象空间）
in vec4 ExitPointCoord; // 出口点（裁剪空间），这是用来算UV坐标的

uniform sampler2D u_ExitPoints;
uniform sampler3D u_VolumeTex;
uniform sampler1D u_TransferFunc;  
uniform float     u_StepSize; // 以百分比的形式
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
    
    vec3 voxelCoord = EntryPoint;   // 用于采样3D纹理的纹理坐标初始值

    vec4 bgColor = vec4(1.0, 1.0, 1.0, 0.0); // 背景色
    bgColor = vec4(1., 1., 1., 1.); // 背景色
    vec4 colorAcum = vec4(0.0); 
    float alphaAcum = 0.0; 
 
    float intensity;
    float lengthAcum = 0.0;
    vec4 colorSample; 
    float alphaSample; 
    
   
    for(int i = 0; i < 1600; i++)
    {
        // 获取体数据
    	intensity =  texture(u_VolumeTex, voxelCoord).x;  

        // 转换成RGBA
    	colorSample = texture(u_TransferFunc, intensity); 
    		
        // 前向混合
    	if (colorSample.a > 0.0) {
            // 对采样点的alpha进行修正，对其乘以一个衰减因子，使其越靠近视点的部分越不透明，越远离视点的部分越透明
    	    //colorSample.a = 1.0 - pow(1.0 - colorSample.a, u_StepSize*200.0f);
    	    colorSample.a = 1.0 - pow(1.0 - colorSample.a, .1);
    	    colorAcum.rgb += (1.0 - colorAcum.a) * colorSample.rgb * colorSample.a;
    	    colorAcum.a += (1.0 - colorAcum.a) * colorSample.a;
    	}
    	
        voxelCoord += deltaDir;     // 前进一步
    	lengthAcum += deltaDirLen;  // 累加前进的距离
    	
        // 如果前进的距离超过包围盒则与背景色混合
        if (lengthAcum >= len ){
    	    colorAcum.rgb = colorAcum.rgb*colorAcum.a + (1 - colorAcum.a)*bgColor.rgb;		
    	    break;
    	}
        // 如果累加透明度已经超过1，则提前终止
    	else if (colorAcum.a > 1.0){
    	    colorAcum.a = 1.0;
    	    break;
    	}
    }

    FragColor = colorAcum;
    // 测试
    //FragColor = vec4(EntryPoint, 1.0);
    //FragColor = vec4(exitPoint, 1.0);
}
