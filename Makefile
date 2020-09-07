CC	:= i686-w64-mingw32-g++

CFLAGS	:= -c -g -Wall -std=c++17 -I../BWAPI/include -I../BWEM/src -I../BWEB/Source

# `-Wl,-Bstatic` is actually required there in order to link static pthread lib
#  (dynamic is for some reason missing when running the exe) and it does not
#  work with `-static-libstdc++`.
LDFLAGS	:= -static-libgcc -L./lib -Wl,-Bstatic -lstdc++ -Wl,-Bstatic -lpthread	\
	  -lBWEB -lBWEM -lBWAPIClient -lBWAPI

SRCDIR	:= src
OBJDIR	:= obj

SRCS	:= $(wildcard $(SRCDIR)/*.cpp)
OBJS	:= $(patsubst $(SRCDIR)/%.o, $(OBJDIR)/%.o, $(SRCS:.cpp=.o))

TARGET	:= camper_ai.exe

all: $(TARGET)

run_linux: $(TARGET)
	wine $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -o $@ $(CFLAGS) $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run_linux clean
