/**
 * @name CopilotWindow
 * @description React component for the Copilot window that displays the tutorial steps and the close button
 * @param {boolean} copOpen state in Copilot.js that determines if the Copilot window is open
 * @param {function} handleClose function in Copilot.js that immediately sets copOpen to false and then sets displayOpenBtn to true and displayCopWindow to false after 150ms
 * @returns {JSX.Element} a div containing the current tutorial step and its content and a button to close the Copilot window
 */
import React, { useState, useEffect, useMemo, startTransition } from 'react';
import CloseIcon from '@mui/icons-material/Close';
import CloseButton from './components/CloseButton';
import { Button } from '@mui/material';
import { useAtom } from 'jotai';
import { tutorialTrackerAtom } from './atoms';
import { tutorials } from './tutorials';
import OneContained from './components/OneContained';
import TwoContainedImage from './components/TwoContainedImage';
import None from './components/None';
import ImgOnly from './components/ImgOnly';
import NoneInnerHTML from './components/NoneInnerHTML';
/* 
some IDEs will tell you to get rid of some of the above imports since they're unused, but don't! 
TutorialComponent turns into a component based on the current step's content, so all of these imports are necessary
*/


export default function CopilotWindow({ copOpen, handleClose }) {
  const animationClass = copOpen ? 'fade-in' : 'fade-out';
  const [tutorialTracker, setTutorialTracker] = useAtom(tutorialTrackerAtom);
  const curTutorial = tutorials[tutorialTracker.currentTutorialId];
  const curStepContent = curTutorial.steps[tutorialTracker.step];

  useEffect(() => {
    /* 
    check if this tutorial step has an advanceOn function
    if it does, check if the tutorial step also has an onAdvance function. if it does have an onAdvance function, run the onAdvance function
    then, regardless of whether the tutorial step has an onAdvance function, advance the tutorial
    */
    if (curStepContent.advanceOn) {
      curStepContent.advanceOn().then(() => {
        if(typeof curStepContent.onAdvance === 'function'){
          curStepContent.onAdvance();
        }
        startTransition(() => {
          setTutorialTracker((prev) => ({
            ...prev,
            step: prev.step + 1
          }));
        });
      })
    }
  }, [curStepContent.advanceOn]);

  let TutorialComponent;
  TutorialComponent = curStepContent.component;
  if(!TutorialComponent){
    console.error(`Tutorial component not found. Check if the current tutorial step (index ${tutorialTracker.step}) has a valid component property and if it is imported in both CopilotWindow.js and tutorials.js.`);
  }
  // TutorialComponent is a component based on the current step's content

  return (
    <div id="copWindow" className={animationClass}>
      <TutorialComponent content={curStepContent} />
      <CloseButton handleClose={handleClose} copOpen={copOpen} />
    </div>
  );
}