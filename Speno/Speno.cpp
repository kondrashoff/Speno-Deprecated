//#define DEBUG

// TODO: проверить среднее число у текстур stbn
// TODO: может стоит переустановить stbn

#include "Engine.h"
using namespace std;

int main(int argc, char** argv) {
	Engine speno;

	/*string path = "C:/Users/Admin/Documents/";
	string filename = "cornell_box.obj";

	Mesh cornell_box(path, filename);
	cornell_box.buildBVH();
	speno.setScene(cornell_box);*/

	speno.generateChunks();

	speno.run();

	/*int stbn_width, stbn_height, stbn_num_channels;
	for (int j = 0; j < 8; j++) {
		std::string filepath_base;

		switch (j) {
		case 0:
			filepath_base = "C:/Users/Admin/Downloads/STBN/stbn_scalar_2Dx1Dx1D_128x128x64x1_";
			break;
		case 1:
			filepath_base = "C:/Users/Admin/Downloads/STBN/stbn_vec1_2Dx1D_128x128x64_";
			break;
		case 2:
			filepath_base = "C:/Users/Admin/Downloads/STBN/stbn_vec2_2Dx1D_128x128x64_";
			break;
		case 3:
			filepath_base = "C:/Users/Admin/Downloads/STBN/stbn_vec3_2Dx1D_128x128x64_";
			break;
		case 4:
			filepath_base = "C:/Users/Admin/Downloads/STBN/stbn_unitvec1_2Dx1D_128x128x64_";
			break;
		case 5:
			filepath_base = "C:/Users/Admin/Downloads/STBN/stbn_unitvec2_2Dx1D_128x128x64_";
			break;
		case 6:
			filepath_base = "C:/Users/Admin/Downloads/STBN/stbn_unitvec3_2Dx1D_128x128x64_";
			break;
		case 7:
			filepath_base = "C:/Users/Admin/Downloads/STBN/stbn_unitvec3_cosine_2Dx1D_128x128x64_";
			break;
		}

		float avg_value = 0.0;
		for (int x = 0; x < 64; x++) {
			std::string stbn_filename = filepath_base + std::to_string(x) + ".png";
			float* stbn_texture = stbi_loadf(stbn_filename.c_str(), &stbn_width, &stbn_height, &stbn_num_channels, 1);

			if (!stbn_texture) {
				printf("Failed to load texture: %s\n", stbn_filename.c_str());
				exit(EXIT_FAILURE);
			}

			for (int i = 0; i < stbn_width * stbn_height; i++) {
				avg_value += stbn_texture[i];
			}

			stbi_image_free(stbn_texture);
		}
		avg_value *= 1.0 / (stbn_width * stbn_height * 64.0);

		std::cout << "Avg value" << j << ": " << avg_value << std::endl;
	}*/

	return 0;
}