#include "Program.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include <easy_encryption/encrypt.h>

#include "SharedEngineData.h"
#include "Console.h"

void Program::initPixelShader(const std::string& fragment_shader_filename) {
    std::string ecncypted_filepath = "EncryptedShaders/";
    std::string encryption_key = "aboba";

#ifdef PUBLIC_RELEASE
    size_t dot_index = fragment_shader_filename.find_last_of('.') + 1;
    if (initBinary(fragment_shader_filename.substr(0, dot_index) + "dat")) return;

    std::ifstream fragment_file(ecncypted_filepath + fragment_shader_filename, std::ios::binary);
    if (!fragment_file.is_open()) {
        Console::Instance.push("There's no uncompiled fragment shader: " + fragment_shader_filename);
        return;
    }

    size_t fragment_length;
    fragment_file.read(reinterpret_cast<char*>(&fragment_length), sizeof(size_t));

    char* temp_fragment = new char[fragment_length + 1];
    fragment_file.read(temp_fragment, fragment_length);
    temp_fragment[fragment_length] = '\0';
    std::string fragment_shader_source = temp_fragment;
    fragment_shader_source = decrypt(fragment_shader_source, encryption_key);
    delete[] temp_fragment;

    std::ifstream vertex_file(ecncypted_filepath + "vertex.vert", std::ios::binary);
    if (!vertex_file.is_open()) {
        Console::Instance.push("There's no uncompiled vertex shader: " + fragment_shader_filename);
        return;
    }

    size_t vertex_length;
    vertex_file.read(reinterpret_cast<char*>(&vertex_length), sizeof(size_t));

    char* temp_vertex = new char[vertex_length + 1];
    vertex_file.read(temp_vertex, vertex_length);
    temp_vertex[vertex_length] = '\0';
    std::string vertex_shader_source = temp_vertex;
    vertex_shader_source = decrypt(vertex_shader_source, encryption_key);
    delete[] temp_vertex;

    std::cout << fragment_shader_source << std::endl;

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(vertex_shader, vertex_shader_source)) {
        glDeleteShader(vertex_shader);
        return;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragment_shader, fragment_shader_source)) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        GLint max_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

        std::vector<GLchar> info_log(max_length);
        glGetProgramInfoLog(program, max_length, &max_length, &info_log[0]);

        glDeleteProgram(program);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        Console::Instance.push(std::string(info_log.begin(), info_log.end()));
        return;
    }

    std::string binary_filepath = "BinaryShaders/";
    std::string binary_filename = fragment_shader_filename.substr(0, fragment_shader_filename.find_last_of('.') + 1) + "dat";

    GLint binary_length = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binary_length);

    GLenum binary_format;
    GLsizei binary_size = 0;

    void* binary = (GLvoid*)malloc(binary_length);
    glGetProgramBinary(program, binary_length, &binary_size, &binary_format, binary);

    std::ofstream binary_file(binary_filepath + binary_filename, std::ios::binary);
    if (binary_file.is_open()) {
        binary_file.write(reinterpret_cast<const char*>(&binary_size), sizeof(GLsizei));
        binary_file.write(reinterpret_cast<const char*>(&binary_format), sizeof(GLenum));
        binary_file.write(reinterpret_cast<const char*>(binary), binary_size);
        binary_file.close();
    }

    free(binary);

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glDeleteProgram(m_id);

    initBinary(fragment_shader_filename.substr(0, dot_index) + "dat");

