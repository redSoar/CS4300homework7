OBJS = Homework7.o View.o Controller.o Model.o
INCLUDES = -I../include
LIBS = -L../lib
LDFLAGS = -lglad -lglfw3
CFLAGS = -g -std=c++11
PROGRAM = Homework7


ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    LDFLAGS += -lopengl32 -lgdi32
    PROGRAM :=$(addsuffix .exe,$(PROGRAM))
	COMPILER = g++
else ifeq ($(shell uname -s),Darwin)     # is MACOSX
    LDFLAGS += -framework Cocoa -framework OpenGL -framework IOKit
	COMPILER = clang++
endif

Homework7: $(OBJS)
	$(COMPILER) -o $(PROGRAM) $(OBJS) $(LIBS) $(LDFLAGS)

Homework7.o: Homework7.cpp
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Homework7.cpp

View.o: View.cpp View.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c View.cpp	

Controller.o: Controller.cpp Controller.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Controller.cpp	

Model.o: Model.cpp Model.h
	$(COMPILER) $(INCLUDES) $(CFLAGS) -c Model.cpp		
	
RM = rm	-f
ifeq ($(OS),Windows_NT)     # is Windows_NT on XP, 2000, 7, Vista, 10...
    RM := del
endif

clean: 
	$(RM) $(OBJS) $(PROGRAM)
    