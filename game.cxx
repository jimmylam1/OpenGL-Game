#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using std::endl;
using std::cerr;

#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>   // glm::vec3
#include <glm/vec4.hpp>   // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>  // glm::translate, glm::rotate, glm::scale

class RenderManager;
class GameObject;

void        SetUpGame(int, RenderManager &, GameObject, std::vector<GameObject>);
const char *GetVertexShader();
const char *GetFragmentShader();

class Triangle
{
  public:
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
};

std::vector<Triangle> SplitTriangle(std::vector<Triangle> &list)
{
    std::vector<Triangle> output(4*list.size());
    output.resize(4*list.size());
    for (unsigned int i = 0 ; i < list.size() ; i++)
    {
        Triangle t = list[i];
        glm::vec3 vmid1, vmid2, vmid3;
        vmid1 = (t.v0 + t.v1) / 2.0f;
        vmid2 = (t.v1 + t.v2) / 2.0f;
        vmid3 = (t.v0 + t.v2) / 2.0f;
        output[4*i+0].v0 = t.v0;
        output[4*i+0].v1 = vmid1;
        output[4*i+0].v2 = vmid3;
        output[4*i+1].v0 = t.v1;
        output[4*i+1].v1 = vmid2;
        output[4*i+1].v2 = vmid1;
        output[4*i+2].v0 = t.v2;
        output[4*i+2].v1 = vmid3;
        output[4*i+2].v2 = vmid2;
        output[4*i+3].v0 = vmid1;
        output[4*i+3].v1 = vmid2;
        output[4*i+3].v2 = vmid3;
    }
    return output;
}

void PushVertex(std::vector<float>& coords,
                const glm::vec3& v)
{
  coords.push_back(v.x);
  coords.push_back(v.y);
  coords.push_back(v.z);
}

//
// Sets up a cylinder that is the circle x^2+y^2=1 extruded from
// Z=0 to Z=1.
//
void GetCylinderData(std::vector<float>& coords, std::vector<float>& normals)
{
  int nfacets = 30;
  for (int i = 0 ; i < nfacets ; i++)
  {
    double angle = 3.14159*2.0*i/nfacets;
    double nextAngle = (i == nfacets-1 ? 0 : 3.14159*2.0*(i+1)/nfacets);
    glm::vec3 fnormal(0.0f, 0.0f, 1.0f);
    glm::vec3 bnormal(0.0f, 0.0f, -1.0f);
    glm::vec3 fv0(0.0f, 0.0f, 1.0f);
    glm::vec3 fv1(cos(angle), sin(angle), 1);
    glm::vec3 fv2(cos(nextAngle), sin(nextAngle), 1);
    glm::vec3 bv0(0.0f, 0.0f, 0.0f);
    glm::vec3 bv1(cos(angle), sin(angle), 0);
    glm::vec3 bv2(cos(nextAngle), sin(nextAngle), 0);
    // top and bottom circle vertices
    PushVertex(coords, fv0);
    PushVertex(normals, fnormal);
    PushVertex(coords, fv1);
    PushVertex(normals, fnormal);
    PushVertex(coords, fv2);
    PushVertex(normals, fnormal);
    PushVertex(coords, bv0);
    PushVertex(normals, bnormal);
    PushVertex(coords, bv1);
    PushVertex(normals, bnormal);
    PushVertex(coords, bv2);
    PushVertex(normals, bnormal);
    // curves surface vertices
    glm::vec3 v1normal(cos(angle), sin(angle), 0);
    glm::vec3 v2normal(cos(nextAngle), sin(nextAngle), 0);
    //fv1 fv2 bv1
    PushVertex(coords, fv1);
    PushVertex(normals, v1normal);
    PushVertex(coords, fv2);
    PushVertex(normals, v2normal);
    PushVertex(coords, bv1);
    PushVertex(normals, v1normal);
    //fv2 bv1 bv2
    PushVertex(coords, fv2);
    PushVertex(normals, v2normal);
    PushVertex(coords, bv1);
    PushVertex(normals, v1normal);
    PushVertex(coords, bv2);
    PushVertex(normals, v2normal);
  }
}

//
// Sets up a sphere with equation x^2+y^2+z^2=1
//
void
GetSphereData(std::vector<float>& coords, std::vector<float>& normals)
{
  int recursionLevel = 5;
  std::vector<Triangle> list;
  {
    Triangle t;
    t.v0 = glm::vec3(1.0f,0.0f,0.0f);
    t.v1 = glm::vec3(0.0f,1.0f,0.0f);
    t.v2 = glm::vec3(0.0f,0.0f,1.0f);
    list.push_back(t);
  }
  for (int r = 0 ; r < recursionLevel ; r++)
  {
      list = SplitTriangle(list);
  }

  for (int octant = 0 ; octant < 8 ; octant++)
  {
    glm::mat4 view(1.0f);
    float angle = 90.0f*(octant%4);
    if(angle != 0.0f)
      view = glm::rotate(view, glm::radians(angle), glm::vec3(1, 0, 0));
    if (octant >= 4)
      view = glm::rotate(view, glm::radians(180.0f), glm::vec3(0, 0, 1));
    for(int i = 0; i < list.size(); i++)
    {
      Triangle t = list[i];
      float mag_reci;
      glm::vec3 v0 = view*glm::vec4(t.v0, 1.0f);
      glm::vec3 v1 = view*glm::vec4(t.v1, 1.0f);
      glm::vec3 v2 = view*glm::vec4(t.v2, 1.0f);
      mag_reci = 1.0f / glm::length(v0);
      v0 = glm::vec3(v0.x * mag_reci, v0.y * mag_reci, v0.z * mag_reci);
      mag_reci = 1.0f / glm::length(v1);
      v1 = glm::vec3(v1.x * mag_reci, v1.y * mag_reci, v1.z * mag_reci);
      mag_reci = 1.0f / glm::length(v2);
      v2 = glm::vec3(v2.x * mag_reci, v2.y * mag_reci, v2.z * mag_reci);
      PushVertex(coords, v0);
      PushVertex(coords, v1);
      PushVertex(coords, v2);
      PushVertex(normals, v0);
      PushVertex(normals, v1);
      PushVertex(normals, v2);
    }
  }
}

