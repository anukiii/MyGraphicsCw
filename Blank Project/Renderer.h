# pragma once
# include "../nclgl/OGLRenderer.h"
# include "../nclgl/SceneNode.h"
# include "../nclgl/Frustum.h"
class Camera;
class Shader;
class HeightMap;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;

protected:
	void DrawHeightmap(); // For skybox + water
	void DrawWater();
	void DrawSkybox();

	void BuildNodeLists(SceneNode* from); // Scene graph
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	SceneNode* root;
	Mesh* cube;
	Shader* shaderCube;
	Frustum frameFrustum;
	vector < SceneNode* > transparentNodeList;
	vector < SceneNode* > nodeList;

	//Skybox + water
	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;

	HeightMap* heightMap;
	Mesh* quad;


	Light* light;
	Camera* camera;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;

	float waterRotate;
	float waterCycle;

};
