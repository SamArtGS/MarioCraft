

#pragma once
/*-------------------------------------------------------------------*/
/* ---------------- PROYECTO FINAL: MARIOCRAFT ----------------------*/
/*-----------------              2021-2          --------------------*/
/*------------- Alumno: Ricardo Antonio Hernández Torres ------------*/
/*------------- Alumno: Jorge Cárdenas Cárdenas          ------------*/
/*------------- Alumno: Samuel Arturo Garrido Sánchez    ------------*/
#include "MarioCraft.h"


#define SDL_MAIN_HANDLED
#include <SDL.h>
#include "StaticModel.h"
#include "DymanicModel.h"
#include "Kart.h"
#include "JumpAnimation.h"
#include "Dragon.h"
#include "Moneda.h"
#include "creeper.h"
#include "Peach.h"
#include "Golem.h"
#include "MobMinecraft.h"
#include "Vaca.h"
#include "Lighting.h"

// Static declarations
vector<Shader*> MarioCraft::shaders;
Camera* MarioCraft::camera;
GLFWwindow* MarioCraft::window;



// Constructors / Destructors
MarioCraft::MarioCraft(const char* title, const char* icono, int WINDOW_WIDTH, int WINDOW_HEIGHT, bool resizable)
  : WINDOW_WIDTH(WINDOW_WIDTH), WINDOW_HEIGHT(WINDOW_HEIGHT) {

  // Init Variables
  this->window = nullptr;
  this->frameBufferWidth = this->WINDOW_WIDTH;
  this->frameBufferHeight = this->WINDOW_HEIGHT;

  // Init Camara
  this->camera = new Camera(glm::vec3(0.0f, 50.0f, 90.0f));

  this->lastMouseX = 0.0;
  this->lastMouseY = 0.0;
  this->mouseX = 0.0;
  this->mouseY = 0.0;
  this->mouseOffsetX = 0.0;
  this->mouseOffsetY = 0.0;
  this->firstMouse = true;

  this->initGLFW();
  // Validate the screen resolution
  if (this->WINDOW_WIDTH <= 0 || this->WINDOW_WIDTH < 0) this->getResolution();

  this->initWindow(title, icono, resizable);
  this->initGLAD();
  this->initOpenGLOptions();

  this->initShaders();
  this->initSkybox();

  this->initModels();
}

MarioCraft::~MarioCraft() {
  // Terminamos los recursos para el Skybox
  skybox->Terminate();

  glfwDestroyWindow(this->window);
  glfwTerminate();


  // Destruir los Shaders
  for (size_t i = 0; i < this->shaders.size(); i++)
    delete this->shaders[i];
}


// Static functions
// framebuffer_resize_callback establece el tama�o de la ventana
void MarioCraft::framebuffer_resize_callback(GLFWwindow* window, int fbW, int fbH) {
  glViewport(0, 0, fbW, fbH);
}

//Accessor
//
int MarioCraft::getWindowShouldClose() {
  return glfwWindowShouldClose(this->window);
}

//Modifier
// setWindowShouldClose detecta el cierre de ventana
void MarioCraft::setWindowShouldClose() {
  glfwSetWindowShouldClose(this->window, GLFW_TRUE);
}

// scroll_callback se ejecuta cada que la rueda de desplazamiento del mouse de desplaza
void  MarioCraft::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
  MarioCraft::camera->ProcessMouseScroll(yoffset);
}


// Functions
// updateMouseInput reacciona a los eventos del mouse
void MarioCraft::updateMouseInput() {
  glfwGetCursorPos(this->window, &this->mouseX, &this->mouseY);

  if (this->firstMouse) {
    this->lastMouseX = this->mouseX;
    this->lastMouseY = this->mouseY;
    this->firstMouse = false;
  }

  // Calcular offset
  this->mouseOffsetX = this->mouseX - this->lastMouseX;
  this->mouseOffsetY = this->lastMouseY - this->mouseY;

  // Establece last X and Y
  this->lastMouseX = this->mouseX;
  this->lastMouseY = this->mouseY;

  // Reposiciona la camara
  camera->ProcessMouseMovement(mouseOffsetX, mouseOffsetY);

}

