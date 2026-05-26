all:
	mpic++ main.cpp filters.cpp -o programa \
	-fopenmp `pkg-config --cflags --libs opencv4`