cc crowbar_npc.c -o ../obj/crowbar_npc.o -c -I../src
cc terrain.c -o ../obj/terrain.o -c -I../src

ARGS="../bin/Lithium.o  -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm"

cc ../obj/crowbar_npc.o -o ../bin/crowbar_npc $ARGS
cc ../obj/terrain.o -o ../bin/terrain $ARGS