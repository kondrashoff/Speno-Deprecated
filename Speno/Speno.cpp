//#define DEBUG

#include "Engine.h"
using namespace std;

int main(int argc, char** argv) {
	Engine speno;

	// TODO: 
	// 1. Изменить небо (код для его создания)
	// 2. Отделить hdri от неба (Сделать 2 разные текстуры, первая для hdri, вторая для неба)
	// 3. Скорее всего стоит сократить количество передаваемых переменных 
	//    (где-то можно объединить текстуры в массивы текстур, где-то стоит создавать структуры с данными, а не передавать кучу переменных по отдельности)
	// 4. Сделать код более читабельным и добавить больше возможностей по управлению вне класса 
	//    (чтобы было больше возможностей по настройке уже main(), а не в классе Engine)
	// 5. Добавить загрузку текстур для 3д моделей

	//string path = "C:/Users/Admin/Documents/";
	//string filename = "cornell_box_new.obj";
	//string filename = "grand_canyon_200k.obj";

	//Mesh cornell_box(path, filename);
	//cornell_box.buildBVH();
	//speno.setScene(cornell_box);

	speno.generateChunks();

	speno.run();

	return EXIT_SUCCESS;
}