#else
    std::string vertex_shader_source = readShaderFile("default.vert");
    std::string fragment_shader_source = readShaderFile(fragment_shader_filename);
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    if (!compileShader(vertex_shader, vertex_shader_source)) {
        glDeleteShader(vertex_shader);
        return;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!compileShader(fragment_shader, fragment_shader_source)) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        GLint max_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

        std::vector<GLchar> info_log(max_length);
        glGetProgramInfoLog(program, max_length, &max_length, &info_log[0]);

        glDeleteProgram(program);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        Console::Instance.push(std::string(info_log.begin(), info_log.end()));
        return;
    }

    std::string encrypted_fragment = encrypt(fragment_shader_source, encryption_key);
    std::string encrypted_vertex = encrypt(vertex_shader_source, encryption_key);

    size_t enctypted_size_fragment = encrypted_fragment.size();
    size_t enctypted_size_vertex = encrypted_vertex.size();

    std::ofstream encrypted_fragment_file(ecncypted_filepath + fragment_shader_filename, std::ios::binary);
    if (encrypted_fragment_file.is_open()) {
        encrypted_fragment_file.write(reinterpret_cast<const char*>(&enctypted_size_fragment), sizeof(size_t));
        encrypted_fragment_file.write(encrypted_fragment.c_str(), enctypted_size_fragment);
        encrypted_fragment_file.close();
    }

    encrypted_fragment_file.close();

    std::ofstream encrypted_vertex_file(ecncypted_filepath + "vertex.vert", std::ios::binary);
    if (encrypted_vertex_file.is_open()) {
        encrypted_vertex_file.write(reinterpret_cast<const char*>(&enctypted_size_vertex), sizeof(size_t));
        encrypted_vertex_file.write(encrypted_vertex.c_str(), enctypted_size_vertex);
        encrypted_vertex_file.close();
    }

    encrypted_vertex_file.close();

    std::string binary_filepath = "BinaryShaders/";
    std::string binary_filename = fragment_shader_filename.substr(0, fragment_shader_filename.find_last_of('.') + 1) + "dat";

    GLint binary_length = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binary_length);

    GLenum binary_format;
    GLsizei binary_size = 0;

    void* binary = (GLvoid*)malloc(binary_length);
    glGetProgramBinary(program, binary_length, &binary_size, &binary_format, binary);

    std::ofstream binary_file(binary_filepath + binary_filename, std::ios::binary);
    if (binary_file.is_open()) {
        binary_file.write(reinterpret_cast<const char*>(&binary_size), sizeof(GLsizei));
        binary_file.write(reinterpret_cast<const char*>(&binary_format), sizeof(GLenum));
        binary_file.write(reinterpret_cast<const char*>(binary), binary_size);
        binary_file.close();
    }

    binary_file.close();
    free(binary);

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    m_id = program;
    m_is_initialized = true;
    m_is_disabled = false;
#endif
}

void Program::initCompute(const std::string& shader_filename) {
#ifdef PUBLIC_RELEASE
    size_t dot_index = shader_filename.find_last_of('.') + 1;
    initBinary(shader_filename.substr(0, dot_index) + "dat");
#else
    std::string shader_source = readShaderFile(shader_filename);
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);

    if (!compileShader(shader, shader_source)) {
        glDeleteShader(shader);
        return;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        GLint max_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

        std::vector<GLchar> info_log(max_length);
        glGetProgramInfoLog(program, max_length, &max_length, &info_log[0]);

        glDeleteProgram(program);
        glDeleteShader(shader);

        Console::Instance.push(std::string(info_log.begin(), info_log.end()));
        return;
    }

    std::string binary_filepath = "BinaryShaders/";
    std::string binary_filename = shader_filename.substr(0, shader_filename.find_last_of('.') + 1) + "dat";

    GLint binary_length = 0;
    glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &binary_length);

    GLenum binary_format;
    GLsizei binary_size = 0;

    void* binary = (GLvoid*)malloc(binary_length);
    glGetProgramBinary(program, binary_length, &binary_size, &binary_format, binary);

    std::ofstream binary_file(binary_filepath + binary_filename, std::ios::binary);
    if (binary_file.is_open()) {
        binary_file.write(reinterpret_cast<const char*>(&binary_size), sizeof(GLsizei));
        binary_file.write(reinterpret_cast<const char*>(&binary_format), sizeof(GLenum));
        binary_file.write(reinterpret_cast<const char*>(binary), binary_size);
        binary_file.close();
    }

    free(binary);

    glDetachShader(program, shader);
    glDeleteShader(shader);

    m_id = program;
    m_is_initialized = true;
    m_is_disabled = false;
#endif
}

bool Program::initBinary(const std::string& shader_filename) {
    std::string binary_filepath = "BinaryShaders/";
    std::ifstream binary_file(binary_filepath + shader_filename, std::ios::binary);

    if (!binary_file.is_open()) return false;

    GLsizei binary_length;
    binary_file.read(reinterpret_cast<char*>(&binary_length), sizeof(GLsizei));

    GLenum binary_format;
    binary_file.read(reinterpret_cast<char*>(&binary_format), sizeof(GLenum));

    void* binary = (GLvoid*)malloc(binary_length);
    binary_file.read(reinterpret_cast<char*>(binary), binary_length);

    binary_file.close();

    GLuint program = glCreateProgram();
    glProgramBinary(program, binary_format, binary, binary_length);
    free(binary);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        GLint max_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);

        std::vector<GLchar> info_log(max_length);
        glGetProgramInfoLog(program, max_length, &max_length, &info_log[0]);

        glDeleteProgram(program);

        Console::Instance.push(std::string(info_log.begin(), info_log.end()));
        return false;
    }

    m_id = program;
    m_is_initialized = true;
    m_is_disabled = false;

    return true;
}

