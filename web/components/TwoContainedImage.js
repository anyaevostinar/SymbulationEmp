import React, { useEffect, useState } from 'react';
import { Button } from '@mui/material';
import { useSetAtom } from 'jotai';
import { tutorialTrackerAtom } from '../atoms';


export default function OneContained({ content }) {
  //const advanceStep = useAdvanceStep();
  const setTutorialTracker = useSetAtom(tutorialTrackerAtom);
  function advanceStep() {
    setTutorialTracker((prev) => ({
      ...prev,
      step: prev.step + 1
    }));
  }
  function retreatStep() {
    setTutorialTracker((prev) => ({
      ...prev,
      step: prev.step - 1
    }));
  }
  return (
    <div className="twoContainedImage">
      <h2 className="titleText">{content.titleText}</h2>
      <p className="bodyText">{content.bodyText}</p>
      <div className="imgHolder">
        <img src={content.imgSrc} className='tutorialImg' />
      </div>

      <div className="btns">
        <Button variant="contained" color="primary" className='tutorialBtnOne' onClick={() => retreatStep()}>
          {content.buttonLabels[0] /* probably something like "next" or "start"*/}
        </Button>
        <Button variant="contained" color="primary" className='tutorialBtnTwo' onClick={() => {
          
          if(typeof content.onAdvance === 'function'){
            console.log('onAdvance is a function');
            content.onAdvance();
          }
          else{
            console.log('onAdvance is not a function');
          }
          advanceStep();
          
          }}>
          {content.buttonLabels[1] /* back button, so something like "back"*/}
        </Button>
      </div>

    </div>
  );
  /*

  );*/
}