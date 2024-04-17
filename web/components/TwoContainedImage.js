import React, { startTransition, useEffect, useState } from 'react';
import { Button } from '@mui/material';
import { useSetAtom } from 'jotai';
import { tutorialTrackerAtom } from '../atoms';


export default function TwoContainedImage({ content }) {
  //const advanceStep = useAdvanceStep();
  const setTutorialTracker = useSetAtom(tutorialTrackerAtom);
  function advanceStep() {
    startTransition(() => {
      setTutorialTracker((prev) => ({
        ...prev,
        step: prev.step + 1
      }));
    })
  }
  function retreatStep() {
    startTransition(() => {
      setTutorialTracker((prev) => ({
        ...prev,
        step: prev.step - 1
      }));
    })
  }
  return (
    <div className="twoContainedImage">
      <h2 className="titleText">{content.titleText}</h2>

      <p className="bodyText">{content.bodyText}</p>

      <div className="imgAndBtns">
        <div className="imgHolder">
          <img src={content.imgSrc} className='tutorialImg' />
        </div>

          <Button variant="contained" color="primary" className='tutorialBtnOne' onClick={() => retreatStep()}>
            {content.buttonLabels[0] /* probably something like "next" or "start"*/}
          </Button>
          <Button variant="contained" color="primary" className='tutorialBtnTwo' onClick={() => {

            if (typeof content.onAdvance === 'function') {
              content.onAdvance();
            }
            advanceStep();

          }}>
            {content.buttonLabels[1] /* back button, so something like "back"*/}
          </Button>
      </div>


    

    </div>
  );

}