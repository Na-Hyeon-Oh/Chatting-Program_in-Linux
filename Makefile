# Makefile

TARGET = server client
SOURCES = *.c
OBJECTS = *.o

CXX = gcc
CXXFLAGS = -Wall

LIBRARY = -lpthread

$(TARGET) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o server server.o
	$(CXX) $(CXXFLAGS) -o client client.o
$(OBJECTS) : $(SOURCES)
	$(CXX) $(CXXFLAGS) -c $(SOURCES)

all: $(RARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)
