#include <GL/freeglut.h>

void display() {
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲
    
    glBegin(GL_POLYGON); // 绘制一个多边形
        glColor3f(1.0, 0.0, 0.0); glVertex2f(-0.5, -0.5);
        glColor3f(0.0, 1.0, 0.0); glVertex2f(-0.5, 0.5);
        glColor3f(0.0, 0.0, 1.0); glVertex2f(0.5, 0.5);
        glColor3f(1.0, 1.0, 0.0); glVertex2f(0.5, -0.5);
    glEnd();
    
    glFlush(); // 强制执行 OpenGL 命令
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("OpenGL Setup Test");
    
    glutDisplayFunc(display);
    
    glutMainLoop();
    return 0;
}