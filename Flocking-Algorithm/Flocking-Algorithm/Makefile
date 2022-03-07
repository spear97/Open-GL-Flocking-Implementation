all: controllingAgent

controllingAgent.o: controllingAgent.cpp
	g++ -c controllingAgent.cpp -I/usr/X11/include 

Simulator.o: Simulator.cpp
	g++ -c Simulator.cpp -I/usr/X11/include

Agent.o: Agent.cpp
	g++ -c Agent.cpp -I/usr/X11/include

Basic.o: Basic.cpp
	g++ -c Basic.cpp -I/usr/X11/include

Environment.o: Environment.cpp
	g++ -c Environment.cpp -I/usr/X11/include

controllingAgent: controllingAgent.o Simulator.o Agent.o Environment.o Basic.o
	g++ controllingAgent.o Simulator.o Agent.o Environment.o -o ca -framework OpenGL -framework GLUT Basic.o

clean:
	rm *.o ca
