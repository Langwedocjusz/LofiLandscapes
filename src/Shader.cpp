#include "Shader.h"

#include "glad/glad.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#include <iostream>

void Shader::Bind()
{
    glUseProgram(m_ID);
}

void Shader::Reload() 
{
    //Release GL handle
    glDeleteProgram(m_ID);

    //Reset member variables
    m_ID = 0;
    m_UniformCache.clear();

    //Rebuild
    Build();
}

uint32_t Shader::getUniformLocation(const std::string& name)
{
    auto search_res = std::find_if(
        m_UniformCache.begin(), m_UniformCache.end(),
        [&name](const std::pair<std::string, uint32_t> element)
        {
            return element.first == name;
        }
    );

    if (search_res != m_UniformCache.end())
        return search_res->second;

    const uint32_t location = glGetUniformLocation(m_ID, name.c_str());
    m_UniformCache.push_back(std::make_pair(name, location));
    return location;
}

//Program type is assumed to be gl enum: {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER}
static void compileShaderCode(const std::string& source, unsigned int& id, int program_type)
{
    const char* source_c = source.c_str();

    int success = 0;
    char info_log[512];

    id = glCreateShader(program_type);

    glShaderSource(id, 1, &source_c, NULL);
    glCompileShader(id);

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) 
    {
        glGetShaderInfoLog(id, 512, NULL, info_log);
        glDeleteShader(id);

        throw std::runtime_error(info_log);
    }
}

static std::string loadSource(std::filesystem::path filepath, bool recursive_call = false)
{
    const std::string include_token{"#include"};

    std::ifstream input{ filepath };

    if (!input)
    {
        throw std::runtime_error(
            "Could not open shader source file:\n" + filepath.string()
        );
    }

    std::string full_source, current_line;

    while (std::getline(input, current_line))
    {
        if (current_line.find(include_token) != std::string::npos)
        {
            auto start_id = current_line.find_first_of('\"') + 1;
            auto end_id = current_line.find_last_of('\"');
            auto length = end_id - start_id;

            std::string filename = current_line.substr(start_id, length);

            std::filesystem::path new_path = filepath.remove_filename() / filename;

            full_source += loadSource(new_path, true);
        }

        else
        {
            full_source += current_line + "\n";
        }
    }

    if (!recursive_call)
        full_source += "\0";

    return full_source;
}

void VertFragShader::Build()
{
    //Get source
    std::filesystem::path current_path{ std::filesystem::current_path() };

    std::string vert_code = loadSource(current_path / m_VertPath);
    std::string frag_code = loadSource(current_path / m_FragPath);

    //Compile shaders
    unsigned int vert_id = 0, frag_id = 0;

    try 
    {
        compileShaderCode(vert_code, vert_id, GL_VERTEX_SHADER);
    }

    catch (const std::runtime_error& e) 
    {
        std::cerr << "Vertex Shader compilation failed: \n"
                  << "filepath: " << m_VertPath << '\n'
                  << e.what() << '\n';
        return;
    }

    try 
    {
        compileShaderCode(frag_code, frag_id, GL_FRAGMENT_SHADER);
    }

    catch (const std::runtime_error& e) 
    {
        std::cerr << "Fragment Shader compilation failed: \n"
                  << "filepath: " << m_FragPath << '\n'
                  << e.what() << '\n';
        return;
    }

    //Link program
    m_ID = glCreateProgram();

    glAttachShader(m_ID, vert_id);
    glAttachShader(m_ID, frag_id);
    glLinkProgram(m_ID);

    int success = 0;
    char info_log[512];

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

    if (!success) 
    {
        glGetProgramInfoLog(m_ID, 512, NULL, info_log);

        std::cerr << "Error: Shader program linking failed: \n"
                  << "filepaths: " << m_VertPath << ", " << m_FragPath << '\n'
                  << info_log << '\n';
    }

    glDeleteShader(vert_id);
    glDeleteShader(frag_id);
}

VertFragShader::VertFragShader(const std::string& vert_path, const std::string& frag_path) 
    : m_VertPath(vert_path), m_FragPath(frag_path)
{
    Build();
}

VertFragShader::~VertFragShader() 
{
    glDeleteProgram(m_ID);
}

