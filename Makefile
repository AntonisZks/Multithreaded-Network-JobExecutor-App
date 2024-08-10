# Compiler and flags compilation
CC = g++
FLAGS = -g -Wall -pthread

# Constants for build, source, headers and bin directories
HDR_DIR = include
SRC_DIR = src
OBJ_DIR = build
EXE_DIR = bin

JC_EXE  = jobCommander
JES_EXE = jobExecutorServer

# Compilation command
all: build bin $(EXE_DIR)/$(JC_EXE) $(EXE_DIR)/$(JES_EXE)

# APPLICATION

$(EXE_DIR)/$(JC_EXE): $(OBJ_DIR)/jobCommander.o $(OBJ_DIR)/client.o $(OBJ_DIR)/commands.o $(OBJ_DIR)/stringEditor.o $(OBJ_DIR)/clientReceivers.o
	$(CC) $(FLAGS) -o $(EXE_DIR)/$(JC_EXE) $(OBJ_DIR)/jobCommander.o $(OBJ_DIR)/client.o $(OBJ_DIR)/commands.o $(OBJ_DIR)/stringEditor.o $(OBJ_DIR)/clientReceivers.o

$(EXE_DIR)/$(JES_EXE): $(OBJ_DIR)/jobExecutorServer.o $(OBJ_DIR)/server.o $(OBJ_DIR)/controllerThread.o $(OBJ_DIR)/workerThread.o $(OBJ_DIR)/commands.o $(OBJ_DIR)/waitingBufferQueue.o $(OBJ_DIR)/stringEditor.o
	$(CC) $(FLAGS) -o $(EXE_DIR)/$(JES_EXE) $(OBJ_DIR)/jobExecutorServer.o $(OBJ_DIR)/server.o $(OBJ_DIR)/controllerThread.o $(OBJ_DIR)/workerThread.o $(OBJ_DIR)/commands.o $(OBJ_DIR)/waitingBufferQueue.o $(OBJ_DIR)/stringEditor.o

$(OBJ_DIR)/commands.o: $(SRC_DIR)/Server/commands.cpp $(HDR_DIR)/clientCommands.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/commands.o -c $(SRC_DIR)/Server/commands.cpp

$(OBJ_DIR)/jobCommander.o: $(SRC_DIR)/App/jobCommander.cpp $(HDR_DIR)/jobCommanderProcess.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/jobCommander.o -c $(SRC_DIR)/App/jobCommander.cpp

$(OBJ_DIR)/jobExecutorServer.o: $(SRC_DIR)/App/jobExecutorServer.cpp $(HDR_DIR)/jobExecutorServerProcess.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/jobExecutorServer.o -c $(SRC_DIR)/App/jobExecutorServer.cpp

$(OBJ_DIR)/server.o: $(SRC_DIR)/Server/server.cpp $(HDR_DIR)/jobExecutorServerProcess.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/server.o -c $(SRC_DIR)/Server/server.cpp

$(OBJ_DIR)/client.o: $(SRC_DIR)/Client/client.cpp $(HDR_DIR)/jobCommanderProcess.h $(HDR_DIR)/communication.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/client.o -c $(SRC_DIR)/Client/client.cpp

$(OBJ_DIR)/controllerThread.o: $(SRC_DIR)/Server/Threads/controllerThread.cpp $(HDR_DIR)/controllerThread.h $(HDR_DIR)/clientCommands.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/controllerThread.o -c $(SRC_DIR)/Server/Threads/controllerThread.cpp

$(OBJ_DIR)/workerThread.o: $(SRC_DIR)/Server/Threads/workerThread.cpp $(HDR_DIR)/workerThread.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/workerThread.o -c $(SRC_DIR)/Server/Threads/workerThread.cpp

$(OBJ_DIR)/waitingBufferQueue.o: $(SRC_DIR)/Tools/waitingBufferQueue.cpp $(HDR_DIR)/waitingBufferQueue.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/waitingBufferQueue.o -c $(SRC_DIR)/Tools/waitingBufferQueue.cpp

$(OBJ_DIR)/stringEditor.o: $(SRC_DIR)/Tools/stringEditor.cpp $(HDR_DIR)/common.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/stringEditor.o -c $(SRC_DIR)/Tools/stringEditor.cpp

$(OBJ_DIR)/clientReceivers.o: $(SRC_DIR)/Client/clientReceivers.cpp $(HDR_DIR)/communication.h
	$(CC) $(FLAGS) -o $(OBJ_DIR)/clientReceivers.o -c $(SRC_DIR)/Client/clientReceivers.cpp

# Create the build directory for the object files
build:
	mkdir build

# Create the bin directory for the executable files
bin:
	mkdir bin

.PHONY: clean

# Commands that cleans the workspace
clean:
	rm $(EXE_DIR)/$(JC_EXE) $(EXE_DIR)/$(JES_EXE)
	rm $(OBJ_DIR)/jobCommander.o $(OBJ_DIR)/jobExecutorServer.o
	rm $(OBJ_DIR)/client.o $(OBJ_DIR)/server.o
	rm $(OBJ_DIR)/controllerThread.o $(OBJ_DIR)/workerThread.o
	rm $(OBJ_DIR)/commands.o
	rm $(OBJ_DIR)/waitingBufferQueue.o $(OBJ_DIR)/stringEditor.o
	rm $(OBJ_DIR)/clientReceivers.o
	rmdir build
	rmdir bin
