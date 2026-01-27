cc crowbar_npc.c -o ../obj/crowbar_npc.o -c -I../src
cc terrain.c -o ../obj/terrain.o -c -I../src
cc minecraft.c -o ../obj/minecraft.o -c -I../src -O3
cc nbodysim.c -o ../obj/nbodysim.o -c -I../src -O3 -march=native

ARGS="../bin/Lithium.o  -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm"
cc ../obj/crowbar_npc.o -o ../bin/crowbar_npc $ARGS
cc ../obj/terrain.o -o ../bin/terrain $ARGS
cc ../obj/minecraft.o -o ../bin/minecraft $ARGS
cc ../obj/nbodysim.o -o ../bin/nbodysim $ARGS