#include "openglwindow.hpp"

#include <imgui.h>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

void OpenGLWindow::initializeGL() {

  // Load a new font
  /*
  ImGuiIO &io{ImGui::GetIO()};
  const auto filename{getAssetsPath() + "Inconsolata-Medium.ttf"};
  m_font = io.Fonts->AddFontFromFileTTF(filename.c_str(), 50.0f);
  if (m_font == nullptr) {
    throw abcg::Exception{abcg::Exception::Runtime("Cannot load font file")};
  }
  */

  abcg::glClearColor(0, 0, 0, 1);


  // Enable depth buffering
  abcg::glEnable(GL_DEPTH_TEST);

  // Create program
  m_program = createProgramFromFile(getAssetsPath() + "depth.vert",
                                   getAssetsPath() + "depth.frag");
  // Create programs
  /*for (const auto& name : m_shaderNames) {
    const auto program{createProgramFromFile(getAssetsPath() + name + ".vert",
                                             getAssetsPath() + name + ".frag")};
    m_programs.push_back(program);
  }*/
  // Load model
  m_model.loadObj(getAssetsPath() + "box.obj");

  m_model.setupVAO(m_program);

  // Camera at (0,0,0) and looking towards the negative z
  m_viewMatrix =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), 
                  glm::vec3(0.0f, 0.0f, -1.0f),
                  glm::vec3(0.0f, 1.0f, 0.0f));

  // Setup stars
  for (const auto index : iter::range(m_numStars)) {
    auto &position{m_starPositions.at(index)};
    auto &rotation{m_starRotations.at(index)};

    randomizeStar(position, rotation);
  }

  m_ship.loadObj(getAssetsPath() + "ship.obj");
  m_ship.setupVAO(m_program);

  cont_collisions = 5;
  m_shipPosition = glm::vec3(0.0f, 0.0f, 0.0f);
}

void OpenGLWindow::handleEvent(SDL_Event& ev) {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  if (ev.type == SDL_KEYDOWN) {
    if (ev.key.keysym.sym == SDLK_UP || ev.key.keysym.sym == SDLK_w){
      m_shipPosition.y += deltaTime * 1.5f;
    }
    if (ev.key.keysym.sym == SDLK_DOWN || ev.key.keysym.sym == SDLK_s){
      m_shipPosition.y -= deltaTime * 1.5f;
    }
    if (ev.key.keysym.sym == SDLK_LEFT || ev.key.keysym.sym == SDLK_a){
      m_shipPosition.x -= deltaTime * 1.5f;
    }
    if (ev.key.keysym.sym == SDLK_RIGHT || ev.key.keysym.sym == SDLK_d){
      m_shipPosition.x += deltaTime * 1.5f;
    }
      /*
        const GLint modelMatrixLoc{abcg::glGetUniformLocation(m_program, "modelMatrix")};

        glm::mat4 modelMatrixShip{1.0f};
        modelMatrixShip = glm::translate(modelMatrixShip, glm::vec3(0.0f, 1.0f, 0.0f));

        abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrixShip[0][0]);
        m_ship.render();
      */
    
  }

}

void OpenGLWindow::randomizeStar(glm::vec3 &position, glm::vec3 &rotation) {
  // Get random position
  // x and y coordinates in the range [-20, 20]
  // z coordinates in the range [-100, 0]
  std::uniform_real_distribution<float> distPosXY(-20.0f, 20.0f);
  std::uniform_real_distribution<float> distPosZ(-100.0f, 0.0f);

  position = glm::vec3(distPosXY(m_randomEngine), distPosXY(m_randomEngine),
                       distPosZ(m_randomEngine));

  //  Get random rotation axis
  std::uniform_real_distribution<float> distRotAxis(-1.0f, 1.0f);

  rotation = glm::normalize(glm::vec3(distRotAxis(m_randomEngine),
                                      distRotAxis(m_randomEngine),
                                      distRotAxis(m_randomEngine)));
}

