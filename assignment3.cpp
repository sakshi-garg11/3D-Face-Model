// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>
#include <sstream>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <common/tga.h>

#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

const int window_width = 600, window_height = 600;

GLuint tex;
GLuint texUniformID;
GLuint textureMatrixID;
GLuint textureID;
typedef struct Vertex {
	float Position[4];
	float Color[4];
	float Normal[3];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];
		Position[3] = 1.0;
	}
	void SetColor(float *color) {
		Color[0] = color[0];
		Color[1] = color[1];
		Color[2] = color[2];
		Color[3] = color[3];
	}
	void SetNormal(float *coords) {
		Normal[0] = coords[0];
		Normal[1] = coords[1];
		Normal[2] = coords[2];
	}
};
typedef struct Vert {
	float Position[3];
	float UV[2];
	void SetPosition(float *coords) {
		Position[0] = coords[0];
		Position[1] = coords[1];
		Position[2] = coords[2];

	}
	void SetUV(float *uv) {
		UV[0] = uv[0];
		UV[1] = uv[1];
	}
};
// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLuint* &, int);
void createVAOs(Vertex[], GLuint[], int);
void createVao(Vert[], GLushort[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);


// GLOBAL VARIABLES
GLFWwindow* window;
float pickingColor[441];

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint gPickedIndex = -1;
std::string gMessage;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 6;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
GLuint VertexArrayId[NumObjects] = { 0,1,2,3,4,5 };
GLuint VertexBufferId[NumObjects] = { 0,1,2,3,4,5 };
GLuint IndexBufferId[NumObjects] = { 0,1,2,3,4,5 };
GLuint* MeshIndices;
Vertex *MeshVerts;
GLuint *GridXIndices, *GridYIndices;

size_t NumIndices[NumObjects] = { 0,0,0,0,0,0 };
size_t VertexBufferSize[NumObjects] = { 0,0,0,0,0,0 };
size_t IndexBufferSize[NumObjects] = { 0,0,0,0,0,0 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint pickingColorID;
GLuint LightID;
GLuint LightID2;

GLint gX = 0.0;
GLint gZ = 0.0;

typedef struct cam_pos {
	float x, y, z;
};

int flag_key_shift = 0;
int sKey = 0;
int lKey = 0;

// animation control
bool animation = false;
GLfloat phi = 0.0;

unsigned int id;
//camera
cam_pos cam;
bool fvertical = false, fsides = false;
GLint countup = 0, countside = 0;
cam_pos temp = { 10.0, 10.0, 10.0 };
GLushort textureInd[441];
Vert textureVert[441];
int C_flag = 0;

void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLuint* &out_Indices, int ObjectId)
{
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ(file, vertices, normals);

	std::vector<GLushort> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);

	const size_t vertCount = indexed_vertices.size();
	const size_t idxCount = indices.size();

	// populate output arrays
	out_Vertices = new Vertex[vertCount];
	for (int i = 0; i < vertCount; i++) {
		out_Vertices[i].SetPosition(&indexed_vertices[i].x);
		out_Vertices[i].SetNormal(&indexed_normals[i].x);
		out_Vertices[i].SetColor(&color[0]);
	}
	out_Indices = new GLuint[idxCount];
	for (int i = 0; i < idxCount; i++) {
		out_Indices[i] = indices[i];
	}

	// set global variables!!
	NumIndices[ObjectId] = idxCount;
	VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
	IndexBufferSize[ObjectId] = sizeof(GLuint) * idxCount;
}

