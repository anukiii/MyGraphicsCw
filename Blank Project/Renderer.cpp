# include "Renderer.h"
# include "../nclgl/Light.h"
# include "../nclgl/Heightmap.h"
# include "../nclgl/Shader.h"
# include "../nclgl/Camera.h"
# include "../nclgl/CubeRobot.h"
# include < algorithm > // For std :: sort ...


Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh"); // CUBE MESH

	heightMap = new HeightMap(TEXTUREDIR "noise.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR "stainedglass.TGA",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS);

	
	earthTex = SOIL_load_OGL_texture(
	TEXTUREDIR "grid.JPG", SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthBump = SOIL_load_OGL_texture(
	TEXTUREDIR "gridMap_2.jpg", SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR "px.png", TEXTUREDIR "nx.png",
		TEXTUREDIR "py.png", TEXTUREDIR "ny.png",
		TEXTUREDIR "pz.png", TEXTUREDIR "nz.png",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex) {
		return;

	}


	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);

	manShader = new Shader(
		"SkinningVertex.glsl", "PerPixelFragment.glsl");
	lampShader = new Shader(
		"LampVertex.glsl", "LampFrag.glsl");
	reflectShader = new Shader(
		"reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader(
		"skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader(
		"MountainVertex.glsl", "MountainFragment.glsl");

	if (!reflectShader->LoadSuccess() ||
		!skyboxShader->LoadSuccess() ||
		!lightShader->LoadSuccess()||
		!lampShader->LoadSuccess()||
		!manShader->LoadSuccess()) {
		return;

	}




	//Scene graph
	root = new SceneNode();
	DrawLamps(root);
	DrawBuildings(root);
	
	
	//root->AddChild(new CubeRobot(cube));
	MakeMan();

	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	camera = new Camera(-45.0f, 0.0f,
		Vector3(2100.0f, -430.0f, 10000.0f));
	light = new Light(Vector3(2000.0f,2000.0f, 6000.0f),
		Vector4(1, 1, 1, 1), 10000);


	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;

	currentFrame = 0;
	frameTime = 0.0f;
	init = true;
}

void Renderer::MakeMan() {
	manMesh = Mesh::LoadFromMeshFile("Role_T.msh");
	anim = new MeshAnimation("Role_T.anm");
	material = new MeshMaterial("Role_T.mat");

	for (int i = 0; i < manMesh->GetSubMeshCount(); ++i) {
		const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);

	}

}

void Renderer::DrawBuildings(SceneNode* root) {

	for (int i = 0; i < 10; ++i) {
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(1.0f, 0.3f, 0.3f, 0.5f));
		s->SetTransform(Matrix4::Translation(Vector3(0 + 400 * i, -460,4500 )));
		s->SetModelScale(Vector3(10, 10, 10));
		s->SetBoundingRadius(450.0f);
		s->SetMesh(Mesh::LoadFromMeshFile("building_1.msh"));
		root->AddChild(s);
	}

}


void Renderer::DrawLamps(SceneNode *root) {
	//evey loop has to make 2
	for (int i = 0; i < 5; ++i) {
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(0.5f, 0.5f, 1.0f, 0.5f));
		s->SetTransform(Matrix4::Translation(Vector3(3000, -500, 6500 + 300 * i)));
		s->SetModelScale(Vector3(1000.0f, 1000.0f, 1000.0f));
		s->SetBoundingRadius(300.0f);
		s->SetMesh(Mesh::LoadFromMeshFile("lamp.msh"));
		root->AddChild(s);
	}
}

Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete light;
	delete cube;
	delete root;
	delete lampShader;
	delete buildingShader;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	frameTime -= dt;

	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
		frameTime += 1.0f / anim->GetFrameRate();

	}


	waterRotate += dt * 2.0f; //2 degrees a second
	waterCycle += dt * 0.25f; // 10 units a second
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);
	




}

void Renderer::UpdateSceneAuto(float dt) {
	camera->CameraPath(dt);
	viewMatrix = camera->BuildViewMatrix();

	frameTime -= dt;

	while (frameTime < 0.0f) {
		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
		frameTime += 1.0f / anim->GetFrameRate();

	}


	waterRotate += dt * 2.0f; //2 degrees a second
	waterCycle += dt * 0.25f; // 10 units a second
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	root->Update(dt);


}


void Renderer::SortNodeLists() {
	std::sort(transparentNodeList.rbegin(), // note the r!
		transparentNodeList.rend(), // note the r!
		SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);

}


void Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		DrawNode(i);

	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);

	}

}


void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 model = n->GetWorldTransform() *
			Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(
			lampShader->GetProgram(), 
			"modelMatrix"), 1, false, model.values);

		glUniform4fv(glGetUniformLocation(
			lampShader->GetProgram(), "nodeColour"), 
			1, (float*)&n->GetColour());



		n->Draw(*this);

	}

}

void Renderer::BuildNodeLists(SceneNode* from) {
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() -
			camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);

		}
		else {
			nodeList.push_back(from);

		}

	}

	for (vector < SceneNode* >::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		BuildNodeLists((*i));

	}

}


void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);

}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();

}



void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	DrawHeightmap();
	DrawWater();
	DrawMan();
	

	BindLamps();
	

	DrawNodes();
	ClearNodeLists();
}


void Renderer::BindLamps() {


	BindShader(lampShader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(lampShader->GetProgram(), "diffuseTex"), 0);

}

void Renderer::BindBuildings(){


	BindShader(buildingShader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(buildingShader->GetProgram(), "diffuseTex"), 0);
	
}

void Renderer::DrawHeightmap() {											//NOT USED
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(
		lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(
		lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity(); // New !
	textureMatrix.ToIdentity(); // New !
	modelMatrix = modelMatrix * Matrix4::Rotation(10, Vector3(1, 0, 0));
	UpdateShaderMatrices();

	heightMap->Draw();

}


void Renderer::DrawWater() {
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(),
		"cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(
		reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(
		reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();

	modelMatrix =
		Matrix4::Translation(Vector3(+2050,-500,5000)) *
		Matrix4::Scale(hSize ) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));

	textureMatrix =
		Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(10, 10, 10)) *
		Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	quad->Draw();
}


void Renderer::DrawMan(){
BindShader(manShader);
SetShaderLight(*light);
glUniform3fv(glGetUniformLocation(lightShader->GetProgram(),
	"cameraPos"), 1, (float*)&camera->GetPosition());


glUniform1i(glGetUniformLocation(manShader->GetProgram(), "diffuseTex"), 0);
std::cout << camera->GetPosition() << '\n';
modelMatrix.ToIdentity();
modelMatrix.SetPositionVector(Vector3(2000,-500,5000));
modelMatrix = modelMatrix* Matrix4::Scale(Vector3(100, 100, 100));
textureMatrix.ToIdentity();


UpdateShaderMatrices();
vector < Matrix4 > frameMatrices;

const Matrix4* invBindPose = manMesh->GetInverseBindPose();
const Matrix4* frameData = anim->GetJointData(currentFrame);

for (unsigned int i = 0; i < manMesh->GetJointCount(); ++i) {
	frameMatrices.emplace_back(frameData[i] * invBindPose[i]);

}

int j = glGetUniformLocation(manShader->GetProgram(), "joints");
glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

for (int i = 0; i < manMesh->GetSubMeshCount(); ++i) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, matTextures[i]);
	manMesh->DrawSubMesh(i);

}





}