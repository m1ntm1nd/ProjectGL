#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#define ASSERT(x) if(!(x)) __debugbreak();
#define GLCall(x) GlClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))


using namespace std;

static void GlClearError() {
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << "): " << function <<
            " " << file << ": " << line <<std::endl;
        return false;
    }
    return true;
}

static unsigned int CompileShader(unsigned int type,const string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        cout << "Failed to compile " <<
            (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "\n";
        cout << message << "\n";
        glDeleteShader(id);
        return 0;
    }


    return id;
}

static unsigned int CreateShader(const string& vertexShader, const string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

   
    return program;
}

struct ShaderSource {
    string vertex;
    string fragment;
};

static ShaderSource ParseShader(string path) {
    std::ifstream input(path);
    string line;
    stringstream ss[2];
    enum {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    int key = NONE;
    while(getline(input, line)) {
        if (line.find("#shader")!= std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                key = VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                key = FRAGMENT;
            }
        }
        else {
            ss[key] << line<<"\n";
        }
    }

    return{ ss[0].str(), ss[1].str() };
}



int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "smth", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        printf("Error\n");

    float position[] = {
        -0.5f, -0.5f,    //0
         0.5f,  -0.5f,   //1
         0.5f, 0.5f,    //2
         -0.5f,  0.5f   //3
  
    };

    unsigned int indexes[]{
        0,1,2,
        2,3,0
    }; 

    unsigned int vao;
    GLCall(glGenVertexArrays(1,& vao));
    GLCall(glBindVertexArray(vao));




    unsigned int buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, 4 *2* sizeof(float), position, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6* sizeof(unsigned int), indexes, GL_STATIC_DRAW);



    ShaderSource shadersrc = ParseShader("res/shaders/Basic.shader");
   
    unsigned int shader = CreateShader(shadersrc.vertex, shadersrc.fragment);
    glUseProgram(shader);

    int location = glGetUniformLocation(shader, "u_Color");
    ASSERT(location != -1);
    GLCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f));


    glUseProgram(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);



    float r = 0.0f;
    float increment = 0.05f;
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);



        glUseProgram(shader);
        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

        glBindBuffer(GL_ARRAY_BUFFER, buf);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,nullptr));
        
        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;

        r += increment;
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}