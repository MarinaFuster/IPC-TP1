COMPILER=gcc
APP_OUTPUT_FILE=application
VIEW_OUTPUT_FILE=view
FLAGS=-pthread

all: application.o view.o
		 $(COMPILER) -o $(APP_OUTPUT_FILE) application.o $(FLAGS) 
		 $(COMPILER) -o $(VIEW_OUTPUT_FILE) view.o $(FLAGS)

application.o: application.c 
			$(COMPILER) -c application.c

view.o: view.c 
			$(COMPILER) -c view.c

clean:
			rm -rf $(APP_OUTPUT_FILE) $(VIEW_OUTPUT_FILE) *.o *.h.gch