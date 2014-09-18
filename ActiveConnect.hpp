#ifndef ACTIVECONNECT_HPP
#  define ACTIVECONNECT_HPP

#  include <vector>
#  include <list>
using std::vector;
using std::list;

#  ifndef PROGRAM_HPP
#    include "Program.hpp"
#  endif
#  ifndef SYSTEMVAR_HPP
#    include "SystemVar.hpp"
#  endif

void ActiveConnect(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "ActiveConnect";
   static IntArg StartNeuron("-Nstart", "start neuron", 1);
   static IntArg EndNeuron("-Nend", "end neuron{-1 gives last neuron}", -1);
   static IntArg StartPat("-Pstart", "start pattern(or vector)", 1);
   static IntArg EndPat("-Pend", "end pattern(or vector) {-1 gives end time}",
                        -1);
   static StrArg SeqName("-name", "sequence name");
   static StrArg ResetSeq("-reset", "reset sequence", "{zeros}");
   static StrArg External("-ext", "external sequence", "{zeros}");
   static FlagArg DoSummary("-sum", "-nosum", "print a summary", 1);
   static int argunset = true;
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@ActiveConnect() \n"
                   "\tGenerates the data about conditional probabilities of firing\n"
                   "\tand saves the results into the ActiveConBuffer.\n"
                   "\tThe reset sequence is used for the 0 time step and\n"
                   "\tif externals are given then they are ignored.\n"
                   "\tActiveConBuffer is a matrix with 6 Vectors:\n"
                   "\t\t1: TimeStep(Pattern Number)\n"
                   "\t\t2: Absolute Activity Last Time\n"
                   "\t\t3: Absolute Activity This Time - Externals\n"
                   "\t\t4: # of Conns between CoActive Neurons\n"
                   "\t\t5: 4/3 = Ave # of Active Conns for Active Neurons\n"
                   "\t\t6: 4/(2*3) = Prob cij=1 given zi=1,zj=1\n"
                   "\tActiveConnect also updates the variable Pczz which\n"
                   "\tholds the Prob(cij=1 | zi=1,zj=1) for the whole sequence\n");
      ComL.IntSet(4, &StartNeuron, &EndNeuron, &StartPat, &EndPat);
      ComL.StrSet(3, &SeqName, &ResetSeq, &External);
      ComL.FlagSet(1, &DoSummary);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   if (!program::Main().getNetworkCreated()) {
      CALL_ERROR << "Error: You must call @CreateNetwork() before you can "
                    "call @ActiveConnect." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   UIPtnSequence Seq = SystemVar::getSequence(SeqName, FunctionName, ComL);
   int SeqSize = Seq.size();
   UIPtnSequence ReSeq;
   if (ResetSeq.getValue() != "{zeros}") {
      ReSeq = SystemVar::getSequence(ResetSeq, FunctionName, ComL);
   } else {
      ReSeq.resize(SeqSize, UIVector(0));
   }
   int lastNeuron = 0;
   for (UIPtnSequenceCIt it = Seq.begin(); it != Seq.end(); it++) {
      updateMax(lastNeuron, it->back());
   }
   if (EndNeuron.getValue() == -1) {
      EndNeuron.setValue(lastNeuron);
   }
   if (EndPat.getValue() == -1) {
      EndPat.setValue(SeqSize);
   }
   if (StartNeuron.getValue() < 1 ||
         StartNeuron.getValue() > EndNeuron.getValue() ||
         EndNeuron.getValue() > static_cast<int>(ni)) {
      CALL_ERROR << "Error in " << FunctionName << ": Neuron range of "
          << StartNeuron.getValue() << "..." << EndNeuron.getValue()
          << " is invalid." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
   }
   if (EndPat.getValue() < 1 || EndPat.getValue() > SeqSize ||
          StartPat.getValue() < 1 || StartPat.getValue() > EndPat.getValue()) {
      CALL_ERROR << "Error in " << FunctionName << ": Pattern range of "
         << StartPat.getValue() << "..." << EndPat.getValue() << " is invalid."
         << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
   }

   // get externals
   UIMatrix ExtSeq;
   if (External.getValue() != "{zeros}") {
      ExtSeq = SystemVar::getSequence(External, FunctionName, ComL);
   } else {
      ExtSeq.resize(SeqSize, UIVector(0));
   }

   // Do the loops
   int NumPats = EndPat.getValue() - StartPat.getValue() + 1;
   DataList Ave(NumPats);
   DataList Sum(NumPats);
   DataList Pats(NumPats);
   DataList jNumFired(NumPats);
   DataList iNumFired(NumPats);
   DataList PczzV(NumPats);
   int index = 0;
   UIPtnSequenceCIt SeqConstIterator = Seq.begin();
   UIPtnSequenceCIt ExtConstIterator = ExtSeq.begin();   
   UIVector pat_b4;
   if (StartPat.getValue() == 1) {
      pat_b4 = ReSeq.front();
   } else {
      for (int inc = 0; inc < StartPat.getValue() - 2; ++inc) ++SeqConstIterator;
      pat_b4 = *(SeqConstIterator++);
   }
   for (int t = StartPat.getValue(); t <= EndPat.getValue(); ++t, ++index, ++SeqConstIterator, ++ExtConstIterator) {
      Pats.at(index) = static_cast<float>(t);
      Pattern pat = UIVectorToPtn(*SeqConstIterator, lastNeuron);
      Pattern pat_ext = UIVectorToPtn(*ExtConstIterator, lastNeuron);
      for (int j = StartNeuron.getValue(); j <= EndNeuron.getValue(); j++) {
         // count the number on
         if (pat_b4.at(j-1)) ++iNumFired.at(index);
         // do the calculation for non-externals and active
         if (pat.at(j-1) && (!(pat_ext.at(j-1)))) {
            ++jNumFired.at(index);
            for (unsigned int c = 0; c < FanInCon.at(j-1); c++) {
               if (pat_b4.at(inMatrix[j-1][c].getSrcNeuron()))
                  ++Sum.at(index);
            }
         }
      }
      pat_b4 = pat;
   }

   // Output the data
   Output::Out() << "\nSummed Active Connections in sequence " << SeqName.getValue()
                 << " using neurons " << StartNeuron.getValue() << "..."
                 << EndNeuron.getValue() << " and patterns " << StartPat.getValue()
                 << "..." << EndPat.getValue() << "\n\tignoring externals given by "
                 << External.getValue();

   if (DoSummary.getValue()) {
      Output::Out() << "\n" << std::setw(11) << "1:TimeStep"
                    << std::setw(9) << "2:#z(t-1)"
                    << std::setw(9) << "3:#z(t)"
                    << std::setw(15) << "4:#cz(t-1)z(t)"
                    << std::setw(9) << "4/3" << std::setw(9) << "4/(2*3)" << "\n";
   }

   float TotalSumC = 0.0f;
   float TotalSumZ = 0.0f;

   for (int i = 0; i < index; i++) {
      if (fabs(Sum.at(i)) < verySmallFloat) {
         Ave.at(i) = 0.0f;
         PczzV.at(i) = 0.0f;
      } else {
         Ave.at(i) = Sum.at(i) / jNumFired.at(i);
         PczzV.at(i) = Ave.at(i) / iNumFired.at(i);
      }
      TotalSumC += Sum.at(i);
      TotalSumZ += iNumFired.at(i) * jNumFired.at(i);
      if (DoSummary.getValue()) {
         Output::Out() << "\n" << std::setw(11) << static_cast<int>(Pats.at(i))
                       << std::setw(9) << static_cast<int>(iNumFired.at(i))
                       << std::setw(9) << static_cast<int>(jNumFired.at(i))
                       << std::setw(15) << static_cast<int>(Sum.at(i))
                       << std::setw(9) << Ave.at(i)
                       << std::setw(9) << PczzV.at(i);
      }
   }
   Output::Out() << "\n";

   float Pczz;
   if (fabs(TotalSumC) < verySmallFloat) {
      Pczz = 0.0f;
   } else {
      Pczz = TotalSumC / TotalSumZ;
   }
   SystemVar::SetFloatVar("Pczz", Pczz);

   if (DoSummary.getValue()) {
      Output::Out() << "Average: " << Pczz << "\n";
   }
   // Save the vector to the ActiveConBuffer
   DataMatrix ActiveConMat;
   ActiveConMat.push_back(Pats);
   ActiveConMat.push_back(iNumFired);
   ActiveConMat.push_back(jNumFired);
   ActiveConMat.push_back(Sum);
   ActiveConMat.push_back(Ave);
   ActiveConMat.push_back(PczzV);
   SystemVar::insertData("ActiveConBuffer", ActiveConMat, DLT_matrix);

   return;
}

void BindActiveConnect()
{
   SystemVar::AddFloatVar("Pczz", 0.0f, true);
   SystemVar::AddAtFun("ActiveConnect", ActiveConnect);
   return;
}

#endif
