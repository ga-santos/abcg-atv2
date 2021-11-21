#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <random>

#include "abcg.hpp"
#include "model.hpp"

class OpenGLWindow : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& ev) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  static const int m_numStars{250};

  GLuint m_program{};

  int m_viewportWidth{};
  int m_viewportHeight{};

  std::default_random_engine m_randomEngine;

  Model m_model;

  Model m_ship;

  std::array<glm::vec3, m_numStars> m_starPositions;
  std::array<glm::vec3, m_numStars> m_starRotations;
  float m_angle{};

  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};
  float m_FOV{171.0f};

/*
  // Shaders
  std::vector<const char*> m_shaderNames{"blinnphong", "phong", "depth"};
  std::vector<GLuint> m_programs;
  int m_currentProgramIndex{-1};

*/
  void randomizeStar(glm::vec3 &position, glm::vec3 &rotation);
  void update();

  void restart();
  glm::vec3 m_shipPosition{glm::vec3(0.0f, 0.0f, 0.0f)};
};

#endif