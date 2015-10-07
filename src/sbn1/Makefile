default:
ifdef filename
	mkdir build
	gcc $(filename).c test.c SBN1.c -o build/$(filename).o -Wvarargs
endif

clean:
	rm -rf build

new: clean
	make

run: new
	build/$(filename).o