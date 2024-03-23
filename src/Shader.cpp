#include "Shader.h"

#include "glad/glad.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

#include <iostream>
#include <cctype>

#define ENABLE_UNIFORM_DEBUG_LOGGING

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

void Shader::RetrieveActiveUniforms(const std::vector<std::string>& source_names)
{
    int count;
    glGetProgramiv(m_ID, GL_ACTIVE_UNIFORMS, &count);

    for (int i = 0; i < count; i++)
    {
        int size;
        uint32_t type;

        constexpr size_t max_name_length = 32;
        char c_name[max_name_length];
        GLsizei name_length;

        glGetActiveUniform(
            m_ID, static_cast<uint32_t>(i), max_name_length, 
            &name_length, &size, &type, c_name
        );

        //Location can be different than 'i' index, 
        //so this call is needed
        int location = glGetUniformLocation(m_ID, c_name);

        const std::string name(c_name, name_length);

        const ShaderUniformInfo info{location, type, size};

        //This will be called within Build(), 
        //so shader cache can be assumed to be clear
        m_UniformCache.insert({name, info});
    }

    for (const auto& name : source_names)
        if (m_UniformCache.count(name) == 0)
        {
            std::cout << "Warning:" << '\n';
            LogFilepaths();
            std::cout << "Uniform " << name << " is present in source, but not active (possibly dead code)" << '\n';
        }
}

int Shader::getUniformLocation(const std::string& name)
{
    if (m_UniformCache.count(name))
        return m_UniformCache[name].Location;

    else
    {
#ifdef ENABLE_UNIFORM_DEBUG_LOGGING
        std::cerr << "Error: " << '\n';
        LogFilepaths();
        std::cerr << "Uniform " << name << " does not exist" << '\n';
#endif

        return -1;
    }
}

//Program type is assumed to be gl enum: {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_COMPUTE_SHADER}
static void compileShaderCode(const std::string& source, uint32_t& id, int program_type)
{
    const char* source_c = source.c_str();

    id = glCreateShader(program_type);

    glShaderSource(id, 1, &source_c, NULL);
    glCompileShader(id);

    int success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success) 
    {
        char info_log[512];
        glGetShaderInfoLog(id, 512, NULL, info_log);
        glDeleteShader(id);

        throw std::runtime_error(info_log);
    }
}

static bool isspace(char ch)
{
    return std::isspace(static_cast<unsigned char>(ch));
}

static std::string loadSource(std::filesystem::path filepath, 
        std::vector<std::string>& uniform_names, 
        bool recursive_call = false)
{
    const std::string include_token{"#include"};
    const std::string uniform_token{"uniform"};

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
        //Save names of all uniform variables
        auto uniform_pos = current_line.find(uniform_token);

        if (uniform_pos != std::string::npos)
        {
            bool commented_out = false;

            auto comment_id1 = current_line.find("//");
            if (comment_id1 != std::string::npos && comment_id1 < uniform_pos)
                commented_out = true;

            auto comment_id2 = current_line.find("/*");
            if (comment_id2 != std::string::npos && comment_id2 < uniform_pos)
                commented_out = true;

            if (!commented_out)
            {
                auto start_id = uniform_pos + uniform_token.size();

                while(isspace(current_line[start_id])) start_id++;
                while(!isspace(current_line[start_id])) start_id++;
                while(isspace(current_line[start_id])) start_id++;

                auto end_id = start_id;
                while(!isspace(current_line[end_id]) && current_line[end_id] != ';') end_id++;

                uniform_names.push_back(current_line.substr(start_id, end_id - start_id));
            }
        }

        //Recursively read from other files if '#include' is present
        if (current_line.find(include_token) != std::string::npos)
        {
            auto start_id = current_line.find_first_of('\"') + 1;
            auto end_id = current_line.find_last_of('\"');
            auto length = end_id - start_id;

            std::string filename = current_line.substr(start_id, length);

            std::filesystem::path new_path = filepath.remove_filename() / filename;

            full_source += loadSource(new_path, uniform_names, true);
        }

        //Append current line to source code
        else
        {
            full_source += current_line + "\n";
        }
    }

    //Add null terminator at the end of file
    if (!recursive_call)
        full_source += "\0";

    return full_source;
}

