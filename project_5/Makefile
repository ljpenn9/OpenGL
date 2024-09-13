# define libraries
ifeq ($(shell uname),Darwin)
LIB = -DMAC -framework OPENGL -framework GLUT
else
LIB = -lGL -lGLU -lglut
endif
LIB2 = src/libim/libim.a src/jpeg/libjpeg.a

# define makefile targets
CC = g++ -std=c++11 -Wall -Wno-deprecated-declarations -O3

hw5: hw5.cpp $(LIB2)
	$(CC) -o hw5 hw5.cpp $(LIB) $(LIB2)

src/jpeg/libjpeg.a:
	cd src;cd jpeg;make;cd ..;cd ..

src/libim/libim.a:
	cd src;cd libim;make;cd ..;cd ..

