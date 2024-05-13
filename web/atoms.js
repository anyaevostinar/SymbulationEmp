import { atom } from "jotai";

export const tutorialTrackerAtom = atom({
  currentTutorialId: 0,
  /* index of the current tutorial in the tutorials array in tutorials.js. this isn't currently used since there's
  only one tutorial, but it's here in case we want to add more tutorials in the future */
  
  step: 0
  // index of the current step in the current tutorial object's steps array (in tutorials.js)
});