void VertFragShader::Build()
{
    std::filesystem::path current_path{ std::filesystem::current_path() };

    uint32_t vert_id = 0, frag_id = 0;
    std::vector<std::string> uniform_names;

    try 
    {
        std::string vert_code = loadSource(current_path / m_VertPath, uniform_names);
        compileShaderCode(vert_code, vert_id, GL_VERTEX_SHADER);
    }

    catch (const std::runtime_error& e) 
    {
        std::cerr << "Vertex Shader compilation failed: \n"
                  << "Filepath: " << m_VertPath << '\n'
                  << "Error:" << e.what() << '\n';
        return;
    }

    try 
    {
        std::string frag_code = loadSource(current_path / m_FragPath, uniform_names);
        compileShaderCode(frag_code, frag_id, GL_FRAGMENT_SHADER);
    }

    catch (const std::runtime_error& e) 
    {
        std::cerr << "Fragment Shader compilation failed: \n"
                  << "Filepath: " << m_FragPath << '\n'
                  << "Error: " << e.what() << '\n';
        return;
    }

    //Link program
    m_ID = glCreateProgram();

    glAttachShader(m_ID, vert_id);
    glAttachShader(m_ID, frag_id);
    glLinkProgram(m_ID);

    int success = 0;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);

    if (!success) 
    {
        char info_log[512];
        glGetProgramInfoLog(m_ID, 512, NULL, info_log);
        std::cerr << "Error: Shader program linking failed: \n"
                  << "Filepaths: " << m_VertPath << ", " << m_FragPath << '\n'
                  << "Info: " << info_log << '\n';
    }

    glDeleteShader(vert_id);
    glDeleteShader(frag_id);

    RetrieveActiveUniforms(uniform_names);
}

void VertFragShader::LogFilepaths()
{
    std::cerr << "Filepaths: " << m_VertPath << ", " << m_FragPath << '\n';
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
    std::filesystem::path current_path{ std::filesystem::current_path() };
        
    uint32_t compute_id = 0;
    std::vector<std::string> uniform_names;

    try 
    {
        std::string compute_code = loadSource(current_path / m_ComputePath, uniform_names);
        RetrieveLocalSizes(compute_code);
        compileShaderCode(compute_code, compute_id, GL_COMPUTE_SHADER);
    }

    catch (const std::runtime_error& e) 
    {
        std::cerr << "Compute Shader compilation failed: \n"
                  << "Filepath: " << m_ComputePath << '\n'
                  << "Error: " << e.what() << '\n';
        return;
    }

    //Link program
    m_ID = glCreateProgram();
    glAttachShader(m_ID, compute_id);
    glLinkProgram(m_ID);

    int success = 0;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    
    if (!success) 
    {
        char info_log[512];
        glGetProgramInfoLog(m_ID, 512, NULL, info_log);
        std::cerr << "Error: Shader program linking failed: \n"
                  << "Filepath: " << m_ComputePath << '\n'
                  << "Info: " << info_log << '\n';
    }

    glDeleteShader(compute_id);

    RetrieveActiveUniforms(uniform_names);
}

void ComputeShader::LogFilepaths()
{
    std::cerr << "Filepath: " << m_ComputePath << '\n';
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

void Shader::setUniform1i(const std::string& name, int x) 
{
    const int location = getUniformLocation(name);
    glUniform1i(location, x);
}

void Shader::setUniform2i(const std::string& name, int x, int y) 
{
    const int location = getUniformLocation(name);
    glUniform2i(location, x, y);
}

void Shader::setUniform3i(const std::string& name, int x, int y, int z) 
{
    const int location = getUniformLocation(name);
    glUniform3i(location, x, y, z);
}

void Shader::setUniform4i(const std::string& name, int x, int y, int z, int w) 
{
    const int location = getUniformLocation(name);
    glUniform4i(location, x, y, z, w);
}

void Shader::setUniform1f(const std::string& name, float x) 
{
    const int location = getUniformLocation(name);
    glUniform1f(location, x);
}

void Shader::setUniform2f(const std::string& name, float x, float y) 
{
    const int location = getUniformLocation(name);
    glUniform2f(location, x, y);
}

void Shader::setUniform3f(const std::string& name, float x, float y, float z) 
{
    const int location = getUniformLocation(name);
    glUniform3f(location, x, y, z);
}

void Shader::setUniform4f(const std::string& name, float x, float y, float z, float w) 
{
    const int location = getUniformLocation(name);
    glUniform4f(location, x, y, z, w);
}

void Shader::setUniformMatrix4fv(const std::string& name, float data[16]) 
{
    const int location = getUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, data);
}

//=====GLM overrides===================================================

void Shader::setUniform2i(const std::string& name, glm::ivec2 v) 
{
    const int location = getUniformLocation(name);
    glUniform2i(location, v.x, v.y);
}

void Shader::setUniform3i(const std::string& name, glm::ivec3 v) 
{
    const int location = getUniformLocation(name);
    glUniform3i(location, v.x, v.y, v.z);
}

void Shader::setUniform4i(const std::string& name, glm::ivec4 v) 
{
    const int location = getUniformLocation(name);
    glUniform4i(location, v.x, v.y, v.z, v.w);
}

void Shader::setUniform2f(const std::string& name, glm::vec2 v) 
{
    const int location = getUniformLocation(name);
    glUniform2f(location, v.x, v.y);
}

void Shader::setUniform3f(const std::string& name, glm::vec3 v) 
{
    const int location = getUniformLocation(name);
    glUniform3f(location, v.x, v.y, v.z);
}

void Shader::setUniform4f(const std::string& name, glm::vec4 v) 
{
    const int location = getUniformLocation(name);
    glUniform4f(location, v.x, v.y, v.z, v.w);
}

void Shader::setUniformMatrix4fv(const std::string& name, glm::mat4 mat) 
{
    const int location = getUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
}