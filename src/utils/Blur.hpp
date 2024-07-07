#include <GL/glew.h>
#include <imgui.h>
#include <iostream>

// Shader kaynak kodları
const char *vertex_shader_src = R"(
#version 330 core
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = inTexCoords;
    gl_Position = vec4(inPos, 0.0, 1.0);
}
)";

const char *fragment_shader_src = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D image;
uniform bool horizontal;

void main()
{
    float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec2 tex_offset = 1.0 / textureSize(image, 0); // Texture size
    vec3 result = texture(image, TexCoords).rgb * weight[0]; // Current pixel

    if (horizontal)
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for (int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    FragColor = vec4(result, 1.0);
}
)";

class ImGuiBlur {
public:
  ImGuiBlur() : m_shaderProgram(0), m_quadVAO(0), m_quadVBO(0) {
    // Shader programı oluşturma
    m_shaderProgram =
        CreateShaderProgram(vertex_shader_src, fragment_shader_src);

    // Quad oluşturma
    SetupQuad();
  }

  ~ImGuiBlur() {
    // OpenGL kaynaklarını temizleme
    glDeleteBuffers(1, &m_quadVBO);
    glDeleteVertexArrays(1, &m_quadVAO);
    glDeleteProgram(m_shaderProgram);
  }

  // ImGui drawlist içinde blur efekti uygulama
  void ApplyBlur(ImDrawList *drawList, ImTextureID textureID, ImVec2 size,
                 ImVec2 uv0, ImVec2 uv1, bool horizontal) {
    // Shader programını kullanarak texture'ı render etme
    glUseProgram(m_shaderProgram);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "image"), 0);
    glUniform1i(glGetUniformLocation(m_shaderProgram, "horizontal"),
                horizontal ? GL_TRUE : GL_FALSE);

    // Vertex array ve buffer'ını bağlama
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);

    // Texture ve viewport boyutlarını shader'a geçirme
    GLint textureSizeLoc = glGetUniformLocation(m_shaderProgram, "textureSize");
    glUniform2f(textureSizeLoc, size.x, size.y);

    // Quad'ı render etme
    RenderQuad(size, uv0, uv1);

    // ImGui drawlist'e callback ekleme
    drawList->AddCallback(DrawCallback, reinterpret_cast<void *>(this));
  }

private:
  GLuint m_shaderProgram;
  GLuint m_quadVAO, m_quadVBO;

  // Shader programı oluşturma
  GLuint CreateShaderProgram(const char *vertex_source,
                             const char *fragment_source) {
    GLuint vertexShader = LoadShader(GL_VERTEX_SHADER, vertex_source);
    GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragment_source);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Hata kontrolü
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
      GLchar infoLog[512];
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      std::cerr << "Shader program linking error: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
  }

  // Shader yükleme ve derleme
  GLuint LoadShader(GLenum type, const char *source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Hata kontrolü
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      GLchar infoLog[512];
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }

    return shader;
  }

  // Quad oluşturma
  void SetupQuad() {
    float quadVertices[] = {// positions   // texture Coords
                            -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                            0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                            -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                            1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
  }

  // Quad render etme
  void RenderQuad(ImVec2 size, ImVec2 uv0, ImVec2 uv1) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
  }

  // ImGui callback fonksiyonu
  static void DrawCallback(const ImDrawList *parent_list,
                           const ImDrawCmd *cmd) {
    // auto blurObj = reinterpret_cast<ImGuiBlur *>(cmd->UserCallbackData);
    // if (blurObj) {
    //   // ImGui drawlist içinde blur efekti uygulama
    //   blurObj->RenderQuad(ImVec2(0, 0), ImVec2(0, 0), ImVec2(0, 0));
    // }
  }
};