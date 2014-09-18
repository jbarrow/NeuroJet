#if !defined(WEIGHTANALYSIS_HPP)
#  define WEIGHTANALYSIS_HPP


#  if !defined(PROGRAM_HPP)
#    include "Program.hpp"
#  endif
#  if !defined(SYSTEMVAR_HPP)
#    include "SystemVar.hpp"
#  endif

void getContextStats(const Pattern &FireVect, const int TotalTimeSteps,
           const int tolerance, const int isLoop, bool &multifire, int &context)
{
   // initialize
   context = 0;
   multifire = false;

   bool hasfired = false;
   bool  turnedoff = false;
   int rpeat = tolerance;
   for (int t = 0; t < TotalTimeSteps; t++) {
      bool fired = BOOL(FireVect.at(t));
      if (fired && hasfired && turnedoff) {
         if (rpeat < 0) {
            multifire = true;
            if (isLoop) {
               hasfired = false;
               turnedoff = false;
               context = 0;
               rpeat = tolerance;
               multifire = false;
               // run the context algorithm starting here and then loop back
               for (int t2 = t; t2 < TotalTimeSteps; t2++) {
                  fired = BOOL(FireVect.at(t2));
                  if (fired && hasfired && turnedoff) {
                     if (rpeat < 0) {
                        multifire = true;
                        break;
                     } else {
                        rpeat = tolerance;
                        context++;
                        turnedoff = false;
                     }
                  } else if (fired && hasfired) {
                     context++;
                  } else if (fired && (!hasfired)) {
                     hasfired = true;
                     context = 1;
                  } else if ((!fired) && hasfired) {
                     turnedoff = true;
                     rpeat--;
                  }
               }
               // now loop back
               if (multifire) break;

               for (int t3 = 0; t3 < t; t3++) {
                  fired = BOOL(FireVect.at(t3));
                  if (fired && hasfired && turnedoff) {
                     if (rpeat < 0) {
                        multifire = true;
                        break;
                     } else {
                        rpeat = tolerance;
                        context++;
                        turnedoff = false;
                     }
                  } else if (fired && hasfired) {
                     context++;
                  } else if (fired && (!hasfired)) {
                     hasfired = true;
                     context = 1;
                  } else if ((!fired) && hasfired) {
                     turnedoff = true;
                     rpeat--;
                  }
               }
            }
            break;
         } else {
            rpeat = tolerance;
            context++;
            turnedoff = false;
         }
      } else if (fired && hasfired) {
         context++;
      } else if (fired && (!hasfired)) {
         hasfired = true;
         context = 1;
      } else if ((!fired) && hasfired) {
         turnedoff = true;
         rpeat--;
      }
   }
}

