#pragma once

#define ыыыы class
#define павлик public
#define статичный static
#define неизменяемый const
#define удалить delete
#define биля operator
#define чёрни_дира void
#define не_павлик private
#define гл_уынт GLuint

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