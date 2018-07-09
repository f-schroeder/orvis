#include "Shader.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include "Util.hpp"

std::shared_ptr<ShaderFile> ShaderFile::load(const std::experimental::filesystem::path& path)
{
    auto absolute_path = util::shadersPath / path.string();

    std::experimental::filesystem::path realPath;
    if(std::experimental::filesystem::exists(path))
        realPath = path;
    else if(std::experimental::filesystem::exists(absolute_path))
        realPath = absolute_path;
    else
    {
        std::cerr << "File not found: " << path;
        return nullptr;
    }

    if(const auto it = m_contents.find(realPath.string()); it != m_contents.end())
    {
        return it->second.baseHandle;
    }
    else
    {
        std::cout << "Loading file: " << realPath << "\n";
        FileContent& content = m_contents[realPath.string()];
        // must be used because of the private constructor.
        content.baseHandle = std::shared_ptr<ShaderFile>(new ShaderFile(realPath));
        return content.baseHandle;
    }
}

ShaderFile::ShaderFile(const std::experimental::filesystem::path& path)
        : m_path(path)
{
    reload();
}

std::unordered_map<std::string, ShaderFile::FileContent> ShaderFile::m_contents;
void                                                     ShaderFile::reload()
{
    static std::mutex            mtx;
    std::unique_lock<std::mutex> lock(mtx);
    if(std::experimental::filesystem::last_write_time(m_path) != m_timestamp)
    {
        if(!std::experimental::filesystem::exists(m_path))
        {
            throw std::invalid_argument("File not found: " + m_path.string());
        }
        std::cout << "Reloading file: " << m_path << "\n";

        std::ifstream in(m_path.string());
        m_timestamp = std::experimental::filesystem::last_write_time(m_path);
        m_source =
                std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    }
}

const std::string& ShaderFile::contents() const { return m_source; }

Shader::Shader(GLenum type, const ShaderSource& source, std::vector<glsp::definition> definitions)
        : Shader(type, std::vector<ShaderSource>{source}, definitions)
{
}

Shader::Shader(GLenum type, const std::vector<ShaderSource>& sources, std::vector<glsp::definition> definitions)
    : m_type(type)
    , m_handle(glCreateShader(m_type))
    , m_definitions(definitions)
{
    for(const auto& source : sources)
        m_sources.emplace_back(source);

    reload(true);
}

Shader::~Shader()
{
    if(glIsShader(m_handle))
        glDeleteShader(m_handle);
}

Shader::Shader(const Shader& other)
        : Shader(other.type(), other.m_sources)
{
    reload();
}

Shader::Shader(Shader&& other)
        : m_type(other.type())
        , m_handle(other.id())
        , m_sources(std::move(other.m_sources))
{
    other.m_handle = 0;
}

Shader& Shader::operator=(const Shader& other)
{
    if(!glIsShader(m_handle))
        m_handle = glCreateShader(m_type);
    m_type    = other.type();
    m_sources = other.m_sources;
    reload();
    return *this;
}

Shader& Shader::operator=(Shader&& other)
{
    if(glIsShader(m_handle))
        glDeleteShader(m_handle);
    m_type         = other.type();
    m_handle       = other.m_handle;
    m_sources      = std::move(other.m_sources);
    other.m_handle = 0;
    return *this;
}

bool Shader::reload(bool checkStatus)
{
    struct Inflater
    {
        Inflater(Shader& parent)
                : parent(parent)
        {
        }
        void operator()(std::string& str)
        {
            std::string processedSource = glsp::preprocess_source(str, "Type: " + std::to_string(static_cast<int>(parent.type())) + " ID: " + std::to_string(parent.id()), { util::shadersPath }, parent.m_definitions).contents;
            const auto cStrSource = new char[processedSource.length()];
            strcpy(cStrSource, processedSource.c_str());
            begin_pointers.emplace_back(cStrSource);
            begin_pointer_lengths.emplace_back(static_cast<int>(processedSource.length()));
        }
        void operator()(std::shared_ptr<ShaderFile>& file)
        {
            file->reload();
            std::string processedSource = glsp::preprocess_source(file->contents(), "Type: " + std::to_string(static_cast<int>(parent.type())) + " ID: " + std::to_string(parent.id()), { util::shadersPath }, parent.m_definitions).contents;
            const auto cStrSource = new char[processedSource.length()];
            strcpy(cStrSource, processedSource.c_str());
            begin_pointers.emplace_back(cStrSource);
            begin_pointer_lengths.emplace_back(static_cast<int>(processedSource.length()));
        }
        Shader&                  parent;
        std::vector<const char*> begin_pointers;
        std::vector<int>         begin_pointer_lengths;
    } inflater(*this);

    for(auto&& source : m_sources)
        std::visit(inflater, source);   

    glShaderSource(m_handle,
                   static_cast<GLsizei>(inflater.begin_pointers.size()),
                   inflater.begin_pointers.data(),
                   inflater.begin_pointer_lengths.data());
    glCompileShader(m_handle);

    if(checkStatus)
    {
        GLint result;
        glGetShaderiv(m_handle, GL_COMPILE_STATUS, &result);
        if(GL_FALSE == result)
        {
            GLint logLen;
            glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &logLen);
            if(logLen > 0)
            {
                std::string log;
                log.resize(logLen);
                GLsizei written;
                glGetShaderInfoLog(m_handle, logLen, &written, &log[0]);
                std::cout << "Shader log: " << log << std::endl;
            }
            return false;
        }
    }
    return true;
}