void WeightAnalysis(ArgListType &arg) //AT_FUN
{
   // process the function arguments
   static string FunctionName = "WeightAnalysis";
   static IntArg NumBins("-bins", "number of bins", 15);
   static StrArg SeqName("-name", "sequence name", "{ones}");
   static FlagArg UsedOnly("-used", "-unused", "include unused neurons", 0);
   static FlagArg Loop("-loop", "-noloop", "assume loop sequence", 0);
   static IntArg Con("-con", "calculate only for a certain context"
                     "\n\t\t\t length (-1 ignores)", -1);
   static FlagArg DoSummary("-sum", "-nosum", "print a summary", 1);
   static DblArg CutOff("-zero",
                        "zero cutoff {< 0 defaults to ZeroCutOff}", -1.0);
   static IntArg Tolerance("-tol", "tolerance", 0);
   static int argunset = true;
   static CommandLine ComL(FunctionName);
   if (argunset) {
      ComL.HelpSet("@WeightAnalysis(...) \n"
                   "\tGenerates the data about weight distributions\n"
                   "\tand saves the results into WeightBuffer.\n"
                   "\tVector 1 of WeightBuffer is the bin centers.\n"
                   "\tVector 2 of WeightBuffer is fraction of included weights.\n"
                   "\tVector 3 of WeightBuffer is fraction of all weights.\n"
                   "\tPuts average of all into AveAllWeights.\n"
                   "\tPuts average of nonzero into AveNonZeroWeights.\n"
                   "\tPuts fraction of zero into FracZeroWeights.\n"
                   "\tIf the flags -used or -con are used then a sequence\n"
                   "\tis expected.\n");
      ComL.IntSet(3, &NumBins, &Con, &Tolerance);
      ComL.StrSet(1, &SeqName);
      ComL.FlagSet(3, &UsedOnly, &Loop, &DoSummary);
      ComL.DblSet(1, &CutOff);
      argunset = false;
   }
   ComL.Process(arg, Output::Err());

   if (!program::Main().getNetworkCreated()) {
      CALL_ERROR << "Error: You must call "
          "@CreateNetwork() before you can call " "@" << FunctionName << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   // check for errors in the arguments

   if (CutOff.getValue() < 0.0) {
      CutOff.setValue(SystemVar::GetFloatVar("ZeroCutOff"));
   }
   
   Sequence InvSeq;
   int SeqSize = 0;
   if (NumBins.getValue() < 1) {
      CALL_ERROR << "Error in " << FunctionName
                 << ": Invalid -bins of " << NumBins.getValue() << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   bool analyzeAll = true;  // If they don't give us a sequence, analyze all
   if (SeqName.getValue() != "{ones}") {
      // If analyzing for all context lengths and for unused, analyze all
      analyzeAll = (Con.getValue() == -1) && !UsedOnly.getValue();
      InvSeq = UISeqToPtnSeq(SystemVar::getSequence(SeqName, FunctionName, ComL));
      SeqSize = InvSeq.size();
      InvSeq = transposeMatrix(InvSeq);
      if ((Con.getValue() < -1) || (Con.getValue() > SeqSize)) {
         CALL_ERROR << "Error in " << FunctionName
                    << ": Invalid -con of " << Con.getValue() << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
      if (UsedOnly.getValue() && !Con.getValue()) {
         CALL_ERROR << "Error in " << FunctionName
                    << ": -used and -con 0 are not compatible." << ERR_WHERE;
         exit(EXIT_FAILURE);
      }
   } else if (Loop.getValue() || UsedOnly.getValue() || (Con.getValue() != -1)) {
      CALL_ERROR << "Error in " << FunctionName
                 << ": A sequence must be given if -unused, -loop, "
          "or -con are given." << ERR_WHERE;
      exit(EXIT_FAILURE);
   }
   if (Tolerance.getValue() < 0) {
      CALL_ERROR << "Error in " << FunctionName << ": Tolerance of "
                 << Tolerance.getValue() << " is invalid." << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   } else if ((Tolerance.getValue() > 0) && (Con.getValue() == -1)) {
      CALL_ERROR << "Error in " << FunctionName << ": Tolerance of "
                 << Tolerance.getValue() << " is invalid with no -con flag."
                 << ERR_WHERE;
      ComL.DisplayHelp(Output::Err());
      exit(EXIT_FAILURE);
   }
   // create the list of neurons to check
   int NumToAnalyze = 0;
   vector<int> NeuronsToAnalyze(ni);
   // if all weights are included then the list is maximal
   if (analyzeAll) {
      NumToAnalyze = ni;
      for (unsigned int i = 0; i < ni; i++) {
         NeuronsToAnalyze.at(i) = i;
      }
// else check if only a certain type of context is being compared
   } else if (Con.getValue() != -1) {
      SequenceCIt nrnIt = InvSeq.begin();
      for (int nrn = 0; nrnIt != InvSeq.end(); ++nrnIt, ++nrn) {
         int  context;
         bool multifire;
         getContextStats(*nrnIt, SeqSize, Tolerance.getValue(),
                         Loop.getValue(), multifire, context);
         if ((!(multifire)) && (context == Con.getValue())) {
            NeuronsToAnalyze.at(NumToAnalyze) = nrn;
            NumToAnalyze++;
         }
      }
// else check if only throwing out unused
   } else if (UsedOnly.getValue()) {
      SequenceCIt InvSCit = InvSeq.begin();
      for (int nrn = 0; InvSCit != InvSeq.end(); ++InvSCit, ++nrn) {
         for (int tt = 0; tt < SeqSize; tt++) {
            if (InvSCit->at(tt)) {
               NeuronsToAnalyze.at(NumToAnalyze) = nrn;
               NumToAnalyze++;
               break;
            }
         }
      }
   }
   // list is complete

   // create counts
   DataList numwij(NumBins.getValue());
   for (int b = 0; b < NumBins.getValue(); b++) {
      numwij.at(b) = 0.0f;
   }
   int TotalCount = 0;
   double binwidth = 1.0 / static_cast<double>(NumBins.getValue());

   double tmpSum = 0.0;
   double tmpNo0Sum = 0.0;
   int tmpNumZero = 0;

   // loop through the list and bin the weights
   for (int nrnToAnalyze = 0; nrnToAnalyze < NumToAnalyze; nrnToAnalyze++) {
      int j = NeuronsToAnalyze.at(nrnToAnalyze);
      DendriticSynapse * dendriticTree = inMatrix[j];
      for (unsigned int c = 0; c < FanInCon.at(j); c++) {
         const float wij = dendriticTree[c].getWeight();
         tmpSum += wij;
         if (wij < CutOff.getValue()) {
            tmpNumZero++;
         } else {
            tmpNo0Sum += wij;
         }
         TotalCount++;
         int binnum = ifloor(wij * static_cast<double>(NumBins.getValue()));
         if (binnum >= NumBins.getValue()) {
            binnum = NumBins.getValue() - 1;
         }
         if (binnum < 0) {
            binnum = 0;
         }
         numwij.at(binnum)++;
      }
   }

   // convert to percentages and make the buffers
   DataList BinCenters(NumBins.getValue());
   DataList WijVect(NumBins.getValue());
   DataList WijNVect(NumBins.getValue());

   double TotalWeights = SystemVar::GetFloatVar("FracConnect") * static_cast<double>(ni * ni);

   for (int b1 = 0; b1 < NumBins.getValue(); b1++) {
      BinCenters.at(b1) = static_cast<float>((b1 + 0.5) * binwidth);
      WijVect.at(b1) = static_cast<float>(numwij.at(b1)) /
                          static_cast<float>(TotalCount);
      WijNVect.at(b1) = static_cast<float>(numwij.at(b1) / TotalWeights);
   }

   float AveAllWeights = static_cast<float>(tmpSum) / static_cast<float>(TotalCount);
   float AveNonZeroWeights;
   if (tmpNumZero == TotalCount) {
      AveNonZeroWeights = 0.0f;
   } else {
      AveNonZeroWeights = static_cast<float>(tmpNo0Sum) /
                             static_cast<float>(TotalCount - tmpNumZero);
   }
   float FracZeroWeights = static_cast<float>(tmpNumZero) /
                              static_cast<float>(TotalCount);

   SystemVar::SetFloatVar("AveAllWeights", AveAllWeights);
   SystemVar::SetFloatVar("AveNonZeroWeights", AveNonZeroWeights);
   SystemVar::SetFloatVar("FracZeroWeights", FracZeroWeights);

   // Save the vectors to Buffers
   DataMatrix WeightBuffer;
   WeightBuffer.push_back(BinCenters);
   WeightBuffer.push_back(WijVect);
   WeightBuffer.push_back(WijNVect);

   SystemVar::insertData("WeightBuffer", WeightBuffer, DLT_matrix);

   // Output the data
   Output::Out() << "\nFound weight distributions";
   if (Con.getValue() != -1) {
      Output::Out() << " of neurons with " << (Loop.getValue() ? "loop" : "")
                    << " context length " << Con.getValue() << " from sequence "
                    << SeqName.getValue();
   } else if (UsedOnly.getValue()) {
      Output::Out() << " of neurons that fired from " "sequence " << SeqName.getValue();
   }
   Output::Out() << " using " << NumBins.getValue() << " bins.\n";

   if (DoSummary.getValue()) {
      Output::Out() << "\n" << std::setw(15) << "Bin Center"
                    << std::setw(15) << "Fraction of"
                    << std::setw(15) << "Fraction of" << "\n"
                    << std::setw(15) << " "
                    << std::setw(15) << "Included"
                    << std::setw(15) << "Total" << "\n"
                    << std::setiosflags(std::ios::fixed | std::ios::showpoint);
      for (int b3 = 0; b3 < NumBins.getValue(); b3++) {
         Output::Out() << std::setw(15) << BinCenters.at(b3)
                       << std::setw(15) << WijVect.at(b3)
                       << std::setw(15) << WijNVect.at(b3) << "\n";
      }
      Output::Out() << "\nFraction Included: "
         << (static_cast<double>(TotalCount) / static_cast<double>(TotalWeights))
         << std::resetiosflags(std::ios::fixed | std::ios::showpoint) << std::endl;
   }

   Output::Out() << std::endl;

   return;
}

void BindWeightAnalysis()
{
   SystemVar::AddFloatVar("AveAllWeights", 0.0f, true);
   SystemVar::AddFloatVar("AveNonZeroWeights", 0.0f, true);
   SystemVar::AddFloatVar("FracZeroWeights", 0.0f, true);
   SystemVar::AddAtFun("WeightAnalysis", WeightAnalysis);

   return;
}

#endif
