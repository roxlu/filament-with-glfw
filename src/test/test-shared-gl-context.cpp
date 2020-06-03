/*
  ---------------------------------------------------------------
  
                                                 oooo
                                                 `888
                  oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo
                  `888""8P d88' `88b  `88b..8P'   888  `888  `888
                   888     888   888    Y888'     888   888   888
                   888     888   888  .o8"'88b    888   888   888
                  d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P'
  
                                                    www.roxlu.com
                                            www.twitter.com/roxlu
  
  ----------------------------------------------------------------

  FILAMENT WITH GLFW
  ===================

  GENERAL INFO:

    This example shows how you can use Filament with GLFW. We
    create a Window using GLFW and let Filament create an OpenGL
    (4.1) context. When you set `USE_GL` to 1, we will request
    GLFW to create another OpenGL context. When GLFW creates an
    OpenGL context we pass this into Filament so that it can
    setup a shared OpenGL context.

    A shared OpenGL context can make use of textures,
    framebuffers, etc. from another context. This is great when
    we want to use Filament to do the heavy lifting of rendering
    beautifull 3D graphics and still be able to use our own GL
    context that for example is used to render 2D content.

    This example is very limited as it's purpose is to show how
    to integrate Filament into your own application/game/etc. We
    don't create any lights or materials.

  IMPORTANT:
 
    Although Filament can create a shared OpenGL context, it does
    not provide any means to actually retrieve the native handles
    to e.g. FBOs and textures. See [this comment][textureid] on an 
    issue that I created for this feature.

    I created [this branch][featuretex] which adds a `getId()`
    function to textures which gives you the ID for GL based
    textures.
    
  REFERENCES:

    [textureid]: https://github.com/google/filament/issues/1899#issuecomment-555688999
    [featuretex]: https://github.com/roxlu/filament/tree/rox/feature-get-texture-id

 */

#define USE_GL 0

/* -------------------------------------------- */

#if defined(__linux)
#  define USE_GL_LINUX USE_GL
#  define GLFW_EXPOSE_NATIVE_X11
#  define GLFW_EXPOSE_NATIVE_GLX
#endif

#if defined(_WIN32)
#  define USE_GL_WIN USE_GL
#  define GLFW_EXPOSE_NATIVE_WGL
#  define GLFW_EXPOSE_NATIVE_WIN32
#endif

/* -------------------------------------------- */

#if defined(__linux)
#  include <unistd.h>
#endif

/* -------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/SwapChain.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/Viewport.h>
#include <filament/Camera.h>
#include <filament/TransformManager.h>
#include <filameshio/MeshReader.h>
#include <math/mat3.h>
#include <math/mat4.h>
#include <utils/Path.h>
#include <utils/EntityManager.h>

/* -------------------------------------------- */

void button_callback(GLFWwindow* win, int bt, int action, int mods);
void cursor_callback(GLFWwindow* win, double x, double y);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void char_callback(GLFWwindow* win, unsigned int key);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

/* -------------------------------------------- */

uint32_t win_w = 1280;
uint32_t win_h = 720;

/* -------------------------------------------- */

int main(int argc, char* argv[]) {

  glfwSetErrorCallback(error_callback);
  
  if(!glfwInit()) {
    printf("Error: cannot setup glfw.\n");
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 0);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_DECORATED, GL_FALSE);

#if !USE_GL
  printf("Not using a GL context.\n");
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif  
       
  GLFWwindow* win = NULL;

  win = glfwCreateWindow(win_w, win_h, "Filament Shared OpenGL Context", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwSetCharCallback(win, char_callback);
  glfwSetCursorPosCallback(win, cursor_callback);
  glfwSetMouseButtonCallback(win, button_callback);
  glfwSetScrollCallback(win, scroll_callback);

#if USE_GL
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  if (!gladLoadGL()) {
    printf("Cannot load GL.\n");
    exit(1);
  }
#endif
  
  /* -------------------------------------------- */

#if USE_GL  
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_DITHER);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif  

  /* -------------------------------------------- */

  void* main_opengl_context = nullptr;
  void* native_window = nullptr;

#if USE_GL_LINUX
  main_opengl_context = (void*)glfwGetGLXContext(win);
#endif

#if USE_GL_WIN
  main_opengl_context = (void*)glfwGetWGLContext(win);
#endif

