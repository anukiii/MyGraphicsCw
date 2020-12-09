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
	void SetPostProcessingType(int i) { postProcessingType = i; }
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
	void BindLamps();
	void MakeMan();
	void DrawMan();


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

	HeightMap* heightMap;
	Mesh* quad;




	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	GLuint moonTex;


	
	GLuint bufferDepthTex;
	GLuint bufferColourTex[2];
	GLuint bufferFBO;
	GLuint processFBO;

	Shader* sceneShader;
	Shader * processShader;
	Shader * HDRShader;
	Shader * CGShader;
	Shader * shadowShader;


	Mesh* manMesh;
	Shader* manShader;
	MeshAnimation* anim;
	MeshMaterial* material;
	vector < GLuint > matTextures;

	int currentFrame;
	float frameTime;
	

	float waterRotate;
	float waterCycle;

	int postProcessingType;


};
