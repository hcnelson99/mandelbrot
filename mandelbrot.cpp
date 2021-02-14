#include <stdio.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>

void gl_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                      const void *_arg) {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        printf("ERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        printf("DEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        printf("UNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        printf("PORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        printf("PERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER:
        return;
    }

    printf(" (");
    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        printf("LOW");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        printf("MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        printf("HIGH");
        break;
    }
    printf("): %s\n", message);

    if (severity == GL_DEBUG_SEVERITY_HIGH && source != GL_DEBUG_SOURCE_SHADER_COMPILER) {
        exit(1);
    }
}

char *load_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("failed to open file %s\n", filename);
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char *)malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;
    return string;
}

bool compile_shader(GLuint shader, const char *filename) {
    char *code = load_file(filename);

    glShaderSource(shader, 1, &code, NULL);
    glCompileShader(shader);

    free(code);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        printf("failed to compile shader %s\n", filename);
        return false;
    }
    return true;
}
bool link_program(GLuint gl_program) {
    glLinkProgram(gl_program);
    GLint success;
    glGetProgramiv(gl_program, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        printf("Failed to link program\n");
        return false;
    }
    return true;
}

void recompile_program(GLuint *program) {
    GLuint gl_program = glCreateProgram();
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    if (!compile_shader(vertex_shader, "vertex.glsl")) {
        return;
    }
    glAttachShader(gl_program, vertex_shader);
    if (!compile_shader(fragment_shader, "fragment.glsl")) {
        return;
    }
    glAttachShader(gl_program, fragment_shader);
    if (!link_program(gl_program)) {
        return;
    }
    glBindAttribLocation(gl_program, 0, "vertex_pos");

    *program = gl_program;
}

void move(double *x0, double *y0, double *x1, double *y1, int xdir, int ydir) {
    double scale = 0.1;
    double dx = scale * (*x1 - *x0);
    double dy = scale * (*y1 - *y0);

    *x0 += dx * xdir;
    *y0 += dy * ydir;

    *x1 += dx * xdir;
    *y1 += dy * ydir;
}

void zoom(double *x0, double *y0, double *x1, double *y1, double scale) {
    double xd = *x1 - *x0;
    double yd = *y1 - *y0;
    *x0 += xd * scale;
    *y0 += yd * scale;
    *x1 -= xd * scale;
    *y1 -= yd * scale;
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to init video\n");
        return 1;
    }

    int width = 800;
    int height = 600;
    SDL_Window *window =
        SDL_CreateWindow("Mandelbrot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);

    if (!window) {
        printf("failed to init window\n");
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        printf("failed to create gl context\n");
        return 1;
    }

    glewExperimental = GL_TRUE;
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        printf("%s\n", glewGetErrorString(res));
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(gl_debug_message, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

    GLuint gl_program = 0;
    recompile_program(&gl_program);
    if (!gl_program) {
        printf("failed to compile program");
        return 1;
    }

    GLuint vbo, ibo;

    glClearColor(0, 0, 0, 1);
    GLfloat vbo_data[] = {-1, -1, 1, -1, 1, 1, -1, 1};
    GLuint ibo_data[] = {0, 1, 2, 3};

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 2 * 4 * sizeof(GLfloat), vbo_data, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLuint), ibo_data, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    double zoom_scale = 0.2;
    double x0 = -2.5, y0 = -1, x1 = 1, y1 = 1;

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_r:
                    printf("Recompiling...\n");
                    recompile_program(&gl_program);
                    printf("Done\n");
                    break;
                case SDLK_LEFT:
                case SDLK_a:
                    move(&x0, &y0, &x1, &y1, -1, 0);
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    move(&x0, &y0, &x1, &y1, 1, 0);
                    break;
                case SDLK_UP:
                case SDLK_w:
                    move(&x0, &y0, &x1, &y1, 0, 1);
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    move(&x0, &y0, &x1, &y1, 0, -1);
                    break;

                case SDLK_z:
                    zoom(&x0, &y0, &x1, &y1, zoom_scale);
                    break;
                case SDLK_x:
                    zoom(&x0, &y0, &x1, &y1, -zoom_scale);
                    break;
                }
            }
        }

        SDL_GetWindowSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(gl_program);

        glUniform2f(glGetUniformLocation(gl_program, "window_size"), width, height);
        glUniform4d(glGetUniformLocation(gl_program, "view"), x0, y0, x1, y1);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(GL_TRIANGLE_FAN, 4, GL_UNSIGNED_INT, NULL);

        glUseProgram(0);

        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
