# pragma once
# include "../nclgl/OGLRenderer.h"
# include "../nclgl/SceneNode.h"
# include "../nclgl/Frustum.h"
#include "../nclgl\MeshAnimation.h"
#include "../nclgl\MeshMaterial.h"
class Camera;
class Shader;
class HeightMap;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;
	void UpdateSceneAuto(float dt) override;

protected:
	void DrawHeightmap(); // For skybox + water
	void DrawWater();
	void DrawSkybox();

	void BuildNodeLists(SceneNode* from); // Scene graph
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);
	void DrawLamps(SceneNode* root);
	void DrawBuildings(SceneNode* root);
	void BindBuildings();
	void BindLamps();
	void MakeMan();
	void DrawMan();

	void GenerateScreenTexture(GLuint& into, bool depth = false);

	SceneNode* root;
	Mesh* cube;

	Frustum frameFrustum;
	vector < SceneNode* > transparentNodeList;
	vector < SceneNode* > nodeList;

	//Skybox + water
	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* lampShader;
	Shader* buildingShader;

	HeightMap* heightMap;
	Mesh* quad;
	Mesh* quad1;
	Mesh* quad2;



	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint moonTex;
	Shader* pointlightShader; // Shader to calculate lighting
	Shader* combineShader; // shader to stick it all together

	
	GLuint bufferDepthTex;
	GLuint bufferColourTex[2];
	GLuint bufferFBO;
	GLuint processFBO;

	Shader* sceneShader;
	Shader * processShader;


	Mesh* manMesh;
	Shader* manShader;
	MeshAnimation* anim;
	MeshMaterial* material;
	vector < GLuint > matTextures;

	int currentFrame;
	float frameTime;
	

	float waterRotate;
	float waterCycle;

};
