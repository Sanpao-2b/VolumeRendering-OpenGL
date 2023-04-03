#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glut.h>
#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtx/transform2.hpp>
#include <GL/glm/gtc/type_ptr.hpp>
#include <vector>

#define GL_ERROR() checkForOpenGLError(__FILE__, __LINE__)
using namespace std;
using glm::mat4;
using glm::vec3;
GLuint g_vao;
GLuint g_programHandle; // shader program
GLuint g_winWidth = 1000;
GLuint g_winHeight = 1000;
GLint g_angle = 0;
GLuint g_frameBuffer;
// transfer function
GLuint g_tffTexObj;
GLuint g_bfTexObj;
GLuint g_texWidth;
GLuint g_texHeight;
GLuint g_volTexObj;
GLuint g_rcVertHandle;
GLuint g_rcFragHandle;
GLuint g_bfVertHandle;
GLuint g_bfFragHandle;
float g_stepSize = 0.001f;


int checkForOpenGLError(const char* file, int line)
{
    // return 1 if an OpenGL error occured, 0 otherwise.
    GLenum glErr;
    int retCode = 0;

    glErr = glGetError();
    while(glErr != GL_NO_ERROR)
    {
	    cout << "glError in file " << file << "@line " << line << gluErrorString(glErr) << endl;
	    retCode = 1;
	    exit(EXIT_FAILURE);
    }
    return retCode;
}
void keyboard(unsigned char key, int x, int y);
void display(void);
void initVBO();
void initShader();
void initFrameBuffer(GLuint, GLuint, GLuint);
GLuint initTFF1DTex(const char* filename);
GLuint initFace2DTex(GLuint texWidth, GLuint texHeight);
GLuint initVol3DTex(const char* filename, GLuint width, GLuint height, GLuint depth);
void render(GLenum cullFace);
void init()
{
    initVBO();
    initShader();
    g_tffTexObj = initTFF1DTex("../tff.dat");
    g_bfTexObj = initFace2DTex(g_winWidth, g_winHeight);
    g_volTexObj = initVol3DTex("../head256.raw", 256, 256, 225);
    GL_ERROR();
    initFrameBuffer(g_bfTexObj, g_winWidth, g_winHeight);
    GL_ERROR();
}
// init the vertex buffer object
void initVBO()
{
    GLfloat vertices[24] = {
	0.0, 0.0, 0.0,
	0.0, 0.0, 1.0,
	0.0, 1.0, 0.0,
	0.0, 1.0, 1.0,
	1.0, 0.0, 0.0,
	1.0, 0.0, 1.0,
	1.0, 1.0, 0.0,
	1.0, 1.0, 1.0
    };

    GLuint indices[36] = {
	1,5,7,
	7,3,1,
	0,2,6,
    6,4,0,
	0,1,3,
	3,2,0,
	7,5,4,
	4,6,7,
	2,3,7,
	7,6,2,
	1,0,4,
	4,5,1
    };
    GLuint gbo[2];
    
    glGenBuffers(2, gbo);
    GLuint vertexdat = gbo[0];
    GLuint veridxdat = gbo[1];
    glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    // used in glDrawElement()
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(GLuint), indices, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    // vao like a closure binding 3 buffer object: verlocdat vercoldat and veridxdat
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0); // for vertexloc

    // the vertex location is the same as the vertex color
    glBindBuffer(GL_ARRAY_BUFFER, vertexdat);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLfloat *)NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, veridxdat);
    // glBindVertexArray(0);
    g_vao = vao;
}
void drawBox(GLenum glFaces)
{
    glEnable(GL_CULL_FACE);
    glCullFace(glFaces);
    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLuint *)NULL);
    glDisable(GL_CULL_FACE);
}
// check the compilation result
GLboolean compileCheck(GLuint shader)
{
    GLint err;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &err);
    if (err) {
        return GL_TRUE;
    }

	GLint logLen;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0)
	{
        std::vector<char>log(logLen);
	    GLsizei written;
	    glGetShaderInfoLog(shader, logLen, &written, log.data());
	    cerr << "Shader log: " << log.data() << endl;	
	}
    return GL_FALSE;
}
// init shader object
GLuint initShaderObj(const GLchar* srcfile, GLenum shaderType)
{
    ifstream ifs(srcfile, ifstream::in | ifstream::ate);
    if (!ifs.is_open()){
	    cerr << "Error openning file: " << srcfile << endl;
	    exit(EXIT_FAILURE);
    }

    std::vector<char> shaderCode(ifs.tellg());
    ifs.seekg(0);
   
    ifs.read(shaderCode.data(), shaderCode.size());
    if (ifs.eof()) {
        size_t bytecnt = ifs.gcount();
        *(shaderCode.data() + bytecnt) = '\0';
    }

    // create the shader Object
    GLuint shader = glCreateShader(shaderType);
    if (0 == shader){
	    cerr << "Error creating vertex shader." << endl;
    }

    const GLchar* codeArray[] = {shaderCode.data()};
    glShaderSource(shader, 1, codeArray, NULL);

    // compile the shader
    glCompileShader(shader);
    if (GL_FALSE == compileCheck(shader)){
	    cerr << "shader compilation failed" << endl;
    }
	    
    return shader;
}
GLint checkShaderLinkStatus(GLuint pgmHandle)
{
    GLint status;
    glGetProgramiv(pgmHandle, GL_LINK_STATUS, &status);
	GLint logLen;
	glGetProgramiv(pgmHandle, GL_INFO_LOG_LENGTH, &logLen);
    if (status == GL_FALSE && logLen > 0)
    {
        std::vector<char>log(logLen);
	    int written;
	    glGetProgramInfoLog(pgmHandle, logLen, &written, log.data());
	    cerr << "Program log: " << log.data() << endl;
    }
    return status;
}
// link shader program
GLuint createShaderPgm()
{
    // Create the shader program
    GLuint programHandle = glCreateProgram();
    if (!programHandle)
    {
	    cerr << "Error create shader program" << endl;
	    exit(EXIT_FAILURE);
    }
    return programHandle;
}


