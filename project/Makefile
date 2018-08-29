# Project: fisica_moto

CPP  = g++
BIN  = vespaGame
OBJ  = main.o vespa/vespa.o mesh/mesh.o text/GLText.o
LIBS = -lGL -lGLU -lglut -lSDL2 -lSDL2_image -lm -lSDL2_ttf

#FLAG = -w
RM = rm -f

all: $(BIN)

clean:
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(LIBS)

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o

vespa.o: vespa
	$(CPP) -c vespa/vespa.cpp -o vespa/vespa.o

mesh.o: mesh
	$(CPP) -c mesh/mesh.cpp -o mesh/mesh.o

GLText.o: text
	$(CPP) -c text/GLText.cpp -o text/GLText.o

run: $(BIN)
	./$(BIN)

