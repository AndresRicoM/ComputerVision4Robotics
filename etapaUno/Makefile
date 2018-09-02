CXXFLAGS+=-Wall -ggdb
LXXLIBS+=-L/usr/local/lib
LXXLIBS+=-lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio  -lSDL -lpthread 
OBJS=$(patsubst %.cpp,%.o,$(wildcard src/*.cpp))
OBJS+=$(patsubst %.c,%.o,$(wildcard src/*.c))
TARGETDIR = ./bin

all: $(OBJS)
	mkdir -p $(TARGETDIR)
	$(CXX) -o $(TARGETDIR)/etapaUno $(OBJS) $(CXXFLAGS) $(CXXDEFINE)  $(CXXINCLUDE) $(LXXLIBS) 
.cpp.o:
	$(CXX) -c $< $(CXXFLAGS) $(CXXDEFINE)  $(CXXINCLUDE) $(LXXLIBS) -o $@ 

clean:
	$(RM) $(OBJS)
