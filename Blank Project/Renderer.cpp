# include "Renderer.h"
# include "../nclgl/Light.h"
# include "../nclgl/Heightmap.h"
# include "../nclgl/Shader.h"
# include "../nclgl/Camera.h"
# include < algorithm >
const int POST_PASSES = 10;




Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	quad1 = Mesh::GenerateQuad();
	quad2 = Mesh::GenerateQuad();
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh"); // CUBE MESH

	heightMap = new HeightMap(TEXTUREDIR "noise.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR "stainedglass.TGA",
		SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS);

	moonTex = SOIL_load_OGL_texture(
		TEXTUREDIR "brick.tga", SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	
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

	sceneShader = new Shader(
		"TexturedVertex.glsl", "TexturedFragment.glsl");
	processShader = new Shader(
		"TexturedVertex.glsl", "processfrag.glsl");
	HDRShader = new Shader(
		"TexturedVertex.glsl", "HDRFrag.glsl");
	CGShader = new Shader(
		"TexturedVertex.glsl", "ColorGradeFrag.glsl");

	manShader = new Shader(
		"SkinningVertex.glsl", "PerPixelFragment.glsl");
	lampShader = new Shader(
		"SceneVertex.glsl", "SceneFragment.glsl");
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
		Vector4(1, 1, 1, 1), 50000);

	

	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,
		(float)width / (float)height, 45.0f);


	// Generate our scene depth texture ...
	glGenTextures(1, &bufferDepthTex);
	glBindTexture(GL_TEXTURE_2D, bufferDepthTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);


	for (int i = 0; i < 2; ++i) {
		glGenTextures(1, &bufferColourTex[i]);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	}

	glGenFramebuffers(1, &bufferFBO); // We ’ll render the scene into this
	glGenFramebuffers(1, &processFBO); // And do post processing in this

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, bufferDepthTex, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
	// We can check FBO attachment success using this command !
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE || !bufferDepthTex || !bufferColourTex[0]) {
		return;

	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);




	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_FRAMEBUFFER_SRGB);
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
		s->SetBoundingRadius(10000.0f);
		s->SetMesh(Mesh::LoadFromMeshFile("building_1.msh"));
		root->AddChild(s);
	}
	SceneNode* s = new SceneNode();
	s->SetColour(Vector4(0.9f, 0.9f, 1.0f, 0.9f));
	s->SetTransform(Matrix4::Translation(Vector3(2000.0f, 2000.0f, 6000.0f)));
	s->SetModelScale(Vector3(200, 200, 200));
	s->SetBoundingRadius(10000.0f);
	s->SetMesh(Mesh::LoadFromMeshFile("Sphere.msh"));
	s->SetTexture(moonTex);
	root->AddChild(s);
}


void Renderer::DrawLamps(SceneNode *root) {
	//evey loop has to make 
	for (int i = 0; i < 10; ++i) {
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(0.5f, 0.5f, 1.0f, 0.5f));
		s->SetTransform(Matrix4::Translation(Vector3(2600, -500, 5000 + 300 * i)));
		s->SetModelScale(Vector3(1000.0f, 1000.0f, 1000.0f));
		s->SetBoundingRadius(10000.0f);
		s->SetMesh(Mesh::LoadFromMeshFile("lamp.msh"));
		root->AddChild(s);

		SceneNode* s1 = new SceneNode();
		s1->SetColour(Vector4(0.5f, 0.5f, 1.0f, 0.5f));
		s1->SetModelScale(Vector3(1000.0f, 1000.0f, 1000.0f));
		s1->SetBoundingRadius(10000.0f);
		s1->SetMesh(Mesh::LoadFromMeshFile("lamp.msh"));
		s1->SetTransform(Matrix4::Translation(Vector3(2600, -500, 5000 + 300 * i))*Matrix4::Rotation(180,Vector3(0,1,0))*Matrix4::Translation(Vector3(1000,0,0)));
		root->AddChild(s1);
	}
}

Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete quad1;
	delete quad2;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete light;
	delete cube;
	delete root;
	delete lampShader;
	delete buildingShader;

	glDeleteTextures(2, bufferColourTex);
	glDeleteTextures(1, &bufferDepthTex);
	glDeleteFramebuffers(1, &bufferFBO);
	glDeleteFramebuffers(1, &processFBO);

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

		moonTex = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, moonTex);



		glUniform1i(glGetUniformLocation(lampShader->GetProgram(), "useTexture"), moonTex);


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

	quad1->Draw();

	glDepthMask(GL_TRUE);

}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();

}


void Renderer::RenderScene() {
	BuildNodeLists(root);
	SortNodeLists();

	glBindFramebuffer(GL_FRAMEBUFFER, bufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	textureMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	DrawSkybox();

	textureMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	DrawWater();


	textureMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	DrawMan();

	textureMatrix.ToIdentity();
	modelMatrix.ToIdentity();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	DrawHeightmap();

	BindLamps();
	DrawNodes();
	ClearNodeLists();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);




	if (postProcessingType == 1) {

		glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		
		BindShader(processShader);
		modelMatrix.ToIdentity();
		viewMatrix.ToIdentity();
		projMatrix.ToIdentity();
		UpdateShaderMatrices();
		
		glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(processShader->GetProgram(), "sceneTex"), 0);
		for (int i = 0; i < POST_PASSES; ++i) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
			glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 0);
		
			glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
			quad->Draw();
			// Now to swap the colour buffers , and do the second blur pass
			glUniform1i(glGetUniformLocation(processShader->GetProgram(), "isVertical"), 1);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
			glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
			quad->Draw();

		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);
	}
	if (postProcessingType == 2) {
		glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		BindShader(HDRShader);
		modelMatrix.ToIdentity();
		viewMatrix.ToIdentity();
		projMatrix.ToIdentity();
		UpdateShaderMatrices();

		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(HDRShader->GetProgram(), "diffuseTex"), 0);

			
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
			glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
			quad->Draw();


			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
			glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
			quad->Draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);
	}
	if (postProcessingType == 3) {
		glBindFramebuffer(GL_FRAMEBUFFER, processFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		BindShader(CGShader);
		modelMatrix.ToIdentity();
		viewMatrix.ToIdentity();
		projMatrix.ToIdentity();
		UpdateShaderMatrices();

		glDisable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(CGShader->GetProgram(), "diffuseTex"), 0);


		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[1], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
		quad->Draw();


		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bufferColourTex[0], 0);
		glBindTexture(GL_TEXTURE_2D, bufferColourTex[1]);
		quad->Draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_DEPTH_TEST);
	}



	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BindShader(sceneShader);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
	projMatrix.ToIdentity();
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, bufferColourTex[0]);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	quad->Draw();



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

void Renderer::DrawHeightmap() {		
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
		Matrix4::Scale(hSize) *
		Matrix4::Rotation(90, Vector3(1, 0, 0));

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	
	textureMatrix =
		Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) *
		Matrix4::Scale(Vector3(1, 1, 1)) *
		Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));
	
	UpdateShaderMatrices();
	quad2->Draw();

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