// init the 1 dimentional texture for transfer function
GLuint initTFF1DTex(const char* filename)
{
    // read in the user defined data of transfer function
    ifstream ifs(filename, ifstream::in | ifstream::ate | ifstream::binary);
    if (!ifs.is_open()) {
        cerr << "Error openning file: " << filename << endl;
        exit(EXIT_FAILURE);
    }
    
    std::vector<char> tff(ifs.tellg());
    ifs.seekg(0, ios::beg);

    ifs.read((char*)tff.data(), tff.size());
    if (ifs.fail()){ 
        cout << filename << "read failed " << endl; 
    }
 
    GLuint tff1DTex;
    glGenTextures(1, &tff1DTex);
    glBindTexture(GL_TEXTURE_1D, tff1DTex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, tff.data());

    return tff1DTex;
}
// init the 2D texture for render backface 'bf' stands for backface
GLuint initFace2DTex(GLuint bfTexWidth, GLuint bfTexHeight)
{
    GLuint backFace2DTex;
    glGenTextures(1, &backFace2DTex);
    glBindTexture(GL_TEXTURE_2D, backFace2DTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, bfTexWidth, bfTexHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    return backFace2DTex;
}
// init 3D texture to store the volume data used fo ray casting
GLuint initVol3DTex(const char* filename, GLuint w, GLuint h, GLuint d)
{
    std::ifstream ifs(filename, ios::in | ios::ate | ios::binary);
    if (!ifs.is_open()){
        std::cerr << "Failed to open file :" << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    size_t size = ifs.tellg();
    if (size != w * h * d){
        std::cerr << "File size is not equal to w * h * d" << std::endl;
    }
    std::vector<GLubyte> volumeData(size);
    ifs.seekg(0);

    ifs.read((char*)volumeData.data(), volumeData.size());
    if (ifs.fail()) {
        std::cerr << "Failed to read .raw file :" << filename << std::endl;
    }
    ifs.close();

    glGenTextures(1, &g_volTexObj);
    // bind 3D texture target
    glBindTexture(GL_TEXTURE_3D, g_volTexObj);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    // pixel transfer happens here from client to OpenGL server
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_INTENSITY, w, h, d, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, volumeData.data());

    //delete []data;
    cout << "volume texture created" << endl;
    return g_volTexObj;
}

void checkFramebufferStatus()
{
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE){
	    cout << "framebuffer is not complete" << endl;
	    exit(EXIT_FAILURE);
    }
}
// init the framebuffer, the only framebuffer used in this program
void initFrameBuffer(GLuint texObj, GLuint texWidth, GLuint texHeight)
{
    // create a depth buffer for our framebuffer
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, texWidth, texHeight);

    // attach the texture and the depth buffer to the framebuffer
    glGenFramebuffers(1, &g_frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, g_frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texObj, 0); // ��ɫ����
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);// ��Ȼ��帽��
    checkFramebufferStatus();
    glEnable(GL_DEPTH_TEST);    
}