// 
// Sets up a cube with 0 < x < 1, 0 < y < 1, 0 < z < 1
//
void GetCubeData(std::vector<float>& coords, std::vector<float>& normals) {
    glm::vec3 fnormal(0.0f, 0.0f, -1.0f);
    glm::vec3 bnormal(0.0f, 0.0f, 1.0f);
    glm::vec3 lnormal(-1.0f, 0.0f, 0.0f);
    glm::vec3 rnormal(1.0f, 0.0f, 0.0f);
    glm::vec3 unormal(0.0f, 1.0f, 0.0f);
    glm::vec3 dnormal(0.0f, -1.0f, 0.0f);

    glm::vec3 fbl(0.0f, 0.0f, 0.0f);
    glm::vec3 fbr(1.0f, 0.0f, 0.0f);
    glm::vec3 ftl(0.0f, 1.0f, 0.0f);
    glm::vec3 ftr(1.0f, 1.0f, 0.0f);
    glm::vec3 bbl(0.0f, 0.0f, 1.0f);
    glm::vec3 bbr(1.0f, 0.0f, 1.0f);
    glm::vec3 btl(0.0f, 1.0f, 1.0f);
    glm::vec3 btr(1.0f, 1.0f, 1.0f);
    
    // front
    PushVertex(coords, fbl);
    PushVertex(normals, fnormal);
    PushVertex(coords, fbr);
    PushVertex(normals, fnormal);
    PushVertex(coords, ftr);
    PushVertex(normals, fnormal);
    PushVertex(coords, fbl);
    PushVertex(normals, fnormal);
    PushVertex(coords, ftr);
    PushVertex(normals, fnormal);
    PushVertex(coords, ftl);
    PushVertex(normals, fnormal);
    
    // back
    PushVertex(coords, bbl);
    PushVertex(normals, bnormal);
    PushVertex(coords, bbr);
    PushVertex(normals, bnormal);
    PushVertex(coords, btr);
    PushVertex(normals, bnormal);
    PushVertex(coords, bbl);
    PushVertex(normals, bnormal);
    PushVertex(coords, btr);
    PushVertex(normals, bnormal);
    PushVertex(coords, btl);
    PushVertex(normals, bnormal);

    // left
    PushVertex(coords, bbl);
    PushVertex(normals, lnormal);
    PushVertex(coords, fbl);
    PushVertex(normals, lnormal);
    PushVertex(coords, ftl);
    PushVertex(normals, lnormal);
    PushVertex(coords, bbl);
    PushVertex(normals, lnormal);
    PushVertex(coords, ftl);
    PushVertex(normals, lnormal);
    PushVertex(coords, btl);
    PushVertex(normals, lnormal);

    // right
    PushVertex(coords, bbr);
    PushVertex(normals, rnormal);
    PushVertex(coords, fbr);
    PushVertex(normals, rnormal);
    PushVertex(coords, ftr);
    PushVertex(normals, rnormal);
    PushVertex(coords, bbr);
    PushVertex(normals, rnormal);
    PushVertex(coords, ftr);
    PushVertex(normals, rnormal);
    PushVertex(coords, btr);
    PushVertex(normals, rnormal);

    // top
    PushVertex(coords, ftl);
    PushVertex(normals, unormal);
    PushVertex(coords, ftr);
    PushVertex(normals, unormal);
    PushVertex(coords, btr);
    PushVertex(normals, unormal);
    PushVertex(coords, ftl);
    PushVertex(normals, unormal);
    PushVertex(coords, btl);
    PushVertex(normals, unormal);
    PushVertex(coords, btr);
    PushVertex(normals, unormal);

    // bottom
    PushVertex(coords, fbl);
    PushVertex(normals, dnormal);
    PushVertex(coords, fbr);
    PushVertex(normals, dnormal);
    PushVertex(coords, bbr);
    PushVertex(normals, dnormal);
    PushVertex(coords, fbl);
    PushVertex(normals, dnormal);
    PushVertex(coords, bbl);
    PushVertex(normals, dnormal);
    PushVertex(coords, bbr);
    PushVertex(normals, dnormal);
}


//
//
// PART 2: RenderManager module
//
//

void _print_shader_info_log(GLuint shader_index) {
  int max_length = 2048;
  int actual_length = 0;
  char shader_log[2048];
  glGetShaderInfoLog(shader_index, max_length, &actual_length, shader_log);
  printf("shader info log for GL index %u:\n%s\n", shader_index, shader_log);
}

class RenderManager
{
  public:
   enum ShapeType
   {
      SPHERE,
      CYLINDER,
      CUBE
   };

                 RenderManager();
   void          SetView(glm::vec3 &c, glm::vec3 &, glm::vec3 &);
   void          SetUpGeometry();
   void          SetColor(double r, double g, double b);
   void          Render(ShapeType, glm::mat4 model);
   GLFWwindow   *GetWindow() { return window; };

  private:
   glm::vec3 color;
   GLuint sphereVAO;
   GLuint sphereNumPrimitives;
   GLuint cylinderVAO;
   GLuint cylinderNumPrimitives;
   GLuint cubeVAO;
   GLuint cubeNumPrimitives;
   GLuint mvploc;
   GLuint colorloc;
   GLuint camloc;
   GLuint ldirloc;
   glm::mat4 projection;
   glm::mat4 view;
   GLuint shaderProgram;
   GLFWwindow *window;

   void SetUpWindowAndShaders();
   void MakeModelView(glm::mat4 &);
};

