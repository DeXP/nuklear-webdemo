# Install
BIN = demo

# Flags
CFLAGS = -std=c99 -pedantic -O2 -DNKCD=NKC_GLFW

SRC = main.c
OBJ = $(SRC:.c=.o)

ifeq ($(OS),Windows_NT)
BIN := $(BIN).exe
LIBS = -lglfw3 -lopengl32 -lm -lGLU32 -lGLEW32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LIBS = -lglfw3 -framework OpenGL -lm -lGLEW
	else
		LIBS = -lglfw -lGL -lm -lGLU -lGLEW
	endif
endif

web:
	emcc $(SRC) -DNKCD=NKC_GLFW -o index.html -s USE_GLFW=3 -Os --preload-file data

$(BIN):
	$(CC) $(SRC) $(CFLAGS) -o $(BIN) $(LIBS)
