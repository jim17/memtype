
DIAGRAMS=$(wildcard diagrams/*.shaape)

all: memtype.html diagrams

memtype.html: memtype.adoc
	asciidoctor memtype.adoc

diagrams: $(DIAGRAMS).png 
	@echo Generating diagrams ... $(DIAGRAMS)

$(DIAGRAMS).png:
	shaape $(DIAGRAMS)

diagrams-clean:
	rm -rf diagrams/*.png
clean:
	rm -rf *.html