// ������ɫ���е�uniform����
void rcSetUinforms()
{
    // ��Ļ�ߴ�
    GLint screenSizeLoc = glGetUniformLocation(g_programHandle, "u_Resolution");
    if (screenSizeLoc >= 0){
	    glUniform2f(screenSizeLoc, (float)g_winWidth, (float)g_winHeight);
    }
    else{
	    cout << "ScreenSize " << "is not bind to the uniform" << endl;
    }
    
    // ����
    GLint stepSizeLoc = glGetUniformLocation(g_programHandle, "u_StepSize");
    GL_ERROR();
    if (stepSizeLoc >= 0){
	    glUniform1f(stepSizeLoc, g_stepSize);
    }
    else{
	    cout << "StepSize " << "is not bind to the uniform" << endl;
    }
    GL_ERROR();
    
    // ת������ 1D texture
    GLint transferFuncLoc = glGetUniformLocation(g_programHandle, "u_TransferFunc");
    if (transferFuncLoc >= 0){
	    glUniform1i(transferFuncLoc, 0);            // ����iniform sampler��������Ԫ0
	    glActiveTexture(GL_TEXTURE0);               // ��������Ԫ0
	    glBindTexture(GL_TEXTURE_1D, g_tffTexObj);  // ����������Ԫ0
    }
    else{
	    cout << "TransferFunc " << "is not bind to the uniform" << endl;
    }
    GL_ERROR();    
    
    // ���棨��Ϊ���ڵ㣩 2D texture 
    GLint backFaceLoc = glGetUniformLocation(g_programHandle, "u_ExitPoints");
    if (backFaceLoc >= 0){
	    glUniform1i(backFaceLoc, 1);
	    glActiveTexture(GL_TEXTURE1);
	    glBindTexture(GL_TEXTURE_2D, g_bfTexObj);
    }
    else{
	    cout << "ExitPoints " << "is not bind to the uniform" << endl;
    }
    GL_ERROR();

    // ������ 3D texture
    GLint volumeLoc = glGetUniformLocation(g_programHandle, "u_VolumeTex");
    if (volumeLoc >= 0){
	    glUniform1i(volumeLoc, 2);
	    glActiveTexture(GL_TEXTURE2);
	    glBindTexture(GL_TEXTURE_3D, g_volTexObj);
    }
    else{
	    cout << "VolumeTex " << "is not bind to the uniform" << endl;
    }
}
// init the shader object and shader program
void initShader()
{
// vertex shader object for first pass
    g_bfVertHandle = initShaderObj("shader/backface.vert", GL_VERTEX_SHADER);
// fragment shader object for first pass
    g_bfFragHandle = initShaderObj("shader/backface.frag", GL_FRAGMENT_SHADER);
// vertex shader object for second pass
    g_rcVertHandle = initShaderObj("shader/raycasting.vert", GL_VERTEX_SHADER);
// fragment shader object for second pass
    g_rcFragHandle = initShaderObj("shader/raycasting.frag", GL_FRAGMENT_SHADER);
// create the shader program , use it in an appropriate time
    g_programHandle = createShaderPgm();
// �������ɫ�����������������(��ѡ)
}

// link the shader objects using the shader program
void linkShader(GLuint shaderPgm, GLuint newVertHandle, GLuint newFragHandle)
{
    const GLsizei maxCount = 2;
    GLuint shaders[maxCount];
    
    // ��ȡĿǰshader program�󶨵���ɫ��������count������ɫ������(shaders)
    GLsizei count;
    glGetAttachedShaders(shaderPgm, maxCount, &count, shaders);
    GL_ERROR();
    // ���Ŀǰ�󶨵���ɫ��
    for (int i = 0; i < count; i++) {
	    glDetachShader(shaderPgm, shaders[i]);
    }
    // ����ɫ�������������ָ��������������
    //glBindAttribLocation(shaderPgm, 0, "VerPos");
    //glBindAttribLocation(shaderPgm, 1, "VerClr");
    GL_ERROR();
    
    // ���µ���ɫ���󶨵���ɫ��������
    glAttachShader(shaderPgm,newVertHandle);
    glAttachShader(shaderPgm,newFragHandle);
    GL_ERROR();

    // ����������ɫ���������
    glLinkProgram(shaderPgm);
    if (GL_FALSE == checkShaderLinkStatus(shaderPgm)){
	    cerr << "Failed to relink shader program!" << endl;
	    exit(EXIT_FAILURE);
    }
    GL_ERROR();
}

