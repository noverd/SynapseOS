all: libvalera.so

libvalera.so:
	@echo "Compiling Valera shared library..."
	@gcc valera.c -shared -o libvalera.so

example:
	@echo "Compiling example..."
	@gcc -ggdb valera.c examples/example.c -I. -o ./example
	@echo "Run program with ./example"

json:
	@echo "Compiling json..."
	@gcc valera.c examples/json-example.c -I. -o ./json
	@echo "Run program with ./json"

simple:
	@echo "Compiling simple..."
	@gcc valera.c examples/simple.c -I. -o ./simple
	@echo "Run program with ./simple"

join:
	@echo "Compiling join..."
	@gcc valera.c examples/join.c -I. -o ./join
	@echo "Run program with ./join"

aoo:
	@echo "Compiling AOO..."
	@gcc valera.c examples/array_objs.c -I. -o ./aoo
	@echo "Run program with ./aoo"

install: libvalera.so
	install libvalera.so $(PREFIX)/lib/
	install valera.h $(PREFIX)/include
	@echo "================================="
	@echo "| Valera installed succesfully! |"
	@echo "================================="
