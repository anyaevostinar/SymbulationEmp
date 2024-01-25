import React, { useState, useEffect } from 'react';
import CloseIcon from '@mui/icons-material/Close';
import CloseButton from './components/CloseButton';

export default function CopilotWindow({ copOpen, handleClose }) {
  const animationClass = copOpen ? 'fade-in' : 'fade-out';

  return (
    <div id="copWindow" className={animationClass}>
      <p>i am the copwindow component</p>
      <CloseButton handleClose={handleClose} copOpen={copOpen}/>
    </div>
  );
}