// the color of the vertex in the back face is also the location
// of the vertex
// save the back face to the user defined framebuffer bound
// with a 2D texture named `g_bfTexObj`
// draw the front face of the box
// in the rendering process, i.e. the ray marching process
// loading the volume `g_volTexObj` as well as the `g_bfTexObj`
// after vertex shader processing we got the color as well as the location of
// the vertex (in the object coordinates, before transformation).
// and the vertex assemblied into primitives before entering
// fragment shader processing stage.
// in fragment shader processing stage. we got `g_bfTexObj`
// (correspond to 'VolumeTex' in glsl)and `g_volTexObj`(correspond to 'ExitPoints')
// as well as the location of primitives.
// the most important is that we got the GLSL to exec the logic. Here we go!
// draw the back face of the box
void display()
{
    glEnable(GL_DEPTH_TEST);
    GL_ERROR();
    // pass1 ���ư�Χ�б��棬��������
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_frameBuffer);
    glViewport(0, 0, g_winWidth, g_winHeight);
    linkShader(g_programHandle, g_bfVertHandle, g_bfFragHandle);
    glUseProgram(g_programHandle);
    // �޳�����
    render(GL_FRONT);
    glUseProgram(0);
    GL_ERROR();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // pass 2 �������棬��������߷��򣬲�ִ�й���Ͷ��
    glViewport(0, 0, g_winWidth, g_winHeight);
    linkShader(g_programHandle, g_rcVertHandle, g_rcFragHandle); // �л���ɫ�������������ɫ������
    GL_ERROR();
    glUseProgram(g_programHandle);
    rcSetUinforms();    // ���ù���Ͷ��pass�е�shader uniform����
    GL_ERROR();
    // glUseProgram(g_programHandle);
    // cull back face
    render(GL_BACK);
    // need or need not to resume the state of only one active texture unit?
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, 0);
    // glDisable(GL_TEXTURE_2D);
    // glActiveTexture(GL_TEXTURE2);
    // glBindTexture(GL_TEXTURE_3D, 0);    
    // glDisable(GL_TEXTURE_3D);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_1D, 0);    
    // glDisable(GL_TEXTURE_1D);
    // glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);
    GL_ERROR(); 
    
    // // for test the first pass
    // glBindFramebuffer(GL_READ_FRAMEBUFFER, g_frameBuffer);
    // checkFramebufferStatus();
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // glViewport(0, 0, g_winWidth, g_winHeight);
    // glClearColor(0.0, 0.0, 1.0, 1.0);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // GL_ERROR();
    // glBlitFramebuffer(0, 0, g_winWidth, g_winHeight,0, 0,
    // 		      g_winWidth, g_winHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // GL_ERROR();
    glutSwapBuffers();
}
// both of the two pass use the "render() function"
// the first pass render the backface of the boundbox
// the second pass render the frontface of the boundbox
// together with the frontface, use the backface as a 2D texture in the second pass
// to calculate the entry point and the exit point of the ray in and out the box.
void render(GLenum cullFace)
{
    GL_ERROR();
    glClearColor(0.2f,0.2f,0.2f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //  MVP uniform
    glm::mat4 projection = glm::perspective(60.0f, (GLfloat)g_winWidth/g_winHeight, 0.1f, 400.f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model = mat4(1.0f);   // ע��model = M_rot * M_rot * M_trans;
    model *= glm::rotate((float)g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
    // to make the "head256.raw" i.e. the volume data stand up.
    model *= glm::rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
    model *= glm::translate(glm::vec3(-0.5f, -0.5f, -0.5f)); 
    // notice the multiplication order: reverse order of transform
    glm::mat4 mvp = projection * view * model;
    
    GLuint mvpIdx = glGetUniformLocation(g_programHandle, "MVP");
    if (mvpIdx >= 0){
    	glUniformMatrix4fv(mvpIdx, 1, GL_FALSE, &mvp[0][0]);
    }
    else{
    	cerr << "can't get the MVP" << endl;
    }
    GL_ERROR();
    
    drawBox(cullFace);
    GL_ERROR();
    // glutWireTeapot(0.5);
}
void rotateDisplay()
{
    g_angle = (g_angle + 1) % 360;
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    g_winWidth = w;
    g_winHeight = h;
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case '\x1B':
	    exit(EXIT_SUCCESS);
	    break;
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // ˫���塢��ɫ�ռ䡢��Ȼ���on
    glutInitWindowSize(g_winWidth, g_winHeight);
    glutCreateWindow("GLUT Test");
    GLenum err = glewInit();
    if (GLEW_OK != err){
	    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
  
    glutKeyboardFunc(&keyboard);
    glutDisplayFunc(&display);
    glutReshapeFunc(&reshape);
    glutIdleFunc(&rotateDisplay);
    init();
    glutMainLoop();
    return EXIT_SUCCESS;
}

