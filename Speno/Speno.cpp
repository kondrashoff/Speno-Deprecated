#include "Engine.h"
using namespace std;

int main(int argc, char** argv) {
	Engine speno;

	string path = "C:/Users/Admin/Documents/";
	string filename = "cornell_box.obj";

	Mesh cornell_box(path, filename);
	speno.setScene(cornell_box);

	speno.run();

	return 0;
}