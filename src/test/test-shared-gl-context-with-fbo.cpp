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

  FILAMENT WITH GLFW + FBO
  ========================

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

    We use Filament to render into a FBO. We use the texture into
    which the scene is rendered with our main OpenGL context that
    GLFW created. This shows how one can use Filament as the
    render engine inside your application that already uses
    OpenGL.

  IMPORTANT:

    This example uses [my rox/feature-get-texture-id][teaturetex] branch
    that allows you to retrieve the native (OpenGL) texture id that a 
    `Filament::Texture` uses. [See this comment][textureid] on an issue
    that I created regarding this.

  REFERENCES:

    [textureid]: https://github.com/google/filament/issues/1899#issuecomment-555688999
    [featuretex]: https://github.com/roxlu/filament/tree/rox/feature-get-texture-id
 
 */

#define USE_GL 1

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
#include <filament/Texture.h>
#include <filament/RenderTarget.h>
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

static int print_shader_compile_info(uint32_t shader); 

/* -------------------------------------------- */

uint32_t win_w = 1280;
uint32_t win_h = 720;

/* -------------------------------------------- */

static const std::string VS = R"(#version 430
  out vec2 v_uv;
  void main() {
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    v_uv.x = (x+1.0)*0.5;
    v_uv.y = (y+1.0)*0.5;
    gl_Position = vec4(x, y, 0, 1);
  }
)";

static const std::string FS = R"(#version 430
  layout (location = 0) uniform sampler2D u_tex;
  layout (location = 0) out vec4 fragcolor;
  in vec2 v_uv;
 
  void main() { 
    fragcolor = vec4(1.0, 0.14, 0.0, 1.0);
    fragcolor.rgb = texture(u_tex, v_uv).rgb;
  }
)";

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
  glEnable(GL_TEXTURE_2D);
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

#if USE_GL  
  
  /* 
     In this example we create a render target to render
     the scene into. Then we use the texture id of this
     render target to render the scene. Note that the 
     `filament::Texture::getId()` function is part of my 
     `rox/feature-get-texture-id` branch of my Filament
     fork.
  */
  filament::Texture* tex_col = filament::Texture::Builder()
    .width(win_w)
    .height(win_h)
    .levels(1)
    .usage(filament::Texture::Usage::COLOR_ATTACHMENT | filament::Texture::Usage::SAMPLEABLE)
    .format(filament::Texture::InternalFormat::RGBA16F)
    .build(*fila_engine);

  if (nullptr == tex_col) {
    printf("Failed to create our color texture for the render target. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  filament::Texture* tex_depth = filament::Texture::Builder()
    .width(win_w)
    .height(win_h)
    .levels(1)
    .usage(filament::Texture::Usage::DEPTH_ATTACHMENT)
    .format(filament::Texture::InternalFormat::DEPTH24)
    .build(*fila_engine);

  if (nullptr == tex_depth) {
    printf("Failed to create our depth texture for the render target. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  filament::RenderTarget::Builder render_target_builder = filament::RenderTarget::Builder();
  render_target_builder.texture(filament::RenderTarget::AttachmentPoint::COLOR, tex_col);
  render_target_builder.texture(filament::RenderTarget::AttachmentPoint::DEPTH, tex_depth);

  filament::RenderTarget* render_target = render_target_builder.build(*fila_engine);
  if (nullptr == render_target) {
    printf("Failed to create the render target. (exiting).\n");
    exit(EXIT_FAILURE);
  }

  fila_view->setRenderTarget(render_target);

  uint32_t tex_col_id = 0;
  tex_col->getId(*fila_engine, (void*)&tex_col_id);

  printf("Texture color id: %u.\n", tex_col_id);
            
  /* -------------------------------------------- */

  /* 
     Create the shader and the necessary GL objects that we use
     to render the result of what Filament renders into the
     framebuffer.
   */
  uint32_t vao = 0;
  uint32_t vert = 0;
  uint32_t frag = 0;
  uint32_t prog = 0;
  const char* vss = VS.c_str();
  const char* fss = FS.c_str();
    
  glfwMakeContextCurrent(win);

  vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vss, nullptr);
  glCompileShader(vert);
  print_shader_compile_info(vert);

  frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &fss, nullptr);
  glCompileShader(frag);
  print_shader_compile_info(frag);

  prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glLinkProgram(prog);

  glGenVertexArrays(1, &vao);
  
#endif /* USE_GL */
    
  /* -------------------------------------------- */

  while(!glfwWindowShouldClose(win)) {

#if USE_GL
    glfwMakeContextCurrent(win);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, win_w, win_h);
    glClearColor(0.0f, 0.6f, 0.13f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif    

    if (true == fila_renderer->beginFrame(fila_swap_chain)) {
      fila_renderer->render(fila_view);
      fila_renderer->endFrame();
    }

#if USE_GL    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_col_id);
    glUseProgram(prog);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
#endif    

#if defined(__linux)    
    usleep(16e3);
#endif    

#if USE_GL    
    glfwSwapBuffers(win);
#endif

    glfwPollEvents();
  }

  /* -------------------------------------------- */

#if USE_GL  
  fila_engine->destroy(tex_col);
  fila_engine->destroy(tex_depth);
  fila_engine->destroy(render_target);
#endif
  
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

/* 
   Checks the compile info, if it didn't compile we return < 0,
   otherwise 0
*/
static int print_shader_compile_info(uint32_t shader) {

  GLint status = 0;
  GLint count = 0;
  GLchar* error = NULL;

  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if(status) {
    return 0;
  }

  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &count);
  if (0 == count) {
    return 0;
  }
    
  error = (GLchar*) malloc(count);
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &count);
  if(count <= 0) {
    free(error);
    error = NULL;
    return 0;
  }

  glGetShaderInfoLog(shader, count, NULL, error);
    
  printf("SHADER COMPILE ERROR\n");
  printf("--------------------------------------------------------\n");
  printf("%s\n", error);
  printf("--------------------------------------------------------\n");

  free(error);
  error = NULL;
    
  return -1;
}

/* -------------------------------------------- */
