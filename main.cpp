#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>

// --- 全局状态变量 ---
float rotateX = 30.0f; // 整体绕X轴旋转角度
float rotateY = -45.0f; // 整体绕Y轴旋转角度
float translateX = 0.0f; // 整体X轴平移
float translateY = 0.0f; // 整体Y轴平移
// --- 旋转动画状态控制 ---
int isAnimating = 0;      // 是否正在播放旋转动画 (0=否, 1=是)
float currentAngle = 0.0f;// 当前旋转角度 (0 到 90)
int animLayer = 1;        // 正在旋转的层 (例如 x=1 代表右侧面)
int animDir = 1;          // 旋转方向 (1 或 -1)

// --- 单个小方块的数据结构 ---
typedef struct {
    int cx, cy, cz;       // 当前的逻辑位置，取值只能是 -1, 0, 1
    float rotMat[16];     // 属于这个方块自己的 4x4 旋转矩阵
} Cubie;

Cubie cube[27];           // 存放27个方块的数组

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
     glEnable(GL_TEXTURE_2D);
     glBindTexture(GL_TEXTURE_2D, textureID);

    /*// 三重循环绘制 3x3x3 = 27 个小方块
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
    }*/

    for (int i = 0; i < 27; i++) {
    glPushMatrix(); // 保存大魔方坐标系
    
    // 1. 检查这个方块是否属于当前正在旋转的层
    // 这里以绕 X 轴旋转（右侧面或左侧面）为例
    if (isAnimating && cube[i].cx == animLayer) {
        // 先做全局旋转，这会让方块绕着魔方中心点“公转”
        glRotatef(currentAngle * animDir, 1.0f, 0.0f, 0.0f); 
    }

    // 2. 将方块平移到它当前的逻辑位置
    glTranslatef(cube[i].cx * spacing, cube[i].cy * spacing, cube[i].cz * spacing);

    // 3. 叠加它历史累积的自身旋转 (保证面上的贴图方向对)
    glMultMatrixf(cube[i].rotMat);

    // 4. 绘制方块
    drawSubCube(2.0f);
    
    glPopMatrix(); // 恢复坐标系，准备画下一个方块
    }

    glDisable(GL_TEXTURE_2D);
    glutSwapBuffers(); // 双缓冲交换
}

// --- 键盘控制 (普通按键：平移) ---
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': translateY -= 0.5f; break; // 向上平移
        case 's': translateY += 0.5f; break; // 向下平移
        case 'a': translateX += 0.5f; break; // 向左平移
        case 'd': translateX -= 0.5f; break; // 向右平移
        case 'r': // 按下 'r' 键，旋转右侧面
            if (!isAnimating) {
                isAnimating = 1;
                animLayer = 1;  // x = 1 的那一层 (右侧面)
                animDir = 1;    // 顺时针
            }
            break;
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
     textureID = loadBMP("texture.bmp"); 
}

// 初始化27个方块的状态
void initCubies() {
    int index = 0;
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            for (int z = -1; z <= 1; z++) {
                cube[index].cx = x;
                cube[index].cy = y;
                cube[index].cz = z;
                
                // 获取一个干净的单位矩阵 (Identity Matrix) 存入 rotMat
                glPushMatrix();
                glLoadIdentity();
                glGetFloatv(GL_MODELVIEW_MATRIX, cube[index].rotMat);
                glPopMatrix();
                
                index++;
            }
        }
    }
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

// 完成 X 轴图层旋转的逻辑更新
void finalizeRotationX() {
    for (int i = 0; i < 27; i++) {
        if (cube[i].cx == animLayer) {
            // 1. 更新逻辑坐标 (位置交换算法)
            int oldY = cube[i].cy;
            int oldZ = cube[i].cz;
            
            if (animDir == 1) { // 顺时针
                cube[i].cy = -oldZ;
                cube[i].cz = oldY;
            } else {            // 逆时针
                cube[i].cy = oldZ;
                cube[i].cz = -oldY;
            }
            
            // 2. 更新自身的旋转矩阵 (把刚刚转过的90度永久“固化”进它的矩阵里)
            glPushMatrix();
            glLoadIdentity();
            glRotatef(90.0f * animDir, 1.0f, 0.0f, 0.0f); // 构造刚转过的90度
            glMultMatrixf(cube[i].rotMat);                // 乘上原来的历史矩阵
            glGetFloatv(GL_MODELVIEW_MATRIX, cube[i].rotMat); // 保存回数组
            glPopMatrix();
        }
    }
}

// 定时器函数：处理旋转动画帧
void timerUpdate(int value) {
    if (isAnimating) {
        currentAngle += 5.0f; // 每次旋转 5 度，控制动画速度
        
        if (currentAngle >= 90.0f) {
            // 动画完成，固化状态
            currentAngle = 0.0f;
            isAnimating = 0;
            finalizeRotationX(); // 更新内存里的数据
        }
        glutPostRedisplay(); // 触发重绘
    }
    glutTimerFunc(16, timerUpdate, 0); // 约 60FPS 循环调用
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutTimerFunc(16, timerUpdate, 0);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Rubik's Cube");

    init();
    initCubies();

    glutDisplayFunc(display); 
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}