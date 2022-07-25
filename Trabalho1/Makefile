EXE = despacho
AWK = $(EXE).awk

all: $(EXE)

$(EXE): $(AWK)
	cp $< $@
	chmod +x $@

clean:
	@ rm -f $(EXE)

.PHONY: clean
