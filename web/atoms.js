import { atom } from "jotai";

export const tutorialTrackerAtom = atom({
  currentTutorialId: 0, // index of the current tutorial in the tutorials array in tutorials.js
  step: 0
});
