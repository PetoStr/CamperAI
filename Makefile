CC	= i686-w64-mingw32-g++

CFLAGS	= -Wall -std=c++17 -I../BWAPI/include -I../BWEM/src -I../BWEB/Source

# `-Wl,-Bstatic` is actually required there in order to link static pthread lib
#  (dynamic is for some reason missing when running the exe) and it does not
#  work with `-static-libstdc++`.
LDFLAGS	= -static-libgcc -L./lib -Wl,-Bstatic -lstdc++ -Wl,-Bstatic -lpthread	\
	  -lBWEB -lBWEM -lBWAPIClient -lBWAPI

SRCS	= $(wildcard src/*.cpp)
HEADERS	= $(wildcard src/*.hpp)

TARGET	= CamperAI.exe

all: $(TARGET)

run_linux: $(TARGET)
	wine $<

$(TARGET): $(SRCS) $(HEADERS)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all run_linux clean