void OpenGLWindow::paintGL() {
  update();

  // Clear color buffer and depth buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);

  // Get location of uniform variables (could be precomputed)
  const GLint viewMatrixLoc{
      abcg::glGetUniformLocation(m_program, "viewMatrix")};
  const GLint projMatrixLoc{
      abcg::glGetUniformLocation(m_program, "projMatrix")};
  const GLint modelMatrixLoc{
      abcg::glGetUniformLocation(m_program, "modelMatrix")};
  const GLint colorLoc{abcg::glGetUniformLocation(m_program, "color")};
  

  // Set uniform variables used by every scene object
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  abcg::glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);  // White

  // Render each star
  for (const auto index : iter::range(m_numStars)) {
    const auto &position{m_starPositions.at(index)};
    const auto &rotation{m_starRotations.at(index)};

    // Compute model matrix of the current star
    glm::mat4 modelMatrix{1.0f};
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.2f));
    modelMatrix = glm::rotate(modelMatrix, m_angle, rotation);

    // Set uniform variable
    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);
    

    m_model.render();
  }

  // Render ship
  glm::mat4 modelMatrixShip{1.0f};

  modelMatrixShip = glm::translate(modelMatrixShip, m_shipPosition);
  modelMatrixShip = glm::scale(modelMatrixShip, glm::vec3(0.07f));

  abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrixShip[0][0]);
  abcg::glUniform4f(colorLoc, 1.0f, 0.25f, 0.25f, 1.0f);
  m_ship.render();

  abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  {
    const auto widgetSize{ImVec2(218, 100)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Widget window", nullptr, ImGuiWindowFlags_NoDecoration);

    {
      ImGui::PushItemWidth(120);
      static std::size_t currentIndex{};
      const std::vector<std::string> comboItems{"Perspective", "Orthographic"};

      if (ImGui::BeginCombo("Projection",comboItems.at(currentIndex).c_str())) {
        for (const auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      ImGui::PushItemWidth(170);
      const auto aspect{static_cast<float>(m_viewportWidth) /
                        static_cast<float>(m_viewportHeight)};
      ImGui::Text("Colis??es restantes: %d", cont_collisions);
      if (currentIndex == 0) {
        m_projMatrix = glm::perspective(glm::radians(m_FOV), aspect, 0.01f, 100.0f);

        // m_projMatrix = glm::perspective(glm::radians(180.0f), aspect, 0.1f, 5.0f);

        ImGui::SliderFloat("FOV", &m_FOV, 5.0f, 179.0f, "%.0f degrees");
      } else {
        m_projMatrix = glm::ortho(-20.0f * aspect, 20.0f * aspect, -20.0f,20.0f, 0.01f, 100.0f);
      }

      ImGui::PopItemWidth();
    }

    ImGui::End();
  }

  {
    const auto size{ImVec2(150, 150)};
    const auto position{ImVec2((m_viewportWidth - size.x) / 2.0f,
                               (m_viewportHeight - size.y) / 2.0f)};
    ImGui::SetNextWindowPos(position);
    ImGui::SetNextWindowSize(size);
    ImGuiWindowFlags flags{ImGuiWindowFlags_NoBackground |
                           ImGuiWindowFlags_NoTitleBar |
                           ImGuiWindowFlags_NoInputs};
    ImGui::Begin(" ", nullptr, flags);
    //ImGui::PushFont(m_font);
    if(isLose)
      {
        ImGui::Text(" *Lose!* ");
      }
    //ImGui::PopFont();
    ImGui::End();
  }

}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;
}

void OpenGLWindow::terminateGL() {
  m_model.terminateGL();
  m_ship.terminateGL();
  abcg::glDeleteProgram(m_program);
}

void OpenGLWindow::update() {

  if(isLose && m_restartWaitTimer.elapsed() > 5){
    restart();
    return;
  }

  // Animate angle by 90 degrees per second
  const float deltaTime{static_cast<float>(getDeltaTime())};
  m_angle = glm::wrapAngle(m_angle + glm::radians(90.0f) * deltaTime);

  // Update stars
  for (const auto index : iter::range(m_numStars)) {
    auto &position{m_starPositions.at(index)};
    auto &rotation{m_starRotations.at(index)};

    // Z coordinate increases by 10 units per second
    position.z += deltaTime * 13.0f;

    if(!isLose){
      // If this star is behind the camera, select a new random position and
      // orientation, and move it back to -100
      if (position.z > 0.1f) {
        randomizeStar(position, rotation);
        position.z = -100.0f;  // Back to -100
      }

      // Check Colisions
      if (  (m_shipPosition.x <= position.x + 1.2f && m_shipPosition.x >= position.x - 1.2f)
            && (m_shipPosition.y <= position.y + 1.2f && m_shipPosition.y >= position.y - 1.2f) 
            && (m_shipPosition.z <= position.z + 1.2f && m_shipPosition.z >= position.z - 1.2f)) 
      {
        if(m_collisionTimer.elapsed() > 1){
          cont_collisions = cont_collisions - 1;

          if(cont_collisions == 0){
            isLose = true;
            m_shipPosition.z = 20.0f;
            m_restartWaitTimer.restart();
          }

          m_collisionTimer.restart();
        }
      }
    }else{
      position.z = 20.0f;
    }
  }    
  
}

void OpenGLWindow::restart() {
    isLose = false;
    // terminateGL();
    initializeGL();
}