// updateKeyboardInput reacciona a los eventos del teclado
void MarioCraft::updateKeyboardInput() {
  // Program
  if (glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(this->window, GLFW_TRUE);
  }

  // Camera
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera->ProcessKeyboard(FORWARD, (float)deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera->ProcessKeyboard(BACKWARD, (float)deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera->ProcessKeyboard(LEFT, (float)deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera->ProcessKeyboard(RIGHT, (float)deltaTime);

  // Models
  models->keyboardInputModels();
}
// updateDeltaTime limita el framerate a los FPS establecidos
void MarioCraft::updateDeltaTime() {
  deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
  //std::cout <<"frame time = " << frameTime << " milli sec"<< std::endl;
  if (deltaTime < LOOP_TIME)
    SDL_Delay((int)(LOOP_TIME - deltaTime));

}

// updateInput reacciona a los eventos de entrada
void MarioCraft::updateInput() {
  glfwPollEvents();

  this->updateKeyboardInput();
  this->updateMouseInput();
}

// update realiza los cambios necesarios antes de renderizar
void MarioCraft::update() {
  this->lastFrame = SDL_GetTicks();

  this->updateInput();
  models->updateModels();
}

//
void MarioCraft::render() {


  // DRAW ---
  // Clear
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Update the uniforms
  update();

  // Render models
  renderSkybox();
  renderLights();
  renderModels();

  // Update Timing
  updateDeltaTime();
  // End Draw
  glfwSwapBuffers(window);
  glFlush();

  /* Reset
  glBindVertexArray(0);
  glUseProgram(0);
  glActiveTexture(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  */
}

// Private functions
//
void MarioCraft::initGLFW() {
  if (glfwInit() == GLFW_FALSE) {
    std::cout << "EROR:GLFW_INIT_FAILED" << "\n";
    glfwTerminate();
  }
}

// getResolution obtiene las dimesiones m�ximas de la pantalla
void MarioCraft::getResolution() {
  const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  this->WINDOW_WIDTH = mode->width;
  this->WINDOW_HEIGHT = (mode->height) - 80;
}

// 
void MarioCraft::initWindow(const char* title, const char* icono, bool resizable) {
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_RESIZABLE, resizable);

  this->window = glfwCreateWindow(this->WINDOW_WIDTH, this->WINDOW_HEIGHT, title, NULL, NULL);

  GLFWimage images[1];
  images[0].pixels = stbi_load(icono, &images[0].width, &images[0].height, 0, 4);
  glfwSetWindowIcon(window, 2, images);
  stbi_image_free(images[0].pixels);
  if (this->window == nullptr) {
    std::cout << "ERROR::GLFW_WINDOW_INIT_FAILED" << "\n";
    glfwTerminate();
  }
  glfwSetWindowPos(window, 0, 30);
  glfwGetFramebufferSize(window, &this->frameBufferWidth, &this->frameBufferHeight);
  glfwSetFramebufferSizeCallback(window, MarioCraft::framebuffer_resize_callback);
  glfwSetScrollCallback(window, MarioCraft::scroll_callback);
  glfwMakeContextCurrent(this->window);
}


// initGLAD cargar todos los punteros de funci�n OpenGL
void MarioCraft::initGLAD() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "ERROR::MAIN.CPP::GLAD_INIT_FAILED" << std::endl;
    glfwTerminate();
  }
}