std::string Program::readShaderFile(const std::string& filename) {
    std::string filepath = "Shader/";
    std::ifstream file_stream(filepath + filename, std::ios::in);

    if (!file_stream.is_open())
        throw std::runtime_error("Could not read file " + filename + ". File does not exist.");

    std::stringstream buffer;
    std::string line;
    std::string shader_source;
    int line_number = 0;

    while (std::getline(file_stream, line)) {
        buffer << line << '\n';
        m_filename_at_line.push_back(filename);
        line_number++;

        std::regex include_pattern("#include \"([^\"]+)\"");
        std::smatch include_matches;
        if (std::regex_search(line, include_matches, include_pattern)) {
            std::string include_file_name = include_matches[1];
            std::string include_source = readShaderFile(include_file_name);
            
			// Add the include source to the shader source
            shader_source += include_source + "\n";

			// Skip the include line
            continue;
        }

        shader_source += line + "\n";
    }

    file_stream.close();
    return shader_source;
}

bool Program::compileShader(GLuint shader, const std::string& shader_source) {
    const GLchar* shader_source_ptr = shader_source.c_str();
    glShaderSource(shader, 1, &shader_source_ptr, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (success) return true;

    GLchar info_log[512];
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    std::string s_info_log = std::string(info_log);
    Console::Instance.push("Shader compilation failed: " + s_info_log);

    std::regex line_number_regex("ERROR: [0-9]+:([0-9]+)");
    std::smatch matches;

    if (std::regex_search(s_info_log, matches, line_number_regex)) {
        int error_line_number = std::stoi(matches[1]);
        std::string error_filename = m_filename_at_line[error_line_number - 1];
        Console::Instance.push(error_filename + " at line " + std::to_string(error_line_number));

        // Print 5 lines of code around the error
        std::stringstream ss(shader_source);
        std::string line;
        int current_line_number = 1;
        while (std::getline(ss, line)) {
            if (current_line_number >= error_line_number - 2 && current_line_number <= error_line_number + 2) {
                Console::Instance.push("Line " + std::to_string(current_line_number) + ": " + line);
            }

            current_line_number++;
        }
    }

    Console::Instance.push("Shader compilation failure");
    return false;
}

void Program::draw() {
	if (m_is_disabled || !m_is_initialized) return;

    glUseProgram(m_id);

	int width, height;
    if(m_use_custom_resolution) {
        width = static_cast<int>(m_resolution_scale * m_width);
		height = static_cast<int>(m_resolution_scale * m_height);
    }
    else {
        width = static_cast<int>(m_resolution_scale * SharedEngineData::Instance.window_width);
        height = static_cast<int>(m_resolution_scale * SharedEngineData::Instance.window_height);
    }

    GLint location = glGetUniformLocation(m_id, "resolution");
    glUniform2f(location, static_cast<float>(width), static_cast<float>(height));
    glViewport(0, 0, width, height);

    m_framebuffer.bind();
    Quad::Instance.draw();
	m_framebuffer.unbind();
}

void Program::setResolutionScale(float scale) {
    m_resolution_scale = scale;

    int width = SharedEngineData::Instance.window_width;
	int height = SharedEngineData::Instance.window_height;
	getFBO()->resize(width, height, scale);
}

void Program::setResolution(int width, int height) {
    m_width = width;
	m_height = height;

	getFBO()->resize(m_width, m_height, m_resolution_scale);
    m_use_custom_resolution = true;
}

void Program::resizeFBO(int width, int height) {
    if (m_use_custom_resolution) return;
    m_framebuffer.resize(width, height, m_resolution_scale);
}

void Program::enable() {
    m_is_disabled = false;
}

void Program::disable() {
    m_is_disabled = true;
}

float Program::getResolutionScale() {
	return m_resolution_scale;
}

Program::~Program() {
    glDeleteProgram(m_id);
}

Framebuffer* Program::getFBO() {
	return &m_framebuffer;
}

GLuint Program::getID() {
    return m_id;
}
