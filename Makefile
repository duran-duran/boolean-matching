CC = gcc
CPP = g++

CFLAGS = -std=gnu99 -g -Wall
CPPFLAGS = -std=gnu++0x -g -Wall
LDFLAGS = -lm -ldl -rdynamic -lpthread -lcrypto

LEX = flex
YACC = bison

SOURCES_DIR = ./src
BUILD_DIR = ./build
BENCH_DIR = ./bench

OUT_FILE = $(BUILD_DIR)/result.out

SOURCES_C = $(wildcard $(SOURCES_DIR)/*.c)
SOURCES_CPP = $(wildcard $(SOURCES_DIR)/*.cpp)
SOURCES = $(SOURCES_C) $(SOURCES_CPP) $(SOURCES_DIR)/verilog.ypp $(SOURCES_DIR)/verilog.l
OBJECTS_C = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES_C:.c=.o)))
OBJECTS_CPP = $(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES_CPP:.cpp=.o)))
OBJECTS = $(OBJECTS_C) $(OBJECTS_CPP) $(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/verilog.tab.o

EXECUTABLE = $(BUILD_DIR)/matcher

ABC_DIR = ./alanmi-abc-f3bca91bd507
ABC_LIB = $(BUILD_DIR)/libabc.a

.PHONY: all clean

all: $(BUILD_DIR) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(ABC_LIB)
	$(CPP) -o $(EXECUTABLE) $(OBJECTS) $(ABC_LIB) $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/lex.yy.o $(BUILD_DIR)/verilog.tab.o: $(SOURCES_DIR)/verilog.l $(SOURCES_DIR)/verilog.ypp
	$(LEX) $(SOURCES_DIR)/verilog.l
	mv lex.yy.c $(BUILD_DIR)
	$(YACC) --defines=$(BUILD_DIR)/verilog.tab.h --output=$(BUILD_DIR)/verilog.tab.cpp $(SOURCES_DIR)/verilog.ypp
	$(CC) -I $(SOURCES_DIR) -I $(BUILD_DIR) $(CFLAGS) -c $(BUILD_DIR)/lex.yy.c -o $(BUILD_DIR)/lex.yy.o
	$(CPP) -I $(SOURCES_DIR) -I $(BUILD_DIR) $(CPPFLAGS) -c $(BUILD_DIR)/verilog.tab.cpp -o $(BUILD_DIR)/verilog.tab.o

$(BUILD_DIR)/%.o: $(SOURCES_DIR)/%.c
	$(CC) -I $(SOURCES_DIR) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	$(CPP) -I $(SOURCES_DIR) $(CPPFLAGS) -c $< -o $@

$(ABC_LIB):
	cd $(ABC_DIR) && make -j9 libabc.a READLINE=0
	mv $(ABC_DIR)/libabc.a $(ABC_LIB)

clean:
	rm -r $(BUILD_DIR)

test-%:
	$(EXECUTABLE) $(BENCH_DIR)/$*/cir1.v $(BENCH_DIR)/$*/cir2.v $(OUT_FILE)