void ComputeShader::Build() 
{
    //Get source
    std::filesystem::path current_path{ std::filesystem::current_path() };
    std::string compute_code = loadSource(current_path / m_ComputePath);

    //Initialize local sizes
    RetrieveLocalSizes(compute_code);

    //Compile shader
    unsigned int compute_id = 0;

    try 
    {
        compileShaderCode(compute_code, compute_id, GL_COMPUTE_SHADER);
    }

    catch (const std::runtime_error& e) 
    {
        std::cerr << "Compute Shader compilation failed: \n"
                  << "filepath: " << m_ComputePath << '\n'
                  << e.what() << '\n';
        return;
    }

    //Link program
    m_ID = glCreateProgram();
    glAttachShader(m_ID, compute_id);
    glLinkProgram(m_ID);

    int success = 0;
    char info_log[512];

    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    
    if (!success) 
    {
        glGetProgramInfoLog(m_ID, 512, NULL, info_log);
        std::cerr << "Error: Shader program linking failed: \n"
                  << "filepath: " << m_ComputePath << '\n'
                  << info_log << '\n';
    }

    glDeleteShader(compute_id);
}

ComputeShader::ComputeShader(const std::string& compute_path) 
    : m_ComputePath(compute_path)
{
    Build();
}

ComputeShader::~ComputeShader() 
{
    glDeleteProgram(m_ID);
}

void ComputeShader::Dispatch(uint32_t size_x, uint32_t size_y, uint32_t size_z) const
{
    //We need to take ceilings of the divisions here since for example 33 invocations with 
    //local size 32 should result in 2 dispatches, not one

    const uint32_t disp_x = (size_x + m_LocalSizeX - 1) / m_LocalSizeX;
    const uint32_t disp_y = (size_y + m_LocalSizeY - 1) / m_LocalSizeY;
    const uint32_t disp_z = (size_z + m_LocalSizeZ - 1) / m_LocalSizeZ;

    glDispatchCompute(disp_x, disp_y, disp_z);
}

void ComputeShader::RetrieveLocalSizes(const std::string& source_code)
{
    auto retrieveInt = [source_code](const std::string& name) -> int
    {
        auto id = source_code.find(name);

        if (id == std::string::npos)
            throw std::runtime_error("Unable to find " + name);

        //Id was before the beggining of the name, so shift it one past the end
        id += name.size() + 2;

        //Skip whitespaces
        while (source_code[id] == ' ' || source_code[id] == '\n' || source_code[id] == '\t')
        {
            id++;
        }

        //Create a string
        std::string value;

        //Load all digits into the string
        while (isdigit(source_code[id]))
        {
            value += source_code[id];
            id++;
        }

        if (value.empty())
            throw std::runtime_error("Unable to parse value of " + name);

        return std::stoi(value);
    };

    auto setSize = [this, retrieveInt](uint32_t& value, const std::string& name) {
        try
        {
            value = retrieveInt(name);
        }

        catch (const std::exception& e)
        {
            std::cerr << "Exception thrown when parsing: " << m_ComputePath << '\n';
            std::cerr << e.what() << '\n';

            //Default to setting sizes as 1 if something goes wrong 
            value = 1;
        }
    };

    setSize(m_LocalSizeX, "local_size_x");
    setSize(m_LocalSizeY, "local_size_y");
    setSize(m_LocalSizeZ, "local_size_z");
}

//=====Uniform setting==================================================

void Shader::setUniform1i(const std::string& name, int x) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform1i(location, x);
}

void Shader::setUniform2i(const std::string& name, int x, int y) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform2i(location, x, y);
}

void Shader::setUniform3i(const std::string& name, int x, int y, int z) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform3i(location, x, y, z);
}

void Shader::setUniform4i(const std::string& name, int x, int y, int z, int w) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform4i(location, x, y, z, w);
}

void Shader::setUniform1f(const std::string& name, float x) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform1f(location, x);
}

void Shader::setUniform2f(const std::string& name, float x, float y) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform2f(location, x, y);
}

void Shader::setUniform3f(const std::string& name, float x, float y, float z) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform3f(location, x, y, z);
}

void Shader::setUniform4f(const std::string& name, float x, float y, float z, float w) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform4f(location, x, y, z, w);
}

void Shader::setUniformMatrix4fv(const std::string& name, float data[16]) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, data);
}

//=====GLM overrides===================================================

void Shader::setUniform2i(const std::string& name, glm::ivec2 v) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform2i(location, v.x, v.y);
}

void Shader::setUniform3i(const std::string& name, glm::ivec3 v) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform3i(location, v.x, v.y, v.z);
}

void Shader::setUniform4i(const std::string& name, glm::ivec4 v) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform4i(location, v.x, v.y, v.z, v.w);
}

void Shader::setUniform2f(const std::string& name, glm::vec2 v) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform2f(location, v.x, v.y);
}

void Shader::setUniform3f(const std::string& name, glm::vec3 v) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform3f(location, v.x, v.y, v.z);
}

void Shader::setUniform4f(const std::string& name, glm::vec4 v) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniform4f(location, v.x, v.y, v.z, v.w);
}

void Shader::setUniformMatrix4fv(const std::string& name, glm::mat4 mat) {
    const unsigned int location = getUniformLocation(name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}