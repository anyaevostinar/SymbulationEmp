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


export default function CopilotWindow({ copOpen, handleClose }) {
  const animationClass = copOpen ? 'fade-in' : 'fade-out';
  const [tutorialTracker, setTutorialTracker] = useAtom(tutorialTrackerAtom);
  const curTutorial = tutorials[tutorialTracker.currentTutorialId];
  const curStepContent = curTutorial.steps[tutorialTracker.step];


  useEffect(() => {
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
  }, [curStepContent.advanceOn])



  let TutorialComponent;
  TutorialComponent = curStepContent.component;
  if(!TutorialComponent){
    console.error('Tutorial component not found.');
  }




  return (
    <div id="copWindow" className={animationClass}>
      <TutorialComponent content={curStepContent} />

      <CloseButton handleClose={handleClose} copOpen={copOpen} />
    </div>
  );
}