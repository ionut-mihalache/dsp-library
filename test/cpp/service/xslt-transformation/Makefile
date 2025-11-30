CC=cl

SERVICE_NAME=xslt-tranformation.exe

CFLAGS=/nologo /W3 /Zi /EHsc /D_GNU_SOURCE /D__COMPILE_MODE_DEBUG__ \
	/I ../../../../utils /I ../../../../utils/macros /I ../../../../utils/hashmap /I ../../../../utils/exit /I ../../../../utils/log /I ../../../../utils/locking \
	/I ../../../../include /I ../../../../service/include /I ../../../../client/include /O2

all: $(SERVICE_NAME)

$(SERVICE_NAME): main.obj
	$(CC) /Fe$(SERVICE_NAME) main.obj dsp.lib

main.obj: main.cpp
	$(CC) $(CFLAGS) /Fomain.obj /c main.cpp

clean:
	del *.obj *.exe *.ilk *.pdb
# Compiler (from Developer Command Prompt)
# CC=cl
# LD=link

# # Flags
# CFLAGS=/nologo /W3 /Zi /EHsc /D_GNU_SOURCE /D__COMPILE_MODE_DEBUG__ \
# 	/I utils /I utils/macros /I utils/hashmap /I utils/exit /I utils/log /I utils/locking \
# 	/I include /I service/include /I client/include /O2

# LDFLAGS=/DLL /NOLOGO

# VERSION=0.0.2

# DLL_NAME=dsp.dll
# LIB_NAME=dsp.lib

# OBJDIR=object_win
# LIBDIR=lib_win

# OBJS= \
# 	$(OBJDIR)\dsp-service-install.obj \
#  	$(OBJDIR)\dsp-service-call.obj \
#  	$(OBJDIR)\dsp-service-return.obj \
#  	$(OBJDIR)\dsp-service.obj \
#  	$(OBJDIR)\dsp-client-connect.obj \
#  	$(OBJDIR)\dsp-client-call.obj \
#  	$(OBJDIR)\dsp-client.obj \
#  	$(OBJDIR)\commons.obj

# # -------------------------
# # Main targets
# # -------------------------

# all: dirs $(LIBDIR)\$(DLL_NAME)

# dirs:
# 	if not exist $(OBJDIR) mkdir $(OBJDIR)
# 	if not exist $(LIBDIR) mkdir $(LIBDIR)

# $(LIBDIR)\$(DLL_NAME): $(OBJS)
# 	$(LD) $(LDFLAGS) /OUT:$(LIBDIR)\$(DLL_NAME) /IMPLIB:$(LIBDIR)\$(LIB_NAME) $(OBJS)

# # -------------------------
# # Compile rules (explicit)
# # -------------------------

# $(OBJDIR)\dsp-service.obj: service\dsp-service.c
# 	$(CC) $(CFLAGS) /Fo$(OBJDIR)\dsp-service.obj /c service\dsp-service.c

# $(OBJDIR)\dsp-service-install.obj: service\install.c
# 	$(CC) $(CFLAGS) /Fo$(OBJDIR)\dsp-service-install.obj /c service\install.c

# $(OBJDIR)\dsp-service-call.obj: service\call.c
# 	$(CC) $(CFLAGS) /Fo$(OBJDIR)\dsp-service-call.obj /c service\call.c

# $(OBJDIR)\dsp-service-return.obj: service\return.c
# 	$(CC) $(CFLAGS) /Fo$(OBJDIR)\dsp-service-return.obj /c service\return.c

# $(OBJDIR)\dsp-client.obj: client\dsp-client.c
# 	$(CC) $(CFLAGS) /Fo$(OBJDIR)\dsp-client.obj /c client\dsp-client.c

# $(OBJDIR)\dsp-client-connect.obj: client\client-connect.c
# 	$(CC) $(CFLAGS) /Fo$(OBJDIR)\dsp-client-connect.obj /c client\client-connect.c

# $(OBJDIR)\dsp-client-call.obj: client\client-call.c
# 	$(CC) $(CFLAGS) /Fo$(OBJDIR)\dsp-client-call.obj /c client\client-call.c

# $(OBJDIR)\commons.obj: utils\commons.c
# 	$(CC) $(CFLAGS) /Fo$(OBJDIR)\commons.obj /c utils\commons.c

# # -------------------------
# # Clean
# # -------------------------

# clean:
# 	if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
# 	if exist $(LIBDIR) rmdir /s /q $(LIBDIR)
# 	if exist $(DLL_NAME) del $(DLL_NAME)
# 	if exist $(LIB_NAME) del $(LIB_NAME)
