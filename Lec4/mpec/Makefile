
OBJ_DIR = ./obj/

OCAML_SRC = ./src/

GENTUPLE_OBJ = ./obj/lexer.o ./obj/parser.o ./obj/gentuple_ocaml_wrapper.o

ZCHAFF_OBJ_DIR  = ./zchaff/obj/

ZCORE_OBJ_DIR = ./zchaff/obj/

ZCHAFF_OBJ = $(ZCHAFF_OBJ_DIR)zchaff_base.o \
             $(ZCHAFF_OBJ_DIR)zchaff_c_wrapper.o  \
	     $(ZCHAFF_OBJ_DIR)zchaff_solver.o \
             $(ZCHAFF_OBJ_DIR)zchaff_cpp_wrapper.o \
	     $(ZCHAFF_OBJ_DIR)zchaff_dbase.o  \
             $(ZCHAFF_OBJ_DIR)zchaff_utils.o  \
             $(ZCHAFF_OBJ_DIR)zchaff_solver_helper.o \
             $(ZCHAFF_OBJ_DIR)zchaff_ocaml_wrapper.o 

ZCORE_OBJ = $(ZCORE_OBJ_DIR)zcore_ocaml_wrapper.o \
            $(ZCORE_OBJ_DIR)zcore_c_wrapper.o  \
            $(ZCORE_OBJ_DIR)zcore_extract.o

OCAML_OBJS = $(OBJ_DIR)zchaff.cmo


OCAML_FILES = $(OCAML_DIR)zchaff.ml

all : mpec

mpec: zchaff.cmo
	make -C ./gentuple
	make -C ./zchaff
	ocamlc -custom -cc g++ -o mpec $(GENTUPLE_OBJ) \
		$(ZCHAFF_OBJ) $(OBJ_DIR)zchaff.cmo \
		unix.cma  $(OCAML_SRC)mpec.ml 

top : 
	ocamlc -c ./src/mpec.ml
	ocamlmktop -custom -cc g++ -o mpectop $(GENTUPLE_OBJ) \
	       $(ZCHAFF_OBJ) $(OBJ_DIR)zchaff.cmo \
		unix.cma  ./src/mpec.cmo 

zchaff.cmo:
	ocamlc -c $(OCAML_SRC)zchaff.mli
	cp $(OCAML_SRC)zchaff.cmi ./
	ocamlc -c $(OCAML_SRC)zchaff.ml
	cp $(OCAML_SRC)zchaff.cm? ./obj

clean:
	@echo Cleaning ...Done 
	@rm -f ./gentuple/*.o
	@rm -rf ./zchaff/obj/*.o
	@rm -f  ./obj/*.*
	@rm -rf mpec 
	@rm resolve_trace
	
