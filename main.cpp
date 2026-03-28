#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>

// --- 全局状态变量 ---
float rotateX = 30.0f; // 整体绕X轴旋转角度
float rotateY = -45.0f; // 整体绕Y轴旋转角度
float translateX = 0.0f; // 整体X轴平移
float translateY = 0.0f; // 整体Y轴平移

GLuint textureID; // 保存载入的纹理ID

// --- BMP 读取函数 (简化版 24-bit BMP 载入) ---
GLuint loadBMP(const char* imagepath) {
    printf("Reading image %s\n", imagepath);
    unsigned char header[54];
    unsigned int dataPos, width, height, imageSize;
    unsigned char * data;

    FILE * file = fopen(imagepath, "rb");
    if (!file) { printf("Image could not be opened\n"); return 0; }
    if (fread(header, 1, 54, file) != 54 || header[0] != 'B' || header[1] != 'M') {
        printf("Not a correct BMP file\n"); return 0;
    }

    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);

    if (imageSize == 0)    imageSize = width * height * 3;
    if (dataPos == 0)      dataPos = 54;

    data = (unsigned char *)malloc(imageSize);
    fread(data, 1, imageSize, file);
    fclose(file);

    // 将图像数据传递给 OpenGL
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    free(data);
    return texID;
}

// --- 绘制单个小方块 (带法线和纹理坐标) ---
void drawSubCube(float size) {
    float s = size / 2.0f;
    glBegin(GL_QUADS);
        // 前面 (Z+)
        glColor3f(1.0f, 0.0f, 0.0f); // 暂时用纯色区分，方便观察结构，贴图启用后会被覆盖
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s,  s);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( s, -s,  s);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s,  s);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s,  s);
        // 后面 (Z-)
        glColor3f(1.0f, 0.5f, 0.0f);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( s, -s, -s);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s, -s);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-s,  s, -s);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( s,  s, -s);
        // 上面 (Y+)
        glColor3f(1.0f, 1.0f, 1.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-s,  s,  s);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( s,  s,  s);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s, -s);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s, -s);
        // 下面 (Y-)
        glColor3f(1.0f, 1.0f, 0.0f);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s, -s);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( s, -s, -s);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( s, -s,  s);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-s, -s,  s);
        // 左面 (X-)
        glColor3f(0.0f, 0.0f, 1.0f);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s, -s);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s,  s);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-s,  s,  s);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s, -s);
        // 右面 (X+)
        glColor3f(0.0f, 1.0f, 0.0f);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( s, -s,  s);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( s, -s, -s);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s, -s);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( s,  s,  s);
    glEnd();
    
    // 绘制黑色边框，方便区分方块
    glColor3f(0.0f, 0.0f, 0.0f);
    glutWireCube(size + 0.01f); 
}

// --- 绘制完整魔方 ---
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // 将相机往后推一点
    glTranslatef(0.0f, 0.0f, -15.0f); 

    // 根据全局变量执行整体平移和旋转
    glTranslatef(translateX, translateY, 0.0f);
    glRotatef(rotateX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotateY, 0.0f, 1.0f, 0.0f);

    float spacing = 2.1f; // 方块之间的间距

    // 启用纹理 (如果在本地放了BMP文件，取消下面的注释)
    // glEnable(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, textureID);

    // 三重循环绘制 3x3x3 = 27 个小方块
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                glPushMatrix(); // 保存当前坐标系状态
                
                // 将当前方块平移到对应位置
                glTranslatef(x * spacing, y * spacing, z * spacing);
                
                // 绘制方块
                drawSubCube(2.0f);
                
                glPopMatrix(); // 恢复坐标系状态
            }
        }
    }

    // glDisable(GL_TEXTURE_2D);
    glutSwapBuffers(); // 双缓冲交换
}

// --- 键盘控制 (普通按键：平移) ---
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': translateY += 0.5f; break; // 向上平移
        case 's': translateY -= 0.5f; break; // 向下平移
        case 'a': translateX -= 0.5f; break; // 向左平移
        case 'd': translateX += 0.5f; break; // 向右平移
        case 27: exit(0); break;             // ESC退出
    }
    glutPostRedisplay(); // 触发重绘
}

// --- 键盘控制 (特殊按键：整体旋转) ---
void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:    rotateX -= 5.0f; break; // 整体上旋
        case GLUT_KEY_DOWN:  rotateX += 5.0f; break; // 整体下旋
        case GLUT_KEY_LEFT:  rotateY -= 5.0f; break; // 整体左旋
        case GLUT_KEY_RIGHT: rotateY += 5.0f; break; // 整体右旋
    }
    glutPostRedisplay();
}

// --- 初始化 OpenGL ---
void init() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // 深灰色背景
    glEnable(GL_DEPTH_TEST);              // 启用深度测试 (遮挡关系)
    
    // 加载纹理 (请确保你的项目目录下有一个名为 texture.bmp 的图片，大小最好是 256x256 或 512x512)
    // textureID = loadBMP("texture.bmp"); 
}

// --- 窗口大小改变时的回调 ---
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = 1.0f * w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, 100.0f); // 设置透视投影
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Rubik's Cube");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}