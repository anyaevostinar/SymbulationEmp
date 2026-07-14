/**
 * @name TwoContainedImage
 * @description React component for a tutorial step that contains a title, body text, image, and two buttons
 * @param {object} content an object containing the tutorial step's title text, body text, image source, and two button labels
 * @param {string} content.titleText the title text for the tutorial step
 * @param {string} content.bodyText the body text for the tutorial step
 * @param {string} content.imgSrc the path to the image for the tutorial step
 * @param {string[]} content.buttonLabels an array containing the labels for the two buttons to retreat and advance the tutorial
 * @param {function} content.onAdvance (optional) a function to run when the advance button is clicked
 * @returns {JSX.Element} a div containing the title, body text, image, and two buttons to retreat and advance the tutorial
 */
import React, { startTransition, useEffect, useState } from 'react';
import { Button } from '@mui/material';
import { useSetAtom } from 'jotai';
import { tutorialTrackerAtom } from '../atoms';

export default function TwoContainedImage({ content }) {
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
          {content.buttonLabels[0] /* this is the continue button, so something like "next" or "start"*/}
        </Button>
        <Button variant="contained" color="primary" className='tutorialBtnTwo' onClick={() => {
          // check if this tutorial step has an onAdvance function. if it does, run it
          if (typeof content.onAdvance === 'function') {
            content.onAdvance();
          }
          advanceStep();
        }}>
          {content.buttonLabels[1] /* this is the back button, so something like "back"*/}
        </Button>
      </div>




    </div>
  );

}