// initOpenGLOptions configura el estado global de OpenGL
void MarioCraft::initOpenGLOptions() {
  glEnable(GL_DEPTH_TEST);

  //Input
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

//
void MarioCraft::initShaders() {
  // staticShader

  this->shaders.push_back(
    new Shader("Shaders/shader_Lights.vs", "Shaders/shader_Lights.fs")
  );

  // skyboxShader
  this->shaders.push_back(new Shader("Shaders/skybox.vs", "Shaders/skybox.fs"));
}


// initLights inicializa los vectores para las luces
void MarioCraft::initLights() {
  lightPosition = glm::vec3(0.0f, 4.0f, -10.0f);
  lightDirection = glm::vec3(0.0f, -1.0f, -1.0f);

}

// initSkybox inicializa el cubo que generar� el cielo del escenario
void MarioCraft::initSkybox() {
  vector<std::string> faces{
    "resources/skybox/right.jpg",
    "resources/skybox/left.jpg",
    "resources/skybox/top.jpg",
    "resources/skybox/bottom.jpg",
    "resources/skybox/front.jpg",
    "resources/skybox/back.jpg"
  };

  skybox = new Skybox(faces);
}


/*
* A�adir Fuentes de luz Aqu�
*/
// setLights establece las propiedades de las fuentes de luz
void MarioCraft::renderLights() {
  int indice = shader_enum::SHADER_CORE_PROGRAM;
  shaders[indice]->use();

  shaders[indice]->setVec3("pointLight[1].position", glm::vec3(-80.0, 0.0f, 0.0f));
  shaders[indice]->setVec3("pointLight[1].ambient", glm::vec3(1.0f, 0.0f, 0.0f));
  shaders[indice]->setVec3("pointLight[1].diffuse", glm::vec3(1.0f, 0.0f, 0.0f));
  shaders[indice]->setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
  shaders[indice]->setFloat("pointLight[1].constant", 1.0f);
  shaders[indice]->setFloat("pointLight[1].linear", 0.009f);
  shaders[indice]->setFloat("pointLight[1].quadratic", 0.032f);
  shaders[indice]->setFloat("material_shininess", 32.0f);

  glm::mat4 model = glm::mat4(1.0f);
  glm::mat4 tmp = glm::mat4(1.0f);
  // view/projection transformations
  ProjectionMatrix = glm::perspective(
    glm::radians(camera->Zoom),
    (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
    0.1f,
    10000.0f
  );
  ViewMatrix = camera->GetViewMatrix();
  shaders[indice]->setMat4("projection", ProjectionMatrix);
  shaders[indice]->setMat4("view", ViewMatrix);
}

// renderSkybox renderiza el cielo
void MarioCraft::renderSkybox() {
  shaders[shader_enum::SHADER_SKYBOX]->use();
  shaders[shader_enum::SHADER_SKYBOX]->setInt("skybox", 0);
  skybox->Draw(
    *shaders[shader_enum::SHADER_SKYBOX],
    ViewMatrix,
    ProjectionMatrix,
    *camera
  );
}

// renderModels renderiza todos los modelos creados
void MarioCraft::renderModels() {
  models->renderModels();
}

void MarioCraft::initModels() {

    models = new ModelManager();
    float altura = 13.0f;
    float coordenadasCasasToad[11][2] = {
        //Tres a lado de la carretera, alineados, a lado de las gallinas
        {-10.0f, -50.0f},
        {-10.0f, -20.0f},
        {-10.0f,  10.0f},

        //2 en medio de la carretera
        { 70.0f, -95.0f},
        { 40.0f, -45.0f},

        //Mini Villa de Champi�ones 6 juntos en cuadr�cula 3x2
        { 130.0f, -80.0f},
        { 130.0f, -40.0f},
        { 130.0f,  0.0f},
        { 170.0f, -80.0f},
        { 170.0f, -40.0f},
        { 170.0f,  0.0f}
    };

    float coordenadasCasasMaicra1[2][2] = {
        {170.0f, 170.0f},
        { 80.0f, 50.0f}
    };

    float coordenadasArboles[13][2] = {
        {20.0f, -300},
        {-100,  -10.0},
        {-100, -300},
        {60, -400.0f},
        {-100, -350},
        {-170, -320},
        {-100, -50},
        {-170, -50},
        {60, -110},
        {100, -110},
        {110, -50},
        {70, -50 },
        {0, -70}
    };

    //Escenario -------------------------------------------
    //
    StaticModel* escenario = new StaticModel("resources/objects/Escenario/Mariocraft.obj");
    escenario
        ->Init(glm::mat4(1.0f))
        ->Scale(15.0f, 15.0f, 15.0f);
    models->addModel(escenario);

    // Arboles
    //

    // SOL Y LUNA ----------------------------------------------------------------
    //
    Lighting* lighthing = new Lighting(
        "resources/objects/SolLuna", shaders[shader_enum::SHADER_CORE_PROGRAM]
    );
    models->addModel(lighthing, DYNAMIC);

    for (int i = 0; i < sizeof(coordenadasArboles) / sizeof(coordenadasArboles[0]); i++) {
        StaticModel* arbol = new StaticModel("resources/objects/ArbolMinecraft1/arbol.obj");
        arbol
            ->Init(glm::mat4(1.0f))
            ->Translate(coordenadasArboles[i][0], 0.0, coordenadasArboles[i][1])
            ->Scale(15.0f, 15.0f, 15.0f);
        models->addModel(arbol);
    }

    //Castillo -------------------------------------------
    //
    StaticModel* castillo = new StaticModel("resources/objects/Castillo/castillo.obj");
    castillo
        ->Init(glm::mat4(1.0f))
        ->Translate(15.0f, 54.0f, -245.0f)
        ->Scale(65.0f, 65.0f, 65.0f);
    models->addModel(castillo);

    // CasasTOAD -----------------------------------------
    //

    for (int i = 0; i < 11; i++) {
      if(i%2==0){
        StaticModel* casaToad = new StaticModel("resources/objects/CasaToad/casaToadAzul/casatoad.obj");
        casaToad
            ->Init(glm::mat4(1.0f))
            ->Translate(coordenadasCasasToad[i][0], altura, coordenadasCasasToad[i][1])
            ->Scale(7.0f, 7.0f, 7.0f);
        models->addModel(casaToad);
      }else{
        StaticModel* casaToad = new StaticModel("resources/objects/CasaToad/casaToadRoja/casatoad.obj");
        casaToad
            ->Init(glm::mat4(1.0f))
            ->Translate(coordenadasCasasToad[i][0], altura, coordenadasCasasToad[i][1])
            ->Scale(7.0f, 7.0f, 7.0f);
        models->addModel(casaToad);
      }
    }


    StaticModel* casa = new StaticModel("resources/objects/CasaMinecraft2/casa2.obj");
    casa
        ->Init(glm::mat4(1.0f))
        ->Translate(150, 7, 190)
        ->Scale(30.0f, 30.0f, 30.0f);
    models->addModel(casa);

    StaticModel* casa2 = new StaticModel("resources/objects/CasaMinecraft1/casa.obj");
    casa2
        ->Init(glm::mat4(1.0f))
        ->Translate(150, 36.2, -180)

        ->Scale(2.0f, 2.0f, 2.0f);
    models->addModel(casa2);

    StaticModel* casa3 = new StaticModel("resources/objects/CasaMinecraft1/casa.obj");
    casa3
        ->Init(glm::mat4(1.0f))
        ->Translate(-150, 36.2, -180)
        ->Scale(2.0f, 2.0f, 2.0f);
    models->addModel(casa3);


    StaticModel* casa4 = new StaticModel("resources/objects/CasaMinecraft4/fortaleza.obj");
    casa4
        ->Init(glm::mat4(1.0f))
        ->Translate(160, 9.7, 120)
        ->Scale(5.5f, 5.5, 5.5);
    models->addModel(casa4);

    StaticModel* casaFortaleza = new StaticModel("resources/objects/CasaMinecraft4/fortaleza.obj");
    casaFortaleza
        ->Init(glm::mat4(1.0f))
        ->Translate(-70, 9.7, -45)
        ->Rotate(90, 0, 1,0)
        ->Scale(5.5f, 5.5, 5.5);
    models->addModel(casaFortaleza);


    StaticModel* casa5 = new StaticModel("resources/objects/CasaMinecraft3/casa.obj");
    casa5
        ->Init(glm::mat4(1.0f))
        ->Translate(-160, 2, -50)
        ->Scale(4.0f, 4.0f, 4.0f);
    models->addModel(casa5);

    StaticModel* casa55 = new StaticModel("resources/objects/CasaMinecraft3/casa.obj");
    casa55
        ->Init(glm::mat4(1.0f))
        ->Translate(-35, 2, 75)
      ->Scale(4.0f, 4.0f, 4.0f);
  models->addModel(casa55);


  StaticModel* casa6 = new StaticModel("resources/objects/CasaMinecraft3/casa.obj");
  casa6
      ->Init(glm::mat4(1.0f))
      ->Translate(60, 2, 190)
      ->Scale(4.0f, 4.0f, 4.0f);
  models->addModel(casa6);

  StaticModel* casa7 = new StaticModel("resources/objects/CasaMinecraft3/casa.obj");
  casa7
      ->Init(glm::mat4(1.0f))
      ->Translate(100, 2, 190)
      ->Scale(4.0f, 4.0f, 4.0f);
  models->addModel(casa7);

  Kart* kart1 = new Kart("resources/objects/Karts/Kart1/kart1.obj");
  kart1
    ->Init(glm::mat4(1.0f))
    ->Translate(13.0f, 12.5f, 120.0f)
    ->Rotate(172.0f, 0.0f, 1.0f, 0.0f)
    ->Scale(1.0f, 1.0f, 1.0f);
  kart1->velocidad = 0.5f;
  models->addModel(kart1, DYNAMIC);



  Kart* kart2 = new Kart("resources/objects/Karts/Kart2/kart2.obj");
  kart2
    ->Init(glm::mat4(1.0f))
    ->Translate(10.0f, 12.5f, 120.0f)
    ->Rotate(172.0f, 0.0f, 1.0f, 0.0f)
    ->Scale(1.0f, 1.0f, 1.0f);
  kart2->velocidad = 0.3f;
  models->addModel(kart2, DYNAMIC);


  Kart* kart3 = new Kart("resources/objects/Karts/Kart3/kart3.obj");
  kart3
    ->Init(glm::mat4(1.0f))
    ->Translate(7.0f, 12.5f, 120.0f)
    ->Rotate(172.0f, 0.0f, 1.0f, 0.0f)
    ->Scale(1.0f, 1.0f, 1.0f);
  kart3->velocidad = 0.7f;
  models->addModel(kart3, DYNAMIC);

  //ESquina superior derecha
  DynamicModel* golem = new Golem("resources/objects/Golem",
      Golem::Tipo::Antropomorfico,
      Golem::TipoMovimiento::sixSag);
  golem
      ->Init(glm::mat4(1.0f))
      ->Translate(140.0f, 10.0f, -110.0f)
      ->Scale(0.3f, 0.3f, 0.3f);
  models->addModel(golem, DYNAMIC);


  //Esquina inferior derecha
  DynamicModel* golem2 = new Golem("resources/objects/Golem",
      Golem::Tipo::Antropomorfico,
      Golem::TipoMovimiento::sixSag);
  golem2
      ->Init(glm::mat4(1.0f))
      ->Translate(50.0f, 10.0f, 150)
      ->Scale(0.3f, 0.3f, 0.3f);
  models->addModel(golem2, DYNAMIC);
  
  
  //Esquina Superior izquierda
  DynamicModel* golem3 = new Golem("resources/objects/Golem",
      Golem::Tipo::Antropomorfico,
      Golem::TipoMovimiento::sixSag);
  golem3
      ->Init(glm::mat4(1.0f))
      ->Translate(-170.0f, 10.0f, -100)
      ->Scale(0.3f, 0.3f, 0.3f);
  models->addModel(golem3, DYNAMIC);

 


  // TOAD -------------------------------------------------------------------
  //
  // TOAD 1
  JumpAnimation* toad1 = new JumpAnimation("resources/objects/Toad", -28.f, 14.2f, 0.f);
  toad1->addEstadoAnimacion(Desplazamiento::INICIO)
    ->addEstadoAnimacion(Desplazamiento::SUR, 0.1, 28)
    ->addEstadoAnimacion(Desplazamiento::GIRO_DERECHA, 1, 90)
    ->addEstadoAnimacion(Desplazamiento::OESTE, 0.1, 33)
    ->addEstadoAnimacion(Desplazamiento::GIRO_DERECHA, 1, 90)
    ->addEstadoAnimacion(Desplazamiento::NORTE, 0.1, 28)
    ->addEstadoAnimacion(Desplazamiento::GIRO_DERECHA, 1, 90)
    ->addEstadoAnimacion(Desplazamiento::ESTE, 0.1, 33)
    ->addEstadoAnimacion(Desplazamiento::GIRO_DERECHA, 1, 90);
  models->addModel(toad1, DYNAMIC);

  // TOAD 2
  JumpAnimation* toad2 = new JumpAnimation("resources/objects/Toad", -150.f, 14.2f, -80.f, 90);
  //toad2->setEstadosAnimacion(toad1->animaciones); // Misma secuencia de animacion que toad1
  toad2->addEstadoAnimacion(Desplazamiento::INICIO)
    ->addEstadoAnimacion(Desplazamiento::ESTE, 0.2, 100)
    ->addEstadoAnimacion(Desplazamiento::GIRO_DERECHA, 2.f, 180)
    ->addEstadoAnimacion(Desplazamiento::OESTE, 0.2, 100)
    ->addEstadoAnimacion(Desplazamiento::GIRO_IZQUIERDA, 2.f, 180);
  models->addModel(toad2, DYNAMIC);

  // DRAGONES ---------------------------------------------------------------
  //
  // DRAGON 1
  Dragon * dragon1 = new Dragon("resources/objects/Dragon", -28.0f, 50.f, 0.f);
  dragon1->addEstadoAnimacion(MoviemientosDragon::INICIO)
    ->addEstadoAnimacion(MoviemientosDragon::SUR, 0.5f, 100)
    ->addEstadoAnimacion(MoviemientosDragon::GIRO_DERECHA, 1.f, 90)
    ->addEstadoAnimacion(MoviemientosDragon::OESTE, 0.5f, 100)
    ->addEstadoAnimacion(MoviemientosDragon::GIRO_DERECHA, 1.f, 90)
    ->addEstadoAnimacion(MoviemientosDragon::NORTE, 0.5f, 100)
    ->addEstadoAnimacion(MoviemientosDragon::GIRO_DERECHA, 1.f, 90)
    ->addEstadoAnimacion(MoviemientosDragon::ESTE, 0.5f, 100)
    ->addEstadoAnimacion(MoviemientosDragon::GIRO_DERECHA, 1.f, 90);
  models->addModel(dragon1, DYNAMIC);

  // DRAGON 2
  Dragon* dragon2 = new Dragon("resources/objects/Dragon", -75.0f, 70.f, -140.f, 90.f);
  dragon2->addEstadoAnimacion(MoviemientosDragon::INICIO)
    ->addEstadoAnimacion(MoviemientosDragon::ESTE, 0.5f, 150)
    ->addEstadoAnimacion(MoviemientosDragon::GIRO_IZQUIERDA, 1.f, 180)
    ->addEstadoAnimacion(MoviemientosDragon::OESTE, 0.5f, 150)
    ->addEstadoAnimacion(MoviemientosDragon::GIRO_DERECHA, 1.f, 180);
  models->addModel(dragon2, DYNAMIC);

  // MONEDAS ----------------------------------------------------------------
  //
  DynamicModel* moneda;
  for (int i = 0; i < 10; i++) {
    moneda = new Moneda("resources/objects/Moneda2/Moneda.obj");
    moneda->Init(glm::mat4(1.0f))
      ->Translate(20.f, 13.f, -50.f + 5 * i)
      ->Scale(5.f, 5.f, 5.f);
    models->addModel(moneda, DYNAMIC);
  }
  // Creeper -------------------------------------------------------------------
    //
  DynamicModel* creeper = new Creeper("resources/objects/Creeper");
  models->addModel(creeper, DYNAMIC);

  // Peach -------------------------------------------------------------------
  //
  DynamicModel* peach = new Peach("resources/objects/Peach");
  models->addModel(peach, DYNAMIC);
}