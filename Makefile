OBJS = agi.o agi_v2.o agi_v3.o agi_v4.o checks.o cli.o console.o cycle.o font.o \
	getopt.o getopt1.o global.o graphics.o hirespic.o id.o inv.o keyboard.o logic.o \
	lzw.o main.o menu.o motion.o objects.o op_cmd.o op_dbg.o op_test.o \
	patches.o picture.o picview.o rand.o savegame.o silent.o sound.o sprite.o text.o \
	view.o words.o \
	daudio.o fileglob.o path.o cga-none.o 

LIB1 = agi.o agi_v2.o agi_v3.o agi_v4.o checks.o cli.o console.o cycle.o font.o \
	getopt.o getopt1.o global.o graphics.o hirespic.o id.o inv.o keyboard.o logic.o \
	lzw.o menu.o motion.o objects.o op_cmd.o op_dbg.o op_test.o

LIB2 = patches.o picture.o picview.o rand.o savegame.o silent.o sound.o sprite.o text.o \
	view.o words.o daudio.o fileglob.o path.o qlvid.o poll.o

CC = gcc
MEM = 
OPT = -Os
CFLAGS = $(MEM) $(OPT) -D__MSDOS__  -DVERSION="0.8.0-cvs" -Iinclude
ZCFLAGS = $(MEM) -O1 -D__MSDOS__  -Iinclude


sar: lib1.a lib2.a main.o 
	ld -o sar main.o lib1.a lib2.a -lgcc -lc -lgcc
	@copy /Y sar \dos\qlay\sar
	@del /f sar

lib1.a: $(LIB1)
	@del lib1.a
	slb -c lib1.a $(LIB1)

lib2.a: $(LIB2)
	@del lib2.a
	slb -c lib2.a $(LIB2)

clean:
	del main.o $(LIB1) $(LIB2) lib1.a lib2.a sar


daudio.o: msdos\dummy.c
	$(CC) $(CFLAGS) -c -o daudio.o msdos\dummy.c

fileglob.o: qdos\fileglob.c
	$(CC) $(CFLAGS) -c qdos\fileglob.c

path.o: qdos\path.c
	$(CC) $(CFLAGS) -c qdos\path.c

