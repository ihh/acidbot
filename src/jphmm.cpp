#include "jphmm.h"

using namespace MachineBoss;

#define jpHMMjumpParamName "jump"
#define jpHMMrowTag "row"
#define jpHMMcolTag "col"
#define jpHMMstartTag "start"
#define jpHMMendTag "end"

const string JPHMM::jumpParam (jpHMMjumpParamName);

JPHMM::JPHMM (const vguard<FastSeq>& seqs)
  : rows (seqs.size()),
    cols (rows > 0 ? seqs[0].length() : 0) {
  
  Assert (rows > 0, "No sequences supplied - can't construct a jpHMM from an empty alignment");
  Assert (cols > 0, "Empty sequences supplied - can't construct a jpHMM from an empty alignment");
  for (const auto& seq : seqs)
    if (seq.length() != cols)
      Abort ("Alignment is not flush - all sequences must be same length to build a jpHMM from an alignment");

  state = vguard<MachineState> (rows*cols + 2);
  state.front().name = jpHMMstartTag;
  state.back().name = jpHMMendTag;
  const auto nRows = WeightAlgebra::intConstant (rows), nOtherRows = WeightAlgebra::intConstant (rows - 1);
  const auto startProb = WeightAlgebra::reciprocal (nRows), pJump = WeightAlgebra::param (jumpParam);
  const auto stayProb = rows == 1 ? WeightAlgebra::one() : WeightAlgebra::negate (pJump), jumpProb = WeightAlgebra::divide (pJump, nOtherRows);
  for (int row = 0; row < rows; ++row)
    state[startState()].trans.push_back (MachineTransition (string(), string (1, seqs[row].seq[0]), emitState(row,0), startProb));
  for (int srcCol = 0; srcCol < cols; ++srcCol) {
    const int destCol = srcCol + 1;
    for (int srcRow = 0; srcRow < rows; ++srcRow) {
      MachineState& srcState = state[emitState(srcRow,srcCol)];
      TransList& srcTrans = srcState.trans;
      srcState.name[jpHMMrowTag] = srcRow + 1;
      srcState.name[jpHMMcolTag] = srcCol + 1;
      if (destCol < cols)
	for (int destRow = 0; destRow < rows; ++destRow)
	  srcTrans.push_back (MachineTransition (string(), string (1, seqs[destRow].seq[destCol]), emitState(destRow,destCol), srcRow == destRow ? stayProb : jumpProb));
      else
	srcTrans.push_back (MachineTransition (string(), string(), endState(), WeightAlgebra::one()));
    }
  }

  cons.prob.push_back (jumpParam);
}

