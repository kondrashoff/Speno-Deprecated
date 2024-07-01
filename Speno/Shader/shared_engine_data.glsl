layout(std140) uniform SharedEngineData {
	int frame;
	float time;
	float delta_time;

	int window_width;
	int window_height;

	int max_depth;
};