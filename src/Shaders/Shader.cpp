#include "Shader.hpp"

#include <fstream>
#include <vector>

Shader::Shader(const std::string &res, const std::string &v, const std::string &f) :
    Shader(res, v, f, "")
{ }

Shader::Shader(const std::string &res, const std::string &vName, const std::string &fName, const std::string &gName) {
	pid = glCreateProgram();
    if (vName.size() && (vShaderId = compileShader(GL_VERTEX_SHADER, res, vName))) {
	    CHECK_GL_CALL(glAttachShader(pid, vShaderId));
    }
    if (fName.size() && (fShaderId = compileShader(GL_FRAGMENT_SHADER, res, fName))) {
        CHECK_GL_CALL(glAttachShader(pid, fShaderId));
    }
    if (gName.size() && (gShaderId = compileShader(GL_GEOMETRY_SHADER, res, gName))) {
	    CHECK_GL_CALL(glAttachShader(pid, gShaderId));
    }
	CHECK_GL_CALL(glLinkProgram(pid));

    // See whether link was successful
    GLint linkSuccess;
	CHECK_GL_CALL(glGetProgramiv(pid, GL_LINK_STATUS, &linkSuccess));
	if (!linkSuccess) {
        GLSL::printProgramInfoLog(pid);
        std::cout << "Error linking shaders " << vName << " and " << fName;
        if (gShaderId) {
            std::cout << " and " << gName << std::endl;
        }
        else {
            std::cout << std::endl;
        }
        std::cin.get();
        exit(EXIT_FAILURE);
	}

    if (vShaderId) {
        findAttributesAndUniforms(res, vName);
    }
    if (fShaderId) {
        findAttributesAndUniforms(res, fName);
    }
    if (gShaderId) {
        findAttributesAndUniforms(res, gName);
    }
}

GLuint Shader::compileShader(GLenum shaderType, const std::string &res, const std::string &shaderName) {
    // Read the shader source file into a string
    char *shaderString = GLSL::textFileRead((res + shaderName).c_str());
    // Stop if there was an error reading the shader source file
    if (shaderString == NULL) return 0;
    
    // Create the shader, assign source code, and compile it
    GLuint shader = glCreateShader(shaderType);
    CHECK_GL_CALL(glShaderSource(shader, 1, &shaderString, NULL));
    CHECK_GL_CALL(glCompileShader(shader));

    // See whether compile was successful
    GLint compileSuccess;
    CHECK_GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSuccess));
    if (!compileSuccess) {
        GLSL::printShaderInfoLog(shader);
        std::cout << "Error compiling shader: " << res << shaderName << std::endl;
        std::cin.get();
        exit(EXIT_FAILURE);
    }
    
    // Free the memory
    free(shaderString);
    
    return shader;
}

void Shader::findAttributesAndUniforms(const std::string &res, const std::string &shaderName) {
    char *fileText = GLSL::textFileRead((res + shaderName).c_str());
    char *token;
    char *lastToken = nullptr;
    
    std::vector<char *> lines;
    
    // Read the first line
    token = strtok(fileText, ";\n");
    lines.push_back(token);
    // Read all subsequent lines
    while((token = strtok(NULL, ";\n")) != NULL) {
        lines.push_back(token);
    }
    
    // Look for keywords per line
    for (char *line : lines) {
        token = strtok(line, " (\n");
        if (token == NULL) {
            continue;
        }
        if (!strcmp(token, "uniform")) {
            // Handle lines with multiple variables separated by commas
            char *lineEnding = line + strlen(line) + 1;
            int lastDelimiter = -1;
            int lineEndingLength = strlen(lineEnding);
            for (int i = 0; i < lineEndingLength; i++) {
                if (lineEnding[i] == ',') {
                    lineEnding[i] = '\0';
                    addUniform(lineEnding + (lastDelimiter + 1));
                    lastDelimiter = i;
                } else if (lineEnding[i] == ' ' || lineEnding[i] == '\t') {
                    lastDelimiter = i;
                }
            }
            addUniform(lineEnding + (lastDelimiter + 1));
        } 
        else if (!strcmp(token, "layout")) {
            while((token = strtok(NULL, " ")) != NULL) {
                lastToken = token;
            }
            if (lastToken) {
                addAttribute(lastToken);
            }
        } 
        else {
            continue;
        }
    }
    
    // Free the memory
    free(fileText);
}

void Shader::bind() {
    CHECK_GL_CALL(glUseProgram(pid));
}

void Shader::unbind() {
    CHECK_GL_CALL(glUseProgram(0));
}

void Shader::addAttribute(const std::string &name) {
    GLint r = glGetAttribLocation(pid, name.c_str());
    if (r < 0) {
        std::cerr << "WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
    }
    attributes[name] = r;
}

void Shader::addUniform(const std::string &name) {
    GLint r = glGetUniformLocation(pid, name.c_str());
    if (r < 0) {
        std::cerr << "WARN: " << name << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it\n" << std::endl;
    }  
    uniforms[name] = r;
}

GLint Shader::getAttribute(const std::string &name) { 
    std::map<std::string, GLint>::const_iterator attribute = attributes.find(name.c_str());
    if (attribute == attributes.end()) {
        std::cerr << name << " is not an attribute variable" << std::endl;
        return -1;
    }
    return attribute->second;
}

GLint Shader::getUniform(const std::string &name) { 
    std::map<std::string, GLint>::const_iterator uniform = uniforms.find(name.c_str());
    if (uniform == uniforms.end()) {
        std::cerr << name << " is not an uniform variable" << std::endl;
        return -1;
    }
    return uniform->second;
}

void Shader::cleanUp() {
    unbind();
    CHECK_GL_CALL(glDetachShader(pid, vShaderId));
    CHECK_GL_CALL(glDetachShader(pid, fShaderId));
    CHECK_GL_CALL(glDetachShader(pid, gShaderId));
    CHECK_GL_CALL(glDeleteShader(vShaderId));
    CHECK_GL_CALL(glDeleteShader(fShaderId));
    CHECK_GL_CALL(glDeleteShader(gShaderId));
    CHECK_GL_CALL(glDeleteProgram(pid));
}

void Shader::loadBool(const int location, const bool b) const {
    CHECK_GL_CALL(glUniform1i(location, b));
}

void Shader::loadInt(const int location, const int i) const {
    CHECK_GL_CALL(glUniform1i(location, i));
}

void Shader::loadFloat(const int location, const float f) const { 
    CHECK_GL_CALL(glUniform1f(location, f));
}

void Shader::loadVector(const int location, const glm::vec2 & v) const { 
    CHECK_GL_CALL(glUniform2f(location, v.x, v.y));
}

void Shader::loadVector(const int location, const glm::vec3 & v) const { 
    CHECK_GL_CALL(glUniform3f(location, v.x, v.y, v.z));
}

void Shader::loadVector(const int location, const glm::vec4 & v) const { 
    CHECK_GL_CALL(glUniform4f(location, v.r, v.g, v.b, v.a));
}

void Shader::loadMatrix(const int location, const glm::mat4 *m) const { 
    CHECK_GL_CALL(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(*m)));
}

void Shader::loadMatrix(const int location, const glm::mat3 *m) const { 
    CHECK_GL_CALL(glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(*m)));
}