#if USE_GL
  if (nullptr == main_opengl_context) {
    printf("Failed to get main opengl context. (exiting)\n");
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(nullptr);
#endif  

  /* 
     Our first step is to create the engine itself, which we use
     in the next couple of lines to create the other base types
     that we need, like a swapchain, renderer, view, etc. We
     create a engine that uses OpenGL as it's backend. This will
     create the correct backend instance e.g. PlatformGLX,
     PlatformCocoaGL, PlatformWGL.
   */
  filament::Engine* fila_engine = filament::Engine::create(
    filament::backend::Backend::OPENGL,
    nullptr,
    main_opengl_context
  );
  
  if (nullptr == fila_engine) {
    printf("Failled to create the filament::Engine. (exiting)\n");
    exit(EXIT_FAILURE);
  }

  /*
    This step hides an important detail: `createSwapChain()`
    expects an X11 Window handle, not an GLXWindow
    handle. GLX/X11 have different handles to window which
    appeared over the years. I'm not going into the details
    but make sure to use **`glfwGetX11Window(win)`** and not 
    the **`glfwGetGLXWindow(win)`**. 
   */
#if defined(__linux)  
  native_window = (void*)glfwGetX11Window(win);
#elif defined(_WIN32)
  native_window = (void*)glfwGetWin32Window(win);
#endif
  
  if (nullptr == native_window) {
    printf("Failed to get the native window. (exiting)\n");
    exit(EXIT_FAILURE);
  }

#if USE_GL
  /*
    When we use a shared GL context, we want Filament to create a
    headless swapchain. On Windows, calling this version of
    `createSwapChain()` will create a new HWND (e.g. using the
    win32 API call `CreateWindowA()`), and this swapchain will be
    used with the GL context that Filament creates.
   */
  filament::SwapChain* fila_swap_chain = fila_engine->createSwapChain(win_w, win_h, 0);
#else
  /*
    Create a swapchain for our native window. We use this
    function when GLFW didn't create a GL context. We use
    Filament to setup the swapchain. On Windows, Filament will
    use the HWND (native_window) and the associated Device
    Context (DC) associated with this HWND. It will choose a
    pixel format for us.
  */
  filament::SwapChain* fila_swap_chain = fila_engine->createSwapChain(native_window);
#endif
  
  if (nullptr == fila_swap_chain) {
    printf("Failed to create the filament::SwapChain. (exiting)\n");
    exit(EXIT_FAILURE);
  }
  
  /*
    Next we create a renderer, scene, view and a camera. These
    are the basic elements that manage the items that we want to
    render using the OpenGL context and swap chain.
   */
  filament::Renderer* fila_renderer = fila_engine->createRenderer();
  if (nullptr == fila_renderer) {
    printf("Failed to create the filament::Renderer. (exiting)\n");
    exit(EXIT_FAILURE);
  }
  
  filament::Scene* fila_scene = fila_engine->createScene();
  if (nullptr == fila_scene) {
    printf("Failed to create the filament::Scene. (exiting)\n");
    exit(EXIT_FAILURE);
  }

  filament::View* fila_view = fila_engine->createView();
  if (nullptr == fila_view) {
    printf("Failed to create the filament::View. (exiting)\n");
    exit(EXIT_FAILURE);
  }

  filament::Camera* fila_cam = fila_engine->createCamera();
  if (nullptr == fila_cam) {
    printf("Failed to create a filament::Camera. (exting)\n");
    exit(EXIT_FAILURE);
  }
  
  /* -------------------------------------------- */

  fila_cam->setExposure(16.0f, 1 / 125.0f, 100.0f);
  fila_cam->setExposure(100.0f);
  fila_cam->setProjection(45.0f, float(win_w)/win_h, 0.1f, 100.0f);
  fila_cam->lookAt({0, 0, 10.0}, {0, 0, 0}, {0, 1, 0});
  
  fila_view->setViewport({0, 0, win_w, win_h});
  fila_view->setScene(fila_scene);
  fila_view->setCamera(fila_cam); /* When we don't set the camera we run into a segfault. */
  fila_view->setName("main-view");

#if defined(__linux)  
  fila_renderer->setClearOptions({
    .clearColor = { 0.0f, 0.13f, 0.0f, 1.0f },
    .clear = true
  });
#endif
  
  /* -------------------------------------------- */

  filamesh::MeshReader::MaterialRegistry material_registry;
  filamesh::MeshReader::Mesh mesh;
  utils::Path mesh_path("./monkey.filamesh");
  
  mesh = filamesh::MeshReader::loadMeshFromFile(
    fila_engine,
    mesh_path,
    material_registry
  );

  fila_scene->addEntity(mesh.renderable);
  
  /* -------------------------------------------- */

  while(!glfwWindowShouldClose(win)) {

#if USE_GL
    glfwMakeContextCurrent(win);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, win_w, win_h);
    glClearColor(0.0f, 0.0f, 0.13f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif    

    if (true == fila_renderer->beginFrame(fila_swap_chain)) {
      fila_renderer->render(fila_view);
      fila_renderer->endFrame();
    }

#if defined(__linux)    
    usleep(16e3);
#endif    

#if USE_GL    
    glfwSwapBuffers(win);
#endif

    glfwPollEvents();
  }

  /* -------------------------------------------- */

  fila_engine->destroy(fila_view);
  fila_engine->destroy(fila_scene);
  fila_engine->destroy(fila_renderer);
  fila_engine->destroy(fila_swap_chain);
  fila_engine->destroy(fila_cam);
  filament::Engine::destroy(&fila_engine);

  fila_view = nullptr;
  fila_scene = nullptr;
  fila_renderer = nullptr;
  fila_swap_chain = nullptr;
  fila_cam = nullptr;
  fila_engine = nullptr;
  
  /* -------------------------------------------- */
      
  glfwTerminate();

  return EXIT_SUCCESS;
}

/* -------------------------------------------- */

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {

  if (GLFW_RELEASE == action) {
    return;
  }
  
  switch(key) {
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
}

void error_callback(int err, const char* desc) {
  printf("GLFW error: %s (%d)\n", desc, err);
}

/* -------------------------------------------- */

void resize_callback(GLFWwindow* window, int width, int height) { }
void cursor_callback(GLFWwindow* win, double x, double y) { }
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { }
void button_callback(GLFWwindow* win, int bt, int action, int mods) { }
void char_callback(GLFWwindow* win, unsigned int key) { }

/* -------------------------------------------- */

