import { useUpdateAtom } from "jotai";
import { tutorialTrackerAtom } from "../atoms";

export default function useAdvanceStep() {
  const setTutorialTracker = useUpdateAtom(tutorialTrackerAtom);

  function advanceStep(){
    setTutorialTracker((prev) => ({
      ...prev,
      step: prev.step + 1
    }));
  }

  return advanceStep;
}
/*
  To use this function:
  
  import useAdvanceStep from './utilities/advanceStep';
  ...
  const advanceStep = useAdvanceStep();
  ...
  <Whatever onClick={advanceStep} />
*/