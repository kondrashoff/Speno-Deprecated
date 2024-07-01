#include "Engine.h"
#include "Scene.h"
#include "ProgramManager.h"
#include "FlagManager.h"
#include "TextureManager.h"
#include "SharedEngineData.h"
#include "Atmosphere.h"
#include "Camera.h"

// TODO:
// Recreate the material editor, perhaps allocate a separate class for this to avoid errors when loading another model

int main(int argc, char** argv) {
	Engine speno;
	speno.initialize();

	//FlagSystem::Instance.window->enable(WINDOW_FULLSCREEN);
	//FlagSystem::Instance.window->enable(WINDOW_VERTICAL_SYNC);

	//Camera::Instance.setPosition(28.279, 3.5, 1.23612e-06);
	//Camera::Instance.setRotation(0.0, 90.0, 0.0);

	//Camera::Instance.setPosition(-11.4, 4.2, 9.2);
	//Camera::Instance.setRotation(2.2, 202.7, 0.0);

	Camera::Instance.setPosition(42.0, 5.3, 30.4);
	Camera::Instance.setRotation(-10.6, 115.1, 0.0);
	
	//Camera::Instance.setPosition(-1312.9, 64.2, -38.3);
	//Camera::Instance.setRotation(17.2, 271.7, 0.0);

	float pt_res_scale = 1.0f;
	ProgramManager& pm = ProgramManager::Instance;
	    pm.add("bvh_heatmap")->initPixelShader("bvh_heatmap_calculation.frag");
	    pm.add("bvh_heatmap")->setResolutionScale(pt_res_scale);
		pm.get("bvh_heatmap")->disable();

		pm.add("pathtracer");
		pm.get("pathtracer")->initPixelShader("pathtracing.frag");
		pm.get("pathtracer")->setResolutionScale(pt_res_scale);

		pm.add("reprojector");
		pm.get("reprojector")->initPixelShader("reprojection.frag");
		pm.get("reprojector")->setResolutionScale(pt_res_scale);

		/*pm.add("variance_estimator");
		pm.get("variance_estimator")->initPixelShader("variance_estimation.frag");
		pm.get("variance_estimator")->setResolutionScale(pt_res_scale);*/

		pm.add("denoiser")->initPixelShader("denoiser.frag"); pm.get("denoiser")->setResolutionScale(pt_res_scale);
		/*pm.add("denoiser2")->initPixelShader("denoiser2.frag"); pm.get("denoiser2")->setResolutionScale(pt_res_scale);
		pm.add("denoiser3")->initPixelShader("denoiser3.frag"); pm.get("denoiser3")->setResolutionScale(pt_res_scale);
		pm.add("denoiser4")->initPixelShader("denoiser4.frag"); pm.get("denoiser4")->setResolutionScale(pt_res_scale);
		pm.add("denoiser5")->initPixelShader("denoiser5.frag"); pm.get("denoiser5")->setResolutionScale(pt_res_scale);
		pm.add("denoiser6")->initPixelShader("denoiser6.frag"); pm.get("denoiser6")->setResolutionScale(pt_res_scale);
		pm.add("denoiser7")->initPixelShader("denoiser7.frag"); pm.get("denoiser7")->setResolutionScale(pt_res_scale);
		pm.add("denoiser8")->initPixelShader("denoiser8.frag"); pm.get("denoiser8")->setResolutionScale(pt_res_scale);*/

		pm.add("storage");
		pm.get("storage")->initPixelShader("storage.frag");
		pm.get("storage")->setResolutionScale(pt_res_scale);

		pm.add("postprocess");
		pm.get("postprocess")->initPixelShader("postprocess.frag");

	TextureParameters tp_sharp_mirrored;
		tp_sharp_mirrored.wrap_s = GL_MIRRORED_REPEAT;
		tp_sharp_mirrored.wrap_t = GL_MIRRORED_REPEAT;
		tp_sharp_mirrored.min_filter = GL_NEAREST;
		tp_sharp_mirrored.mag_filter = GL_NEAREST;
		
	TextureParameters tp_smooth_mirrored;
		tp_smooth_mirrored.wrap_s = GL_MIRRORED_REPEAT;
		tp_smooth_mirrored.wrap_t = GL_MIRRORED_REPEAT;
		tp_smooth_mirrored.min_filter = GL_LINEAR;
		tp_smooth_mirrored.mag_filter = GL_LINEAR;
		
	Framebuffer* pathtracer_fbo = pm.get("pathtracer")->getFBO();
		pathtracer_fbo->init();

		pathtracer_fbo->addAttachment("diffuse", GL_RGB16F);
		pathtracer_fbo->addAttachment("albedo", GL_RGB16F);
		pathtracer_fbo->addAttachment("position", GL_RGBA32F);
		pathtracer_fbo->addAttachment("normal", GL_RGB16F);
		pathtracer_fbo->addAttachment("godrays", GL_RGBA16F);
		pathtracer_fbo->addAttachment("reservoir1", GL_RGBA32F, tp_sharp_mirrored);
		pathtracer_fbo->addAttachment("reservoir2", GL_RGBA32F, tp_sharp_mirrored);

		//pathtracer_fbo->getAttachment("diffuse")->bind("denoisedTexture", "postprocess"); //
		pathtracer_fbo->getAttachment("diffuse")->bind("diffuseTexture", "reprojector");
		pathtracer_fbo->getAttachment("godrays")->bind("godraysTexture", "reprojector");
		pathtracer_fbo->getAttachment("albedo")->bind("albedoTexture", "postprocess");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "reprojector");
		//pathtracer_fbo->getAttachment("position")->bind("positionTexture", "variance_estimator");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "denoiser");
		/*pathtracer_fbo->getAttachment("position")->bind("positionTexture", "denoiser2");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "denoiser3");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "denoiser4");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "denoiser5");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "denoiser6");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "denoiser7");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "denoiser8");*/
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "storage");
		pathtracer_fbo->getAttachment("position")->bind("positionTexture", "postprocess");
		//pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "variance_estimator");
		pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "denoiser");
		/*pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "denoiser2");
		pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "denoiser3");
		pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "denoiser4");
		pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "denoiser5");
		pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "denoiser6");
		pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "denoiser7");
		pathtracer_fbo->getAttachment("normal")->bind("normalTexture", "denoiser8");*/
		pathtracer_fbo->getAttachment("reservoir1")->bind("reservoir1Texture", "storage");
		pathtracer_fbo->getAttachment("reservoir2")->bind("reservoir2Texture", "storage");

	Framebuffer* reprojector_fbo = pm.get("reprojector")->getFBO();
		reprojector_fbo->init();
		
		reprojector_fbo->addAttachment("accumulated", GL_RGB16F);
		reprojector_fbo->addAttachment("godrays", GL_RGBA16F);
		reprojector_fbo->addAttachment("moment", GL_RG16F);

		//reprojector_fbo->getAttachment("accumulated")->bind("accumulatedTexture", "variance_estimator");
		reprojector_fbo->getAttachment("accumulated")->bind("accumulatedTexture", "denoiser");
		//reprojector_fbo->getAttachment("moment")->bind("momentTexture", "variance_estimator");
		reprojector_fbo->getAttachment("moment")->bind("momentTexture", "storage");
		reprojector_fbo->getAttachment("godrays")->bind("godraysTexture", "storage");
		reprojector_fbo->getAttachment("godrays")->bind("godraysTexture", "postprocess");

	/*Framebuffer* variance_fbo = pm.get("variance_estimator")->getFBO();
		variance_fbo->init();
		variance_fbo->addAttachment("variance", GL_R16F);
		variance_fbo->getAttachment("variance")->bind("varianceTexture", "denoiser");*/

	Framebuffer* denoiser_fbo = pm.get("denoiser")->getFBO();
		denoiser_fbo->init();

		denoiser_fbo->addAttachment("denoised", GL_RGB16F);
		denoiser_fbo->addAttachment("variance", GL_R16F);

		denoiser_fbo->getAttachment("denoised")->bind("denoisedTexture", "postprocess"); //
		//denoiser_fbo->getAttachment("denoised")->bind("denoisedTexture", "denoiser2");
		denoiser_fbo->getAttachment("denoised")->bind("denoisedTexture", "storage");
		//denoiser_fbo->getAttachment("variance")->bind("varianceTexture", "denoiser2");

	/*Framebuffer* denoiser2_fbo = pm.get("denoiser2")->getFBO();
		denoiser2_fbo->init();

		denoiser2_fbo->addAttachment("denoised", GL_RGB16F);
		denoiser2_fbo->addAttachment("variance", GL_R16F);

		denoiser2_fbo->getAttachment("denoised")->bind("denoisedTexture", "denoiser3");
		denoiser2_fbo->getAttachment("variance")->bind("varianceTexture", "denoiser3");

	Framebuffer* denoiser3_fbo = pm.get("denoiser3")->getFBO();
		denoiser3_fbo->init();

		denoiser3_fbo->addAttachment("denoised", GL_RGB16F);
		denoiser3_fbo->addAttachment("variance", GL_R16F);

		denoiser3_fbo->getAttachment("denoised")->bind("denoisedTexture", "denoiser4");
		denoiser3_fbo->getAttachment("variance")->bind("varianceTexture", "denoiser4");

	Framebuffer* denoiser4_fbo = pm.get("denoiser4")->getFBO();
		denoiser4_fbo->init();

		denoiser4_fbo->addAttachment("denoised", GL_RGB16F);
		denoiser4_fbo->addAttachment("variance", GL_R16F);

		denoiser4_fbo->getAttachment("denoised")->bind("denoisedTexture", "denoiser5");
		denoiser4_fbo->getAttachment("variance")->bind("varianceTexture", "denoiser5");
		
	Framebuffer* denoiser5_fbo = pm.get("denoiser5")->getFBO();
		denoiser5_fbo->init();
		denoiser5_fbo->addAttachment("denoised", GL_RGB16F);
		denoiser5_fbo->addAttachment("variance", GL_R16F);

		denoiser5_fbo->getAttachment("denoised")->bind("denoisedTexture", "denoiser6");
		denoiser5_fbo->getAttachment("variance")->bind("varianceTexture", "denoiser6");

	Framebuffer* denoiser6_fbo = pm.get("denoiser6")->getFBO();
		denoiser6_fbo->init();
		denoiser6_fbo->addAttachment("denoised", GL_RGB16F);
		denoiser6_fbo->addAttachment("variance", GL_R16F);

		denoiser6_fbo->getAttachment("denoised")->bind("denoisedTexture", "denoiser7");
		denoiser6_fbo->getAttachment("variance")->bind("varianceTexture", "denoiser7");
		
	Framebuffer* denoiser7_fbo = pm.get("denoiser7")->getFBO();
		denoiser7_fbo->init();
		denoiser7_fbo->addAttachment("denoised", GL_RGB16F);
		denoiser7_fbo->addAttachment("variance", GL_R16F);

		denoiser7_fbo->getAttachment("denoised")->bind("denoisedTexture", "denoiser8");
		denoiser7_fbo->getAttachment("variance")->bind("varianceTexture", "denoiser8");

	Framebuffer* denoiser8_fbo = pm.get("denoiser8")->getFBO();
		denoiser8_fbo->init();
		denoiser8_fbo->addAttachment("denoised", GL_RGB16F, tp_smooth_mirrored);
		denoiser8_fbo->addAttachment("variance", GL_R16F);
		denoiser8_fbo->getAttachment("denoised")->bind("denoisedTexture", "postprocess");*/

	Framebuffer* storage_fbo = pm.get("storage")->getFBO();
		storage_fbo->init();

		storage_fbo->addAttachment("current_denoised", GL_RGB16F);
		storage_fbo->addAttachment("current_position", GL_RGBA32F);
		storage_fbo->addAttachment("current_moment", GL_RG16F);
		storage_fbo->addAttachment("current_godrays", GL_RGBA16F);
		storage_fbo->addAttachment("current_reservoir1", GL_RGBA32F, tp_sharp_mirrored);
		storage_fbo->addAttachment("current_reservoir2", GL_RGBA32F, tp_sharp_mirrored);

		storage_fbo->getAttachment("current_denoised")->bind("previousDenoisedTexture", "reprojector");
		storage_fbo->getAttachment("current_position")->bind("previousPositionTexture", "reprojector");
		storage_fbo->getAttachment("current_moment")->bind("previousMomentTexture", "reprojector");
		storage_fbo->getAttachment("current_godrays")->bind("previousGodraysTexture", "reprojector");
		storage_fbo->getAttachment("current_reservoir1")->bind("previousReservoir1Texture", "pathtracer");
		storage_fbo->getAttachment("current_reservoir2")->bind("previousReservoir2Texture", "pathtracer");

	SharedEngineData::Instance.bind("pathtracer");
	SharedEngineData::Instance.bind("reprojector");

	Camera::Instance.bind("bvh_heatmap");
	Camera::Instance.bind("pathtracer");
	Camera::Instance.bind("reprojector");

	Atmosphere::Instance.getTexture()->bind("atmosphereTexture", "pathtracer");
	Atmosphere::Instance.bind("pathtracer");

	TextureManager::Instance.bindSTBN("pathtracer");

	//Scene::Instance.loadOBJ("Meshes/EmeraldSquare/CityDusk.obj");
	//Scene::Instance.loadOBJ("Meshes/Bistro/bistroExterior.obj");
	//Scene::Instance.loadOBJ("Meshes/Subway/Untitled.obj");
	//Scene::Instance.loadOBJ("Meshes/Sci Fi Corridor/Corridor.obj");
	//Scene::Instance.loadOBJ("Meshes/CityModel/CityOBJ.obj");
	//Scene::Instance.loadOBJ("Meshes/SponzaAtrium/sponza_lights.obj");
	//Scene::Instance.loadOBJ("Meshes/SponzaAtrium/sponza.obj");
	//Scene::Instance.loadOBJ("Meshes/Canyon/Canyon.obj");
	//Scene::Instance.loadOBJ("Meshes/LowPolyForest/untitled.obj");
	//Scene::Instance.loadOBJ("Meshes/Experiment/canyons.obj");
	//Scene::Instance.loadOBJ("Meshes/MIS/MIS.obj");
	//Scene::Instance.loadOBJ("Meshes/VeachAjar/VeachDoor.obj");
	//Scene::Instance.loadOBJ("Meshes/powerplant/powerplant.obj");
	//Scene::Instance.loadOBJ("Meshes/sibenik/sibenik.obj");
	//Scene::Instance.loadOBJ("Meshes/salle_de_bain/salle_de_bain.obj");
	//Scene::Instance.loadOBJ("Meshes/San_Miguel/san-miguel-low-poly.obj");
	//Scene::Instance.loadOBJ("Meshes/breakfast_room/breakfast_room.obj");
	//Scene::Instance.loadOBJ("Meshes/LoneMonk/lone_monk.obj");

	speno.run();
	
	return 0;
}