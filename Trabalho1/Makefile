EXE = despacho
SRC = $(EXE).awk

all: $(EXE)

$(EXE): $(SRC)
	@ cp $< $@ && chmod +x $@
	@ echo "'$@' succesfully created!"

clean:
	@ rm -f $(EXE)

.PHONY: clean
