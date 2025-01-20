#OBJS specifies which files to compile as part of the project
OBJS = main.cpp

#CC specifies which compiler we're using
CC = g++ -std=c++17

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
# Adding SDL2 include paths for both Intel and Apple Silicon Macs
COMPILER_FLAGS = -w -I/opt/homebrew/include/SDL2 -I/usr/local/include/SDL2 -Iinclude

#LINKER_FLAGS specifies the libraries we're linking against
# Adding library paths for both Intel and Apple Silicon Macs
LINKER_FLAGS = -L/opt/homebrew/lib -L/usr/local/lib -lSDL2 -lSDL2_image -lSDL2_mixer

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = SDLGame


#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)

# Add clean target (optional but useful)
clean:
	rm -f $(OBJ_NAME)