GLenum Shader::type() const { return m_type; }

GLuint Shader::id() const { return m_handle; }

std::mutex            Program::m_programMutex;
std::vector<Program*> Program::m_allPrograms;
Program::Program()
        : m_handle(glCreateProgram())
        , m_linked(false)
{
    std::unique_lock<std::mutex> lock(m_programMutex);
    m_allPrograms.emplace_back(this);
}
Program::~Program()
{
    if(glIsProgram(m_handle))
        glDeleteProgram(m_handle);

    std::unique_lock<std::mutex> lock(m_programMutex);
    if(auto it = std::find(m_allPrograms.begin(), m_allPrograms.end(), this);
       it != m_allPrograms.end())
        m_allPrograms.erase(it);
}

bool Program::reloadAll(bool checkStatus)
{
    const auto                   timeBefore = std::chrono::high_resolution_clock::now();
    std::unique_lock<std::mutex> lock(m_programMutex);
    bool                         success = true;
    std::cout << "Reloading all Programs...\n";
    for(auto&& program_ptr : m_allPrograms)
    {
        success &= program_ptr->reload(checkStatus);
    }
    if(success)
        std::cout << "All programs reloaded successfully. ("
                  << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                             std::chrono::high_resolution_clock::now() - timeBefore)
                             .count()
                  << " ms)\n";
    else
        std::cerr << "Some programs could not be reloaded successfully. ("
                  << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
                             std::chrono::high_resolution_clock::now() - timeBefore)
                             .count()
                  << " ms)\n";
    return success;
}

const std::shared_ptr<Shader>& Program::attachNew(GLenum type, const Shader::ShaderSource& source)
{
    return attach(std::make_shared<Shader>(type, source));
}

const std::shared_ptr<Shader>& Program::attachNew(GLenum                                   type,
                                                  const std::vector<Shader::ShaderSource>& sources)
{
    return attach(std::make_shared<Shader>(type, sources));
}

const std::shared_ptr<Shader>& Program::attach(const std::shared_ptr<Shader>& shader)
{
    glAttachShader(m_handle, shader->id());
    return m_shaders[shader->type()] = shader;
}

bool Program::link(bool checkStatus) const
{
    m_linked = false;
    glLinkProgram(m_handle);
    if(checkStatus)
    {
        GLint status;
        glGetProgramiv(m_handle, GL_LINK_STATUS, &status);
        if(GL_FALSE == status)
        {
            GLint logLen;
            glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &logLen);
            if(logLen > 0)
            {
                std::string log;
                log.resize(logLen);
                GLsizei written;
                glGetProgramInfoLog(m_handle, logLen, &written, &log[0]);
                std::cout << "Program log: " << log << std::endl;
            }
            return false;
        }
    }
    m_linked = true;
    return true;
}

Program::Program(const Program& other)
        : Program()
{
    for(const auto& shader : other.m_shaders)
        attach(shader.second);
    if(other.m_linked)
        link();
}

Program::Program(Program&& other)
        : m_handle(other.m_handle)
        , m_shaders(std::move(other.m_shaders))
        , m_linked(other.m_linked)
{
    other.m_handle = 0;
    std::unique_lock<std::mutex> lock(m_programMutex);
    m_allPrograms.emplace_back(this);
}

Program& Program::operator=(const Program& other)
{
    if(!glIsProgram(m_handle))
        m_handle = glCreateProgram();
    for(const auto& shader : other.m_shaders)
        attach(shader.second);
    if(other.m_linked)
        link();
    return *this;
}

Program& Program::operator=(Program&& other)
{
    if(glIsProgram(m_handle))
        glDeleteProgram(m_handle);
    m_handle       = other.m_handle;
    m_shaders      = std::move(other.m_shaders);
    m_linked       = other.m_linked;
    other.m_handle = 0;
    return *this;
}

void Program::use() const
{
    if(!m_linked)
        link();
    glUseProgram(m_handle);
}
GLuint Program::id() const { return m_handle; }
bool   Program::reload(bool checkStatus)
{
    if(m_handle == 0)
        return false;

    bool success = true;
    for(auto&& pair : m_shaders)
    {
        success = success && pair.second->reload(checkStatus);
    }
    return success && link(checkStatus);
}
