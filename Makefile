# CC=gcc
CC=clang
LINTER_DEFINES=-D_GNU_SOURCE
DEBUG_DEFINES=-D__COMPILE_MODE_DEBUG__
INCLUDES=-Iutils -Iutils/macros -Iutils/hashmap -Iutils/exit -Iutils/log -Iinclude -Iservice/include -Iclient/include
OPTIONS=-Wall -Wextra -Werror
OPTIMIZATIONS=-O3
VERSION=0.0.2

LIB=libdsp.so.$(VERSION)

OBJECTS_DIR=object

CREATE_OBJECT_FILE = $(CC) $(OPTIONS) $(DEBUG_DEFINES) $(LINTER_DEFINES) $(INCLUDES) $(OPTIMIZATIONS) -c $(1) -o $(2) -fPIC

all: $(OBJECTS_DIR) build

build: $(LIB)
	ln -s $(LIB) libdsp.so

$(OBJECTS_DIR):
	mkdir $(OBJECTS_DIR)

$(LIB): \
		$(OBJECTS_DIR)/dsp-service-install.o \
		$(OBJECTS_DIR)/dsp-service-call.o \
		$(OBJECTS_DIR)/dsp-service-return.o \
		$(OBJECTS_DIR)/dsp-service.o \
		$(OBJECTS_DIR)/dsp-client-connect.o \
		$(OBJECTS_DIR)/dsp-client-call.o \
		$(OBJECTS_DIR)/dsp-client.o \
		$(OBJECTS_DIR)/commons.o
	$(CC) $(OPTIONS) $(DEBUG_DEFINES) $(LINTER_DEFINES) $(INCLUDES) $(OPTIMIZATIONS) $^ -shared -o $@

$(OBJECTS_DIR)/dsp-service.o: service/dsp-service.c \
		service/include/dsp-service.h \
		include/dsp.h \
		utils/commons.h \
		utils/hashmap/hashmap.h \
		utils/macros/macros.h
	$(call CREATE_OBJECT_FILE,$<,$@)

$(OBJECTS_DIR)/dsp-service-install.o: service/install.c \
		service/include/install.h \
		service/include/dsp-service.h \
		include/dsp.h \
		utils/commons.h
	$(call CREATE_OBJECT_FILE,$<,$@)

$(OBJECTS_DIR)/dsp-service-call.o: service/call.c \
		service/include/call.h \
		service/include/dsp-service.h \
		include/dsp.h \
		utils/commons.h
	$(call CREATE_OBJECT_FILE,$<,$@)

$(OBJECTS_DIR)/dsp-service-return.o: service/return.c \
		service/include/return.h \
		service/include/dsp-service.h \
		include/dsp.h \
		utils/commons.h
	$(call CREATE_OBJECT_FILE,$<,$@)

$(OBJECTS_DIR)/dsp-client.o: client/dsp-client.c \
		client/include/dsp-client.h \
		include/dsp.h \
		utils/commons.h \
		utils/hashmap/hashmap.h \
		utils/macros/macros.h
	$(call CREATE_OBJECT_FILE,$<,$@)

$(OBJECTS_DIR)/dsp-client-connect.o: client/client-connect.c \
		client/include/client-connect.h \
		client/include/dsp-client.h \
		include/dsp.h \
		utils/commons.h
	$(call CREATE_OBJECT_FILE,$<,$@)

$(OBJECTS_DIR)/dsp-client-call.o: client/client-call.c \
		client/include/client-call.h \
		client/include/dsp-client.h \
		include/dsp.h \
		utils/commons.h
	$(call CREATE_OBJECT_FILE,$<,$@)

$(OBJECTS_DIR)/commons.o: utils/commons.c \
		utils/commons.h \
		include/dsp.h \
		utils/macros/macros.h
	$(call CREATE_OBJECT_FILE,$<,$@)

clean:
	rm *.so
	rm *.so.$(VERSION)
	rm -rf $(OBJECTS_DIR)

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/user/dsp-library