RenderManager::RenderManager()
{
  SetUpWindowAndShaders();
  SetUpGeometry();
  projection = glm::perspective(
        glm::radians(45.0f), (float)1000 / (float)1000,  5.0f, 110.0f);

  // Get a handle for our MVP and color uniforms
  mvploc = glGetUniformLocation(shaderProgram, "MVP");
  colorloc = glGetUniformLocation(shaderProgram, "color");
  camloc = glGetUniformLocation(shaderProgram, "cameraloc");
  ldirloc = glGetUniformLocation(shaderProgram, "lightdir");

  glm::vec4 lightcoeff(0.3, 0.7, 0, 50.5); // Lighting coeff, Ka, Kd, Ks, alpha
  GLuint lcoeloc = glGetUniformLocation(shaderProgram, "lightcoeff");
  glUniform4fv(lcoeloc, 1, &lightcoeff[0]);
}

void
RenderManager::SetView(glm::vec3 &camera, glm::vec3 &origin, glm::vec3 &up)
{ 
   glm::mat4 v = glm::lookAt(
                       camera, // Camera in world space
                       origin, // looks at the origin
                       up      // and the head is up
                 );
   view = v; 
   glUniform3fv(camloc, 1, &camera[0]);
   // Direction of light
   // glm::vec3 lightdir = glm::normalize(camera - origin);   

   glm::vec3 newLight(0, 6, -10);
   glm::vec3 lightdir = glm::normalize(newLight);   
   glUniform3fv(ldirloc, 1, &lightdir[0]);
};

void
RenderManager::SetUpWindowAndShaders()
{
  // start GL context and O/S window using the GLFW helper library
  if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(700, 700, "Game", NULL, NULL);
  if (!window) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte *version = glGetString(GL_VERSION);   // version as a string
  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable(GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

  const char* vertex_shader = GetVertexShader();
  const char* fragment_shader = GetFragmentShader();

  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vs, 1, &vertex_shader, NULL);
  glCompileShader(vs);
  int params = -1;
  glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf(stderr, "ERROR: GL shader index %i did not compile\n", vs);
    _print_shader_info_log(vs);
    exit(EXIT_FAILURE);
  }

  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fs, 1, &fragment_shader, NULL);
  glCompileShader(fs);
  glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
  if (GL_TRUE != params) {
    fprintf(stderr, "ERROR: GL shader index %i did not compile\n", fs);
    _print_shader_info_log(fs);
    exit(EXIT_FAILURE);
  }

  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, fs);
  glAttachShader(shaderProgram, vs);
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);
}

void RenderManager::SetColor(double r, double g, double b)
{
   color[0] = r;
   color[1] = g;
   color[2] = b;
}

void RenderManager::MakeModelView(glm::mat4 &model)
{
   glm::mat4 modelview = projection * view * model;
   glUniformMatrix4fv(mvploc, 1, GL_FALSE, &modelview[0][0]);
}

void RenderManager::Render(ShapeType st, glm::mat4 model)
{
   int numPrimitives = 0;
   if (st == SPHERE)
   {
      glBindVertexArray(sphereVAO);
      numPrimitives = sphereNumPrimitives;
   }
   else if (st == CYLINDER)
   {
      glBindVertexArray(cylinderVAO);
      numPrimitives = cylinderNumPrimitives;
   }
   else if (st == CUBE)
   {
      glBindVertexArray(cubeVAO);
      numPrimitives = cubeNumPrimitives;
   }
   MakeModelView(model);
   glUniform3fv(colorloc, 1, &color[0]);
   glDrawElements(GL_TRIANGLES, numPrimitives, GL_UNSIGNED_INT, NULL);
}

