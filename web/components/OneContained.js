/**
 * @name OneContained
 * @description React component for a tutorial step that contains a title, body text, and one button to advance the tutorial
 * @param {object} content an object containing the tutorial step's title text, body text, and button label
 * @param {string} content.titleText the title text for the tutorial step
 * @param {string} content.bodyText the body text for the tutorial step
 * @param {string[]} content.buttonLabels an array containing the label for the button to advance the tutorial in its first index
 * @returns {JSX.Element} a div containing the title, body text, and a button to advance the tutorial
 */
import React, { startTransition, useEffect, useState } from 'react';
import { Button } from '@mui/material';
import { useSetAtom } from 'jotai';
import { tutorialTrackerAtom } from '../atoms';

export default function OneContained({ content }) {
  const setTutorialTracker = useSetAtom(tutorialTrackerAtom);
  function advanceStep() {
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
          // check if this tutorial step has an onAdvance function. if it does, run it
          if (typeof content.onAdvance === 'function') {
            content.onAdvance();
          }
          advanceStep();
        }
      }>
        {content.buttonLabels[0]}
      </Button>
    </div>
  );
}