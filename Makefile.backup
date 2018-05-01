CFLAGS = -I ./include
#LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr
LFLAGS = -Wall -lX11 -lGLU -lGL -lm

doomish: doomish.cpp mytime.cpp ppm.cpp
	g++ $(CFLAGS) \
	    doomish.cpp \
		libggfonts.a \
		mytime.cpp \
		ppm.cpp \
	    -Wall -Wextra $(LFLAGS) \
	    -o doomish

#	    #/usr/lib/x86_64-linux-gnu/libopenal.so \
#	    #/usr/lib/x86_64-linux-gnu/libalut.so \
#		#-Wall -Wextra $(LFLAGS) -DUSE_OPENAL_SOUND \

clean:
	rm -f doomish
	rm -f *.o