void SetUpVBOs(std::vector<float> &coords, std::vector<float> &normals,
               GLuint &points_vbo, GLuint &normals_vbo, GLuint &index_vbo)
{
  int numIndices = coords.size()/3;
  std::vector<GLuint> indices(numIndices);
  for(int i = 0; i < numIndices; i++)
    indices[i] = i;

  points_vbo = 0;
  glGenBuffers(1, &points_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
  glBufferData(GL_ARRAY_BUFFER, coords.size() * sizeof(float), coords.data(), GL_STATIC_DRAW);

  normals_vbo = 0;
  glGenBuffers(1, &normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);

  index_vbo = 0;    // Index buffer object
  glGenBuffers(1, &index_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

void RenderManager::SetUpGeometry()
{
  std::vector<float> sphereCoords;
  std::vector<float> sphereNormals;
  GetSphereData(sphereCoords, sphereNormals);
  sphereNumPrimitives = sphereCoords.size() / 3;
  GLuint sphere_points_vbo, sphere_normals_vbo, sphere_indices_vbo;
  SetUpVBOs(sphereCoords, sphereNormals, 
            sphere_points_vbo, sphere_normals_vbo, sphere_indices_vbo);

  std::vector<float> cylCoords;
  std::vector<float> cylNormals;
  GetCylinderData(cylCoords, cylNormals);
  cylinderNumPrimitives = cylCoords.size() / 3;
  GLuint cyl_points_vbo, cyl_normals_vbo, cyl_indices_vbo;
  SetUpVBOs(cylCoords, cylNormals, 
            cyl_points_vbo, cyl_normals_vbo, cyl_indices_vbo);

  std::vector<float> cubeCoords;
  std::vector<float> cubeNormals;
  GetCubeData(cubeCoords, cubeNormals);
  cubeNumPrimitives = cylCoords.size() / 3;
  GLuint cube_points_vbo, cube_normals_vbo, cube_indices_vbo;
  SetUpVBOs(cubeCoords, cubeNormals, 
            cube_points_vbo, cube_normals_vbo, cube_indices_vbo);

  GLuint vao[3];
  glGenVertexArrays(3, vao);

  glBindVertexArray(vao[SPHERE]);
  sphereVAO = vao[SPHERE];
  glBindBuffer(GL_ARRAY_BUFFER, sphere_points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, sphere_normals_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_indices_vbo);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindVertexArray(vao[CYLINDER]);
  cylinderVAO = vao[CYLINDER];
  glBindBuffer(GL_ARRAY_BUFFER, cyl_points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, cyl_normals_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cyl_indices_vbo);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindVertexArray(vao[CUBE]);
  cubeVAO = vao[CUBE];
  glBindBuffer(GL_ARRAY_BUFFER, cube_points_vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ARRAY_BUFFER, cube_normals_vbo);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_indices_vbo);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
}

class GameObject {
public:
    float color[3];     // the RGB color
    float position[3];  // the xyz position
    float size[3];      // the size. used for collision detection
    bool  enabled;      // true = show the GameObject, false = hide it (don't render)

    GameObject();
    GameObject(float, float, float, float, float, float, float, float, float);

    void setColor(float, float, float);
    void setRandomColor(void);
    void setPosition(float, float, float);
    void setSize(float, float, float);

    bool willCollide(GameObject);
    void moveHorizontal(float);
    void moveForward(float);
};
GameObject::GameObject(void) {
    setRandomColor();
    setSize(1, 1, 1);
    enabled = true;
}
GameObject::GameObject(float cx, float cy, float cz, 
                       float px, float py, float pz,
                       float sx, float sy, float sz) 
{
    setColor(cx, cy, cz);
    setPosition(px, py, pz);
    setSize(sx, sy, sz);
    enabled = true;
}

void GameObject::setColor(float r, float g, float b) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
}
void GameObject::setRandomColor(void) {
    float colors[6][3] = {
                            {0.807, 0.803, 0.815}, // light grey
                            {1, 0.227, 0.235},     // red
                            {1, 0.768, 0.031},     // gold
                            {0.015, 0.749, 0.007}, // green
                            {0, 0.870, 0.913},     // turquoise
                            {0.835, 0, 1},         // purple
                         };
    int val = rand() % 6;
    color[0] = colors[val][0];
    color[1] = colors[val][1];
    color[2] = colors[val][2];
}
void GameObject::setPosition(float x, float y, float z) {
    position[0] = x;
    position[1] = y;
    position[2] = z;
}
void GameObject::setSize(float x, float y, float z) {
    size[0] = x;
    size[1] = y;
    size[2] = z;
}
bool GameObject::willCollide(GameObject other) {
    bool onLeft = position[0] <= other.position[0] + other.size[0]; 
    bool onRight  = position[0] + size[0] >= other.position[0];
    bool onFront = position[2] + size[2] >= other.position[2];
    bool onBack  = position[2] <= other.position[2] + other.size[2];

    return onRight && onLeft && onFront && onBack;
}
void GameObject::moveHorizontal(float dx) {
    if (dx > 0.0) {
        position[0] = fmin(1.4, position[0] + dx);
    }
    else {
        position[0] = fmax(-1.4, position[0] + dx);
    }
}

void GameObject::moveForward(float dz) {
    position[2] -= dz;
}

void resetEnemyCarRow(GameObject* cars[3], float moveBackAmount, bool lastRowEnabledStatus[3]) {
    for (int i = 0; i < 3; i++) {
        cars[i]->moveForward(moveBackAmount);
        cars[i]->setRandomColor();
        cars[i]->enabled = true;
    }

    // randomly choose one or two cars to be disabled
    int randomIndex = rand() % 3;
    int shouldDisableTwoCars = rand() % 4; // 25% there will be only one car for the row

    cars[randomIndex]->enabled = false;
    if (shouldDisableTwoCars == 0) {
        // disable the car to the right, including wraparound
        cars[(randomIndex+1) % 3]->enabled = false;  
    }

    // Make sure the new rows do not match the previous row's positions (enabled value)
    int theSame = 0;
    for (int i = 0; i < 3; i++) {
        if (cars[i]->enabled == lastRowEnabledStatus[i]) {
            theSame++;
        }
    }
    if (theSame == 3) {
        resetEnemyCarRow(cars, 0.0, lastRowEnabledStatus);
    }

    // set lastRowEnabledStatus
    for (int i = 0; i < 3; i++) {
        lastRowEnabledStatus[i] = cars[i]->enabled;
    }
}


//
// PART3: main function
//

glm::mat4 RotateMatrix(float degrees, float x, float y, float z)
{
   glm::mat4 identity(1.0f);
   glm::mat4 rotation = glm::rotate(identity, 
                                    glm::radians(degrees), 
                                    glm::vec3(x, y, z));
   return rotation;
}

glm::mat4 ScaleMatrix(double x, double y, double z)
{
   glm::mat4 identity(1.0f);
   glm::vec3 scale(x, y, z);
   return glm::scale(identity, scale);
}

glm::mat4 TranslateMatrix(double x, double y, double z)
{
   glm::mat4 identity(1.0f);
   glm::vec3 translate(x, y, z);
   return glm::translate(identity, translate);
}

void SetUpWheel(glm::mat4 modelSoFar, RenderManager &rm) {
    // tire
    rm.SetColor(0, 0, 0);
    glm::mat4 s1 = ScaleMatrix(1, 1, 0.5);
    rm.Render(RenderManager::CYLINDER, modelSoFar*s1);

    // rim
    rm.SetColor(0.666, 0.666, 0.666);
    glm::mat4 s2 = ScaleMatrix(0.6, 0.6, 0.51);
    glm::mat4 t2 = TranslateMatrix(0, 0, -0.005);
    rm.Render(RenderManager::CYLINDER, modelSoFar*t2*s2);
}

void SetUpCar(glm::mat4 modelSoFar, RenderManager &rm, float r, float g, float b) {
    rm.SetColor(r, g, b);

    // main rectangle for body
    glm::mat4 t1 = TranslateMatrix(-0.5, 0, 0);
    glm::mat4 s1 = ScaleMatrix(1, 0.05, 2);
    rm.Render(RenderManager::CUBE, modelSoFar*t1*s1);

    // bottom block between wheels
    glm::mat4 s6 = ScaleMatrix(1, 0.3, 0.6);
    glm::mat4 t6 = TranslateMatrix(-0.5, -0.2, 0.7);
    rm.Render(RenderManager::CUBE, modelSoFar*t6*s6);

    // bumpers
    glm::mat4 s7 = ScaleMatrix(1, 0.2, 0.2);
    glm::mat4 t7 = TranslateMatrix(-0.5, -0.2, 1.8);
    rm.Render(RenderManager::CUBE, modelSoFar*t7*s7);
    glm::mat4 t8 = TranslateMatrix(-0.5, -0.2, 0);
    rm.Render(RenderManager::CUBE, modelSoFar*t8*s7);

    // wheel wells
    glm::mat4 s9 = ScaleMatrix(1, 0.3, 0.1);
    glm::mat4 t9 = TranslateMatrix(-0.5, -0.13, 0.05);
    glm::mat4 r9 = RotateMatrix(45, 1, 0, 0);
    rm.Render(RenderManager::CUBE, modelSoFar*t9*r9*s9);
    glm::mat4 t10 = TranslateMatrix(-0.5, -0.13, 1.15);
    rm.Render(RenderManager::CUBE, modelSoFar*t10*r9*s9);
    glm::mat4 r11 = RotateMatrix(-45, 1, 0, 0);
    glm::mat4 t11 = TranslateMatrix(-0.5, -0.2, 0.8);
    rm.Render(RenderManager::CUBE, modelSoFar*t11*r11*s9);
    glm::mat4 t12 = TranslateMatrix(-0.5, -0.2, 1.9);
    rm.Render(RenderManager::CUBE, modelSoFar*t12*r11*s9);

    // upper car body
    glm::mat4 s13 = ScaleMatrix(1, 0.5, 0.9);
    glm::mat4 t13 = TranslateMatrix(-0.5, 0, 0.6);
    rm.Render(RenderManager::CUBE, modelSoFar*t13*s13);
    // back side
    glm::mat4 s14 = ScaleMatrix(1, 0.4, 0.2);
    glm::mat4 t14 = TranslateMatrix(-0.5, 0.13, 0.44);
    glm::mat4 r14 = RotateMatrix(20, 1, 0, 0);
    rm.Render(RenderManager::CUBE, modelSoFar*t14*r14*s14);
    // front side
    glm::mat4 t15 = TranslateMatrix(-0.5, 0.06, 1.45);
    glm::mat4 r15 = RotateMatrix(-20, 1, 0, 0);
    rm.Render(RenderManager::CUBE, modelSoFar*t15*r15*s14);
    // trunk
    glm::mat4 s20 = ScaleMatrix(1, 0.12, 0.7);
    glm::mat4 r20 = RotateMatrix(-10, 1, 0, 0);
    glm::mat4 t20 = TranslateMatrix(-0.5, -0.06, 0.02);
    rm.Render(RenderManager::CUBE, modelSoFar*t20*r20*s20);
    // hood
    glm::mat4 r21 = RotateMatrix(10, 1, 0, 0);
    glm::mat4 t21 = TranslateMatrix(-0.5, 0.07, 1.29);
    rm.Render(RenderManager::CUBE, modelSoFar*t21*r21*s20);

    // windows
    rm.SetColor(0, 0, 0);
    // back
    glm::mat4 s16 = ScaleMatrix(0.8, 0.3, 0.2);
    glm::mat4 t16 = TranslateMatrix(-0.4, 0.19, 0.45);
    rm.Render(RenderManager::CUBE, modelSoFar*t16*r14*s16);
    // front
    glm::mat4 t17 = TranslateMatrix(-0.4, 0.12, 1.43);
    rm.Render(RenderManager::CUBE, modelSoFar*t17*r15*s16);
    // front side
    glm::mat4 s18 = ScaleMatrix(1.02, 0.3, 0.35);
    glm::mat4 t18 = TranslateMatrix(-0.51, 0.15, 1.07);
    rm.Render(RenderManager::CUBE, modelSoFar*t18*s18);
    // back side
    glm::mat4 t19 = TranslateMatrix(-0.51, 0.15, 0.64);
    rm.Render(RenderManager::CUBE, modelSoFar*t19*s18);

    // wheels
    glm::mat4 s2 = ScaleMatrix(0.2, 0.2, 0.2);
    glm::mat4 r2 = RotateMatrix(90, 0, 1, 0);
    glm::mat4 t2 = TranslateMatrix(0.4, -0.2, 0.45);
    glm::mat4 t4 = TranslateMatrix(-0.5, -0.2, 0.45);
    glm::mat4 t3 = TranslateMatrix(0.4, -0.2, 1.55);
    glm::mat4 t5 = TranslateMatrix(-0.5, -0.2, 1.55);
    SetUpWheel(modelSoFar*t2*r2*s2, rm);
    SetUpWheel(modelSoFar*t3*r2*s2, rm);
    SetUpWheel(modelSoFar*t4*r2*s2, rm);
    SetUpWheel(modelSoFar*t5*r2*s2, rm);

    // taillights
    rm.SetColor(0.784, 0, 0);
    glm::mat4 s22 = ScaleMatrix(0.06, 0.09, 0.02);
    glm::mat4 t22a = TranslateMatrix(-0.4, -0.07, 0);
    rm.Render(RenderManager::SPHERE, modelSoFar*t22a*s22);
    glm::mat4 t22b = TranslateMatrix(-0.25, -0.07, 0);
    rm.Render(RenderManager::SPHERE, modelSoFar*t22b*s22);
    glm::mat4 t23a = TranslateMatrix(0.4, -0.07, 0);
    rm.Render(RenderManager::SPHERE, modelSoFar*t23a*s22);
    glm::mat4 t23b = TranslateMatrix(0.25, -0.07, 0);
    rm.Render(RenderManager::SPHERE, modelSoFar*t23b*s22);

    // headlights
    rm.SetColor(1, 1, 1);
    glm::mat4 s24 = ScaleMatrix(0.1, 0.1, 0.02);
    glm::mat4 t24 = TranslateMatrix(-0.33, -0.07, 1.99);
    rm.Render(RenderManager::SPHERE, modelSoFar*t24*s24);
    glm::mat4 t25 = TranslateMatrix(0.33, -0.07, 1.99);
    rm.Render(RenderManager::SPHERE, modelSoFar*t25*s24);
}

void SetUpTree(glm::mat4 modelSoFar, RenderManager &rm) {
    rm.SetColor(0.517, 0.270, 0);

    // main trunk
    glm::mat4 scaleTrunk = ScaleMatrix(0.15, 0.15, 3);
    glm::mat4 rotateTrunk = RotateMatrix(90, 1, 0, 0);
    glm::mat4 rotateTrunk2 = RotateMatrix(180, 0, 0, 1);
    glm::mat4 translateTrunk = TranslateMatrix(0, 0, -3.0);
    rm.Render(RenderManager::CYLINDER, modelSoFar*rotateTrunk*rotateTrunk2*translateTrunk*scaleTrunk);

    // middle branches
    glm::mat4 s1 = ScaleMatrix(0.05, 0.05, 0.7);
    glm::mat4 t1 = TranslateMatrix(0, 1.5, -0.3);
    rm.Render(RenderManager::CYLINDER, modelSoFar*t1*s1);
    glm::mat4 s2 = ScaleMatrix(0.03, 0.03, 0.4);
    glm::mat4 t2 = TranslateMatrix(0, 1.5, 0.4);
    glm::mat4 r2 = RotateMatrix(-45, 1, 0, 0);
    rm.Render(RenderManager::CYLINDER, modelSoFar*t2*r2*s2);

    // upper branches
    glm::mat4 s3 = ScaleMatrix(0.05, 0.05, 0.5);
    glm::mat4 t3 = TranslateMatrix(0, 2.2, 0);
    glm::mat4 r3a = RotateMatrix(20, 1, 0, 0);
    glm::mat4 r3b = RotateMatrix(160, 0, 1, 0);
    rm.Render(RenderManager::CYLINDER, modelSoFar*t3*r3a*r3b*s3);
    glm::mat4 s4 = ScaleMatrix(0.05, 0.05, 0.5);
    glm::mat4 t4 = TranslateMatrix(0.15, 2.35, -0.42);
    glm::mat4 r4 = RotateMatrix(-110, 1, 0, 0);
    rm.Render(RenderManager::CYLINDER, modelSoFar*t4*r4*s4);

    // leaves
    rm.SetColor(0.027, 0.611, 0);
    glm::mat4 s5 = ScaleMatrix(1.2, 0.5, 1.2);
    glm::mat4 t5 = TranslateMatrix(0, 3.5, 0);
    rm.Render(RenderManager::SPHERE, modelSoFar*t5*s5);
    glm::mat4 s6 = ScaleMatrix(0.7, 0.3, 0.7);
    glm::mat4 t6 = TranslateMatrix(0, 2, 0.8);
    rm.Render(RenderManager::SPHERE, modelSoFar*t6*s6);
    glm::mat4 s7 = ScaleMatrix(0.7, 0.3, 0.7);
    glm::mat4 t7 = TranslateMatrix(0, 2.9, -0.9);
    rm.Render(RenderManager::SPHERE, modelSoFar*t7*s7);
}

void SetUpGround(glm::mat4 modelSoFar, RenderManager &rm, GameObject ground) {
    // road
    rm.SetColor(0.286, 0.286, 0.286); // dark grey
    glm::mat4 rTranslate = TranslateMatrix(ground.position[0]-2.25, ground.position[1]-6.0, ground.position[2]-1.0);
    glm::mat4 rScale = ScaleMatrix(4.5, 0.5, 10.0);
    rm.Render(RenderManager::CUBE, modelSoFar*rTranslate*rScale);

    // grass
    rm.SetColor(0.031, 0.749, 0); // green
    glm::mat4 g1t = TranslateMatrix(ground.position[0]-7.25, ground.position[1]-5.8, ground.position[2]-1.0);
    glm::mat4 g1s = ScaleMatrix(5.0, 0.5, 10.0);
    rm.Render(RenderManager::CUBE, modelSoFar*g1t*g1s);
    glm::mat4 g2t = TranslateMatrix(ground.position[0]+2.25, ground.position[1]-5.8, ground.position[2]-1.0);
    glm::mat4 g2s = ScaleMatrix(5.0, 0.5, 10.0);
    rm.Render(RenderManager::CUBE, modelSoFar*g2t*g2s);

    // road lane markings
    rm.SetColor(1, 0.913, 0); // yellow
    glm::mat4 laneScale = ScaleMatrix(0.15, 0.5, 1.5);
    for (int i = 0; i < 2; i++) {
        glm::mat4 lt1 = TranslateMatrix(ground.position[0]-0.8, ground.position[1]-5.99, ground.position[2]-1.0+5.0*i);
        rm.Render(RenderManager::CUBE, modelSoFar*lt1*laneScale);
        glm::mat4 lt2 = TranslateMatrix(ground.position[0]+0.65, ground.position[1]-5.99, ground.position[2]-1.0+5.0*i);
        rm.Render(RenderManager::CUBE, modelSoFar*lt2*laneScale);
    }

    // fence beams
    rm.SetColor(0.823, 0.615, 0.172); // light brown
    glm::mat4 beamScale = ScaleMatrix(0.05, 0.05, 10.0);
    for (int i = 0; i < 2; i++) {
        glm::mat4 bt1 = TranslateMatrix(ground.position[0]-2.7, ground.position[1]-5.0+i*0.3, ground.position[2]-1.0);
        rm.Render(RenderManager::CYLINDER, modelSoFar*bt1*beamScale);
        glm::mat4 bt2 = TranslateMatrix(ground.position[0]+2.7, ground.position[1]-5.0+i*0.3, ground.position[2]-1.0);
        rm.Render(RenderManager::CYLINDER, modelSoFar*bt2*beamScale);
    }
    
    // fence poles
    rm.SetColor(0.6, 0.388, 0); // brown
    glm::mat4 poleScale = ScaleMatrix(0.1, 0.6, 0.1);
    for (int i = 0; i < 5; i++) {
        glm::mat4 pt1 = TranslateMatrix(ground.position[0]-2.75, ground.position[1]-5.2, ground.position[2]-1.1+i*2.0);
        rm.Render(RenderManager::CUBE, modelSoFar*pt1*poleScale);
        glm::mat4 pt2 = TranslateMatrix(ground.position[0]+2.65, ground.position[1]-5.2, ground.position[2]-1.1+i*2.0);
        rm.Render(RenderManager::CUBE, modelSoFar*pt2*poleScale);
    }

    // trees
    glm::mat4 treeScale = ScaleMatrix(0.7, 0.7, 0.7);
    glm::mat4 treeTrans = TranslateMatrix(ground.position[0] + 3.5, ground.position[1]-5.0, ground.position[2]);
    glm::mat4 treeRotate = RotateMatrix(90, 0, 1, 0);
    SetUpTree(modelSoFar*treeTrans*treeRotate*treeScale, rm);
    glm::mat4 treeTrans2 = TranslateMatrix(ground.position[0] - 3.5, ground.position[1]-5.0, ground.position[2] + 5.0);
    SetUpTree(modelSoFar*treeTrans2*treeRotate*treeScale, rm);

}

void SetUpGame(int counter, RenderManager &rm, GameObject mpCar,
               std::vector<GameObject> cars, std::vector<GameObject> grounds)
{
    glm::mat4 identity(1.0f);
    glm::mat4 roadTrans = TranslateMatrix(0, 0.5, 0);

    double var = (counter%10)/9.0; // oscillates between 0 and 1
    if ((counter/10 % 2) == 1)
       var=1-var; 

    glm::mat4 mainCarTrans = TranslateMatrix(mpCar.position[0], 0.41, 0);
    SetUpCar(identity*mainCarTrans, rm, mpCar.color[0], mpCar.color[1], mpCar.color[2]);

    for (int i = 0; i < cars.size(); i++) {
        if (cars[i].enabled) {
            glm::mat4 t = TranslateMatrix(cars[i].position[0], 0.4, cars[i].position[2]);
            SetUpCar(identity*t, rm, cars[i].color[0], cars[i].color[1], cars[i].color[2]);
        }
    }

    for (int i = 0; i < grounds.size(); i++) {
        SetUpGround(identity*roadTrans, rm , grounds[i]);
    }
}

void movePlayerLeftOrRight(GameObject &car, float lrSpeed, float moveToX, float minX = -1.5, float maxX = 1.5) {
    float curX = car.position[0];
    float nextX = curX + lrSpeed;

    if (curX > moveToX) {
        nextX = curX - lrSpeed;
    }

    if ((curX < 0 && nextX > 0) || (curX > 0 && nextX < 0))  {
        car.position[0] = 0;
    }
    else if ((curX < moveToX && nextX > moveToX) || (curX > moveToX && nextX < moveToX)) {
        car.position[0] = moveToX;
    }
    else {
        if (moveToX > curX) {
            car.position[0] += lrSpeed;
        }
        else if (moveToX < curX){
            car.position[0] -= lrSpeed;
        }
    }
}

GameObject 
setUpMainPlayerCar(float color[3]) {
    //                      (            color           |  position | scale  )
    GameObject mainPlayerCar(color[0], color[1], color[2], 0, 0.01, 0, 1, 1, 2);
    return mainPlayerCar;
}

std::vector<GameObject>
setUpEnemyCars(int numRows, float spacing, bool lastRowEnabledStatus[3]) {
    std::vector<GameObject> cars;

    for (int row = 0; row < numRows; row++) {
        //             ( color |     position     |  scale )
        GameObject car1(0, 0, 0, -1.5, 0, spacing*row, 1, 1, 2);
        GameObject car2(0, 0, 0, 0   , 0, spacing*row, 1, 1, 2);
        GameObject car3(0, 0, 0, 1.5 , 0, spacing*row, 1, 1, 2);

        GameObject* curRow[3] = {&car1, &car2, &car3};
        resetEnemyCarRow(curRow, 0.0, lastRowEnabledStatus);

        // disable the first set of cars so that the player can orient themselves
        if (row < 2) {
            car1.enabled = false;
            car2.enabled = false;
            car3.enabled = false;
        }

        cars.push_back(car1);
        cars.push_back(car2);
        cars.push_back(car3);
    }

    return cars;
}

std::vector<GameObject> 
setUpGrounds(int numRows) {
    std::vector<GameObject> grounds;

    for (int i = 0; i < numRows; i++) {
        //          ( color |    position   |  scale )
        GameObject g(0, 0, 0, 0, 5.0, i*10.0, 0, 0, 0);
        grounds.push_back(g);
    }

    return grounds;
}

int main() 
{
  // ------------ CONFIG --------------

  const float defaultForwardSpeed = 0.3;
  const int   numGroundRows       = 12; 
  const int   numCarRows          = 7;
  const float carRowSpacing       = 18.0;

  // ----------------------------------
  RenderManager rm;
  GLFWwindow *window = rm.GetWindow();

  glm::vec3 origin(0, 0, 8);
  glm::vec3 up(0, 1, 0);
  glm::vec3 camera(0, 6, -7);

  bool lastRowEnabledStatus[3] = {false, false, false}; // keep track of which cars were enabled in the previous row
  float defaultMainPlayerColor[3] = {0, 0.396, 1}; // blue
  GameObject mainPlayerCar = setUpMainPlayerCar(defaultMainPlayerColor);
  std::vector<GameObject> cars = setUpEnemyCars(numCarRows, carRowSpacing,lastRowEnabledStatus);
  std::vector<GameObject> grounds = setUpGrounds(numGroundRows);

  int counter = 0;
  int gameOverCounter = 0;

  float forwardSpeed = defaultForwardSpeed;

  bool rightKeyPressed = false;
  bool leftKeyPressed = false;
  const float locations[3] = {1.5, 0.0, -1.5};
  int curIdx = 1;
  int score = 0;
  bool gameOver = false;
  bool shouldPrintScore = true;

  cerr << "\n\n----------------------------------------\n";

  while (!glfwWindowShouldClose(window)) 
  {
    double angle=counter/300.0*2*M_PI;
    counter++;

    // increase the forward speed by a little over time
    if (counter % 100 == 0) { 
        forwardSpeed += 0.03;
    }

    float lrSpeed = forwardSpeed / 1.5; // the speed to move the main player left or right

    rm.SetView(camera, origin, up);

    // wipe the drawing surface clear
    glClearColor(0.501, 0.819, 1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gameOver) {
        forwardSpeed = 0.0;
        lrSpeed = 0.0;

        if (shouldPrintScore) {
            cerr << "\rFinal score: " << score << "\t\t\t\n\n";
            cerr << "- Press SPACE to play again!\n";
            cerr << "- Close the window to exit.\n";
            shouldPrintScore = false;
            gameOverCounter = 0;
        }

        // reset the level if the user presses the space key
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            mainPlayerCar = setUpMainPlayerCar(defaultMainPlayerColor);
            cars = setUpEnemyCars(numCarRows, carRowSpacing,lastRowEnabledStatus);
            grounds = setUpGrounds(numGroundRows);
            gameOver = false;
            shouldPrintScore = true;
            forwardSpeed = defaultForwardSpeed;
            counter = 0;
            curIdx = 1;
            score = 0;
            cerr << "\n\n----------------------------------------\n";
        }

        // make the main player color flash between red and original color
        gameOverCounter++;
        if (gameOverCounter < 15) {
            mainPlayerCar.setColor(1.0, 0.0, 0.0);
        }
        else if (gameOverCounter < 30) {
            mainPlayerCar.setColor(defaultMainPlayerColor[0], defaultMainPlayerColor[1], defaultMainPlayerColor[2]);
        }
        else {
            gameOverCounter = 0;
        }
    }
    else {
        mainPlayerCar.setColor(defaultMainPlayerColor[0], defaultMainPlayerColor[1], defaultMainPlayerColor[2]);
    }

    // move the car by snapping it into one of the lanes
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (!rightKeyPressed)
            curIdx = fmin(2, curIdx + 1);
        rightKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
        rightKeyPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (!leftKeyPressed)
            curIdx = fmax(0, curIdx - 1);
        leftKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
        leftKeyPressed = false;
    }

    movePlayerLeftOrRight(mainPlayerCar, lrSpeed, locations[curIdx]);

    // move the enemy cars and ground forward each frame
    for (int i = 0; i < cars.size(); i+=3) {
        cars[i].moveForward(forwardSpeed);
        cars[i+1].moveForward(forwardSpeed);
        cars[i+2].moveForward(forwardSpeed);

        // check if a collision will happen
        for (int j = 0; j < 3; j++) {
            if (cars[i + j].enabled && mainPlayerCar.willCollide(cars[i + j])) {
                // mainPlayerCar.setColor(1, 0, 0);
                gameOver = true;
            }
        }

        // check if the score should increase
        if (cars[i].position[2] < -5.0) {
            // make sure all three cars are not enabled
            if (!(!cars[i].enabled && !cars[i+1].enabled && !cars[i+2].enabled)) {
                score++;
                cerr << "\rScore: " << score  << "\t\t\t"; // <<<<<<<<<<<<<
            }
        }

        // respawn to the back if the car is behind camera
        if (cars[i].position[2] < -5.0) {
            GameObject* row[3] = {&cars[i], &cars[i+1], &cars[i+2]};
            resetEnemyCarRow(row, -numCarRows*carRowSpacing, lastRowEnabledStatus); // reset back
        }
    }

    for (int i = 0; i < grounds.size(); i++) {
        grounds[i].moveForward(forwardSpeed);
        if (grounds[i].position[2] <= -10.0) {
            grounds[i].moveForward(-10.0 * numGroundRows); // reset back
        }
    }

    SetUpGame(counter, rm, mainPlayerCar, cars, grounds);



    // update other events like input handling
    glfwPollEvents();
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers(window);
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
    
const char *GetVertexShader()
{
   static char vertexShader[1024];
   strcpy(vertexShader, 
           "#version 400\n"
           "layout (location = 0) in vec3 vertex_position;\n"
           "layout (location = 1) in vec3 vertex_normal;"
           "uniform mat4 MVP;\n"
           "uniform vec3 cameraloc;\n"
           "uniform vec3 lightdir;\n"
           "uniform vec4 lightcoeff;\n"
           "out float shading_amount;\n"
           "void main() {\n"
           "  gl_Position = MVP*vec4(vertex_position, 1.0);\n"

           "  vec3 viewdir = cameraloc - vertex_position;"
           "       viewdir = normalize(viewdir);"

           "  float diffuse = dot(lightdir, vertex_normal);"
           "        diffuse = max(0.0, diffuse);"

           "  vec3 r = (2.0 * diffuse) * vertex_normal - lightdir;"
           "       r = normalize(r);"

           "  float specular = dot(r, viewdir);"
           "        specular = max(0.0, specular);"
           "        specular = pow(specular, lightcoeff[3]);"

           "  shading_amount = lightcoeff[0] + lightcoeff[1]*diffuse + lightcoeff[2]*specular;"
           "}\n"
         );
   return vertexShader;
}

const char *GetFragmentShader()
{
   static char fragmentShader[1024];
   strcpy(fragmentShader, 
           "#version 400\n"
           "uniform vec3 color;\n"
           "in float shading_amount;\n"
           "out vec4 frag_color;\n"
           "void main() {\n"
           "  frag_color = vec4(color, 1.0);\n"
           "  frag_color[0] = min(1.0, frag_color[0] * shading_amount);"
           "  frag_color[1] = min(1.0, frag_color[1] * shading_amount);"
           "  frag_color[2] = min(1.0, frag_color[2] * shading_amount);"
           "}\n"
         );
   return fragmentShader;
}

