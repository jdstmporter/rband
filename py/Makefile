
PREFIX := /usr/local

CXX	:= g++
CC := gcc

MKDIR	:= mkdir -p

listItems = $(shell find $1 -name $2)
objectList = $(patsubst %.$2,%.o,$(filter %.$2, $(call listItems,$1,*.$2)))

INCLUDES := -Isrc -I/usr/local/include

OPTFLAGS	:= -DNDEBUG
CFLAGS		:= -O3
CXXFLAGS	:= $(CFLAGS) $(INCLUDES) -std=c++17 

APP := tests/rb
OBJECTS := $(filter-out src/rubber.o,$(call objectList,src,cpp))
LIBS     := -lrubberband -lsndfile -L/usr/local/lib


.PHONY: all
all:	$(APP)

$(APP): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(LIBS) $^ -o $@ 


.PHONY: clean
clean:
	rm -f $(OBJECTS) 

.PHONY: distclean
distclean:	clean
	rm -f $(APP)

.PHONY: list	
list:
	$(info Objects is $(OBJECTS))
	$(info LDFLAGS  = $(LDFLAGS) $(LIBS))
	$(info CFLAGS   = $(CFLAGS))
	$(info CXXFLAGS = $(CXXFLAGS))

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
