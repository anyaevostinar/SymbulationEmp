import React, { useEffect, useState } from 'react';
import { Button } from '@mui/material';
import { advanceStep } from '../utilities';

export default function OneContained({ content }) {
  return (
    <div className="oneContained">
      <h2 className="tutorialTitle">{content.titleText}</h2>
      <p className="tutorialDescription">{content.bodyText}</p>
      <Button variant="contained" color="primary" className='tutorialPrimaryBtn' onClick={() => advanceStep()}>
        {content.buttonLabels[0]}
      </Button>
    </div>
  );
}