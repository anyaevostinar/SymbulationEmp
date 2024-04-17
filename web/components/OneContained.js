import React, { startTransition, useEffect, useState } from 'react';
import { Button } from '@mui/material';
import { useSetAtom } from 'jotai';
import { tutorialTrackerAtom } from '../atoms';


export default function OneContained({ content }) {
  //const advanceStep = useAdvanceStep();
  const setTutorialTracker = useSetAtom(tutorialTrackerAtom);
  function advanceStep(){
    startTransition(() => {
      setTutorialTracker((prev) => ({
        ...prev,
        step: prev.step + 1
      }));
    })
  }
  return (
    <div className="oneContained">
      <h2 className="tutorialTitle">{content.titleText}</h2>
      <p className="tutorialDescription">{content.bodyText}</p>
      <Button variant="contained" color="primary" className='tutorialPrimaryBtn' onClick={
        () => {
          if(typeof content.onAdvance === 'function'){
            console.log('onAdvance is a function');
            content.onAdvance();
          }
          else{
            console.log('onAdvance is not a function');
          }
          advanceStep();
          
        }
        
        }>
        {content.buttonLabels[0]}
      </Button>
    </div>
  );
}