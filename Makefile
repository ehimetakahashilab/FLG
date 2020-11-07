CC = gcc
CFLAGS = -O3
SOURCE_DIR = ./src

DEFFILE = $(SOURCE_DIR)/declare.h $(SOURCE_DIR)/def_flt.h
OBJ = $(SOURCE_DIR)/flist.o $(SOURCE_DIR)/main.o

flg : $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ)

clean:
	rm -f $(SOURCE_DIR)/*.o $(SOURCE_DIR)/*~ ./*~ ./flg
