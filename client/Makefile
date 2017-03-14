CPP_CODE_FILES := $(sort $(wildcard *.cpp) $(wildcard *.cc))
CPP_OBJ_FILES := $(addsuffix .o,$(basename $(CPP_CODE_FILES)))
H_FILES := $(wildcard *.h)

.PHONY: all clean

all: test_client

test_client: $(CPP_OBJ_FILES) 
	g++ $(CPP_OBJ_FILES) -o test_client

%.o: %.cpp $(H_FILES)
	g++ $< -I -ldl -std=c++11 -c -o $@ -g -fno-inline

clean:
	-rm -rf *.o
	-rm test_client

