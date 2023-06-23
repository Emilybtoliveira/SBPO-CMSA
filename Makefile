CPLEXFLAGS=-O3 -m64 -O -fPIC -fexceptions -DNDEBUG -DIL_STD -I/opt/ibm/ILOG/CPLEX_Studio129/cplex/include -I/opt/ibm/ILOG/CPLEX_Studio129/concert/include  -L/opt/ibm/ILOG/CPLEX_Studio129/cplex/lib/x86-64_linux/static_pic -lilocplex -lcplex -L/opt/ibm/ILOG/CPLEX_Studio129/concert/lib/x86-64_linux/static_pic -lconcert -lm -pthread -std=c++0x -ldl

CFLAGS=-std=c++11 -static-libstdc++ -static-libgcc -Wall -g

all:PCCP_CMSA.o 
	g++  PCCP_CMSA.o -o PCCP_CMSA.run $(CPLEXFLAGS) $(CFLAGS)


%.o: %.cpp %.hpp
	g++ -c $< -o $@ $(CFLAGS)

PCCP_CMSA.o: PCCP_CMSA.cpp
	g++ -c -o PCCP_CMSA.o PCCP_CMSA.cpp $(CPLEXFLAGS) $(CFLAGS)

clean:
	rm -f *.o