void setUV()
{
	/*
	int j = 0;
	float n[3] = { 1,1,1 };
	for (int i = 0; i < 1323; i=i+3)
	{
	texVert[i] = controlMesh[j].Position[0];
	texVert[i+1] = controlMesh[j].Position[1];
	texVert[i+2] = controlMesh[j].Position[2];
	j++;
	}
	j = 0;
	for (int i = 0; i < 882; i=i+2)
	{
	texUV[i] = (controlMesh[j].Position[0]+5)/10;
	texUV[i+1] = (controlMesh[j].Position[1])/7;
	j++;
	}
	for (int i = 0; i < 441; i++)
	{
	texInd[i] = i;
	}
	j = 0;
	for (int k = 0; k < 441; k++)
	{

	printf("Position[%d]:   X: %0.02f, Y: %0.02f, Z: %0.02f\n",k, texVert[tmp], texVert[tmp+1], texVert[tmp+2]);
	printf("Position[%d]:   X: %0.02f, Y: %0.02f\n", k, texUV[j], texUV[j + 1]);

	printf("Position[%d]:   X: %0.02f, Y: %0.02f, Z: %0.02f\n\n-----\n", k, controlMesh[k].Position[0], controlMesh[k].Position[1], controlMesh[k].Position[2]);
	tmp = tmp + 3;
	j = j + 2;
	}
	tmp = 0;
	j = 0;
	*/
	int k = 0;
	for (int i = 0; i < 441; i++)
	{
		textureInd[i] = i;
		textureVert[i].Position[0] = MeshVerts[i].Position[0];
		textureVert[i].Position[1] = MeshVerts[i].Position[1];
		textureVert[i].Position[2] = MeshVerts[i].Position[2];
		textureVert[i].UV[0] = (MeshVerts[i].Position[0] + 5) / 10;
		textureVert[i].UV[1] = (MeshVerts[i].Position[1] + 5) / 10;

		printf("Position[%d]:   X: %0.02f, Y: %0.02f, Z: %0.02f\n", k, textureVert[k].Position[0], textureVert[k].Position[1], textureVert[k].Position[2]);
		printf("Position[%d]:   U: %0.02f, V: %0.02f\n", k, textureVert[k].UV[0], textureVert[k].UV[1]);
		printf("Position[%d]:   X: %0.02f, Y: %0.02f, Z: %0.02f\n\n-----\n", k, MeshVerts[k].Position[0], MeshVerts[k].Position[1], MeshVerts[k].Position[2]);
		k++;
	}
	VertexBufferSize[7] = sizeof(textureVert);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVao(textureVert, textureInd, 7);

}



void create_mesh()
{
	//control mesh

	int Meshcount = 441;
	MeshVerts = new Vertex[Meshcount];


	int i;
	float green[] = { 0.0f,5.0f,0.0f,1.0f };
	float white[] = { 2.0f,2.0f,2.0f,2.0f };
	float n[] = { 0.0f,0.0f,1.0f };
	float pos[3];
	float x = -5.0f;
	float y = -5.0f;
	int j, k;

	for (i = 0; i < Meshcount;)
	{
		k = 0;
		while (k < 21)
		{
			j = 0;
			while (j < 21)
			{
				pos[0] = x + 0.5f * j;
				pos[1] = y + 0.5f * k;
				pos[2] = 5.0f; //fixed
				MeshVerts[i].SetColor(green);
				MeshVerts[i].SetNormal(n);
				MeshVerts[i].SetPosition(pos);
				i++;
				j++;
			}
			k++;
		}
	}

	MeshIndices = new GLuint[2400];

	j = 0;
	i = 0;

	while (i<2400)
	{
		k = 0;
		while (k < 20)
		{
			MeshIndices[i] = k + j;
			MeshIndices[i + 1] = k + 1 + j;
			MeshIndices[i + 2] = k + 22 + j;
			MeshIndices[i + 3] = k + 22 + j;
			MeshIndices[i + 4] = k + 21 + j;
			MeshIndices[i + 5] = k + j;
			i += 6;
			k += 1;
		}
		j += 21;
	}

	GridXIndices = new GLuint[840];
	i = 0;
	k = 0;
	while (k < 441)
	{
		j = 0;
		while (j < 20)
		{
			GridXIndices[i] = j + k;
			GridXIndices[i + 1] = j + 1 + k;
			i += 2;
			j++;
		}
		k += 21;
	}

	GridYIndices = new GLuint[840];
	i = 0;
	k = 0;
	while (k < 420)
	{
		j = 0;
		while (j < 21)
		{
			GridYIndices[i] = j + k;
			GridYIndices[i + 1] = j + 21 + k;
			i += 2;
			j++;
		}
		k += 21;
	}


	NumIndices[3] = 2400;
	VertexBufferSize[3] = sizeof(Vertex) * Meshcount;
	IndexBufferSize[3] = sizeof(GLuint) * 2400;

	createVAOs(MeshVerts, MeshIndices, 3);

	for (i = 0; i < Meshcount; i++)
	{
		MeshVerts[i].SetColor(white);
	}

	NumIndices[4] = 840;
	VertexBufferSize[4] = sizeof(Vertex) * Meshcount;
	IndexBufferSize[4] = sizeof(GLuint) * 840;

	createVAOs(MeshVerts, GridXIndices, 4);

	NumIndices[5] = 840;
	VertexBufferSize[5] = sizeof(Vertex) * Meshcount;
	IndexBufferSize[5] = sizeof(GLuint) * 840;

	createVAOs(MeshVerts, GridYIndices, 5);

	for (i = 0; i < Meshcount; i++)
	{
		MeshVerts[i].SetColor(green);
	}
}

