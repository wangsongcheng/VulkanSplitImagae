.PHONY:clean
SRC=$(wildcard *.cpp)
OBJ=$(SRC:%.cpp=%.o)
BIN=demo
LIB=-lvulkan -lglfw3 -lpthread -ldl
INC_PATH=-I./include/
LIB_PATH=-L./lib/

COMPILER=g++
$(BIN):$(OBJ)
	$(COMPILER) -g $^ $(LIB_PATH) $(LIB) -o $@
%.o:%.cpp
	$(COMPILER) -g -c -std=c++17 $^ $(INC_PATH) -o $@
clean:
	rm -rf $(OBJ)
