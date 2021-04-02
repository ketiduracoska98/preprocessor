CC = cl
CFLAGS = /MD /W3 /nologo

build: so-cpp.exe

so-cpp.exe: so-cpp.obj
	$(CC) /so-cpp.exe so-cpp.obj
so-cpp.obj: so-cpp.c
	$(CC) $(CFLAGS) /so-cpp.obj /c so-cpp.c
IGNORE:
clean:
	del /Q /S /F *.obj so-cpp.exe