void createObjects(void)
{
	//-- COORDINATE AXES --//
	Vertex CoordVerts[] =
	{
		{ { 0.0, 0.0, 0.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 },{ 1.0, 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 },{ 0.0, 1.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 5.0, 0.0, 1.0 },{ 0.0, 1.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 0.0, 1.0 },{ 0.0, 0.0, 1.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, 5.0, 1.0 },{ 0.0, 0.0, 1.0, 1.0 },{ 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(CoordVerts, NULL, 0);

	//-- GRID --//
	Vertex GridVerts[] =
	{
		{ { -5.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -4.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -4.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -4.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -4.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -3.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -3.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -3.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -3.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -2.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -2.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -2.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -2.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -1.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -1.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -1.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -1.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -0.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -0.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 0.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 1.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 1.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 1.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 1.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 2.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 2.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 2.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 2.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 3.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 3.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 3.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 3.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 4.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 4.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 4.5, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 4.5, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0,  5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },

		{ { -5.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -4.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -4.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -4.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -4.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -3.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -3.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -3.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -3.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -2.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -2.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -2.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -2.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -1.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -1.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -1.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -1.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, -0.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, -0.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 0.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 0.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 0.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 1.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 1.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 1.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 1.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 2.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 2.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 2.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 2.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 3.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 3.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 3.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 3.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 4.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 4.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 4.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 4.5, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { -5.0, 0.0, 5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
		{ { 5.0, 0.0, 5.0, 1.0 },{ 4.0, 4.0, 4.0, 1.0 },{ 0.0, 0.0, 1.0 } },
	};

	VertexBufferSize[1] = sizeof(GridVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
	createVAOs(GridVerts, NULL, 1);


	// ATTN: create your grid vertices here!

	create_mesh();

	//-- .OBJs --//

	// ATTN: load your models here
	Vertex* Verts;
	GLuint* Idcs;
	loadObject("me2.obj", glm::vec4(0.55, 0.2, 0.2, 1.0), Verts, Idcs, 2);
	createVAOs(Verts, Idcs, 2);

}

void changecamera(int i)
{
	float r = 10 * 1.732;
	if (fvertical)
	{
		//vertical rotation will be around global x-axis
		cam.x = temp.x;
		cam.z = (sqrt(pow(r, 2) - pow(cam.x, 2))*cosf(2 * 3.1415*(i + 2) / 16));
		cam.y = (sqrt(pow(r, 2) - pow(cam.x, 2))*sinf(2 * 3.1415*(i + 2) / 16));
		gViewMatrix = glm::lookAt(
			glm::vec3(cam.x, cam.y, cam.z),
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		temp.z = cam.z;
		temp.y = cam.y;

	}
	else if (fsides)
	{
		//horizontal rotation will be around y-axis
		cam.y = temp.y;
		cam.x = (sqrt(pow(r, 2) - pow(cam.y, 2))*cosf(2 * 3.1415*(i + 2) / 16));
		cam.z = (sqrt(pow(r, 2) - pow(cam.y, 2))*sinf(2 * 3.1415*(i + 2) / 16));
		gViewMatrix = glm::lookAt(
			glm::vec3(cam.x, cam.y, cam.z),
			glm::vec3(0, 0, 0), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);
		temp.x = cam.x;
		temp.z = cam.z;

	}

	setUV();
}

void renderScene(void)
{
	//ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!


	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
	// Re-clear the screen for real rendering
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(programID);
	{
		//glm::vec3 lightPos = glm::vec3(3, 3, 4);
		//glm::vec3 lightPos2 = glm::vec3(-4, 6, -3);
		glm::mat4x4 ModelMatrix = glm::mat4(1.0);
		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		//glUniform3f(LightID2, lightPos2.x, lightPos2.y, lightPos2.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
		glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);


		glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);

		glBindVertexArray(VertexArrayId[1]);	//draw grid
		glDrawArrays(GL_LINES, 0, 88);
		glBindVertexArray(1);

		glBindVertexArray(VertexArrayId[2]);	//draw face
		glDrawElements(GL_TRIANGLES, NumIndices[2], GL_UNSIGNED_INT, (void*)0);
		glBindVertexArray(2);

		if (C_flag)
		{

			glPointSize(5.0);

			glBindVertexArray(VertexArrayId[4]);	//draw control mesh
			glDrawElements(GL_LINES, NumIndices[4], GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(4);

			glBindVertexArray(VertexArrayId[5]);	//draw control mesh
			glDrawElements(GL_LINES, NumIndices[5], GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(5);

			// Save To File
			if (sKey == 1)
			{
				FILE *fp;
				fp = fopen("cm.p3", "w+");
				printf("Writing in Progress!");
				for (int i = 0; i < 441; i++)
				{
					for (int j = 0; j < 4; j++) {
						fprintf(fp, "%0.2f", MeshVerts[i].Position[j]);
						fprintf(fp, "%s", ",");
					}
					for (int j = 0; j < 4; j++) {
						fprintf(fp, "%0.2f", MeshVerts[i].Color[j]);
						fprintf(fp, "%s", ",");
					}
					for (int j = 0; j < 3; j++) {
						fprintf(fp, "%0.2f", MeshVerts[i].Normal[j]);
						fprintf(fp, "%s", ",");
					}
					if (i != (440))fprintf(fp, "%s", "\n");
				}
				fprintf(fp, "%s", "p");
				fclose(fp);
				sKey = 0;
				printf("\nWriting Completed\n");
			}

			//Load From File
			if (lKey == 1)
			{
				//char fname[10];
				//printf("Enter the name of the file");
				//cin >> fname;
				FILE *fp;
				fp = fopen("cm.p3", "r");
				if (!fp)
					printf("no such file");
				char cRead = NULL;
				char*  vertexValue = (char*)malloc(6 * sizeof(char));
				int pos = 0, cnt = 0, j = 0;
				float floatValofVertex = 0;
				int i = 0;
				while (cRead != 'p') {
					cRead = (char)fgetc(fp);
					if (cRead == ',')
					{
						j = 0;
						floatValofVertex = (float)atof(vertexValue);
						if (cnt == 0)
						{
							MeshVerts[pos].Position[0] = floatValofVertex;
						}
						else if (cnt == 1)
						{
							MeshVerts[pos].Position[1] = floatValofVertex;
						}
						else if (cnt == 2)
						{
							MeshVerts[pos].Position[2] = floatValofVertex;
						}
						else if (cnt == 3)
						{
							MeshVerts[pos].Position[3] = floatValofVertex;
						}
						else if (cnt == 4)
						{
							MeshVerts[pos].Color[0] = floatValofVertex;
						}
						else if (cnt == 5)
						{
							MeshVerts[pos].Color[1] = floatValofVertex;
						}
						else if (cnt == 6)
						{
							MeshVerts[pos].Color[2] = floatValofVertex;
						}
						else if (cnt == 7)
						{
							MeshVerts[pos].Color[3] = floatValofVertex;
						}
						else if (cnt == 8)
						{
							MeshVerts[pos].Normal[0] = floatValofVertex;
						}
						else if (cnt == 9)
						{
							MeshVerts[pos].Normal[1] = floatValofVertex;
						}
						else if (cnt == 10)
						{
							MeshVerts[pos].Normal[2] = floatValofVertex;
						}
						cnt++;
						if (cnt == 11)
							pos++;
						continue;
					}
					else if (cRead != '\n' && cRead != 'p') {
						vertexValue[j] = cRead;
						j++;
					}
					if (cRead == '\n') {
						cnt = 0;
					}
				}
				fclose(fp);
				lKey = 0;
				printf("\nReading Completed: Control[0] value : %0.2f\n", MeshVerts[0].Position[0]);
				createVAOs(MeshVerts, MeshIndices, 3);
			}
			glBindVertexArray(VertexArrayId[3]);	//draw control mesh
			glDrawElements(GL_POINTS, NumIndices[3], GL_UNSIGNED_INT, (void*)0);
			glBindVertexArray(3);

			//draw grid
			int i;
			float green[] = { 0.0f,5.0f,0.0f,1.0f };
			float white[] = { 2.0f,2.0f,2.0f,2.0f };

			for (i = 0; i < 441; i++)
			{
				MeshVerts[i].SetColor(white);
			}

			NumIndices[4] = 840;
			VertexBufferSize[4] = sizeof(Vertex) * 441;
			IndexBufferSize[4] = sizeof(GLuint) * 840;

			createVAOs(MeshVerts, GridXIndices, 4);

			NumIndices[5] = 840;
			VertexBufferSize[5] = sizeof(Vertex) * 441;
			IndexBufferSize[5] = sizeof(GLuint) * 840;

			createVAOs(MeshVerts, GridYIndices, 5);

			for (i = 0; i < 441; i++)
			{
				MeshVerts[i].SetColor(green);
			}

		}

		if (fvertical)
		{
			changecamera(countup);
		}
		if (fsides)
		{
			changecamera(countside);
		}

	}
	glUseProgram(0);
	// Draw GUI

	glUseProgram(textureID);
	{
		if (C_flag == 1) {
			glm::mat4 ModelMatrix1 = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
			glm::mat4 MVP1 = gProjectionMatrix * gViewMatrix * ModelMatrix1;
			glUniformMatrix4fv(textureMatrixID, 1, GL_FALSE, &MVP1[0][0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex);
			glUniform1i(texUniformID, 0);

			glBindVertexArray(VertexArrayId[7]);	// draw Vertices
			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(textureVert), textureVert);				// update buffer data
																								//glDrawElements(GL_POINTS, 441, GL_UNSIGNED_SHORT, (void*)0);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 441);
		}
	}
	glUseProgram(0);
	TwDraw();

	// Swap buffers
	glfwSwapBuffers(window);
	glfwPollEvents();
}

vec3 findCLosestPoint(vec3 rayStartPos, vec3 rayEndPos, vec3 pointPos, double *proj)
{
	vec3 rayVector = rayEndPos - rayStartPos;
	double raySquared = glm::dot(rayVector, rayVector);
	vec3 projection = pointPos - rayStartPos;
	double projectionVal = glm::dot(projection, rayVector);
	*proj = projectionVal / raySquared;
	vec3 closestPoint = rayStartPos + glm::vec3(rayVector.x * (*proj), rayVector.y * (*proj), rayVector.z * (*proj));
	return closestPoint;
}

bool rayTest(vec3 pointPos, vec3 startPos, vec3 endPos, vec3 *closestPoint, double *proj, double epsilon)
{
	*closestPoint = findCLosestPoint(startPos, endPos, pointPos, proj);
	double len = glm::distance2(*closestPoint, pointPos);
	return len < epsilon;
}

bool rayTestPoints(Vertex* vert, vec3 start, vec3 end, unsigned int *id, double *proj, double epsilon, int maxRange)
{
	unsigned int pointID = maxRange + 1;
	bool foundCollision = false;
	double minDistToStart = 10000000.0;
	double distance;
	vec3 point;

	for (unsigned int i = 0; i<maxRange; ++i)
	{
		vec3 pointPos = vec3(vert[i].Position[0], vert[i].Position[1], vert[i].Position[2]);

		if (rayTest(pointPos, start, end, &point, proj, epsilon))
		{
			distance = glm::distance2(start, point);
			if (distance<minDistToStart)
			{
				minDistToStart = distance;
				pointID = i;
				foundCollision = true;
			}
		}
	}

	*id = pointID;
	return foundCollision;

}

void pickObject(void)
{
	// Clear the screen in white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 ModelMatrix = glm::mat4(1.0);

	glUseProgram(pickingProgramID);
	{
		// TranslationMatrix * RotationMatrix;
		glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);

		// ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
		//	glPointSize(5.0);

		//	glUniform1fv(pickingColorArrayID, 441, pickingColor);
		//	glBindVertexArray(VertexArrayId[3]);	//draw points
		//	glDrawElements(GL_POINTS, NumIndices[3], GL_UNSIGNED_INT, (void*)0);
		//	glBindVertexArray(3);
		glBindVertexArray(0);

	}
	glUseProgram(0);
	// Wait until all the pending drawing commands are really done.
	// Ultra-mega-over slow !
	// There are usually a long time between glDrawElements() and
	// all the fragments completely rasterized.
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Read the pixel at the center of the screen.
	// You can also use glfwGetMousePos().
	// Ultra-mega-over slow too, even for 1 pixel,
	// because the framebuffer is on the GPU.

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	unsigned char data[4];
	glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	mat4 nModelMatrix = gViewMatrix * ModelMatrix;

	vec3 startMousePos = glm::unProject(vec3(xpos, window_height - ypos, 0.0f), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
	vec3 endMousePos = glm::unProject(vec3(xpos, window_height - ypos, 1.0f), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));

	double epsilon = 0.1;
	double proj;
	bool found = rayTestPoints(MeshVerts, startMousePos, endMousePos, &id, &proj, epsilon, 441);

	if (id>440)
	{
		id = 9999;
	}
	// Convert the color back to an integer ID
	//gPickedIndex = int(data[0]) + int(data[1]);
	gPickedIndex = int(data[0]);

	if (gPickedIndex == 510) { // Full white, must be the background !
		gMessage = "background";
	}
	else {
		std::ostringstream oss;
		oss << "point " << gPickedIndex;
		gMessage = oss.str();
	}

	// Uncomment these lines to see the picking shader in effect
	//glfwSwapBuffers(window);
	//continue; // skips the normal rendering
}

void moveVertex()
{
	GLint viewport[4];
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::vec4 vp = glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	mat4 nModelMatrix = gViewMatrix * ModelMatrix;

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	vec3 W;
	vec3 p;
	float coords[3];

	if (state == GLFW_PRESS)
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			p = glm::project(vec3(MeshVerts[id].Position[0], MeshVerts[id].Position[1], MeshVerts[id].Position[2]), nModelMatrix, gProjectionMatrix, vp);
			W = glm::unProject(vec3(xpos, window_height - ypos, p.z), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
			coords[0] = W.x;
			coords[1] = W.y;
			coords[2] = W.z;
			MeshVerts[id].SetPosition(coords);
		}
		else
		{

			p = glm::project(vec3(MeshVerts[id].Position[0], MeshVerts[id].Position[1], MeshVerts[id].Position[2]), nModelMatrix, gProjectionMatrix, vp);
			W = glm::unProject(vec3(xpos, window_height - ypos, p.z), nModelMatrix, gProjectionMatrix, vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
			coords[0] = W.x;
			coords[1] = W.y;
			coords[2] = MeshVerts[id].Position[2];
			MeshVerts[id].SetPosition(coords);
		}

	}
	NumIndices[3] = 2400;
	VertexBufferSize[3] = sizeof(Vertex) * 441;
	IndexBufferSize[3] = sizeof(GLuint) * 2400;

	createVAOs(MeshVerts, MeshIndices, 3);

	int i;
	float green[] = { 0.0f,5.0f,0.0f,1.0f };
	float white[] = { 2.0f,2.0f,2.0f,2.0f };

	for (i = 0; i < 441; i++)
	{
		MeshVerts[i].SetColor(white);
	}

	NumIndices[4] = 840;
	VertexBufferSize[4] = sizeof(Vertex) * 441;
	IndexBufferSize[4] = sizeof(GLuint) * 840;

	createVAOs(MeshVerts, GridXIndices, 4);

	NumIndices[5] = 840;
	VertexBufferSize[5] = sizeof(Vertex) * 441;
	IndexBufferSize[5] = sizeof(GLuint) * 840;

	createVAOs(MeshVerts, GridYIndices, 5);

	for (i = 0; i < 441; i++)
	{
		MeshVerts[i].SetColor(green);
	}
}

int initWindow(void)
{
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(window_width, window_height, "Neha Rana(7996-6911)", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Initialize the GUI
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(window_width, window_height);
	TwBar * GUI = TwNewBar("Picking");
	TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
	TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);

	// Set up inputs
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseCallback);

	return 0;
}

void initOpenGL(void)
{

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	gProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
		glm::vec3(0.0, 0.0, 0.0),	// center
		glm::vec3(0.0, 1.0, 0.0));	// up

									// Create and compile our GLSL program from the shaders
	programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(programID, "MVP");
	ModelMatrixID = glGetUniformLocation(programID, "M");
	ViewMatrixID = glGetUniformLocation(programID, "V");
	ProjMatrixID = glGetUniformLocation(programID, "P");

	PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
	// Get a handle for our "pickingColorID" uniform
	pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
	pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
	// Get a handle for our "LightPosition" uniform
	LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	LightID2 = glGetUniformLocation(programID, "LightPosition_worldspace1");

	textureID = LoadShaders("texture.vertexshader", "texture.fragmentshader");

	texUniformID = glGetUniformLocation(textureID, "myTextureSampler");
	textureMatrixID = glGetUniformLocation(textureID, "MVP");

	tex = load_texture_TGA("singh_suman1.tga", NULL, NULL, GL_REPEAT, GL_REPEAT);
	//tex = loadBMP_custom("singh_suman.bmp");


	createObjects();
}

void createVAOs(Vertex Vertices[], unsigned int Indices[], int ObjectId) {

	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);
	const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

													// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);

	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color
	glEnableVertexAttribArray(2);	// normal

									// Disable our Vertex Buffer Object
	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}
}
void createVao(Vert Vertices[], unsigned short Indices[], int ObjectId) {
	GLenum ErrorCheckValue = glGetError();
	const size_t VertexSize = sizeof(Vertices[0]);
	const size_t RgbOffset = sizeof(Vertices[0].Position);

	// Create Vertex Array Object
	glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
	glBindVertexArray(VertexArrayId[ObjectId]);		//

													// Create Buffer for vertex data
	glGenBuffers(1, &VertexBufferId[ObjectId]);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
	glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);
	// Create Buffer for indices
	if (Indices != NULL) {
		glGenBuffers(1, &IndexBufferId[ObjectId]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
	}

	// Assign vertex attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, VertexSize, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);

	glEnableVertexAttribArray(0);	// position
	glEnableVertexAttribArray(1);	// color

	glBindVertexArray(0);

	ErrorCheckValue = glGetError();
	if (ErrorCheckValue != GL_NO_ERROR)
	{
		fprintf(
			stderr,
			"ERROR: Could not create a VBO: %s \n",
			gluErrorString(ErrorCheckValue)
		);
	}

}

void cleanup(void)
{
	// Cleanup VBO and shader
	for (int i = 0; i < NumObjects; i++) {
		glDeleteBuffers(1, &VertexBufferId[i]);
		glDeleteBuffers(1, &IndexBufferId[i]);
		glDeleteVertexArrays(1, &VertexArrayId[i]);
	}
	glDeleteProgram(programID);
	glDeleteProgram(pickingProgramID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// ATTN: MODIFY AS APPROPRIATE
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_UP:
			fvertical = !fvertical;
			{
				countup++;
				if (countup == 16)
				{
					countup = 0;
				}
			}
			break;
		case GLFW_KEY_DOWN:
			fvertical = !fvertical;
			{
				countup--;
				if (countup == 16)
				{
					countup = 0;
				}
			}
			break;
		case GLFW_KEY_RIGHT:
			fsides = !fsides;
			{
				countside++;
				if (countside == 16)
				{
					countside = 0;
				}
			}
			break;
		case GLFW_KEY_LEFT:
			fsides = !fsides;
			{
				countside--;
				if (countside == 16)
				{
					countside = 0;
				}
			}
			break;
		case GLFW_KEY_R:
			gViewMatrix = glm::lookAt(glm::vec3(10.0, 10.0, 10.0f),	// eye
				glm::vec3(0.0, 0.0, 0.0),	// center
				glm::vec3(0.0, 1.0, 0.0));
			countup = 0;
			countside = 0;
			fsides = false;
			fvertical = false;
			temp = { 10.0, 10.0, 10.0 };
			break;
		case GLFW_KEY_C:
			if (!C_flag)
				C_flag = 1;
			else
				C_flag = 0;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			flag_key_shift = 1;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			flag_key_shift = 1;
			break;
		case GLFW_KEY_S:
			sKey = 1;
			break;
		case GLFW_KEY_L:
			lKey = 1;
			break;
		case GLFW_KEY_SPACE:
			break;
		default:
			break;
		}
	}
	if (action == GLFW_RELEASE) {
		switch (key)
		{
		case GLFW_KEY_UP:
			fvertical = !fvertical;
			break;
		case GLFW_KEY_DOWN:
			fvertical = !fvertical;
			break;
		case GLFW_KEY_RIGHT:
			fsides = !fsides;
			break;
		case GLFW_KEY_LEFT:
			fsides = !fsides;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			flag_key_shift = 0;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			flag_key_shift = 0;
			break;
		}
	}
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		pickObject();
	}
}



int main(void)
{
	int i;
	// initialize window
	int errorCode = initWindow();
	if (errorCode != 0)
		return errorCode;

	for (i = 0; i < 441; i++)
	{
		pickingColor[i] = i / 255.0f;
	}

	// initialize OpenGL pipeline
	initOpenGL();

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;
	do {
		if (animation) {
			phi += 0.01;
			if (phi > 360)
				phi -= 360;
		}

		//dragging
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
		{
			moveVertex();
		}


		// DRAWING POINTS
		renderScene();


	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	cleanup();

	return 0;
}
