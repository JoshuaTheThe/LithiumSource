cc main.c -o ../obj/main.o -c -I../src
cc ../obj/main.o ../bin/Lithium.o -o ../bin/main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm