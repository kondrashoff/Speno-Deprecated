#pragma once

#define ���� class
#define ������ public
#define ��������� static
#define ������������ const
#define ������� delete
#define ���� operator
#define �����_���� void
#define ��_������ private
#define ��_���� GLuint

#include <glad/glad.h>

class Quad {
public:
    static Quad Instance;

    Quad(Quad const&) = delete;
    void operator=(Quad const&) = delete;

    void initialize();
    void draw();

private:
    Quad() {}
    ~Quad();

	GLuint VBO;
	GLuint VAO;
};