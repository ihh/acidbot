#ifndef ASSEMBLY_INCLUDED
#define ASSEMBLY_INCLUDED

#include <random>

#include "eval.h"
#include "seqpair.h"
#include "logsumexp.h"

struct CompactMachinePath {
  typedef EvaluatedMachineState::TransIndex TransIndex;
  vguard<TransIndex> trans;

  void readJson (const json&);
  void writeJson (ostream&) const;

  static CompactMachinePath fromMachinePath (const MachinePath&, const Machine&);
};

struct CompactLocalMachinePath : CompactMachinePath {
  typedef Envelope::InputIndex InputIndex;
  InputIndex start;

  void readJson (const json&);
  void writeJson (ostream&) const;
};

struct Assembly {
  typedef CompactMachinePath::TransIndex TransIndex;
  typedef CompactLocalMachinePath::InputIndex InputIndex;

  Machine generator, error;
  CompactMachinePath generatorPath;
  vguard<CompactLocalMachinePath> errorPaths;

  EvaluatedMachine evalGen, evalErr;

  // I/O and initialization
  void readJson (const json&);
  void writeJson (ostream&) const;

  void validate() const;
  void evaluateMachines();

  // Accessors
  vguard<OutputSymbol> sequence() const;
  
  // Scoring
  LogProb logProb() const;  // log P(sequence,annotation,alignments|machines)

  // MCMC
  void resampleAlignment (mt19937&, size_t maxAlignSlideWidth);
  void resampleSequence (mt19937&, size_t maxResampledTransitions);
  void resampleAnnotation (mt19937&);

  // Compression
  uint8_t* writeEncoded() const;
  void readDecoded (uint8_t*);
};

#endif /* ASSEMBLY_INCLUDED */
