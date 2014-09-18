########################################
###       Makefile for NeuroJet      ###
###    Ben Hocking, 2003 March 21    ###
###   Blake Thomas, 2011 Dec 14      ###
########################################

# Executable #
NAME1 = NeuroJet

GC = g++
OPTIONS =-O3 -fpermissive

OBJ1 = NeuroJet.o ArgFuncts.o Calc.o Interneuron.o NeuronType.o Noise.o Output.o Parser.o Population.o Program.o rdtsc.o Synapse.o SynapseType.o SystemVar.o

BINDLISTHPPINC = BindList.hpp
DATATYPESHPPINC = DataTypes.hpp
OUTPUTHPPINC = Output.hpp

ARGFUNCTSHPPINC = ArgFuncts.hpp $(DATATYPESHPPINC) $(OUTPUTHPPINC)
FILTERHPPINC = Filter.hpp $(DATATYPESHPPING)

INTERNEURONHPPINC = Interneuron.hpp $(ARGFUNCTSHPPINC) $(DATATYPESHPPINC)
NOISEHPPINC = Noise.hpp $(ARGFUNCTSHPPINC)
SYMBOLSHPPINC = Symbols.hpp $(ARGFUNCTSHPPINC)
SYNAPSETYPEHPPINC = SynapseType.hpp $(FILTERHPPINC)
SYSTEMVARHPPINC = SystemVar.hpp $(ARGFUNCTSHPPINC) $(BINDLISTHPPINC)

CALCHPPINC = Calc.hpp $(NOISEHPPINC) $(SYSTEMVARHPPINC)
NEURONTYPEHPPINC = NeuronType.hpp $(DATATYPESHPPINC) $(FILTERHPPINC) $(SYNAPSETYPEHPPINC) $(SYSTEMVARHPPINC)
PARALLELRANDHPPINC = ParallelRand.hpp $(NOISEHPPINC)
SYNAPSEHPPINC = Synapse.hpp $(ARGFUNCTSHPPINC) $(NOISEHPPINC) $(SYNAPSETYPEHPPINC)
WEIGHTANALYSISHPPINC = WeightAnalysis.hpp  $(SYSTEMVARHPPINC)

PARALLELHPPINC = Parallel.hpp $(PARALLELRANDHPPINC) $(SYNAPSEHPPINC)
PARSERHPPINC = Parser.hpp $(CALCHPPINC) $(SYSTEMVARHPPINC)
POPULATIONHPPINC = Population.hpp $(DATATYPESHPPINC) $(INTERNEURONHPPINC) $(NEURONTYPEHPPINC)
PROGRAMHPPINC = Program.hpp $(ARGFUNCTSHPPINC) $(BINDLISTHPPINC) $(FILTERHPPINC) $(INTERNEURONHPPINC) $(OUTPUTHPPINC) $(NOISEHPPINC) $(PARALLELRANDHPPINC)

USERHPPINC = User.hpp $(WEIGHTANALYSISHPPINC)

NEUROJETHPPINC = NeuroJet.hpp $(NEURONTYPEHPPINC) $(PARALLELHPPINC) $(USERHPPINC) $(SYNAPSEHPPINC)

clean:
	rm *.o $(NAMEA) $(NAMEB) $(NAMEC) $(NAMEPSC) $(NAME1)

$(NAME1): $(OBJ1)
	$(GC) -o $(NAME1) $(OBJ1)

NEURODEPEND = NeuroJet.cpp $(CALCHPPINC) $(NEUROJETHPPINC) $(POPULATIONHPPINC) $(PARSERHPPINC) Makefile

NeuroJet.o: $(NEURODEPEND)
	$(GC) $(OPTIONS) -c NeuroJet.cpp -o NeuroJet.o

ARGDEPEND = ArgFuncts.cpp $(ARGFUNCTSHPPINC) Makefile
ArgFuncts.o: $(ARGDEPEND)
	$(GC) $(OPTIONS) -c ArgFuncts.cpp -o ArgFuncts.o

CALCDEPEND = Calc.cpp $(CALCHPPINC) Makefile
Calc.o: $(CALCDEPEND)
	$(GC) $(OPTIONS) -c Calc.cpp -o Calc.o


INTERNRNDEPEND = Interneuron.cpp $(INTERNEURONHPPINC) $(FILTERHPPINC) $(OUTPUTHPPINC) $(PARSERHPPINC) Makefile
Interneuron.o: $(INTERNRNDEPEND)
	$(GC) $(OPTIONS) -c Interneuron.cpp -o Interneuron.o


NEURONTYPEDEPEND = NeuronType.cpp $(CALCHPPINC) $(NEURONTYPEHPPINC) $(OUTPUTHPPINC) $(SYSTEMVARHPPINC) Makefile
NeuronType.o: $(NEURONTYPEDEPEND)
	$(GC) $(OPTIONS) -c NeuronType.cpp -o NeuronType.o

NOISEDEPEND = Noise.cpp $(NOISEHPPINC) Makefile
Noise.o: $(NOISEDEPEND)
	$(GC) $(OPTIONS) -c Noise.cpp -o Noise.o

OUTPUTDEPEND = Output.cpp $(OUTPUTHPPINC) $(DATATYPESHPPINC) Makefile
Output.o: $(OUTPUTDEPEND)
	$(GC) $(OPTIONS) -c Output.cpp -o Output.o

PARSERDEPEND = Parser.cpp $(PARSERHPPINC) $(CALCHPPINC) $(OUTPUTHPPINC) $(PARALLELHPPINC) $(SYMBOLSHPPINC) Makefile
Parser.o: $(PARSERDEPEND)
	$(GC) $(OPTIONS) -c Parser.cpp -o Parser.o

POPULATIONDEPEND = Population.cpp $(POPULATIONHPPINC) $(SYSTEMVARHPPINC) Makefile
Population.o: $(POPULATIONDEPEND)
	$(GC) $(OPTIONS) -c Population.cpp -o Population.o

PGMDEPEND = Program.cpp $(CALCHPPINC) $(PROGRAMHPPINC) $(ALGORITHMHPPINC) $(NEURONTYPEHPPINC) $(PARALLELHPPINC) $(SYMBOLSHPPINC) $(SYNAPSEHPPINC) $(SYSTEMVARHPPINC) Makefile
Program.o: $(PGMDEPEND)
	$(GC) $(OPTIONS) -c Program.cpp -o Program.o

SYNAPSEDEPEND = Synapse.cpp $(SYNAPSEHPPINC) Makefile
Synapse.o: $(SYNAPSEDEPEND)
	$(GC) $(OPTIONS) -c Synapse.cpp -o Synapse.o

SYNAPSETYPEDEPEND = SynapseType.cpp $(SYNAPSETYPEHPPINC) $(OUTPUTHPPINC) Makefile
SynapseType.o: $(SYNAPSETYPEDEPEND)
	$(GC) $(OPTIONS) -c SynapseType.cpp -o SynapseType.o

rdtsc.o: rdtsc.s
	$(GC) $(OPTIONS) -c rdtsc.s -o rdtsc.o

SYSVARDEPEND = SystemVar.cpp $(OUTPUTHPPINC) $(SYSTEMVARHPPINC) Makefile
SystemVar.o: $(SYSVARDEPEND)
	$(GC) $(OPTIONS) -c SystemVar.cpp -o SystemVar.o

NeuroJet-depends.dvi: NeuroJet-depends.tex
	latex NeuroJet-depends
