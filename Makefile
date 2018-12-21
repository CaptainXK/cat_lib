.PHONY:clean check_obj_dir check_lib_dir

CC := gcc
OBJ_DIR := obj
LIB_DIR := lib
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS) )
INC := -I.
CFLAGS := -g3 -O2
LIBS := -lpqos

test.app:check_obj_dir $(OBJS)
	$(CC) $(INC) $(OBJS) -o $@ $(CFLAGS) $(LIBS)

slib:check_lib_dir $(OBJS)
	ar crv $(LIB_DIR)/libcat.a $(OBJ_DIR)/*.o

check_obj_dir:
	@if test ! -d $(OBJ_DIR);\
		then\
		mkdir $(OBJ_DIR);\
	fi

check_lib_dir:
	@if test ! -d $(LIB_DIR);\
		then\
		mkdir $(LIB_DIR);\
	fi

$(OBJ_DIR)/%.o:%.c
	$(CC) $(INC) -c $< -o $@ $(CFLAGS)


test:test.app
	$(EXEC) ./test.app


clean:
	rm -r $(OBJ_DIR)/*.o *.app lib/*
